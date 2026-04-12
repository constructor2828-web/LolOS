#include "net_stack.h"
#include "rtl8139.h"
#include "memory.h"
#include <stdint.h>

#define ETH_TYPE_ARP  0x0806
#define ETH_TYPE_IPV4 0x0800
#define IP_PROTO_UDP  17
#define ARP_OP_REQ    1
#define ARP_OP_REPLY  2

static const uint8_t LOCAL_IP[4] = {10, 0, 2, 15};
static const uint8_t DNS_IP[4]   = {10, 0, 2, 3};

typedef struct __attribute__((packed)) {
    uint8_t  dst[6];
    uint8_t  src[6];
    uint16_t eth_type;
} eth_hdr_t;

typedef struct __attribute__((packed)) {
    uint16_t hw_type;
    uint16_t proto_type;
    uint8_t  hw_len;
    uint8_t  proto_len;
    uint16_t op;
    uint8_t  sender_mac[6];
    uint8_t  sender_ip[4];
    uint8_t  target_mac[6];
    uint8_t  target_ip[4];
} arp_pkt_t;

typedef struct __attribute__((packed)) {
    uint8_t  ver_ihl;
    uint8_t  dscp_ecn;
    uint16_t total_len;
    uint16_t id;
    uint16_t flags_frag;
    uint8_t  ttl;
    uint8_t  proto;
    uint16_t hdr_checksum;
    uint8_t  src_ip[4];
    uint8_t  dst_ip[4];
} ipv4_hdr_t;

typedef struct __attribute__((packed)) {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t len;
    uint16_t checksum;
} udp_hdr_t;

static uint16_t bswap16(uint16_t v) { return (uint16_t)((v << 8) | (v >> 8)); }

static void zero_mem(void* dst, uint32_t len) {
    uint8_t* p = (uint8_t*)dst;
    for (uint32_t i = 0; i < len; i++) p[i] = 0;
}

static uint16_t ip_checksum(const void* data, uint16_t len) {
    const uint16_t* w = (const uint16_t*)data;
    uint32_t sum = 0;
    for (uint16_t i = 0; i < len / 2; i++) sum += bswap16(w[i]);
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)(~sum);
}

static int ip_eq(const uint8_t a[4], const uint8_t b[4]) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static int net_arp_resolve(const uint8_t target_ip[4], uint8_t out_mac[6]) {
    uint8_t src_mac[6];
    rtl8139_get_mac(src_mac);

    uint8_t frame[64];
    zero_mem(frame, sizeof(frame));
    eth_hdr_t* eth = (eth_hdr_t*)frame;
    for (int i = 0; i < 6; i++) eth->dst[i] = 0xFF;
    for (int i = 0; i < 6; i++) eth->src[i] = src_mac[i];
    eth->eth_type = bswap16(ETH_TYPE_ARP);

    arp_pkt_t* arp = (arp_pkt_t*)(frame + sizeof(eth_hdr_t));
    arp->hw_type = bswap16(1);
    arp->proto_type = bswap16(ETH_TYPE_IPV4);
    arp->hw_len = 6;
    arp->proto_len = 4;
    arp->op = bswap16(ARP_OP_REQ);
    for (int i = 0; i < 6; i++) arp->sender_mac[i] = src_mac[i];
    for (int i = 0; i < 4; i++) arp->sender_ip[i] = LOCAL_IP[i];
    for (int i = 0; i < 4; i++) arp->target_ip[i] = target_ip[i];

    if (rtl8139_send_packet(frame, sizeof(frame)) != 0) return -1;

    uint8_t rx[1600];
    for (int tries = 0; tries < 50000; tries++) {
        int n = rtl8139_receive_packet(rx, sizeof(rx));
        if (n < (int)(sizeof(eth_hdr_t) + sizeof(arp_pkt_t))) continue;
        eth_hdr_t* re = (eth_hdr_t*)rx;
        if (bswap16(re->eth_type) != ETH_TYPE_ARP) continue;
        arp_pkt_t* ra = (arp_pkt_t*)(rx + sizeof(eth_hdr_t));
        if (bswap16(ra->op) != ARP_OP_REPLY) continue;
        if (!ip_eq(ra->sender_ip, target_ip)) continue;
        for (int i = 0; i < 6; i++) out_mac[i] = ra->sender_mac[i];
        return 0;
    }
    return -1;
}

static uint16_t g_dns_txid = 0x2211;

