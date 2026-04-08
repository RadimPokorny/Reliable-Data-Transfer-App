//
// Created by radim on 04.04.2026.
//

#ifndef IPK_PROJ2_RDTHEADER_H
#define IPK_PROJ2_RDTHEADER_H
#include <cstdint>
#include <vector>
#include <cstring>
#include "rdt_protocol.h"

class RDTPacket {
public:
    Header header;
    std::vector<uint8_t> payload;

    // Calculate the checksum from the inner data
    uint16_t calcChecksum() {
        // Checksum must be NIL in the very beginning
        uint32_t sum = 0;
        Header tmpHeader = header;
        tmpHeader.checksum = 0;

        // Header and payload helper data
        std::vector<uint8_t> combined;
        combined.insert(combined.end(), reinterpret_cast<uint8_t*>(&tmpHeader), reinterpret_cast<uint8_t*>(&tmpHeader) + sizeof(Header));
        combined.insert(combined.end(), payload.begin(), payload.end());
        uint8_t* data = combined.data();
        size_t len = combined.size();

        // RFC 1071 Algorithm
        while (len > 1) {
            sum += (data[0] << 8) | data[1];
            data += 2;
            len -= 2;
            if (sum > 0xFFFF) sum = (sum & 0xFFFF) + 1;
        }
        if (len == 1) {
            sum += (data[0] << 8);
            if (sum > 0xFFFF) sum = (sum & 0xFFFF) + 1;
        }

        return static_cast<uint16_t>(~sum & 0xFFFF);
    }

    std::vector<uint8_t> serialize() {
        // Calculating the data and header length
        const size_t totalSize = sizeof(Header) + payload.size();
        std::vector<uint8_t> buffer(totalSize);

        // Null the checksum and calculate
        header.checksum = 0;
        header.checksum = calcChecksum();

        // Copying the header and adding to the start of the buffer
        std::memcpy(buffer.data(), &header, sizeof(Header));

        // If there is any data, add that after the header
        if (!payload.empty()) {
            std::memcpy(buffer.data() + sizeof(Header), payload.data(), payload.size());
        }
        return buffer;
    }
};

#endif //IPK_PROJ2_RDTHEADER_H
