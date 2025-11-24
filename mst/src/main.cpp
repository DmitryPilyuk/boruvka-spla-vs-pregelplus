#include <cstdlib>
#include <iostream>
#include <spla.hpp>
#include <string>

#include "mst.hpp"

int main(int argc, char** argv) {

    std::string acc_info;

    spla::Library* library = spla::Library::get();
    library->set_accelerator(spla::AcceleratorType::OpenCL);
    library->set_platform(atoi(argv[1]));
    library->set_device(atoi(argv[2]));
    library->set_queues_count(1);
    library->get_accelerator_info(acc_info);

    std::cout << "env: " << acc_info << std::endl;


    uint nnz;
    auto A =
            load_mtx("/home/dmitry/Programming/graphs/boruvka-spla-vs-pregelplus/data/full.mtx",
                     nnz);
    uint N       = A->get_n_cols();
    auto sp_tree = spla::Matrix::make(N, N, spla::INT);

    mst(sp_tree, A, nnz);

    print_matrix(A, "A");
    print_matrix(sp_tree, "sp_tree");


    return 0;
}