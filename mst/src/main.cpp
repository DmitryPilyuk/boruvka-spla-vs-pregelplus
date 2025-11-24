#include <cstdlib>
#include <iostream>
#include <spla.hpp>
#include <string>

#include "mst.hpp"


int main(int argc, char** argv) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << "  < 0 | 1 > <path_to_graph> \n";
        return 1;
    }

    std::string acc_info;

    spla::Library* library = spla::Library::get();
    library->set_accelerator(spla::AcceleratorType::OpenCL);
    library->set_platform(atoi(argv[1]));
    library->set_device(0);
    library->set_queues_count(1);
    library->get_accelerator_info(acc_info);

    std::cout << "env: " << acc_info << std::endl;


    uint        nnz;
    std::string file_path = argv[2];

    spla::ref_ptr<spla::Matrix> A;


    if (file_path.ends_with(".mtx")) {
        A = load_mtx(file_path, nnz);
    } else if (file_path.ends_with(".gr")) {
        A = load_gr(file_path, nnz);
    } else {
        std::cerr << "Unsupported file format: " << file_path << std::endl;
        return 1;
    }

    uint N       = A->get_n_cols();
    auto sp_tree = spla::Matrix::make(N, N, spla::INT);

    mst(sp_tree, A, nnz);// Uncommment mst call

    std::cout << "MST total weight: " << calculate_tree_weight(A) << "\n";

    std::cout << "MST total weight: " << calculate_tree_weight(sp_tree) << "\n";

    // print_matrix(A, "A");
    // print_matrix(sp_tree, "sp_tree");


    return 0;
}