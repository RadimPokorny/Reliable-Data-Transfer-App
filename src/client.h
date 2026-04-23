//
// Created by radim on 03.04.2026.
//

#ifndef IPK_PROJ2_CLIENT_H
#define IPK_PROJ2_CLIENT_H
#include "udpsocket.h"
#include "config.h"
#include "rdtpacket.h"
#include <random>
#include <ctime>


class client {
public:
    void run(Config& config) {

        // Random number utilites
        std::mt19937 rng(static_cast<uint32_t>(std::time(nullptr)));
        std::uniform_int_distribution<uint32_t> dist(1, 0xFFFFFFFF);

        UDPSocket socket;
        if (!socket.connect(config.address, config.port)) {
            std::cerr << "Connection failed!" << std::endl;
            exit(1);
        }

        socket.setTimeout(config.timeout);

        // Handshake phase
        RDTPacket synPacket;
        synPacket.header.conn_id = dist(rng);
        synPacket.header.flags = 1;
        synPacket.header.seq_number = dist(rng);

        bool connected = false;
        int attempts = 0;
        const int MAX_ATTEMPTS = 5;

        std::cerr << "Starting handshake..." << std::endl;

        // Send a SYN and wait for SYN-ACK

        while (!connected && attempts < MAX_ATTEMPTS) {
            socket.send(synPacket.serialize());
            attempts++;
            std::cout << "Attempt: " << attempts << "..."<<std::endl;
            std::vector<uint8_t> response;

            if (socket.receive(response) > 0) {
                RDTPacket responsePacket;
                if (responsePacket.deserialize(response.data(), response.size()) &&
                    (responsePacket.header.flags == 3) &&
                    (responsePacket.header.conn_id == synPacket.header.conn_id)) {

                    std::cerr << "SYN-ACK received. Sending ACK..." << std::endl;


                    // Final acknowledge ACK
                    RDTPacket ackPacket;
                    ackPacket.header.conn_id = responsePacket.header.conn_id;
                    ackPacket.header.flags = 2;
                    ackPacket.header.ack = responsePacket.header.seq_number + 1;
                    socket.send(ackPacket.serialize());
                    std::cout << "Connected to the server!" << std::endl;
                    connected = true;
                }
            }
        }
        if (!connected) {
            std::cerr << "Failed to connect after " << MAX_ATTEMPTS << " attempts." << std::endl;
            exit(1);
        }
    }
};



#endif //IPK_PROJ2_CLIENT_H
