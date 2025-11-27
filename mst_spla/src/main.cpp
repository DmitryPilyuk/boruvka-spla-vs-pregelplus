#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <spla.hpp>
#include <string>

#include "mst.hpp"


int main(int argc, char** argv) {

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << "  < 0 | 1 > <path_to_graph> n_iters\n";
        return 1;
    }

    std::string acc_info;

    int platform = atoi(argv[1]);
    int n_iters  = atoi(argv[3]);

    spla::Library* library = spla::Library::get();
    library->set_accelerator(spla::AcceleratorType::OpenCL);
    library->set_platform(platform);
    library->set_device(0);
    library->set_queues_count(1);
    library->get_accelerator_info(acc_info);

    std::cout << "env: " << acc_info << std::endl;

    std::string file_path = argv[2];
    std::string CL_back;
    if (platform == 0) {
        CL_back = "Nvidia";
    } else if (platform == 1) {
        CL_back = "Pocl";
    }

    std::string out_filename = std::string(file_path) + "." + CL_back + ".time.txt";

    spla::ref_ptr<spla::Matrix> A = load_gr(file_path);


    uint N = A->get_n_cols();

    spla::ref_ptr<spla::Matrix> sp_tree;
    for (int i = 0; i < 2; i++) {
        sp_tree = spla::Matrix::make(N, N, spla::UINT);
        mst(sp_tree, A);
    }

    // Truncate file to store new measurements
    {
        std::ofstream out(out_filename);
    }


    for (int i = 0; i < n_iters; i++) {
        sp_tree = spla::Matrix::make(N, N, spla::UINT);

        auto start = std::chrono::high_resolution_clock::now();
        mst(sp_tree, A);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::ofstream out(out_filename, std::ios_base::app);
        out << duration.count() << std::endl;
    }


    //std::cout << "MST total weight: " << calculate_tree_weight(sp_tree) << "\n";

    // print_matrix(A, "A");
    // print_matrix(sp_tree, "sp_tree");


    return 0;
}