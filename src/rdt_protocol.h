//
// Created by radim on 04.04.2026.
//

#ifndef IPK_PROJ2_RDT_PROTOCOL_H
#define IPK_PROJ2_RDT_PROTOCOL_H

#include <cstdint>

static constexpr uint8_t FLAG_SYN = 1;
static constexpr uint8_t FLAG_ACK = 2;
static constexpr uint8_t FLAG_FIN = 4;

struct Header {
    uint32_t seq_number;
    uint32_t ack;
    uint32_t conn_id;
    uint16_t checksum;
    uint8_t flags;
} __attribute__((packed));

#endif //IPK_PROJ2_RDT_PROTOCOL_H