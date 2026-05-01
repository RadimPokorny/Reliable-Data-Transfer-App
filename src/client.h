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
#include <map>
#include <iostream>

extern volatile sig_atomic_t stop_flag;

constexpr double DELAY = 0.2;

constexpr double FAST_RECV_DELAY = 0.001;

const int MAX_ATTEMPTS = 5;

class client {
private:
    void sendFile(UDPSocket& socket, Config& config, uint32_t conn_id, uint32_t start_seq) {
        std::ifstream fs;
        std::istream* input = &std::cin;

        // Handle the file or STDin input
        if (config.input_file != "-") {
            fs.open(config.input_file, std::ios::binary);
            if (!fs.is_open()) {
                std::cout << "Error: Could not open the file." << std::endl;
                exit(1);
            }
            input = &fs;
        }

        uint32_t nextSeq = start_seq;
        uint32_t lastAck = start_seq;
        int dupAckCount = 0;
        std::map<uint32_t, RDTPacket> window;
        bool endOfFile = false;
        constexpr size_t windowSize = 64;
        std::vector<uint8_t> fileBuffer(1185);

        socket.setTimeout(DELAY);

        while (!endOfFile || !window.empty()) {
            // Fill the window
            while (window.size() < windowSize && !endOfFile) {
                input->read(reinterpret_cast<char*>(fileBuffer.data()), (std::streamsize)fileBuffer.size());
                size_t bytesRead = (size_t)input->gcount();

                if (bytesRead == 0) {
                    endOfFile = true;
                    break;
                }

                RDTPacket dataPacket;
                dataPacket.header.conn_id = conn_id;
                dataPacket.header.seq_number = nextSeq;
                dataPacket.header.flags = 0;
                dataPacket.payload.assign(fileBuffer.begin(), fileBuffer.begin() + bytesRead);

                socket.send(dataPacket.serialize());
                window[nextSeq] = dataPacket;
                nextSeq += bytesRead;
            }

            // ACK receive
            if (!window.empty()) {
                socket.setTimeout(DELAY);
                std::vector<uint8_t> ackBuf;
                ssize_t n = socket.receive(ackBuf);

                if (n > 0) {
                    socket.setTimeout(FAST_RECV_DELAY);
                    do {
                        RDTPacket res;
                        if (res.deserialize(ackBuf.data(), ackBuf.size()) && (res.header.flags & FLAG_ACK)) {
                            // Confirm the new data
                            if ((int32_t)(res.header.ack - lastAck) > 0) {
                                lastAck = res.header.ack;
                                dupAckCount = 0;
                                auto it = window.begin();
                                while (it != window.end() && (int32_t)(lastAck - it->first) > 0) {
                                    it = window.erase(it);
                                }
                                // Handle duplicate ACKs
                            } else if (res.header.ack == lastAck && !window.empty()) {
                                if (++dupAckCount == 3) {
                                    socket.send(window.begin()->second.serialize());
                                }
                            }
                        }
                    } while (socket.receive(ackBuf) > 0);
                    // Handle timeout
                } else if (n == -1) {
                    if (!window.empty()) {
                        socket.send(window.begin()->second.serialize());
                    }
                }
            }
        }
        // Terminate the connection
        if (!stop_flag) {
            RDTPacket finPacket;
            finPacket.header.conn_id = conn_id;
            finPacket.header.flags = FLAG_FIN;
            finPacket.header.seq_number = nextSeq;

            bool finAcked = false;
            int finAttempts = 0;
            socket.setTimeout(DELAY);
            while (!finAcked && finAttempts < MAX_ATTEMPTS && !stop_flag) {
                socket.send(finPacket.serialize());
                std::vector<uint8_t> resBuffer;
                if (socket.receive(resBuffer) > 0) {
                    RDTPacket res;
                    if (res.deserialize(resBuffer.data(), resBuffer.size()) && (res.header.flags & FLAG_ACK)) {
                        finAcked = true;
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
            return;
        }

        socket.setTimeout(DELAY);

        // Handshake phase
        RDTPacket synPacket;
        synPacket.header.conn_id = dist(rng);
        synPacket.header.flags = 1;
        synPacket.header.seq_number = dist(rng);

        bool connected = false;
        int attempts = 0;

        // Send a SYN and wait for SYN-ACK

        while (!connected && attempts < MAX_ATTEMPTS && !stop_flag) {
            socket.send(synPacket.serialize());
            attempts++;
            std::vector<uint8_t> response;

            if (socket.receive(response) > 0) {
                RDTPacket responsePacket;
                if (responsePacket.deserialize(response.data(), response.size()) &&
                    (responsePacket.header.flags == 3) &&
                    (responsePacket.header.conn_id == synPacket.header.conn_id)) {



                    // Final acknowledge ACK
                    RDTPacket ackPacket;
                    ackPacket.header.conn_id = responsePacket.header.conn_id;
                    ackPacket.header.flags = 2;
                    ackPacket.header.ack = responsePacket.header.seq_number + 1;
                    socket.send(ackPacket.serialize());
                    connected = true;
                }
            }
        }
        if (!connected) {
            return;
        }
        else {
            sendFile(socket, config, synPacket.header.conn_id, synPacket.header.seq_number + 1);
        }
    }
};

#endif //IPK_PROJ2_CLIENT_H