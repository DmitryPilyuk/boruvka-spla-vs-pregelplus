#include <cstdint>
#include <fstream>
#include <iostream>
#include <spla.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>

enum Field { INTEGER,
             PATTERN };

enum Symmetry { SYMMETRIC,
                GENERAL };

struct edge {
    uint src;
    uint dst;
    int  w;
};

spla::ref_ptr<spla::Matrix> load_mtx(const std::string& path, uint& nnz) {

    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Failed to open " + path);
    }

    Field    field    = PATTERN;
    Symmetry symmetry = GENERAL;
    nnz               = 0;
    uint        ncols = 0, nrows = 0;
    bool        is_header_parsed = false;
    bool        is_size_parsed   = false;
    std::string line;

    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '%') {
            if (!is_header_parsed && line.starts_with("%%MatrixMarket")) {
                std::istringstream iss(line);
                std::string        word;

                iss >> word;// %%MatrixMarket skip
                iss >> word;// matrix skip

                iss >> word;// format
                if (!(word == "coordinate")) {
                    throw std::runtime_error("Only support coordinate type");
                }

                iss >> word;// field
                if (word == "pattern") {
                    field = PATTERN;
                } else if (word == "integer") {
                    field = INTEGER;
                } else {
                    throw std::runtime_error("Unsupported field type: " + word);
                }

                iss >> word;// symmetry
                if (word == "symmetric") {
                    symmetry = SYMMETRIC;
                } else if (word == "general") {
                    symmetry = GENERAL;
                } else {
                    throw std::runtime_error("Unsupported symmetry type: " + word);
                }
                is_header_parsed = true;
            }
            continue;
        }

        if (!is_size_parsed) {
            std::istringstream iss(line);
            iss >> nrows >> ncols >> nnz;
            is_size_parsed = true;
            break;
        }
    }

    if (!is_header_parsed) {
        throw std::runtime_error("Bad file: no %%MatrixMarket header");
    }

    if (!is_size_parsed) {
        throw std::runtime_error("Bad file: no size line");
    }

    if (ncols != nrows) {
        throw std::runtime_error("Wrong matrixx size.");
    }

    spla::ref_ptr<spla::Matrix> matrix =
            spla::Matrix::make(nrows, ncols, spla::INT);
    matrix->set_fill_value(spla::Scalar::make_int(INT32_MAX));

    for (uint i = 0; i < nnz; i++) {
        if (!std::getline(f, line)) {
            throw std::runtime_error("Bad file: not enough data lines.");
        }

        if (line.empty() || line[0] == '%') {
            i--;
            continue;
        }

        uint         src, dst;
        std::int32_t weight = 1;

        std::istringstream iss(line);

        iss >> src >> dst;

        if (field == INTEGER) {
            iss >> weight;
        }

        matrix->set_int(src - 1, dst - 1, weight);
        if (symmetry == SYMMETRIC) {
            matrix->set_int(dst - 1, src - 1, weight);
        }
    }

    if (symmetry == SYMMETRIC) {
        nnz *= 2;
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

spla::Status mst(spla::ref_ptr<spla::Matrix>& spanning_tree, const spla::ref_ptr<spla::Matrix>& A, uint nnz) {

    uint n = A->get_n_rows();
    if (n != A->get_n_cols() || spanning_tree->get_n_cols() != n ||
        spanning_tree->get_n_rows() != n) {
        return spla::Status::InvalidArgument;
    }

    auto desc = spla::Descriptor::make();
    auto inf  = spla::Scalar::make_int(INT32_MAX);
    auto S    = spla::Matrix::make(n, n, spla::INT);
    spla::exec_m_eadd(S, A, A, spla::FIRST_INT, desc);
    auto S_new    = spla::Matrix::make(n, n, spla::INT);
    auto edge_w   = spla::Vector::make(n, spla::INT);
    auto edge_dst = spla::Vector::make(n, spla::UINT);
    // auto              parent   = spla::Vector::make(n, spla::UINT);
    std::vector<uint> parent_(n);
    for (uint i = 0; i < n; i++) {
        parent_[i] = i;
    }


    while (nnz > 0) {

        spla::exec_m_reduce_by_row(edge_w, S, spla::MIN_INT, inf);

        spla::ref_ptr<spla::MemView> rows_view, cols_view, values_view;
        S->read(rows_view, cols_view, values_view);
        uint* rows   = (uint*) rows_view->get_buffer();
        uint* cols   = (uint*) cols_view->get_buffer();
        int*  values = (int*) values_view->get_buffer();

        for (uint i = 0; i < nnz; i++) {
            int w;
            edge_w->get_int(rows[i], w);
            if (w == values[i]) {
                edge_dst->set_uint(rows[i], cols[i]);
            }
        }

        std::vector<edge> cedge(n, {0, 0, INT32_MAX});

        for (uint i = 0; i < n; i++) {
            uint comp = parent_[i];
            int  w;
            edge_w->get_int(i, w);
            if (w < cedge[comp].w) {
                uint dst;
                edge_dst->get_uint(i, dst);
                cedge[comp].src = i;
                cedge[comp].dst = dst;
                cedge[comp].w   = w;
            }
        }

        for (uint i = 0; i < n; i++) {
            auto edge = cedge[i];
            if (edge.w != INT32_MAX) {
                spanning_tree->set_int(edge.src, edge.dst, edge.w);
                parent_[i] = edge.dst;
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

        uint new_nnz = 0;
        S_new->set_fill_value(inf);

        for (uint i = 0; i < nnz; i++) {
            if (parent_[rows[i]] != parent_[cols[i]]) {
                S_new->set_int(rows[i], cols[i], values[i]);
                new_nnz++;
            }
        }
        std::swap(S, S_new);
        S_new->clear();
        nnz = new_nnz;
    }

    return spla::Status::Ok;
}