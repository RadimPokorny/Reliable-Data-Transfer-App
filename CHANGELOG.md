# Changelog

## [1.0.0] - 2026-05-03
### Added
Visual Documentation: Integrated state and sequence diagrams into README.md to illustrate protocol logic and packet flow.

Compliance: Added AI usage and academic integrity admissions as per IPK project guidelines.

Theory: Updated the description of the chosen reliable transfer mechanism (Selective Repeat) in the documentation.

### Fixed
IPv6 Support: Resolved issues with IPv6 address binding and communication.

Signal Handling: Improved SIGINT and SIGTERM handling to ensure graceful termination across both client and server modes.

Tester Stability: Fixed various bugs in the automated test script and optimized thread management.

### Changed
Performance Tuning: Optimized server-side output flush density and reduced receive delays to increase overall throughput.

Code Cleanup: Refactored client logic to remove unnecessary conditional blocks and moved debug statements to standard error (stderr).

Resource Management: Automated the cleanup of temporary files generated during testing.

## [0.5.0] - 2026-04-30
### Added
Automated Testing: Implemented a Python-based tester (tester.py) for validating protocol correctness under various network impairments.

Initial Documentation: Added basic instructions and theoretical background to README.md.

Project Housekeeping: Added .gitignore to maintain a clean repository.

### Removed
Unused colleague-provided testing files and obsolete debug resources.

## [0.4.0] - 2026-04-29
### Added
Sliding Window: Fully implemented the Sliding Window mechanism (Selective Repeat) to replace the initial Stop-and-Wait prototype.

Out-of-Order Buffering: Added server-side buffer management using std::map to handle segments arriving out of sequence.

Flow Control: Integrated ACK control and session observation logic.

### Fixed
Protocol Accuracy: Fixed RDT packet header offsets and corrected checksum calculation logic.

Timeouts: Fixed type casting issues for timeout values and optimized delay management for better speed.

## [0.2.0] - 2026-04-23
### Added
Session Management: Implemented the 3-way handshake (SYN, SYN-ACK, ACK) and connection teardown (FIN, ACK).

Connection Security: Added conn_id logic to prevent accidental confusion of packets from different transfer sessions.

Sequence Logic: Integrated sequence and acknowledgement numbers into the packet header and flag logic.

### Changed
Optimized serialization and deserialization functions for better data alignment.

## [0.1.0] - 2026-04-03
### Added
Foundation: Established the basic project structure and Makefile.

UDP Socket Wrapper: Developed a custom UDPSocket class to wrap low-level system calls.

Reliability Layer: Implemented the RFC 1071-compliant checksum function and initial RDTPacket structure design.