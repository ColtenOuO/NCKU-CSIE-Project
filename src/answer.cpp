#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <utility>
#include <bitset>
#include <stack>
#include <set>
#include <cassert>
#include <fstream>
#include <pthread.h>
#include <mutex>
#define int long long
#define SZ 5001
#define NUM_THREADS 4  // 可以根據您的 CPU 核心數調整
using namespace std;

ofstream outputFile("./testing/answer/result.txt");
vector<pair<int,int>> e[100005];
mutex result_mutex;  // 保護共享資源的互斥鎖

class DSU {
    private:
        vector<int> dsu, sz;
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
                if (sz[a] > sz[b]) swap(a, b);
                dsu[b] = a;
                sz[a] += sz[b];
            }
        }
};

struct EdgeData {
    int vertex1, vertex2, weight;
};

struct BitsetComparator {
    bool operator()(const bitset<SZ>& a, const bitset<SZ>& b) const {
        return a.to_string() < b.to_string();
    }
    bool operator()(const pair<int, bitset<SZ>>& a, const pair<int, bitset<SZ>>& b) const {
        if (a.first != b.first) return a.first < b.first;
        return a.second.to_string() < b.second.to_string();
    }
};

class SPTree_Solver {
    private:
        set<pair<int, bitset<SZ>>, BitsetComparator> result;
        set<bitset<SZ>, BitsetComparator> check;
        vector<int> weight;
        int VERTEX_COUNT, EDGE_COUNT;
        vector<vector<int>> graph;
        vector<EdgeData> edge_list;

        // 執行緒工作結構體
        struct ThreadWork {
            SPTree_Solver* solver;
            int start_edge;
            int end_edge;
            pair<int, bitset<SZ>> target;
        };

        vector<int> Finding_Cycle(int edge_idx, bitset<SZ> &status) {
            vector<bool> visited(VERTEX_COUNT + 1, false);
            stack<pair<int, int>> route;
            vector<vector<pair<int, int>>> rebuild_graph(VERTEX_COUNT + 1);

            for (int i = 1; i <= EDGE_COUNT; i++) {
                if (status[i] == 1) {
                    rebuild_graph[edge_list[i].vertex1].push_back({edge_list[i].vertex2, i});
                    rebuild_graph[edge_list[i].vertex2].push_back({edge_list[i].vertex1, i});
                }
            }
            rebuild_graph[edge_list[edge_idx].vertex1].push_back({edge_list[edge_idx].vertex2, edge_idx});
            rebuild_graph[edge_list[edge_idx].vertex2].push_back({edge_list[edge_idx].vertex1, edge_idx});

            vector<int> route_result;
            function<void(int, int)> dfs = [&](int now, int last) {
                for (auto &[vertex, id] : rebuild_graph[now]) {
                    if( route_result.size() != 0 ) return;
                    if (vertex == last) continue;
                    if( visited[vertex] != 0 ) {
                        while (!route.empty()) {
                            route_result.push_back(route.top().second);
                            if (route.top().first == vertex) return;
                            route.pop();
                        }
                        if( route_result.size() != 0 ) return;
                        assert(false);
                    }
                    visited[vertex] = true;
                    route.push({vertex, id});
                    dfs(vertex, now);
                    if( route_result.size() != 0 ) return;
                    route.pop();
                }
                return;
            };
            visited[edge_list[edge_idx].vertex1] = true;
            dfs(edge_list[edge_idx].vertex1, -1);
            return route_result;
        }

        static void* thread_process_edges(void* arg) {
            ThreadWork* work = (ThreadWork*)arg;
            vector<pair<int, bitset<SZ>>> local_results;
            
            for (int j = work->start_edge; j <= work->end_edge; j++) {
                if (work->target.second[j] == 0) {
                    vector<int> cycle = work->solver->Finding_Cycle(j, work->target.second);
                    for (int id : cycle) {
                        if (work->solver->edge_list[id].weight <= work->solver->edge_list[j].weight && id != j) {
                            bitset<SZ> new_tree = work->target.second;
                            new_tree[j] = 1;
                            new_tree[id] = 0;
                            int total_weight = work->target.first - work->solver->edge_list[id].weight + work->solver->edge_list[j].weight;
                            
                            // 使用互斥鎖保護共享資源
                            lock_guard<mutex> lock(result_mutex);
                            if (work->solver->check.find(new_tree) == work->solver->check.end()) {
                                work->solver->check.insert(new_tree);
                                work->solver->result.insert({total_weight, new_tree});
                            }
                        }
                    }
                }
            }
            delete work;
            return nullptr;
        }

    public:
        SPTree_Solver(int VERTEX_COUNT, int EDGE_COUNT) : VERTEX_COUNT(VERTEX_COUNT), EDGE_COUNT(EDGE_COUNT) {
            weight.resize(EDGE_COUNT + 1);
            graph.resize(VERTEX_COUNT + 1);
            edge_list.resize(EDGE_COUNT + 1);
            for (int i = 1; i <= EDGE_COUNT; i++) {
                int u, v, w;
                cin >> u >> v >> w;
                graph[u].push_back(v);
                graph[v].push_back(u);
                edge_list[i] = {u, v, w};
            }
        }
        void solveMST() {
            vector<pair<EdgeData, int>> tmp;
            for (int i = 1; i <= EDGE_COUNT; i++) {
                tmp.push_back({edge_list[i], i});
            }
            sort(tmp.begin(), tmp.end(), [](auto &a, auto &b) {
                return a.first.weight < b.first.weight;
            });

            DSU dsu(VERTEX_COUNT);
            int mst_total = 0;
            bitset<SZ> mst_result;
            for (auto &[edge, id] : tmp) {
                if (dsu.find(edge.vertex1) != dsu.find(edge.vertex2)) {
                    dsu.unite(edge.vertex1, edge.vertex2);
                    mst_total += edge.weight;
                    mst_result[id] = 1;
                }
            }
            result.insert({mst_total, mst_result});
        }
        void solveST_by_kth(int kth) {
            for (int i = 1; i < kth; i++) {
                cout << i << " ok\n";
                outputFile << i << "-th Spanning Tree = ";
                outputFile << output_kth_tree() << "\n";
                
                if (result.empty()) return;
                auto target = *result.begin();
                result.erase(result.begin());

                // 創建執行緒
                pthread_t threads[NUM_THREADS];
                int edges_per_thread = EDGE_COUNT / NUM_THREADS;

                for (int t = 0; t < NUM_THREADS; t++) {
                    ThreadWork* work = new ThreadWork;
                    work->solver = this;
                    work->start_edge = t * edges_per_thread + 1;
                    work->end_edge = (t == NUM_THREADS - 1) ? EDGE_COUNT : (t + 1) * edges_per_thread;
                    work->target = target;

                    pthread_create(&threads[t], nullptr, thread_process_edges, (void*)work);
                }

                // 等待所有執行緒完成
                for (int t = 0; t < NUM_THREADS; t++) {
                    pthread_join(threads[t], nullptr);
                }
            }
        }
        int output_kth_tree() {
            return result.begin()->first;
        }
};

signed main(int32_t argc, char *argv[]) {
    int n, m;
    cin >> n >> m;
    SPTree_Solver solver(n, m);
    int k = stoi(argv[1]);
    solver.solveMST();
    solver.solveST_by_kth(k);
    outputFile << k << "-th Spanning Tree = ";
    outputFile << solver.output_kth_tree() << "\n";
    outputFile.close();
    return 0;
}
