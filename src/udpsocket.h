//
// Created by radim on 08.04.2026.
//

#ifndef IPK_PROJ2_UDPSOCKET_H
#define IPK_PROJ2_UDPSOCKET_H

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>

class UDPSocket {
private:
    int fd; // File descriptor
    struct sockaddr_storage remote_addr; // Store the remote address
    socklen_t addr_len;

public:
    UDPSocket() : fd(-1), addr_len(sizeof(remote_addr)) {}

    // Close the socket
    ~UDPSocket() {
        if (fd != -1) close(fd);
    }

    // Server init on the port
    bool bind(int port, const std::string& address = "") {
        struct addrinfo hints, *res;
        std::memset(&hints, 0, sizeof(hints));

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;

        std::string portStr = std::to_string(port);
        const char* addrPtr = address.empty() ? nullptr : address.c_str();

        if (getaddrinfo(addrPtr, portStr.c_str(), &hints, &res) != 0) {
            return false;
        }

        // Iterate the results and try to create a socket and bind
        for (struct addrinfo* ptr = res; ptr != nullptr; ptr = ptr->ai_next) {
            fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (fd < 0)
                continue;

            if (::bind(fd, ptr->ai_addr, ptr->ai_addrlen) == 0) {
                freeaddrinfo(ptr);
                return true;
            }
            close(fd);
        }
        freeaddrinfo(res);
        return false;
    }

    // Init for the client (we will prepare the destination address)
    bool connect(const std::string& host, int port) {

        struct addrinfo hints, *res;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0) {
            return false;
        }

        // Try the first available address
        fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (fd < 0) {
            freeaddrinfo(res);
            return false;
        }

        // Save a destination address for sending send()
        std::memcpy(&remote_addr, res->ai_addr, res->ai_addrlen);
        addr_len = res->ai_addrlen;

        freeaddrinfo(res);
        return true;
    }

    // Send the data
    ssize_t send(const std::vector<uint8_t>& data) {
        return sendto(fd, data.data(), data.size(), 0,
                      (struct sockaddr*)&remote_addr, addr_len);
    }

    // Method to receive. Returns the amount of the bytes (-1 if timeout)
    ssize_t receive(std::vector<uint8_t>& buffer) {
        uint8_t temp[1500]; // Max ethernet frame amount
        ssize_t n = recvfrom(fd, temp, sizeof(temp), 0,
                             (struct sockaddr*)&remote_addr, &addr_len);
        if (n > 0) {
            buffer.assign(temp, temp + n);
        }
        return n;
    }

    void setTimeout(int seconds) {
        struct timeval tv;
        tv.tv_sec = seconds;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }
};



#endif //IPK_PROJ2_UDPSOCKET_H
