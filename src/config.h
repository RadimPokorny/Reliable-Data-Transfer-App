//
// Created by radim on 09.04.2026.
//

#ifndef IPK_PROJ2_CONFIG_H
#define IPK_PROJ2_CONFIG_H

#include <string>
#include <iostream>
#include <getopt.h>
#include <vector>

enum class AppMode {
    UNSET,
    SERVER,
    CLIENT
};

class Config {
public:
    AppMode mode = AppMode::UNSET;
    int port = -1;
    std::string address = ""; // For server bind, for client the destination
    std::string input_file = "-";  // Default stdin
    std::string output_file = "-"; // Default stdout
    double timeout = 1;               // Default timeout

    /*
     * Print the help of usage
     */
    void printHelp(const char* progName) {
        std::cout << "Usage:\n"
                  << "  Server: " << progName << " -s -p PORT [-a ADDRESS] [-o OUTPUT] [-w TIMEOUT]\n"
                  << "  Client: " << progName << " -c -a HOST -p PORT [-i INPUT] [-w TIMEOUT]\n"
                  << "Options:\n"
                  << "  -h, --help      Show this help\n"
                  << "  -s              Server mode (receive)\n"
                  << "  -c              Client mode (send)\n"
                  << "  -p PORT         UDP port number\n"
                  << "  -a ADDR/HOST    Address to bind to (server) or destination (client)\n"
                  << "  -i INPUT        Input file (default: stdin)\n"
                  << "  -o OUTPUT       Output file (default: stdout)\n"
                  << "  -w TIMEOUT      Timeout in seconds (default: 1)\n";
    }

    /*
     * Process the arguments
     */
    bool parse(int argc, char* argv[]) {
        struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };

        int opt;
        while ((opt = getopt_long(argc, argv, "hscp:a:i:o:w:", long_options, nullptr)) != -1) {
            switch (opt) {
                case 'h':
                    printHelp(argv[0]);
                    exit(0); // Close successfully if the user wants to print the help
                case 's':
                    if (mode == AppMode::CLIENT) return false; // Cannot be both
                    mode = AppMode::SERVER;
                    break;
                case 'c':
                    if (mode == AppMode::SERVER) return false; // Cannot be both
                    mode = AppMode::CLIENT;
                    break;
                case 'p':
                    try { port = std::stoi(optarg); } catch (...) { return false; }
                    break;
                case 'a':
                    address = optarg;
                    break;
                case 'i':
                    input_file = optarg;
                    break;
                case 'o':
                    output_file = optarg;
                    break;
                case 'w':
                    try {
                        timeout = std::stod(optarg);
                        if (timeout <= 0) return false;
                    } catch (...) { return false; }
                    break;
                default:
                    return false;
            }
        }

        return validate();
    }

private:
    /*
     * Rules check
     */
    bool validate() {
        if (mode == AppMode::UNSET) return false; // -s or -c is missing
        if (port < 0 || port > 65535) return false; // Invalid port

        if (mode == AppMode::CLIENT && address.empty()) {
            // The client must have a valid target address
            return false;
        }
        return true;
    }
};


#endif //IPK_PROJ2_CONFIG_H
