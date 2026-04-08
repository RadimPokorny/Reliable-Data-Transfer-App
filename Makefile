CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror
TARGET = ipk-rdt
SOURCES = src/*.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

NixDevShellName:
	@echo c

test: $(TARGET)
	@echo "Testing placeholder..."

clean:
	rm -f $(TARGET) *.o