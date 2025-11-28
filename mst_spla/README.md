# MST (Minimum Spanning Tree) using SPLA

This directory contains the implementation of a Minimum Spanning Tree algorithm using the SPLA (Sparse Parallel Linear Algebra) library.

## How to Compile

To compile the project, simply run the `make` command in this directory:

```bash
make
```

This will create an executable file at `build/mst`.

## How to Run

The program can be executed with the following command:

```bash
./build/mst <platform> <path_to_graph> <n_iters>
```

- `<platform>`: An integer (`0` or `1`) to select the OpenCL platform.
- `<path_to_graph>`: The path to the input graph file.
- `<n_iters>`: The number of iterations to run the MST algorithm for performance measurement.

Example:
```bash
./build/mst 0 ../data/USA-road-d.W.gr 10
```

## Data Format

The input graph file should be in a format similar to DIMACS. The file should contain:
- Comment lines starting with `c`.
- A problem line: `p sp <num_nodes> <num_edges>`
- Arc descriptor lines: `a <source> <destination> <weight>`

Node numbers are 1-based.

Example:
```
c This is a comment.
p sp 4 5
a 1 2 10
a 1 3 6
a 1 4 5
a 2 4 15
a 3 4 4
```

## Output Format

The program will create a file named `<path_to_graph>.<CL_back>.time.txt`, where `<CL_back>` is either `Nvidia` or `Pocl` depending on the selected platform. This file will contain the execution time in milliseconds for each iteration of the MST algorithm, with each measurement on a new line.

Example output in `../data/USA-road-d.W.gr.Nvidia.time.txt`:
```
123
121
125
...
```