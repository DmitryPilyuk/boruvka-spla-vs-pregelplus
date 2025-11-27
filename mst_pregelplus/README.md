# Boruvka's Algorithm with PregelPlus

This directory contains implementations of Boruvka's algorithm for finding the Minimum Spanning Tree (MST) using the PregelPlus framework. Two versions are provided: a standard Boruvka implementation and a version utilizing PregelPlus's request-response mechanism (`boruvka_reqresp`).

## Prerequisites

To build and run these applications, you need to have the following installed and configured:

*   **Java Development Kit (JDK)**: Required by Hadoop. Ensure `JAVA_HOME` environment variable is set and points to your JDK installation directory.
    ```bash
    export JAVA_HOME=/path/to/your/jdk
    ```

*   **MPICH (or other MPI implementation)**: Used by `mpic++` for parallel compilation and `mpiexec` for running distributed applications.
    ```bash
    # Example for Ubuntu/Debian
    sudo apt-get install mpich libmpich-dev
    ```

*   **Hadoop**: Specifically, the Hadoop Distributed File System (HDFS) is used for input data. Ensure `HADOOP_HOME` environment variable is set and points to your Hadoop installation directory. The `libhdfs` library is required for linking.
    ```bash
    export HADOOP_HOME=/path/to/your/hadoop
    ```
    Make sure your Hadoop cluster is running and accessible.

For more details on deploying Hadoop for PregelPlus, refer to: [PregelPlus Hadoop Deployment Guide](https://www.cse.cuhk.edu.hk/pregelplus/deploy-hadoop2.html)


## Building

Navigate to the `mst_pregelplus` directory and run `make`:

```bash
cd mst_pregelplus
make
```

This will compile `boruvka.cpp` and `boruvka_reqresp.cpp` into executables named `boruvka.out` and `boruvka_reqresp.out` respectively.

## Running

The `run.sh` script is provided to facilitate running the Boruvka implementations with proper HDFS setup and MPI execution.

### `run.sh` Usage

```bash
./run.sh <input_file_path> <number_of_runs> [reqresp]
```

*   `<input_file_path>`: The path to your graph input file on the local filesystem. This file will be uploaded to HDFS `/input/`.
*   `<number_of_runs>`: The number of times to execute the Boruvka algorithm.
*   `[reqresp]`: An optional argument. If present, the `boruvka_reqresp.out` executable will be used. Otherwise, `boruvka.out` will be used.

### Environment Variables

*   `N_JOBS`: (Optional) Specifies the number of MPI processes (workers) to use. If not set, it defaults to `1`.
    ```bash
    export N_JOBS=4 # To run with 4 MPI processes
    ```

### Example

To run the standard Boruvka implementation on `../data/gr1.gr` twice with 4 MPI processes:

```bash
export N_JOBS=4
./run.sh ../data/gr1.gr 2
```

To run the request-response version on `../data/gr1.gr` once with 8 MPI processes:

```bash
export N_JOBS=8
./run.sh ../data/gr1.gr 1 reqresp
```

The script will:
1.  Clear and recreate the `/input/` directory in HDFS.
2.  Upload your specified `<input_file_path>` to `/input/`.
3.  Execute the chosen Boruvka executable using `mpiexec` for the specified number of runs.
4.  Save the output of each run to log files in a `logs_<input_filename>` directory.

---
**Note:** Ensure your HDFS is properly configured and running, and that you have the necessary permissions to write to HDFS.