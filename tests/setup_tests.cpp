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
#include "csv_parser.h"
#include "setup.h"

using namespace fea;

namespace {
    void writeStringToTxt(std::string filename, std::string data) {
        std::ofstream output_file;
        output_file.open(filename);

        if (!output_file.is_open()) {
            std::cerr << "Error opening file" << filename << ".\n";
        }
        else {
            output_file << data;
            output_file.close();
        }
    }
}

TEST(SetupTest, CreatesCorrectConfigFromJSON) {
    std::string json = "{\"nodes\":\"nodes_file\"}\n";
    std::string filename = "CreatesCorrectConfig.json";
    writeStringToTxt(filename, json);

    rapidjson::Document doc = parseJSONConfig(filename);

    EXPECT_TRUE(doc.HasMember("nodes"));
    std::string nodes_file(doc["nodes"].GetString());
    EXPECT_EQ("nodes_file", nodes_file);

    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << filename << ".\n";
    }
}

TEST(SetupTest, CreatesCorrectNodesFromJSON) {
    std::string nodes_file = "CreatesCorrectNodes.csv";
    std::string json = "{\"nodes\":\"" + nodes_file + "\"}\n";
    std::string filename = "CreatesCorrectNodes.json";
    writeStringToTxt(filename, json);

    rapidjson::Document doc = parseJSONConfig(filename);

    std::vector<std::vector<double> > expected = {{1, 2, 3},
                                                  {4, 5, 6}};

    CSVParser csv;
    csv.write(nodes_file, expected, 1, ",");

    std::vector<Node> nodes = createNodeVecFromJSON(doc);

    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = 0; j < nodes[i].size(); ++j) {
            EXPECT_EQ(expected[i][j], nodes[i][j]);
        }
    }

    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << filename << ".\n";
    }
    if (std::remove(nodes_file.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << nodes_file << ".\n";
    }
}

