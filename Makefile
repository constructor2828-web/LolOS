CC = gcc
CXX = g++
AS = nasm
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -m32 -fno-pie -fno-stack-protector -Isrc/kernel -Isrc/drivers -Isrc/drivers/net -Isrc/drivers/input -Isrc/drivers/video -Isrc/gui -Isrc/apps -Isrc/fs
CXXFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -m32 -fno-pie -fno-stack-protector
LDFLAGS = -ffreestanding -O2 -nostdlib -m32 -Wl,-m,elf_i386 -no-pie -Wl,--build-id=none

SRC_DIR = src
BUILD_DIR = build
ISO_DIR = iso

OBJS = $(BUILD_DIR)/boot.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/gdt_flush.o $(BUILD_DIR)/gdt.o $(BUILD_DIR)/idt_flush.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/isr.o $(BUILD_DIR)/pic.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/graphics.o $(BUILD_DIR)/font.o $(BUILD_DIR)/gui.o $(BUILD_DIR)/mouse.o $(BUILD_DIR)/vfs.o $(BUILD_DIR)/apps.o $(BUILD_DIR)/login.o $(BUILD_DIR)/desktop.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/rtl8139.o $(BUILD_DIR)/icons.o
BIN = $(BUILD_DIR)/lolos.bin
ISO = lolos.img

all: $(ISO)

$(ISO): $(BIN) $(BUILD_DIR)/stage1.bin $(BUILD_DIR)/stage2.bin
	# Create a 1.44MB floppy image (or just enough for our needs)
	dd if=/dev/zero of=$(ISO) bs=512 count=2880
	# Write Stage 1 to Sector 1
	dd if=$(BUILD_DIR)/stage1.bin of=$(ISO) conv=notrunc
	# Write Stage 2 to Sector 2
	dd if=$(BUILD_DIR)/stage2.bin of=$(ISO) seek=1 conv=notrunc
	# Write Kernel to Sector 10
	objcopy -O binary $(BIN) $(BUILD_DIR)/kernel.bin
	dd if=$(BUILD_DIR)/kernel.bin of=$(ISO) seek=9 conv=notrunc

$(BIN): $(OBJS) $(SRC_DIR)/linker.ld
	$(CC) -T $(SRC_DIR)/linker.ld -o $@ $(LDFLAGS) $(OBJS)

$(BUILD_DIR)/stage1.bin: $(SRC_DIR)/bootloader/stage1.s
	mkdir -p $(BUILD_DIR)
	$(AS) -f bin $< -o $@

$(BUILD_DIR)/stage2.bin: $(SRC_DIR)/bootloader/stage2.s
	mkdir -p $(BUILD_DIR)
	$(AS) -f bin $< -o $@

$(BUILD_DIR)/boot.o: $(SRC_DIR)/kernel/boot.s
	mkdir -p $(BUILD_DIR)
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel/kernel.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/gdt_flush.o: $(SRC_DIR)/kernel/gdt_flush.s
	mkdir -p $(BUILD_DIR)
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/gdt.o: $(SRC_DIR)/kernel/gdt.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/idt_flush.o: $(SRC_DIR)/kernel/idt_flush.s
	mkdir -p $(BUILD_DIR)
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/idt.o: $(SRC_DIR)/kernel/idt.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/interrupt.o: $(SRC_DIR)/kernel/interrupt.s
	mkdir -p $(BUILD_DIR)
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/isr.o: $(SRC_DIR)/kernel/isr.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/pic.o: $(SRC_DIR)/kernel/pic.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/keyboard.o: $(SRC_DIR)/drivers/input/keyboard.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/graphics.o: $(SRC_DIR)/drivers/video/graphics.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/font.o: $(SRC_DIR)/gui/font.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/gui.o: $(SRC_DIR)/gui/gui.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/mouse.o: $(SRC_DIR)/drivers/input/mouse.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/vfs.o: $(SRC_DIR)/fs/vfs.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/apps.o: $(SRC_DIR)/apps/apps.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/login.o: $(SRC_DIR)/gui/login.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/desktop.o: $(SRC_DIR)/gui/desktop.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/memory.o: $(SRC_DIR)/kernel/memory.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/rtl8139.o: $(SRC_DIR)/drivers/net/rtl8139.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/icons.o: $(SRC_DIR)/gui/icons.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO)

run: $(ISO)
	qemu-system-i386 -drive format=raw,file=$(ISO) -vga std -serial stdio -d int,cpu_reset -no-reboot
