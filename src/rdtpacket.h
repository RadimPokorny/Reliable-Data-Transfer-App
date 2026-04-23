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
    uint16_t calcChecksum(const uint8_t* data, size_t len) {
        // Checksum must be NIL in the very beginning
        uint32_t sum = 0;
        const uint16_t* ptr = reinterpret_cast<const uint16_t*>(data);

        // RFC 1071 Algorithm
        while (len > 1) {
            sum += *ptr++;
            len -= 2;
        }
        if (len > 0) {
            sum += *reinterpret_cast<const uint8_t*>(ptr);
        }
        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
        return static_cast<uint16_t>(~sum);
    }

    std::vector<uint8_t> serialize() {
        // Calculating the data and header length
        size_t packet_size = sizeof(Header) + payload.size();
        std::vector<uint8_t> buffer(packet_size);

        // Null the checksum and calculate
        header.checksum = 0;

        // Copy the entire structure into the buffer
        std::memcpy(buffer.data(), &header, sizeof(Header));

        // Data and header into the buffer
        if (!payload.empty()) {
            std::memcpy(buffer.data() + sizeof(Header), payload.data(), payload.size());
        }

        uint16_t compChecksum = calcChecksum(buffer.data(), buffer.size());

        // Copying the header and adding to the start of the buffer
        std::memcpy(buffer.data() + 12, &compChecksum, sizeof(uint16_t));

        // If there is any data, add that after the header
        return buffer;
    }

    bool deserialize(const uint8_t* data, size_t size) {

        // Is at least packet bigger than header?
        if (size < sizeof(Header))
            return false;

        // Get the header in the front of the buffer
        std::memcpy(&header, data, sizeof(Header));

        uint16_t recvChecksum = header.checksum;

        // Null the header to recalculate
        Header temp_header = header;
        temp_header.checksum = 0;

        std::vector<uint8_t> check_buffer(size);
        std::memcpy(check_buffer.data(), &temp_header, sizeof(Header));
        std::memcpy(check_buffer.data() + sizeof(Header), data + sizeof(Header), size - sizeof(Header));

        if (calcChecksum(check_buffer.data(), size) != recvChecksum) {
            std::cerr << "Packet corrupted. Checksum is corrupted." << std::endl;
            return false;
        }

        // 3. Getting the payload
        payload.assign(data + sizeof(Header), data + size);

        return true;
    }
};

#endif //IPK_PROJ2_RDTHEADER_H