TEST(SetupTest, CreatesCorrectElemsFromJSON) {
    std::string elems_file = "CreatesCorrectElems_elems.csv";
    std::string props_file = "CreatesCorrectElems_props.csv";
    std::string json = "{\"elems\":\"" + elems_file + "\",\"props\":\"" + props_file + "\"}\n";
    std::string filename = "CreatesCorrectElems.json";
    writeStringToTxt(filename, json);

    rapidjson::Document doc = parseJSONConfig(filename);

    std::vector<std::vector<unsigned int> > expected_elems = {{1, 2},
                                                              {2, 3}};

    std::vector<std::vector<double> > expected_props = {{1, 2, 3, 4, 5, 6, 7},
                                                        {8, 9, 10, 11, 12, 13, 14}};

    CSVParser csv;
    csv.write(elems_file, expected_elems, 0, ",");
    csv.write(props_file, expected_props, 1, ",");

    std::vector<Elem> elems = createElemVecFromJSON(doc);

    for (size_t i = 0; i < expected_elems.size(); ++i) {
        for (size_t j = 0; j < expected_elems[i].size(); ++j)
        {
            EXPECT_EQ(expected_elems[i][j], elems[i].node_numbers[j]);
        }
    }

    for (size_t i = 0; i < expected_props.size(); ++i) {
        EXPECT_DOUBLE_EQ(expected_props[i][0], elems[i].props.EA);
        EXPECT_DOUBLE_EQ(expected_props[i][1], elems[i].props.EIz);
        EXPECT_DOUBLE_EQ(expected_props[i][2], elems[i].props.EIy);
        EXPECT_DOUBLE_EQ(expected_props[i][3], elems[i].props.GJ);
        EXPECT_DOUBLE_EQ(expected_props[i][4], elems[i].props.normal_vec[0]);
        EXPECT_DOUBLE_EQ(expected_props[i][5], elems[i].props.normal_vec[1]);
        EXPECT_DOUBLE_EQ(expected_props[i][6], elems[i].props.normal_vec[2]);
    }

    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << filename << ".\n";
    }
    if (std::remove(elems_file.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << elems_file << ".\n";
    }
    if (std::remove(props_file.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << props_file << ".\n";
    }
}

TEST(SetupTest, CreatesCorrectBCsFromJSON) {
    std::string bcs_file = "CreatesCorrectBCs.csv";
    std::string json = "{\"bcs\":\"" + bcs_file + "\"}\n";
    std::string filename = "CreatesCorrectBCs.json";
    writeStringToTxt(filename, json);

    rapidjson::Document doc = parseJSONConfig(filename);

    std::vector<std::vector<double> > expected = {{10, 20, 30},
                                                  {40, 50, 60}};

    CSVParser csv;
    csv.write(bcs_file, expected, 1, ",");

    std::vector<BC> bcs = createBCVecFromJSON(doc);

    for (size_t i = 0; i < bcs.size(); ++i) {
        EXPECT_EQ((unsigned int) expected[i][0], bcs[i].node);
        EXPECT_EQ((unsigned int) expected[i][1], bcs[i].dof);
        EXPECT_DOUBLE_EQ(expected[i][2], bcs[i].value);
    }

    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << filename << ".\n";
    }
    if (std::remove(bcs_file.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << bcs_file << ".\n";
    }
}

TEST(SetupTest, CreatesCorrectForcesFromJSON) {
    std::string forces_file = "CreatesCorrectForces.csv";
    std::string json = "{\"forces\":\"" + forces_file + "\"}\n";
    std::string filename = "CreatesCorrectForces.json";
    writeStringToTxt(filename, json);

    rapidjson::Document doc = parseJSONConfig(filename);

    std::vector<std::vector<double> > expected = {{10, 20, 30},
                                                  {40, 50, 60}};

    CSVParser csv;
    csv.write(forces_file, expected, 1, ",");

    std::vector<Force> forces = createForceVecFromJSON(doc);

    for (size_t i = 0; i < forces.size(); ++i) {
        EXPECT_EQ((unsigned int) expected[i][0], forces[i].node);
        EXPECT_EQ((unsigned int) expected[i][1], forces[i].dof);
        EXPECT_DOUBLE_EQ(expected[i][2], forces[i].value);
    }

    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << filename << ".\n";
    }
    if (std::remove(forces_file.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << forces_file << ".\n";
    }
}

TEST(SetupTest, CreatesCorrectTiesFromJSON) {
    std::string ties_file = "CreatesCorrectTies.csv";
    std::string json = "{\"ties\":\"" + ties_file + "\"}\n";
    std::string filename = "CreatesCorrectTies.json";
    writeStringToTxt(filename, json);

    rapidjson::Document doc = parseJSONConfig(filename);

    std::vector<std::vector<double> > expected = {{1, 2, 30, 40},
                                                  {5, 6, 70, 80}};

    CSVParser csv;
    csv.write(ties_file, expected, 1, ",");

    std::vector<Tie> ties = createTieVecFromJSON(doc);

    for (size_t i = 0; i < ties.size(); ++i) {
        EXPECT_EQ((unsigned int) expected[i][0], ties[i].node_number_1);
        EXPECT_EQ((unsigned int) expected[i][1], ties[i].node_number_2);
        EXPECT_DOUBLE_EQ(expected[i][2], ties[i].lmult);
        EXPECT_DOUBLE_EQ(expected[i][3], ties[i].rmult);
    }

    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << filename << ".\n";
    }
    if (std::remove(ties_file.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << ties_file << ".\n";
    }
}

TEST(SetupTest, CreatesCorrectJobFromJSON) {
    std::string elems_file = "CreatesCorrectJob_elems.csv";
    std::string props_file = "CreatesCorrectJob_props.csv";
    std::string nodes_file = "CreatesCorrectJob_nodes.csv";

    std::vector<std::vector<unsigned int> > expected_elems = {{1, 2},
                                                              {2, 3}};
    std::vector<std::vector<double> > expected_props = {{1, 2, 3, 4, 5, 6, 7},
                                                        {8, 9, 10, 11, 12, 13, 14}};
    std::vector<std::vector<double> > expected_nodes = {{1, 2, 3},
                                                        {4, 5, 6}};

    CSVParser csv;
    csv.write(elems_file, expected_elems, 0, ",");
    csv.write(props_file, expected_props, 1, ",");
    csv.write(nodes_file, expected_nodes, 1, ",");

    std::string json = "{\"elems\":\"" + elems_file + "\",\"props\":\"" + props_file + "\",\"nodes\":\"" + nodes_file + "\"}\n";
    std::string filename = "CreatesCorrectJob.json";
    writeStringToTxt(filename, json);

    rapidjson::Document doc = parseJSONConfig(filename);

    Job job = createJobFromJSON(doc);

    for (size_t i = 0; i < job.nodes.size(); ++i) {
        for (size_t j = 0; j < job.nodes[i].size(); ++j) {
            EXPECT_EQ(expected_nodes[i][j], job.nodes[i][j]);
        }
    }
    for (size_t i = 0; i < expected_elems.size(); ++i) {
        for (size_t j = 0; j < expected_elems[i].size(); ++j)
        {
            EXPECT_EQ(expected_elems[i][j], job.elems[i][j]);
        }
    }

    for (size_t i = 0; i < expected_props.size(); ++i) {
        EXPECT_DOUBLE_EQ(expected_props[i][0], job.props[i].EA);
        EXPECT_DOUBLE_EQ(expected_props[i][1], job.props[i].EIz);
        EXPECT_DOUBLE_EQ(expected_props[i][2], job.props[i].EIy);
        EXPECT_DOUBLE_EQ(expected_props[i][3], job.props[i].GJ);
        EXPECT_DOUBLE_EQ(expected_props[i][4], job.props[i].normal_vec[0]);
        EXPECT_DOUBLE_EQ(expected_props[i][5], job.props[i].normal_vec[1]);
        EXPECT_DOUBLE_EQ(expected_props[i][6], job.props[i].normal_vec[2]);
    }

    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << filename << ".\n";
    }
    if (std::remove(elems_file.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << elems_file << ".\n";
    }
    if (std::remove(props_file.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << props_file << ".\n";
    }
    if (std::remove(nodes_file.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << nodes_file << ".\n";
    }
}

TEST(SetupTest, CreatesCorrectOptionsFromJSON) {
    std::string json = "{\"options\":{\"epsilon\":1E-10,\"csv_precision\":10,\"csv_delimiter\":\" \","
            "\"save_nodal_displacements\":true,\"save_nodal_forces\":true,\"save_nodal_forces\":true,"
            "\"save_tie_forces\":true,\"verbose\":true,\"save_report\":true,"
            "\"nodal_displacements_filename\":\"ndf.csv\",\"nodal_forces_filename\":\"nff.csv\","
            "\"tie_forces_filename\":\"tff.csv\",\"report_filename\":\"rf.txt\"}}\n";
    std::string filename = "CreatesCorrectOptions.json";
    writeStringToTxt(filename, json);

    rapidjson::Document doc = parseJSONConfig(filename);

    Options expected;
    expected.epsilon = 1E-10;
    expected.csv_precision = 10;
    expected.csv_delimiter = " ";
    expected.save_nodal_displacements = true;
    expected.save_nodal_forces = true;
    expected.save_tie_forces = true;
    expected.save_report = true;
    expected.nodal_displacements_filename = "ndf.csv";
    expected.nodal_forces_filename = "nff.csv";
    expected.tie_forces_filename = "tff.csv";
    expected.report_filename = "rf.txt";

    Options options = createOptionsFromJSON(doc);

    EXPECT_DOUBLE_EQ(expected.epsilon, options.epsilon);
    EXPECT_EQ(expected.csv_precision, options.csv_precision);
    EXPECT_EQ(expected.csv_delimiter, options.csv_delimiter);
    EXPECT_EQ(expected.save_nodal_displacements, options.save_nodal_displacements);
    EXPECT_EQ(expected.save_nodal_forces, options.save_nodal_forces);
    EXPECT_EQ(expected.save_tie_forces, options.save_tie_forces);
    EXPECT_EQ(expected.save_report, options.save_report);
    EXPECT_EQ(expected.nodal_displacements_filename, options.nodal_displacements_filename);
    EXPECT_EQ(expected.nodal_forces_filename, options.nodal_forces_filename);
    EXPECT_EQ(expected.tie_forces_filename, options.tie_forces_filename);
    EXPECT_EQ(expected.report_filename, options.report_filename);

    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error removing test csv file " << filename << ".\n";
    }
}
