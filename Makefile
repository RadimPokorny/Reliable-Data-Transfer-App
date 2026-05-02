CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror
TARGET = ipk-rdt
SOURCES = src/*.cpp
TEST_BIN = run_unit_tests
TEST_SOURCES = test/test_main.cpp test/tests.cpp
PROXY_BIN = ./impairment_proxy

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -Isrc -o $(TARGET)

NixDevShellName:
	@echo c

$(PROXY_BIN): test/test_ipk_rdt.c
	gcc -O2 test/test_ipk_rdt.c -o $(PROXY_BIN) -lpthread

test: clean $(TARGET)
	python3 test/tester.py

clean:
	rm -f $(TARGET) test_in.bin test_out.bin