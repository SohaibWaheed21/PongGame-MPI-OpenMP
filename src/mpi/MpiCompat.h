#pragma once

#if __has_include(<mpi.h>)
#include <mpi.h>
#define PINGPONG_HAS_MPI 1
#else
#include <cstddef>

#define PINGPONG_HAS_MPI 0

using MPI_Comm = int;
using MPI_Datatype = int;
using MPI_Status = int;

constexpr MPI_Comm MPI_COMM_WORLD = 0;
constexpr MPI_Datatype MPI_BYTE = 0;
#define MPI_STATUS_IGNORE nullptr

inline int MPI_Init(int*, char***) { return 0; }
inline int MPI_Finalize() { return 0; }
inline int MPI_Comm_rank(MPI_Comm, int* rank) {
    if (rank) *rank = 0;
    return 0;
}
inline int MPI_Comm_size(MPI_Comm, int* size) {
    if (size) *size = 1;
    return 0;
}
inline int MPI_Send(const void*, int, MPI_Datatype, int, int, MPI_Comm) { return 0; }
inline int MPI_Recv(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status*) { return 0; }

#endif
