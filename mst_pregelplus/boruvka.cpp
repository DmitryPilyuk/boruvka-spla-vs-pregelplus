#include "apps/basic/msf/pregel_app_mst.h"

int main(int argc, char* argv[]) {
    init_workers();
    pregel_mst("/input", "/output");
    worker_finalize();
    return 0;
}