int net_dns_resolve(const char* host, uint8_t out_ip[4]) {
    if (!host || !host[0] || !out_ip || !rtl8139_is_available()) return -1;

    uint8_t dns_mac[6];
    if (net_arp_resolve(DNS_IP, dns_mac) != 0) return -2;

    uint8_t src_mac[6];
    rtl8139_get_mac(src_mac);

    uint8_t pkt[1514];
    zero_mem(pkt, sizeof(pkt));

    eth_hdr_t* eth = (eth_hdr_t*)pkt;
    for (int i = 0; i < 6; i++) eth->dst[i] = dns_mac[i];
    for (int i = 0; i < 6; i++) eth->src[i] = src_mac[i];
    eth->eth_type = bswap16(ETH_TYPE_IPV4);

    ipv4_hdr_t* ip = (ipv4_hdr_t*)(pkt + sizeof(eth_hdr_t));
    ip->ver_ihl = 0x45;
    ip->dscp_ecn = 0;
    ip->id = bswap16(0x1001);
    ip->flags_frag = bswap16(0x4000);
    ip->ttl = 64;
    ip->proto = IP_PROTO_UDP;
    for (int i = 0; i < 4; i++) { ip->src_ip[i] = LOCAL_IP[i]; ip->dst_ip[i] = DNS_IP[i]; }

    udp_hdr_t* udp = (udp_hdr_t*)(pkt + sizeof(eth_hdr_t) + sizeof(ipv4_hdr_t));
    udp->src_port = bswap16(43210);
    udp->dst_port = bswap16(53);

    uint8_t* dns = (uint8_t*)(pkt + sizeof(eth_hdr_t) + sizeof(ipv4_hdr_t) + sizeof(udp_hdr_t));
    uint16_t txid = ++g_dns_txid;
    dns[0] = (uint8_t)(txid >> 8); dns[1] = (uint8_t)txid;
    dns[2] = 0x01; dns[3] = 0x00; // recursion desired
    dns[4] = 0x00; dns[5] = 0x01; // qdcount
    dns[6] = dns[7] = dns[8] = dns[9] = dns[10] = dns[11] = 0;

    uint16_t p = 12;
    uint16_t label_len = 0;
    const char* seg = host;
    for (uint16_t i = 0;; i++) {
        char ch = host[i];
        if (ch == '.' || ch == '\0') {
            dns[p - label_len - 1] = (uint8_t)label_len;
            if (ch == '\0') break;
            seg = &host[i + 1];
            label_len = 0;
            p++;
            dns[p - 1] = 0;
            (void)seg;
        } else {
            if (label_len == 0) dns[p++] = 0; // reserve length byte
            dns[p++] = (uint8_t)ch;
            label_len++;
        }
        if (p > 240) return -3;
    }
    dns[p++] = 0x00;
    dns[p++] = 0x00; dns[p++] = 0x01; // QTYPE A
    dns[p++] = 0x00; dns[p++] = 0x01; // QCLASS IN

    uint16_t udp_len = (uint16_t)(sizeof(udp_hdr_t) + p);
    uint16_t ip_len = (uint16_t)(sizeof(ipv4_hdr_t) + udp_len);
    udp->len = bswap16(udp_len);
    udp->checksum = 0;
    ip->total_len = bswap16(ip_len);
    ip->hdr_checksum = 0;
    ip->hdr_checksum = bswap16(ip_checksum(ip, sizeof(ipv4_hdr_t)));

    uint16_t frame_len = (uint16_t)(sizeof(eth_hdr_t) + ip_len);
    if (rtl8139_send_packet(pkt, frame_len) != 0) return -4;

    uint8_t rx[1600];
    for (int tries = 0; tries < 80000; tries++) {
        int n = rtl8139_receive_packet(rx, sizeof(rx));
        if (n < 64) continue;
        eth_hdr_t* re = (eth_hdr_t*)rx;
        if (bswap16(re->eth_type) != ETH_TYPE_IPV4) continue;
        ipv4_hdr_t* rip = (ipv4_hdr_t*)(rx + sizeof(eth_hdr_t));
        if (rip->proto != IP_PROTO_UDP) continue;
        if (!ip_eq(rip->src_ip, DNS_IP)) continue;
        uint16_t ihl = (uint16_t)((rip->ver_ihl & 0x0F) * 4);
        udp_hdr_t* rudp = (udp_hdr_t*)(rx + sizeof(eth_hdr_t) + ihl);
        if (bswap16(rudp->src_port) != 53 || bswap16(rudp->dst_port) != 43210) continue;
        uint8_t* rdns = (uint8_t*)rudp + sizeof(udp_hdr_t);
        if (rdns[0] != (uint8_t)(txid >> 8) || rdns[1] != (uint8_t)txid) continue;
        uint16_t ancount = (uint16_t)((rdns[6] << 8) | rdns[7]);
        if (ancount == 0) return -5;

        // Skip question
        uint16_t q = 12;
        while (rdns[q] && q < 300) q += (uint16_t)(rdns[q] + 1);
        q += 5;

        // First answer
        if (rdns[q] == 0xC0) q += 2; else return -6;
        uint16_t atype = (uint16_t)((rdns[q] << 8) | rdns[q + 1]); q += 2;
        uint16_t aclass = (uint16_t)((rdns[q] << 8) | rdns[q + 1]); q += 2;
        q += 4; // ttl
        uint16_t rdlen = (uint16_t)((rdns[q] << 8) | rdns[q + 1]); q += 2;
        if (atype == 1 && aclass == 1 && rdlen == 4) {
            for (int i = 0; i < 4; i++) out_ip[i] = rdns[q + i];
            return 0;
        }
        return -7;
    }
    return -8;
}
