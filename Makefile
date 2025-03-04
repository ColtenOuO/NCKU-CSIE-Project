MPICC = mpic++
CXXFLAGS = -pthread -Wall -O2 -std=c++17

TARGET = ./src/main.out
SRCS = ./src/main.cpp
INPUT = ./testing/input.txt

all: $(TARGET)

$(TARGET): $(SRCS)
	$(MPICC) $(CXXFLAGS) -o $@ $^

run: $(TARGET)
	mpirun -np 1 ./$(TARGET) 50 10 3 < $(INPUT)

clean:
	rm -f $(TARGET)
