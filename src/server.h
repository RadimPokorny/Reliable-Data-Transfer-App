//
// Created by radim on 03.04.2026.
//

#ifndef IPK_PROJ2_SERVER_H
#define IPK_PROJ2_SERVER_H
#include <vector>
#include "rdtpacket.h"
#include "udpsocket.h"
#include "config.h"

class server {
public:
    void run(Config& config) {
        UDPSocket socket;
        if (!socket.bind(config.port, config.address)) {
            std::cerr << "Failed to bind port " << config.port << std::endl;
            return;
        }

        std::cout << "Server is listening on port " << config.port << std::endl;

        while (true) {
            std::vector<uint8_t> buffer;
            if (socket.receive(buffer) > 0) {
                RDTPacket incPacket;
                if (incPacket.deserialize(buffer.data(), buffer.size()) && incPacket.header.flags == 1) {
                    std::cerr << "Received SYN, sending SYN-ACK" << std::endl;
                    RDTPacket response;
                    response.header.flags = 3; // SYN + ACK = 3
                    response.header.conn_id = incPacket.header.conn_id;
                    socket.send(response.serialize());
                }
            }
        }
    }
};



#endif //IPK_PROJ2_SERVER_H
