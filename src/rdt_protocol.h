//
// Created by radim on 04.04.2026.
//

#ifndef IPK_PROJ2_RDT_PROTOCOL_H
#define IPK_PROJ2_RDT_PROTOCOL_H
#include <cstdint>

#endif //IPK_PROJ2_RDT_PROTOCOL_H

struct Header {
    uint32_t seq_number;
    uint32_t ack;
    uint32_t conn_id;
    uint16_t checksum;
    uint8_t flags; // SYN=1, ACK=2, FIN=4
} __attribute__((packed));