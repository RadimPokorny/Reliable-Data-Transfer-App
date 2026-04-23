//
// Created by radim on 03.04.2026.
//

#ifndef IPK_PROJ2_SERVER_H
#define IPK_PROJ2_SERVER_H
#include <fstream>
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

        std::ofstream fs;
        std::ostream* output = &std::cout;
        if (config.output_file != "-" && !config.output_file.empty()) {
            fs.open(config.output_file, std::ios::binary);
            output = &fs;
        }

        while (true) {
            std::vector<uint8_t> buffer;
            if (socket.receive(buffer) > 0) {
                RDTPacket incPacket;
                if (!incPacket.deserialize(buffer.data(), buffer.size())) {
                    continue; // Skip corrupted packets
                }

                if (incPacket.header.flags == 1) {
                    // Handle SYN
                    RDTPacket response;
                    response.header.flags = 3; // SYN-ACK
                    response.header.conn_id = incPacket.header.conn_id;
                    response.header.seq_number = 0;
                    socket.send(response.serialize());
                }
                else if (incPacket.header.flags == 0) {
                    // Handle DATA
                    output->write(reinterpret_cast<const char*>(incPacket.payload.data()), incPacket.payload.size());
                    output->flush();

                    // Send ACK
                    RDTPacket ack;
                    ack.header.flags = 2;
                    ack.header.conn_id = incPacket.header.conn_id;
                    ack.header.ack = incPacket.header.seq_number + incPacket.payload.size();
                    socket.send(ack.serialize());
                }
                else if (incPacket.header.flags == 4) {
                    // Handle FIN
                    RDTPacket finAck;
                    finAck.header.flags = 2; // ACK the FIN
                    finAck.header.conn_id = incPacket.header.conn_id;
                    socket.send(finAck.serialize());
                    break; // End transmission
                }
            }
        }
    }
};



#endif //IPK_PROJ2_SERVER_H
