#include "Game.h"
#include "src/mpi/MpiRuntime.h"
#include "src/mpi/MasterLoop.h"
#include "src/mpi/WorkerLoop.h"
#include <iostream>

int main(int argc, char** argv) {
    try {
        // Initialize MPI
        MpiRuntime::init(argc, argv);

        int rank = MpiRuntime::getRank();
        int size = MpiRuntime::getSize();

        if (size < 2) {
            // Fallback: run sequential Game if MPI not configured
            if (rank == 0) {
                std::cout << "Running in sequential mode (single process)\n";
                Game game;
                game.run();
            }
        }
        else {
            // MPI mode: branch based on rank
            if (MpiRuntime::isMaster()) {
                std::cout << "Rank 0: Running Master Loop (Render + Authority)\n";
                MasterLoop master;
                master.run();
            }
            else if (MpiRuntime::isWorker()) {
                std::cout << "Rank 1: Running Worker Loop (Physics Simulation)\n";
                WorkerLoop worker;
                worker.run();
            }
            else {
                std::cerr << "Rank " << rank << ": Idle (only rank 0 and 1 are used)\n";
            }
        }

        // Finalize MPI
        MpiRuntime::finalize();
    }
    catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}