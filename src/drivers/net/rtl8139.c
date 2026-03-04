#include "rtl8139.h"
#include "io.h"
#include "isr.h"
#include "pic.h"
#include "memory.h"

/* RTL8139 registers */
#define RTL_IDR0        0x00    /* MAC address */
#define RTL_MAR0        0x08
#define RTL_TSD0        0x10    /* Transmit status */
#define RTL_TSAD0       0x20    /* Transmit start address */
#define RTL_RBSTART     0x30    /* Receive buffer start */
#define RTL_CMD         0x37
#define RTL_CAPR        0x38    /* Current address of packet read */
#define RTL_IMR         0x3C    /* Interrupt mask */
#define RTL_ISR         0x3E    /* Interrupt status */
#define RTL_RCR         0x44    /* Receive config */
#define RTL_CONFIG1     0x52

/* PCI Configuration */
#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC
#define RTL8139_VENDOR  0x10EC
#define RTL8139_DEVICE  0x8139

static uint16_t rtl_iobase = 0;
static uint8_t  rtl_mac[6];
static uint8_t  rx_buffer[8192 + 16 + 1500]; /* 8K + 16 header + 1500 MTU */
static int      rx_offset = 0;
static uint8_t  tx_buffers[4][1792];          /* 4 TX descriptors */
static int      tx_cur = 0;
static int      net_available = 0;

/* PCI read config */
static uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t addr = (1u << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDR, addr);
    return inl(PCI_CONFIG_DATA);
}

/* PCI write config */
static void pci_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
    uint32_t addr = (1u << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDR, addr);
    outl(PCI_CONFIG_DATA, val);
}

/* Scan PCI bus for RTL8139 */
static int rtl_pci_find(uint8_t *bus_out, uint8_t *slot_out) {
    for (uint8_t bus = 0; bus < 8; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id = pci_read(bus, slot, 0, 0);
            uint16_t vendor = id & 0xFFFF;
            uint16_t device = (id >> 16) & 0xFFFF;
            if (vendor == RTL8139_VENDOR && device == RTL8139_DEVICE) {
                *bus_out = bus;
                *slot_out = slot;
                return 1;
            }
        }
    }
    return 0;
}

static void rtl_callback(registers_t *r) {
    (void)r;
    if (!rtl_iobase) return;

    uint16_t status = inw(rtl_iobase + RTL_ISR);
    /* Acknowledge all interrupts */
    outw(rtl_iobase + RTL_ISR, status);
}

void rtl8139_init(void) {
    uint8_t bus, slot;

    if (!rtl_pci_find(&bus, &slot)) {
        net_available = 0;
        return;
    }

    /* Get I/O base address from BAR0 */
    uint32_t bar0 = pci_read(bus, slot, 0, 0x10);
    rtl_iobase = (uint16_t)(bar0 & 0xFFFC);

    /* Enable PCI bus mastering */
    uint32_t cmd = pci_read(bus, slot, 0, 0x04);
    cmd |= (1 << 2); /* Bus master */
    pci_write(bus, slot, 0, 0x04, cmd);

    /* Power on */
    outb(rtl_iobase + RTL_CONFIG1, 0x00);

    /* Software reset */
    outb(rtl_iobase + RTL_CMD, 0x10);
    while (inb(rtl_iobase + RTL_CMD) & 0x10);

    /* Set receive buffer */
    outl(rtl_iobase + RTL_RBSTART, (uint32_t)(uintptr_t)rx_buffer);

    /* Enable RX and TX */
    outb(rtl_iobase + RTL_CMD, 0x0C);

    /* Configure receive: accept all, wrap */
    outl(rtl_iobase + RTL_RCR, 0x0000000F);

    /* Enable interrupts (RX OK, TX OK) */
    outw(rtl_iobase + RTL_IMR, 0x0005);

    /* Read MAC address */
    for (int i = 0; i < 6; i++) {
        rtl_mac[i] = inb(rtl_iobase + RTL_IDR0 + i);
    }

    /* Get IRQ from PCI config */
    uint32_t irq_line = pci_read(bus, slot, 0, 0x3C) & 0xFF;
    if (irq_line > 0 && irq_line < 16) {
        register_interrupt_handler(32 + irq_line, rtl_callback);
        pic_unmask_irq(irq_line);
    }

    net_available = 1;
    rx_offset = 0;
    tx_cur = 0;
}

int rtl8139_send_packet(const void *data, uint16_t len) {
    if (!net_available || !rtl_iobase || len > 1792) return -1;

    memcpy(tx_buffers[tx_cur], data, len);

    /* Set transmit address */
    outl(rtl_iobase + RTL_TSAD0 + tx_cur * 4, (uint32_t)(uintptr_t)tx_buffers[tx_cur]);
    /* Set transmit status (start send) */
    outl(rtl_iobase + RTL_TSD0 + tx_cur * 4, len);

    tx_cur = (tx_cur + 1) % 4;
    return 0;
}

int rtl8139_receive_packet(void *buffer, uint16_t max_len) {
    if (!net_available || !rtl_iobase) return -1;

    /* Check if buffer is empty */
    if (inb(rtl_iobase + RTL_CMD) & 0x01) return 0; /* RX buffer empty */

    /* Read packet header */
    uint16_t *header = (uint16_t *)(rx_buffer + rx_offset);
    uint16_t status  = header[0];
    uint16_t length  = header[1];

    if (!(status & 0x01)) return 0; /* ROK not set */

    /* Packet data starts after 4-byte header */
    uint16_t copy_len = length - 4; /* minus CRC */
    if (copy_len > max_len) copy_len = max_len;
    memcpy(buffer, rx_buffer + rx_offset + 4, copy_len);

    /* Update read pointer */
    rx_offset = (rx_offset + length + 4 + 3) & ~3; /* Align to 4 */
    if (rx_offset >= 8192) rx_offset -= 8192;
    outw(rtl_iobase + RTL_CAPR, (uint16_t)(rx_offset - 16));

    return copy_len;
}
