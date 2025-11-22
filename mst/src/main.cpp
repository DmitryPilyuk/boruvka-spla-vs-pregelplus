#include "mst.hpp"

#include <cstdlib>
#include <spla.hpp>
#include <string>
#include <iostream>

int main(int argc, char **argv) {

  std::string acc_info;

  spla::Library *library = spla::Library::get();
  library->set_accelerator(spla::AcceleratorType::OpenCL);
  library->set_platform(atoi(argv[1]));
  library->set_device(atoi(argv[2]));
  library->set_queues_count(1);
  library->get_accelerator_info(acc_info);

  std::cout << "env: " << acc_info << std::endl;
  return 0;
}