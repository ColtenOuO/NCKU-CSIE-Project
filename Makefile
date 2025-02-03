MPICC = mpic++
CXXFLAGS = -pthread -Wall -O2

TARGET = ./src/main.out
SRCS = ./src/main.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(MPICC) $(CXXFLAGS) -o $@ $^

run: $(TARGET)
	mpirun -np 4 ./$(TARGET) 20 10 5000

clean:
	rm -f $(TARGET)
