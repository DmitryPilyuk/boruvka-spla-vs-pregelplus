#include "spla/op.hpp"
#include "spla/scalar.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <spla.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>


struct Edge {
    uint src;
    uint dst;
    uint w;
};

spla::ref_ptr<spla::Matrix> load_gr(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Failed to open " + path);
    }

    uint nnz            = 0;
    uint nodes          = 0;
    bool is_size_parsed = false;

    std::string line;

    while (std::getline(f, line)) {
        if (line.empty() || line[0] == 'c') {
            continue;
        }

        if (line[0] == 'p') {
            std::istringstream iss(line);
            char               p, sp[3];
            iss >> p >> sp >> nodes >> nnz;
            is_size_parsed = true;
            break;
        }
    }

    if (!is_size_parsed) {
        throw std::runtime_error("Bad file: no size line");
    }

    spla::ref_ptr<spla::Matrix> matrix =
            spla::Matrix::make(nodes, nodes, spla::UINT);
    matrix->set_fill_value(spla::Scalar::make_uint(UINT32_MAX));


    for (uint i = 0; i < nnz; i++) {
        if (!std::getline(f, line)) {
            throw std::runtime_error("Bad file: not enough data lines.");
        }

        if (line.empty() || line[0] == 'c') {
            i--;
            continue;
        }

        if (line[0] == 'a') {
            uint src, dst;
            uint weight = 1;

            std::istringstream iss(line);
            char               a;
            iss >> a >> src >> dst >> weight;

            matrix->set_uint(src - 1, dst - 1, weight);
        }
    }

    return matrix;
}


void print_matrix(const spla::ref_ptr<spla::Matrix>& matrix,
                  const std::string&                 title) {
    if (!title.empty()) {
        std::cout << title << std::endl;
    }

    uint n_rows = matrix->get_n_rows();
    uint n_cols = matrix->get_n_cols();

    // Determine the maximum width needed for printing values
    int max_width = 1;// At least one digit for 0
    for (uint i = 0; i < n_rows; ++i) {
        for (uint j = 0; j < n_cols; ++j) {
            std::int32_t value;
            if (matrix->get_int(i, j, value) == spla::Status::Ok) {
                max_width = std::max(max_width, (int) std::to_string(value).length());
            }
        }
    }

    for (uint i = 0; i < n_rows; ++i) {
        for (uint j = 0; j < n_cols; ++j) {
            std::int32_t value;
            if (matrix->get_int(i, j, value) == spla::Status::Ok) {
                std::cout << std::setw(max_width + 1) << value;
            } else {
                std::cout << std::setw(max_width + 1) << 0;
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void print_vector(const spla::ref_ptr<spla::Vector>& vector, const std::string& title) {
    if (!title.empty()) {
        std::cout << title << std::endl;
    }

    uint n_rows = vector->get_n_rows();

    int max_width = 1;
    for (uint i = 0; i < n_rows; ++i) {
        std::int32_t value;
        if (vector->get_int(i, value) == spla::Status::Ok) {
            max_width = std::max(max_width, (int) std::to_string(value).length());
        }
    }

    for (uint i = 0; i < n_rows; ++i) {
        std::int32_t value;
        if (vector->get_int(i, value) == spla::Status::Ok) {
            std::cout << std::setw(max_width + 1) << value << std::endl;
        } else {
            std::cout << std::setw(max_width + 1) << 0 << std::endl;
        }
    }
    std::cout << std::endl;
}

spla::Status mst(spla::ref_ptr<spla::Matrix>& spanning_tree, const spla::ref_ptr<spla::Matrix>& A) {

    uint n = A->get_n_rows();
    if (n != A->get_n_cols() || spanning_tree->get_n_cols() != n ||
        spanning_tree->get_n_rows() != n) {
        return spla::Status::InvalidArgument;
    }

    bool S_empty = false;
    auto desc    = spla::Descriptor::make();
    auto inf     = spla::Scalar::make_uint(UINT32_MAX);
    auto S       = spla::Matrix::make(n, n, spla::UINT);

    auto S_new = spla::Matrix::make(n, n, spla::UINT);
    ;
    auto edge_w = spla::Vector::make(n, spla::UINT);
    S->set_fill_value(inf);
    spla::exec_m_eadd(S, A, A, spla::FIRST_UINT, desc);
    S_new->set_fill_value(inf);

    std::vector<Edge> cedge(n, {UINT32_MAX, UINT32_MAX, UINT32_MAX});
    std::vector<uint> parent_(n);
    std::vector<uint> edge_dst(n);
    for (uint i = 0; i < n; i++) {
        parent_[i] = i;
    }

    while (!S_empty) {
        S_new->clear();

        spla::exec_m_reduce_by_row(edge_w, S, spla::MIN_UINT, inf);

        spla::ref_ptr<spla::MemView> rows_view, cols_view, values_view;
        S->read(rows_view, cols_view, values_view);
        uint* rows   = (uint*) rows_view->get_buffer();
        uint* cols   = (uint*) cols_view->get_buffer();
        uint* values = (uint*) values_view->get_buffer();

        uint nnz = values_view->get_size() / sizeof(uint);
        std::fill(edge_dst.begin(), edge_dst.end(), UINT32_MAX);
        for (uint i = 0; i < nnz; i++) {
            uint w;

            edge_w->get_uint(rows[i], w);
            if (w == values[i] && edge_dst[rows[i]] == UINT32_MAX) {
                edge_dst[rows[i]] = cols[i];
            }
        }

        std::fill(cedge.begin(), cedge.end(), Edge(UINT32_MAX, UINT32_MAX, UINT32_MAX));

        for (uint i = 0; i < n; i++) {
            uint comp = parent_[i];
            uint w;
            edge_w->get_uint(i, w);
            if (w < cedge[comp].w) {
                uint dst        = edge_dst[i];
                cedge[comp].src = i;
                cedge[comp].dst = dst;
                cedge[comp].w   = w;
            }
        }

        for (uint i = 0; i < n; i++) {
            auto edge = cedge[i];
            if (edge.w != UINT32_MAX && parent_[edge.dst] != i) {
                spanning_tree->set_uint(edge.src, edge.dst, edge.w);
                spanning_tree->set_uint(edge.dst, edge.src, edge.w);
                parent_[i] = parent_[edge.dst];
            }
        }

        for (uint i = 0; i < n; i++) {
            if (parent_[parent_[i]] == i) parent_[i] = i;
        }

        bool changed = true;
        while (changed) {
            changed = false;
            for (uint i = 0; i < n; i++) {
                if (parent_[parent_[i]] != parent_[i]) {
                    parent_[i] = parent_[parent_[i]];
                    changed    = true;
                }
            }
        }

        S_empty = true;
        for (uint i = 0; i < nnz; i++) {
            if (parent_[rows[i]] != parent_[cols[i]]) {
                S_new->set_int(rows[i], cols[i], values[i]);
                S_empty = false;
            }
        }
        std::swap(S, S_new);
    }

    return spla::Status::Ok;
}

unsigned long calculate_tree_weight(const spla::ref_ptr<spla::Matrix>& tree) {
    spla::ref_ptr<spla::MemView> rows_view, cols_view, values_view;
    tree->read(rows_view, cols_view, values_view);

    unsigned long total_weight = 0;
    int*          values       = (int*) values_view->get_buffer();
    uint          nnz          = values_view->get_size() / sizeof(int);

    for (uint i = 0; i < nnz; ++i) {
        total_weight += values[i];
    }

    // Since each edge is added twice (src,dst) and (dst,src),
    // we need to divide the total weight by 2.
    return total_weight / 2;
}