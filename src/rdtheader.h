//
// Created by radim on 04.04.2026.
//

#ifndef IPK_PROJ2_RDTHEADER_H
#define IPK_PROJ2_RDTHEADER_H
#include <cstdint>
#include <vector>
#include <cstring>
#include "rdt_protocol.h"



class RDTheader {
public:
    Header header;
    std::vector<uint8_t> payload;

    uint16_t calcChecksum() {
        return 0;
    }

    std::vector<uint8_t> serialize() {
        std::vector<uint8_t> buffer(sizeof(RDTheader)+ payload.size());

        header.checksum = 0;
        header.checksum = calcChecksum();

        std::memcpy(buffer.data(), &header, sizeof(RDTheader));
        if (!payload.empty()) {
            std::memcpy(buffer.data() + sizeof(RDTheader), payload.data(), payload.size());
        }
        return buffer;
    }

};



#endif //IPK_PROJ2_RDTHEADER_H
