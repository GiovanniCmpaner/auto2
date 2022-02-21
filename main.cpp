#include <immintrin.h> 

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <cstring>

#include "Simulation/Simulation.hpp"

Simulation simulation{};

int main(int argc, char* args[])
{
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    
    simulation.init();
    
    return EXIT_SUCCESS;
}