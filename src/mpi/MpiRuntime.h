#pragma once
#include "MpiCompat.h"

class MpiRuntime {
public:
    static void init(int argc, char** argv);
    static void finalize();
    
    static int getRank();
    static int getSize();
    
    static bool isMaster() { return getRank() == 0; }
    static bool isWorker() { return getRank() == 1; }
    
private:
    static int s_rank;
    static int s_size;
};
