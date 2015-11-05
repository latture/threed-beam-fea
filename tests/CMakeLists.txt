if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # force this option to ON so that Google Test will use /MD instead of /MT
    # /MD is now the default for Visual Studio, so it should be our default, too
    option(gtest_force_shared_crt
           "Use shared (DLL) run-time lib even when Google Test is built as static lib."
           ON)
endif()

add_subdirectory(gtest-1.7.0)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(runFEAUnitTests beam_element_tests.cpp)
target_link_libraries(runFEAUnitTests threed_beam_fea boostlib gtest gtest_main)

add_test(runUnitTests runFEAUnitTests)

add_executable(runCSVParserUnitTests csv_parser_tests.cpp)
target_link_libraries(runCSVParserUnitTests threed_beam_fea boostlib gtest gtest_main)

add_test(runUnitTests runCSVParserUnitTests)

add_executable(runSetupUnitTests setup_tests.cpp)
target_link_libraries(runSetupUnitTests threed_beam_fea boostlib gtest gtest_main)

add_test(runUnitTests runSetupUnitTests)
