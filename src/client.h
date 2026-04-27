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
#include <fstream>
#include <csignal>
extern volatile sig_atomic_t stop_flag;

const int MAX_ATTEMPTS = 5;

class client {
private:
    void sendFile(UDPSocket& socket, Config& config, uint32_t conn_id, uint32_t start_seq) {
        std::ifstream fs;
        std::istream* input = &std::cin;

        if (config.input_file != "-") {
            fs.open(config.input_file, std::ios::binary);
            if (!fs.is_open()) {
                std::cerr << "Error: Could not open the file." << std::endl;
                exit(1);
            }
            input = &fs;
        }

        std::vector<uint8_t> fileBuffer(1185);
        uint32_t currentSeq = start_seq;

        while (input->good() && !stop_flag) {
            input->read(reinterpret_cast<char*>(fileBuffer.data()), fileBuffer.size());
            size_t bytesRead = input->gcount();
            if (bytesRead == 0) break;

            RDTPacket dataPacket;
            dataPacket.header.conn_id = conn_id;
            dataPacket.header.seq_number = currentSeq;
            dataPacket.header.flags = 0;
            dataPacket.payload.assign(fileBuffer.begin(), fileBuffer.begin() + bytesRead);

            // Basic stop and wait (TODO: complete the implementation)
            bool acked = false;
            while (!acked && !stop_flag) {
                socket.send(dataPacket.serialize());

                std::vector<uint8_t> ackBuf;
                if (socket.receive(ackBuf) > 0) {
                    RDTPacket res;
                    if (res.deserialize(ackBuf.data(), ackBuf.size()) &&
                        (res.header.flags & 2) &&
                        res.header.ack >= (currentSeq + bytesRead)) {
                        acked = true;
                    }
                }
            }
            currentSeq += bytesRead;
        }
        if (!stop_flag) {
            std::cerr << "File sent. Now the sending FIN..." << std::endl;
            RDTPacket finPacket;
            finPacket.header.conn_id = conn_id;
            finPacket.header.flags = FLAG_FIN;
            finPacket.header.seq_number = currentSeq;

            bool finAcked = false;
            int finAttempts = 0;
            while (!finAcked && finAttempts < MAX_ATTEMPTS && !stop_flag) {
                socket.send(finPacket.serialize());
                std::vector<uint8_t> resBuffer;
                if (socket.receive(resBuffer) > 0) {
                    RDTPacket res;
                    if (res.deserialize(resBuffer.data(), resBuffer.size()) && (res.header.flags & FLAG_ACK)) {
                        finAcked = true;
                        std::cerr << "FIN accepted by the server." << std::endl;
                    }
                }
                finAttempts++;
            }
        }
    }
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

        std::cerr << "Starting handshake..." << std::endl;

        // Send a SYN and wait for SYN-ACK

        while (!connected && attempts < MAX_ATTEMPTS && !stop_flag) {
            socket.send(synPacket.serialize());
            attempts++;
            std::cerr << "Attempt: " << attempts << "..."<<std::endl;
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
                    std::cerr << "Connected to the server!" << std::endl;
                    connected = true;
                }
            }
        }
        if (!connected) {
            std::cerr << "Failed to connect after " << MAX_ATTEMPTS << " attempts." << std::endl;
            exit(1);
        }
        else {
            sendFile(socket, config, synPacket.header.conn_id, synPacket.header.seq_number + 1);
        }
    }
};



#endif //IPK_PROJ2_CLIENT_H
