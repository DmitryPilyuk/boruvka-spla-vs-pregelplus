#!/usr/bin/env python3
import sys
import os
import bisect

# -----------------------------
#   ARGUMENTS
# -----------------------------
if len(sys.argv) < 2:
    print("Usage: python3 compress_gr.py <input.gr> ")
    sys.exit(1)

INPUT = sys.argv[1]

if not os.path.exists(INPUT):
    print(f"Error: file not found: {INPUT}")
    sys.exit(1)

# number of quantization bins (default 128)

OUTPUT = os.path.splitext(INPUT)[0] + f".clean.gr"

print(f"Input file:       {INPUT}")
print(f"Output file:      {OUTPUT}")
print("------------------------------------------------------------")


print("Reading graph and merging multi-edges...")

# key: (u_min, u_max) ; value: w
edges = {}

n_nodes = 0

with open(INPUT) as f:
    for line in f:
        if line.startswith("p"):
            n_nodes = int(line.split()[2])
            continue
        if not line.startswith("a "):
            continue

        _, u, v, w = line.split()
        u = int(u)
        v = int(v)
        w = int(w)

        # undirected normalization
        a, b = (u, v) if u < v else (v, u)
        key = (a, b)

        if key not in edges:
            edges[key] = w
        else:
            # always take MIN
            if w < edges[key]:
                edges[key] = w

print(f"Unique undirected edges: {len(edges)}")


print("Writing compressed symmetric graph...")

with open(INPUT) as fin, open(OUTPUT, "w") as fout:
    # copy header until first edge
    for line in fin:
        if line.startswith("a "):
            break
    fout.write(f"p sp {n_nodes} {len(edges)*2}\n")

    # write merged, symmetric edges with quantized weights
    for (u, v), w in edges.items():
        fout.write(f"a {u} {v} {w}\n")
        fout.write(f"a {v} {u} {w}\n")  # symmetric

print("Done.")