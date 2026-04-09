//
// Created by radim on 03.04.2026.
//

#include "main.h"
#include <iostream>
#include "config.h"
#include "server.h"
#include "client.h"
using namespace std;

int main(int argc, char** argv) {
    Config cfg;

    if (!cfg.parse(argc, argv)) {
        std::cerr << "Error: Invalid arguments. Use -h for help." << std::endl;
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