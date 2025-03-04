import random
import sys

def generate_complete_graph(n, output_file):
    edges = []
    for i in range(1, n + 1):
        for j in range(i + 1, n + 1):
            w = random.randint(1, 1000000000)
            edges.append((i, j, w))
    
    with open(output_file, 'w') as f:
        f.write(f"{n} {len(edges)}\n")
        for u, v, w in edges:
            f.write(f"{u} {v} {w}\n")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("argv[1] = the number of vertex.")
        sys.exit(1)
    
    n = int(sys.argv[1])
    generate_complete_graph(n, "./testing/input.txt")
