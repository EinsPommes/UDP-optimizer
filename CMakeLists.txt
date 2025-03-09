cmake_minimum_required(VERSION 3.10)
project(UDPOptimizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    main.cpp
    network_optimizer.cpp
)

# Create executable
add_executable(UDPOptimizer WIN32 ${SOURCES})
