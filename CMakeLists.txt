cmake_minimum_required(VERSION 2.8.11)
project(ThreedBeamFEA)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

option(FEA_BUILD_UNIT_TESTS "Build unit tests" ON)
option(FEA_BUILD_EXAMPLES "Build examples" ON)
option(FEA_BUILD_GUI "Build Qt GUI" OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -fopenmp")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(EXT_BOOST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ext/boost_1_59_0)
set(EXT_EIGEN_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ext/eigen-3.2.4)
set(EXT_RAPIDJSON_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ext/rapidjson/include)
set(EXT_TCLAP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ext/tclap-1.2.1/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${EXT_EIGEN_ROOT}
                    ${EXT_BOOST_ROOT}
                    ${EXT_RAPIDJSON_ROOT}
                    ${EXT_TCLAP_ROOT}
                    )

add_library(boostlib ${EXT_BOOST_ROOT}/libs/smart_ptr/src/sp_collector.cpp
                     ${EXT_BOOST_ROOT}/libs/smart_ptr/src/sp_debug_hooks.cpp)

add_subdirectory(src)

if (FEA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif(FEA_BUILD_EXAMPLES)

if(FEA_BUILD_UNIT_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif(FEA_BUILD_UNIT_TESTS)

if(FEA_BUILD_GUI)
    add_subdirectory(gui)
endif(FEA_BUILD_GUI)
