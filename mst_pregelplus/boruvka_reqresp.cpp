#include "apps/reqresp/msf/req_app_mst.h"

int main(int argc, char* argv[]) {
    init_workers();
    req_mst("/input", "/output");
    worker_finalize();
    return 0;
}