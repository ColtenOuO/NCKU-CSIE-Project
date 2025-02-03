#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <fstream>
#include <mpi.h>
#include <pthread.h>
#include <random>
#include <limits>
#include <ctime>
#include <queue>

const double ALPHA = 1.0; // Influence of pheromone
const double BETA = 5.0;  // Influence of distance
const double RHO = 0.5;  // Pheromone evaporation rate
const double Q = 100.0;  // Pheromone deposit factor

int NUM_VERTEX, NUM_ANTS, NUM_ITERATIONS, K;
pthread_mutex_t mutex;

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    pthread_mutex_init(&mutex, nullptr);

    if (argc < 4) {
        if (rank == 0) std::cerr << "Usage: " << argv[0] << " <NUM_VERTEX> <NUM_ANTS> <NUM_ITERATIONS>\n";
        MPI_Finalize();
        return 1;
    }

    NUM_VERTEX = std::stoi(argv[1]);
    NUM_ANTS = std::stoi(argv[2]);
    NUM_ITERATIONS = std::stoi(argv[3]);

    std::cout << "Hello world\n";

    pthread_mutex_destroy(&mutex);
    MPI_Finalize();
    return 0;
}