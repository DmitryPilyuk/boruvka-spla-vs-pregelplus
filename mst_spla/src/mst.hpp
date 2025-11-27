#include <spla.hpp>

spla::ref_ptr<spla::Matrix> load_gr(const std::string& path);

void print_matrix(const spla::ref_ptr<spla::Matrix>& matrix,
                  const std::string&                 title);

void print_vector(const spla::ref_ptr<spla::Vector>& vector,
                  const std::string&                 title);

spla::Status mst(spla::ref_ptr<spla::Matrix>& spanning_tree, const spla::ref_ptr<spla::Matrix>& A);

unsigned long calculate_tree_weight(const spla::ref_ptr<spla::Matrix>& tree);
