//
// Created by radim on 03.04.2026.
//

#include <iostream>
#include "config.h"
#include "server.h"
#include "client.h"
#include <csignal>
using namespace std;

// Global variable to stop the program
volatile sig_atomic_t stop_flag = 0;

// Function to handle the termination
void handle_signal(int sig) {
    (void)sig;
    stop_flag = 1;
}

int main(int argc, char** argv) {
    Config cfg;

    // Handlers active
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (!cfg.parse(argc, argv)) {
        std::cout << "Error: Invalid arguments. Use -h for help." << std::endl;
        return 1;
    }

    if (cfg.mode == AppMode::SERVER) {
        server srv;
        srv.run(cfg);
    } else {
        client clt;
        clt.run(cfg);
    }


    return 0; // Success
}