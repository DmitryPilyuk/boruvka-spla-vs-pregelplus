#include <spla.hpp>

spla::ref_ptr<spla::Matrix> load_mtx(const std::string& path, uint& nnz);

void print_matrix(const spla::ref_ptr<spla::Matrix>& matrix,
                  const std::string&                 title);

void print_vector(const spla::ref_ptr<spla::Vector>& vector,
                  const std::string&                 title);

spla::Status mst(spla::ref_ptr<spla::Matrix>&       spanning_tree,
                 const spla::ref_ptr<spla::Matrix>& A, uint nnz);
