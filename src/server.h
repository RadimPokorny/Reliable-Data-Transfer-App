//
// Created by radim on 03.04.2026.
//

// Similar to client.h, AI was used here for file-reading functions
// and for gradually correcting logic errors in the loop.

#ifndef IPK_PROJ2_SERVER_H
#define IPK_PROJ2_SERVER_H
#include <fstream>
#include <vector>
#include "rdtpacket.h"
#include "udpsocket.h"
#include "config.h"
#include <csignal>
#include <map>
extern volatile sig_atomic_t stop_flag;

constexpr int FLUSH_DENSITY = 100;

class server {
    size_t packetsSinceLastFlush = 0;
public:
    void run(Config& config) {

        UDPSocket socket;
        if (!socket.bind(config.port, config.address)) {
            return;
        }

        std::ofstream fs;
        std::ostream* output = &std::cout;
        if (config.output_file != "-" && !config.output_file.empty()) {
            fs.open(config.output_file, std::ios::binary);
            output = &fs;
        }

        socket.setTimeout(config.timeout);

        this->receiveBuffer.clear();
        bool sessionStarted = false;

        while (!stop_flag) {
            std::vector<uint8_t> buffer;
            if (socket.receive(buffer) > 0) {
                RDTPacket incPacket;
                if (!incPacket.deserialize(buffer.data(), buffer.size())) {
                    continue; // Skip corrupted packets
                }

                if (incPacket.header.flags == 1) {
                    // Init the expected seq. number
                    this->expectedSeq = incPacket.header.seq_number + 1;
                    sessionStarted = true;
                    // Handle SYN
                    RDTPacket response;
                    response.header.flags = 3; // SYN-ACK
                    response.header.conn_id = incPacket.header.conn_id;
                    response.header.seq_number = 0;
                    socket.send(response.serialize());
                }
                else if (incPacket.header.flags == 0) {
                    if (!sessionStarted) {
                        // ignore the data, if SYN not yet
                        continue;
                    }
                    uint32_t seq = incPacket.header.seq_number;

                    if (seq < expectedSeq) {
                        // We got the packet, but not ACK -> send actual expectedSeq
                        RDTPacket oldAck;
                        oldAck.header.flags = FLAG_ACK;
                        oldAck.header.conn_id = incPacket.header.conn_id;
                        oldAck.header.ack = expectedSeq;
                        socket.send(oldAck.serialize());
                        // Don't work with this packet anymore
                        continue;
                    }

                    size_t payloadSize = incPacket.payload.size();

                    if (seq == expectedSeq) {
                        // If we got what we want we will write it
                        output->write(reinterpret_cast<const char*>(incPacket.payload.data()), payloadSize);
                        packetsSinceLastFlush++;

                        if (packetsSinceLastFlush >= FLUSH_DENSITY) {
                            output->flush();
                            packetsSinceLastFlush = 0;
                        }
                        expectedSeq += payloadSize;

                        // Are there another packets in the buffer?
                        auto it = receiveBuffer.begin();
                        while (it != receiveBuffer.end() && it->first == expectedSeq) {
                            output->write(reinterpret_cast<const char*>(it->second.data()), it->second.size());
                            packetsSinceLastFlush++;

                            if (packetsSinceLastFlush >= FLUSH_DENSITY) {
                                output->flush();
                                packetsSinceLastFlush = 0;
                            }

                            expectedSeq += it->second.size();
                            it = receiveBuffer.erase(it); // Writen will get off the buffer
                        }
                    }
                    else if (seq > expectedSeq) {
                        // If the packet got ahead we will store it to the buffer
                        if (receiveBuffer.find(seq) == receiveBuffer.end()) {
                            receiveBuffer[seq] = incPacket.payload;
                        }
                    }

                    // Send ack with info about the next one
                    RDTPacket ack;
                    ack.header.flags = FLAG_ACK;
                    ack.header.conn_id = incPacket.header.conn_id;
                    ack.header.ack = expectedSeq;
                    socket.send(ack.serialize());
                }
                else if (incPacket.header.flags == 4) {
                    // Handle FIN
                    RDTPacket finAck;
                    finAck.header.flags = 2; // ACK the FIN
                    finAck.header.conn_id = incPacket.header.conn_id;
                    socket.send(finAck.serialize());
                    // One more FIN check
                    socket.setTimeout(0.2);
                    std::vector<uint8_t> dummy;
                    socket.receive(dummy);
                    break; // End transmission
                }
            }
        }
    }
private:
    // Variable to show what is next in order
    uint32_t expectedSeq = 0;
    std::map<uint32_t, std::vector<uint8_t>> receiveBuffer;
};



#endif //IPK_PROJ2_SERVER_H
