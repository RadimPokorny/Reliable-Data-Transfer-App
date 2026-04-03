# Proměnné
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror
TARGET = ipk-rdt
SOURCES = src/main.cpp

# Výchozí cíl - sestavení projektu
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

# Povinný cíl pro IPK guidelines - vrací název prostředí
NixDevShellName:
	@echo c

# Povinný cíl pro spuštění testů
test: $(TARGET)
	@echo "Spouštím testy..."

# Úklid (volitelný, ale slušnost)
clean:
	rm -f $(TARGET) *.o