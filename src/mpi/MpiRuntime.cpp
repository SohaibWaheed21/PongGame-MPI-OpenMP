#include "MpiRuntime.h"

int MpiRuntime::s_rank = 0;
int MpiRuntime::s_size = 1;

void MpiRuntime::init(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &s_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &s_size);
}

void MpiRuntime::finalize() {
    MPI_Finalize();
}

int MpiRuntime::getRank() {
    return s_rank;
}

int MpiRuntime::getSize() {
    return s_size;
}
