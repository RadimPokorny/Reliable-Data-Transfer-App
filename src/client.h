//
// Created by radim on 03.04.2026.
//

#ifndef IPK_PROJ2_CLIENT_H
#define IPK_PROJ2_CLIENT_H
#include "udpsocket.h"
#include "main.cpp"
#include "rdtpacket.h"


class client {
public:
    void run(Config& config) {
        UDPSocket socket;
        if (!socket.connect(config.address, config.port)) {
            std:cerr << "Connection failed!" << std::endl;
            return;
        }

        socket.setTimeout(config.timeout);

        // Handshake phase
        RDTPacket synPacket;
        synPacket.header.conn_id = 123; // TODO: Remove placeholder
        synPacket.header.flags = 1;

        // Send a SYN and wait for SYN-ACK
        socket.send(synPacket.serialize());

        std::vector<uint8_t> response;

        if (socket.receive(response) > 0) {
            RDTPacket responsePacket;
            if (responsePacket.deserialize(response.data(), response.size()) &&
                (responsePacket.header.flags == 3)) {

                std::cout << "Connected to the server!" << std::endl;

                // Final acknowledge ACK
                RDTPacket ackPacket;
                ackPacket.header.conn_id = responsePacket.header.conn_id;
                ackPacket.header.flags = 2;
                socket.send(ackPacket.serialize());
            }
        }
    }
};



#endif //IPK_PROJ2_CLIENT_H
