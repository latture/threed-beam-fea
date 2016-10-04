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


#include "boost/format.hpp"
#include <exception>
#include "setup.h"

namespace fea {

    namespace {
        template<typename T>
        void createVectorFromJSON(const rapidjson::Document &config_doc,
                                  const std::string &variable,
                                  std::vector< std::vector<T> > &data) {
            if (!config_doc.HasMember(variable.c_str())) {
                throw std::runtime_error(
                        (boost::format("Configuration file does not have requested member variable %s.") %
                         variable).str()
                );
            }
            if (!config_doc[variable.c_str()].IsString()) {
                throw std::runtime_error(
                        (boost::format("Value associated with variable %s is not a string.") % variable).str()
                );
            }
            CSVParser csv;
            std::string filename(config_doc[variable.c_str()].GetString());
            csv.parseToVector(filename, data);
            if (data.size() == 0) {
                throw std::runtime_error(
                        (boost::format("No data was loaded for variable %s.") % variable).str()
                );
            }
        }
    }

    rapidjson::Document parseJSONConfig(const std::string &config_filename) {
        rapidjson::Document config_doc;

        FILE *config_file_ptr = fopen(config_filename.c_str(), "r");

        if (!config_file_ptr) {
            throw std::runtime_error(
                    (boost::format("Cannot open configuration input file %s.") % config_filename).str()
            );
        }
        char readBuffer[65536];
        rapidjson::FileReadStream config_stream(config_file_ptr, readBuffer, sizeof(readBuffer));
        config_doc.ParseStream(config_stream);
        fclose(config_file_ptr);
        return config_doc;
    }

    std::vector<Node> createNodeVecFromJSON(const rapidjson::Document &config_doc) {
        std::vector<std::vector<double> > nodes_vec;
        try {
            fea::createVectorFromJSON(config_doc, "nodes", nodes_vec);
        }
        catch (std::runtime_error &e) {
            throw;
        }

        std::vector<Node> nodes_out(nodes_vec.size());
        Node n;

        for (size_t i = 0; i < nodes_vec.size(); ++i) {

            if (nodes_vec[i].size() != 3) {
                throw std::runtime_error(
                        (boost::format("Row %d in nodes does not specify x, y and z coordinates.") % i).str()
                );
            }
            n << nodes_vec[i][0], nodes_vec[i][1], nodes_vec[i][2];
            nodes_out[i] = n;
        }
        return nodes_out;
    }

    std::vector<Elem> createElemVecFromJSON(const rapidjson::Document &config_doc) {
        std::vector<std::vector<unsigned int> > elems_vec;
        std::vector<std::vector<double> > props_vec;
        try {
            fea::createVectorFromJSON(config_doc, "elems", elems_vec);
            fea::createVectorFromJSON(config_doc, "props", props_vec);
        }
        catch (std::runtime_error &e) {
            throw;
        }

        if (elems_vec.size() != props_vec.size()) {
            throw std::runtime_error("The number of rows in elems did not match props.");
        }

        std::vector<Elem> elems_out(elems_vec.size());
        Props p;
        for (size_t i = 0; i < elems_vec.size(); ++i) {
            if (elems_vec[i].size() != 2) {
                throw std::runtime_error(
                        (boost::format("Row %d in elems does not specify 2 nodal indices [nn1,nn2].") % i).str()
                );
            }
            if (props_vec[i].size() != 7) {
                throw std::runtime_error(
                        (boost::format("Row %d  in props does not specify the 7 property values "
                                               "[EA, EIz, EIy, GJ, nx, ny, nz]") % i).str()
                );
            }
            p.EA = props_vec[i][0];
            p.EIz = props_vec[i][1];
            p.EIy = props_vec[i][2];
            p.GJ = props_vec[i][3];
            p.normal_vec << props_vec[i][4], props_vec[i][5], props_vec[i][6];
            elems_out[i] = Elem(elems_vec[i][0], elems_vec[i][1], p);
        }
        return elems_out;
    }

    std::vector<BC> createBCVecFromJSON(const rapidjson::Document &config_doc) {
        std::vector<std::vector<double> > bcs_vec;
        try {
            fea::createVectorFromJSON(config_doc, "bcs", bcs_vec);
        }
        catch (std::runtime_error &e) {
            throw;
        }

        std::vector<BC> bcs_out(bcs_vec.size());

        for (size_t i = 0; i < bcs_vec.size(); ++i) {
            if (bcs_vec[i].size() != 3) {
                throw std::runtime_error(
                        (boost::format("Row %d in bcs does not specify [node number,DOF,value].") % i).str()
                );
            }
            bcs_out[i] = BC((unsigned int) bcs_vec[i][0], (unsigned int) bcs_vec[i][1], bcs_vec[i][2]);
        }
        return bcs_out;
    }

    std::vector<Force> createForceVecFromJSON(const rapidjson::Document &config_doc) {
        std::vector<std::vector<double> > forces_vec;
        try {
            fea::createVectorFromJSON(config_doc, "forces", forces_vec);
        }
        catch (std::runtime_error &e) {
            throw;
        }

        std::vector<Force> forces_out(forces_vec.size());

        for (size_t i = 0; i < forces_vec.size(); ++i) {
            if (forces_vec[i].size() != 3) {
                throw std::runtime_error(
                        (boost::format("Row %d in forces does not specify [node number,DOF,value].") % i).str()
                );
            }
            forces_out[i] = Force((unsigned int) forces_vec[i][0], (unsigned int) forces_vec[i][1], forces_vec[i][2]);
        }
        return forces_out;
    }

