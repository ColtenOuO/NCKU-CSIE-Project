#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <pthread.h>
#include <random>
#include <limits>
#include <ctime>
#include <set>
#include <algorithm>
#include <bitset>
#include <climits>

#define SZ 5001
#define int long long

int TARGET_NUMBER = 0;

class DSU {
    private:
        std::vector<int> dsu, sz;
    public:
        DSU(int n) : dsu(n + 1), sz(n + 1, 1) {
            for (int i = 1; i <= n; i++) dsu[i] = i;
        }
        int find(int idx) {
            return (dsu[idx] == idx) ? idx : (dsu[idx] = find(dsu[idx]));
        }
        void unite(int a, int b) {
            a = find(a), b = find(b);
            if (a != b) {
                if (sz[a] > sz[b]) std::swap(a, b);
                dsu[b] = a;
                sz[a] += sz[b];
            }
        }
};

const double ALPHA = 1.0;
const double BETA = 5.0;
const double RHO = 0.4;
const double Q = 100.0;

int NUM_VERTEX, NUM_EDGE, NUM_ANTS, NUM_ITERATIONS, K;
int time_tmp = INT_MAX;
pthread_mutex_t mutex;

struct EdgeData {
    int u, v, lengths;
    friend std::istream& operator>>(std::istream& in, EdgeData& edge) {
        in >> edge.u >> edge.v >> edge.lengths;
        return in;
    }
};

std::vector<std::vector<std::pair<int, int> > > graph;
std::vector<EdgeData> edge_data;
std::vector<double> pheromone;
struct ComparePair {
    bool operator()(const std::pair<int, std::bitset<SZ> >& a,
                    const std::pair<int, std::bitset<SZ> >& b) const {
        if (a.first != b.first) return a.first < b.first;
        return a.second.to_string() < b.second.to_string();
    }
};
std::set<std::pair<int, std::bitset<SZ> >, ComparePair> s;

bool check_tree(std::bitset<SZ> &result) {
    DSU dsu(NUM_VERTEX);
    int count = 0;
    for (int i = 1; i <= NUM_EDGE; i++) {
        if (result[i] == 1) {
            if (dsu.find(edge_data[i].u) == dsu.find(edge_data[i].v)) {
                return false;
            }
            dsu.unite(edge_data[i].u, edge_data[i].v);
            count++;
        }
    }
    return (count == NUM_VERTEX - 1);
}

std::bitset<SZ> create_tree(std::mt19937& rng) {
    std::bitset<SZ> result;
    std::vector<bool> visited(NUM_VERTEX + 1, false);
    class DSU dsu(NUM_VERTEX);

    for (int step = 0; step < NUM_VERTEX - 1; step++) {
        std::vector<double> probabilities(NUM_EDGE + 1, 0.0);
        double total_prob = 0.0;
        for(int edge_id = 1; edge_id <= NUM_EDGE; edge_id++) {
            if( !result[edge_id] && dsu.find(edge_data[edge_id].u) != dsu.find(edge_data[edge_id].v) ) {
                probabilities[edge_id] = std::pow(pheromone[edge_id], ALPHA) *
                                         std::pow(1.0 / edge_data[edge_id].lengths, BETA);
                total_prob += probabilities[edge_id];
            }
        }
        if (total_prob == 0) break;
        std::uniform_real_distribution<double> prob_dist(0.0, total_prob);
        double random_value = prob_dist(rng);
        for(int edge_id = 1; edge_id <= NUM_EDGE; edge_id++) {
            if( !result[edge_id] ) {
                random_value -= probabilities[edge_id];
                if (random_value <= 0.0) {
                    result[edge_id] = 1;
                    dsu.unite(edge_data[edge_id].u, edge_data[edge_id].v);
                    break;
                }
            }
        }
    }
    return result;
}

void* ant_worker(void* arg) {
    int rank = *(int*)arg;
    std::mt19937 rng(rank + time(nullptr));
    std::vector<std::bitset<SZ> > ant_choose(NUM_ANTS);
    std::vector<int> total_length(NUM_ANTS, 0);

    for (int iteration = 0; iteration < NUM_ITERATIONS; iteration++) {
        for (int ant = 0; ant < NUM_ANTS; ant++) {
            std::bitset<SZ> result = create_tree(rng);
            int total = 0;
            for (int i = 1; i <= NUM_EDGE; i++) {
                if (result[i] == 1) total += edge_data[i].lengths;
            }

            if( result.count() != static_cast<std::size_t>(NUM_VERTEX - 1) ) {
                ant_choose[ant] = result;
                total_length[ant] = INT_MAX;
                continue;
            }

            pthread_mutex_lock(&mutex);
            s.insert(make_pair(total, result));
            if ( s.size() > static_cast<std::size_t>(K) )  s.erase(--s.end());
            pthread_mutex_unlock(&mutex);
            ant_choose[ant] = result;
            total_length[ant] = total;

            if( total == TARGET_NUMBER ) time_tmp = std::min(time_tmp, iteration+1);
        }

        pthread_mutex_lock(&mutex);
        for (int i = 1; i <= NUM_EDGE; i++) pheromone[i] *= (1.0 - RHO);
        for (int ant = 0; ant < NUM_ANTS; ant++) {
            if ( total_length[ant] < INT_MAX ) {
                double contribution = Q / total_length[ant];
                for (int i = 1; i <= NUM_EDGE; i++) {
                    if (ant_choose[ant][i]) pheromone[i] += contribution;
                }
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    
    return nullptr;
}

signed main(int32_t argc, char* argv[]) {
    
    int size = 5;
    pthread_mutex_init(&mutex, nullptr);
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <NUM_ANTS> <NUM_ITERATIONS> <K> <TARGET_NUMBER>\n";
        return 1;
    }

    std::cin >> NUM_VERTEX >> NUM_EDGE;
    if (NUM_EDGE > SZ) {
        std::cerr << "Error: NUM_EDGE exceeds SZ (" << SZ << "). Increase SZ if necessary.\n";
        return 1;

    }

    graph.resize(NUM_VERTEX + 1);
    edge_data.resize(NUM_EDGE + 1);
    pheromone.assign(NUM_EDGE + 1, 1.0);

    for (int i = 1; i <= NUM_EDGE; i++) {
        EdgeData input_data;
        std::cin >> input_data;
        graph[input_data.u].emplace_back(input_data.v, i);
        graph[input_data.v].emplace_back(input_data.u, i);
        edge_data[i] = input_data;
    }

    NUM_ANTS = std::stoi(argv[1]);
    NUM_ITERATIONS = std::stoi(argv[2]);
    K = std::stoi(argv[3]);
    TARGET_NUMBER = std::stoi(argv[4]);

    pthread_t threads[size];
    int thread_ids[size];
    for (int i = 0; i < size; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], nullptr, ant_worker, &thread_ids[i]);
    }
    for (int i = 0; i < size; i++) {
        pthread_join(threads[i], nullptr);      
    }

    std::cout << s.rbegin() -> first << ", " << time_tmp << "\n";
    pthread_mutex_destroy(&mutex);
    return 0;
}
