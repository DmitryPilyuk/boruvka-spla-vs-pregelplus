import sys
import argparse
from collections import defaultdict


def convert_dimacs_to_adjlist(input_filename, output_filename):
    """
    Reads a 9th DIMACS shortest-path format graph file (.gr) and
    converts it to a 'vertex_id \t num_neighbours v1 d1 v2 d2 ...'
    adjacency list format.

    Args:
       input_filename (str): Path to the DIMACS .gr input file.
       output_filename (str): Path for the resulting output file.

    Raises:
        FileNotFoundError: If input_file cannot be opened.
        ValueError: If a line format is severely malformed.
        RuntimeError: If the 'p sp' line is missing.
    """

    graph = defaultdict(list)
    node_count = 0

    print(f"--> Reading {input_filename}...")

    try:
        with open(input_filename, "r") as fin:
            for line in fin:
                line = line.strip()
                if not line:
                    continue  # skip blank lines

                parts = line.split()
                line_type = parts[0]

                if line_type == "c":
                    # ignore comment
                    continue
                elif line_type == "a":
                    # Arc definition: a U V W
                    try:
                        u = int(parts[1])
                        v = int(parts[2])
                        w = int(parts[3])
                        graph[u].append((v, w))
                    except (ValueError, IndexError) as e:
                        print(
                            f"WARNING: Skipping malformed arc line: '{line}' - {e}",
                            file=sys.stderr,
                        )
                elif line_type == "p":
                    # Problem definition: p sp NODES ARCS
                    try:
                        node_count = int(parts[2])
                    except (ValueError, IndexError) as e:
                        print(
                            f"ERROR: Could not parse node count from 'p' line: '{line}' - {e}",
                            file=sys.stderr,
                        )
                        raise ValueError(f"Invalid 'p' line: {line}")
                # else:
                # print(f"INFO: Ignoring unknown line type '{line_type}'", file=sys.stderr)

    except FileNotFoundError:
        print(f"ERROR: Input file not found: {input_filename}", file=sys.stderr)
        raise

    if node_count <= 0:
        raise RuntimeError(
            f"Could not find valid 'p sp N M' line defining node count in {input_filename}"
        )

    print(f"    Graph has {node_count} nodes declared.")
    print(f"--> Writing to {output_filename}...")

    # --- Write Output ---
    lines_written = 0
    with open(output_filename, "w") as fout:
        # Ensure we output a line for ALL nodes from 1 to node_count
        for i in range(1, node_count + 1):

            neighbours = graph.get(
                i, []
            )  # Use .get() to return [] if node i has no outgoing
            num_neighbours = len(neighbours)

            if num_neighbours > 0:
                # Create "v1 d1 v2 d2 ..." string
                neighbours_str = " ".join([f"{v} {w}" for v, w in neighbours])
                fout.write(f"{i}\t{num_neighbours} {neighbours_str}\n")
            else:
                # Node with 0 outgoing edges
                fout.write(f"{i}\t0\n")
            lines_written += 1

    print(f"--> Done. Wrote {lines_written} lines.")


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description="Convert 9th DIMACS SP graph format to 'ID \\t COUNT N1 D1 N2 D2...' format.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("input_file", help="Path to the input DIMACS .gr file")

    args = parser.parse_args()

    try:
        convert_dimacs_to_adjlist(args.input_file, args.input_file[:-2]+"txt")
        sys.exit(0)

    except Exception as e:
        print(f"\nSCRIPT FAILED: {e}", file=sys.stderr)
        sys.exit(1)