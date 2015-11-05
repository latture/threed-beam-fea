// Copyright 2015. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Author: ryan.latture@gmail.com (Ryan Latture)

#include <gtest/gtest.h>
#include <csv_parser.h>

using namespace fea;

TEST(CSVParserTest, ReadWriteElemsFromFile) {

    CSVParser csv;

    std::vector<std::vector<unsigned int> > expected = {{1, 2},
                                                        {3, 4}};

    std::string filename = "correct_elems_from_file.csv";
    csv.write(filename, expected, 0, ",");

    std::vector<std::vector<unsigned int> > elems;
    csv.parseToVector(filename, elems);
    for (size_t i = 0; i < elems.size(); ++i) {
        for (size_t j = 0; j < elems[i].size(); ++j) {
            EXPECT_EQ(expected[i][j], elems[i][j]);
        }
    }

    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << filename << ".\n";
    }
}

TEST(CSVParserTest, ReadWriteNodesFromFile) {

    CSVParser csv;

    std::vector<std::vector<double> > expected = {{1, 2},
                                                  {3, 4}};

    std::string filename = "correct_nodes_from_file.csv";
    csv.write(filename, expected, 1, ",");

    std::vector<std::vector<unsigned int> > nodes;
    csv.parseToVector(filename, nodes);
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = 0; j < nodes[i].size(); ++j) {
            EXPECT_EQ(expected[i][j], nodes[i][j]);
        }
    }

    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << filename << ".\n";
    }
}

TEST(CSVParserTest, ReadWriteElemSetFromFile) {

    CSVParser csv;

    std::vector<std::vector<unsigned int> > expected = {{1},
                                                        {2},
                                                        {3},
                                                        {4}};

    std::string filename = "correct_elem_set_from_file.csv";
    csv.write(filename, expected, 0, ",");

    std::vector<std::vector<unsigned int> > nodes;
    csv.parseToVector(filename, nodes);
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = 0; j < nodes[i].size(); ++j) {
            EXPECT_EQ(expected[i][j], nodes[i][j]);
        }
    }

    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << filename << ".\n";
    }
}
