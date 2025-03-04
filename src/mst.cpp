#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

using namespace std;

struct EdgeData {
    int u, v, lengths;
    friend std::istream& operator>>(std::istream& in, EdgeData& edge) {
        in >> edge.u >> edge.v >> edge.lengths;
        return in;
    }
};

int main() {
    int n, m;
    cin >> n >> m;
    
    vector<EdgeData> graph(m);
    for (int i = 0; i < m; i++) {
        cin >> graph[i];
    }

    sort(graph.begin(), graph.end(), [](const EdgeData& a, const EdgeData& b) {
        return a.lengths < b.lengths;
    });

    vector<int> parent(n + 1), rank(n + 1, 0);
    for (int i = 1; i <= n; i++) parent[i] = i;

    function<int(int)> find = [&](int x) -> int {
        return (parent[x] == x) ? x : (parent[x] = find(parent[x]));
    };

    auto unionSets = [&](int a, int b) -> bool {
        int rootA = find(a);
        int rootB = find(b);
        if (rootA != rootB) {
            if (rank[rootA] < rank[rootB])
                swap(rootA, rootB);
            parent[rootB] = rootA;
            if (rank[rootA] == rank[rootB])
                rank[rootA]++;
            return true;
        }
        return false;
    };

    int mstWeight = 0, edgesUsed = 0;
    for (const auto& edge : graph) {
        if (unionSets(edge.u, edge.v)) {
            mstWeight += edge.lengths;
            edgesUsed++;
            if (edgesUsed == n - 1) break;
        }
    }

    cout << mstWeight << "\n";

    return 0;
}