    std::vector<Tie> createTieVecFromJSON(const rapidjson::Document &config_doc) {
        std::vector<std::vector<double> > ties_vec;
        try {
            fea::createVectorFromJSON(config_doc, "ties", ties_vec);
        }
        catch (std::runtime_error &e) {
            throw;
        }

        std::vector<Tie> ties_out(ties_vec.size());

        for (size_t i = 0; i < ties_vec.size(); ++i) {
            if (ties_vec[i].size() != 4) {
                throw std::runtime_error(
                        (boost::format("Row %d in ties does not specify [node number 1,node number 2,lmult,rmult].") %
                         i).str()
                );
            }
            ties_out[i] = Tie((unsigned int) ties_vec[i][0], (unsigned int) ties_vec[i][1], ties_vec[i][2],
                              ties_vec[i][3]);
        }
        return ties_out;
    }

    Job createJobFromJSON(const rapidjson::Document &config_doc) {
        try {
            std::vector<Node> nodes = createNodeVecFromJSON(config_doc);
            std::vector<Elem> elems = createElemVecFromJSON(config_doc);
            return Job(nodes, elems);
        }
        catch (std::runtime_error &e) {
            throw;
        }
    }

    Options createOptionsFromJSON(const rapidjson::Document &config_doc) {
        Options options;

        if (config_doc.HasMember("options")) {
            if (config_doc["options"].HasMember("epsilon")) {
                if (!config_doc["options"]["epsilon"].IsNumber()) {
                    throw std::runtime_error("epsilon provided in options configuration is not a number.");
                }
                options.epsilon = config_doc["options"]["epsilon"].GetDouble();
            }
            if (config_doc["options"].HasMember("csv_precision")) {
                if (!config_doc["options"]["csv_precision"].IsNumber()) {
                    throw std::runtime_error("csv_precision provided in options configuration is not a number.");
                }
                options.csv_precision = config_doc["options"]["csv_precision"].GetUint();
            }
            if (config_doc["options"].HasMember("csv_delimiter")) {
                if (!config_doc["options"]["csv_delimiter"].IsString()) {
                    throw std::runtime_error("csv_delimiter provided in options configuration is not a string.");
                }
                options.csv_delimiter = config_doc["options"]["csv_delimiter"].GetString();
            }
            if (config_doc["options"].HasMember("save_nodal_displacements")) {
                if (!config_doc["options"]["save_nodal_displacements"].IsBool()) {
                    throw std::runtime_error(
                            "save_nodal_displacements provided in options configuration is not a bool.");
                }
                options.save_nodal_displacements = config_doc["options"]["save_nodal_displacements"].GetBool();
            }
            if (config_doc["options"].HasMember("save_nodal_forces")) {
                if (!config_doc["options"]["save_nodal_forces"].IsBool()) {
                    throw std::runtime_error("save_nodal_forces provided in options configuration is not a bool.");
                }
                options.save_nodal_forces = config_doc["options"]["save_nodal_forces"].GetBool();
            }
            if (config_doc["options"].HasMember("save_tie_forces")) {
                if (!config_doc["options"]["save_tie_forces"].IsBool()) {
                    throw std::runtime_error("save_tie_forces provided in options configuration is not a bool.");
                }
                options.save_tie_forces = config_doc["options"]["save_tie_forces"].GetBool();
            }
            if (config_doc["options"].HasMember("verbose")) {
                if (!config_doc["options"]["verbose"].IsBool()) {
                    throw std::runtime_error("verbose provided in options configuration is not a bool.");
                }
                options.verbose = config_doc["options"]["verbose"].GetBool();
            }
            if (config_doc["options"].HasMember("save_report")) {
                if (!config_doc["options"]["save_report"].IsBool()) {
                    throw std::runtime_error("save_report provided in options configuration is not a bool.");
                }
                options.save_report = config_doc["options"]["save_report"].GetBool();
            }
            if (config_doc["options"].HasMember("nodal_displacements_filename")) {
                if (!config_doc["options"]["nodal_displacements_filename"].IsString()) {
                    throw std::runtime_error(
                            "nodal_displacements_filename provided in options configuration is not a string.");
                }
                options.nodal_displacements_filename = config_doc["options"]["nodal_displacements_filename"].GetString();
            }
            if (config_doc["options"].HasMember("nodal_forces_filename")) {
                if (!config_doc["options"]["nodal_forces_filename"].IsString()) {
                    throw std::runtime_error(
                            "nodal_forces_filename provided in options configuration is not a string.");
                }
                options.nodal_forces_filename = config_doc["options"]["nodal_forces_filename"].GetString();
            }
            if (config_doc["options"].HasMember("tie_forces_filename")) {
                if (!config_doc["options"]["tie_forces_filename"].IsString()) {
                    throw std::runtime_error("tie_forces_filename provided in options configuration is not a string.");
                }
                options.tie_forces_filename = config_doc["options"]["tie_forces_filename"].GetString();
            }
            if (config_doc["options"].HasMember("report_filename")) {
                if (!config_doc["options"]["report_filename"].IsString()) {
                    throw std::runtime_error("report_filename provided in options configuration is not a string.");
                }
                options.report_filename = config_doc["options"]["report_filename"].GetString();
            }
        }
        return options;
    }
} // namespace fea
