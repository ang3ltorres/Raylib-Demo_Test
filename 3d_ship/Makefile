OBJS = main.o
BIN = bin/3DSHIP.exe
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Os -Iinclude
LDFLAGS = -Llib -lraylib

all: $(BIN)
	$(BIN)

$(BIN): $(OBJS)
	$(CXX) $(OBJS) -o $(BIN) $(LDFLAGS)

clean:
	rm -f $(BIN) $(OBJS)

main.o: main.cpp
