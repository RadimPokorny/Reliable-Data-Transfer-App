# IPK Project 2 - RDT Application
## Author: Radim Pokorný (xpokorr00)

## Project Overview
This application is implemented in C++ and is object-oriented. The main task was to
implement the sliding window in client-cerver communication with a goal to
make the communication as fast as possible. 

## Build and run instructions
### Prerequisites
* **Operating System:** Linux (tested on x86_64).
* **Privileges:** Root/sudo access is mandatory for raw socket operations.
* **Libraries:** `libpcap` development headers.
* **Compiler:** `gcc` with `make`.

### Compilation
To compile the project, execute the following command in the root directory:
```bash
make all
```

This produces the ipk-rdt binary.

## Testing
Python was chosen as the testing framework for a better work with proxies and to 
simulate the two terminal communication easily.

### Run the tests
To run the tests you can run this single command 
```bash
make test
```

## Known limitations
* Algorithm has a hard-coded value for the timeout and does not support the dynamic changes in network
* The implementation does not support flow control using the window size in ACK packets.






