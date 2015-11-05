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

#ifndef FEA_SETUP_H
#define FEA_SETUP_H

#include "containers.h"
#include "csv_parser.h"
#include "options.h"
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

namespace fea {

    /**
     * Opens the specified json file and parses the data into a rapidjson::Document and returns the result.
     * The config document should have key's "nodes", "elems", and "props". Optionally, there can be keys
     * "bcs" for boundary conditions, "forces" for prescribed forces, and "ties" for and tie constraints between nodes.
     *
     * @param config_filename `std::string`. The location of the configuration json file.
     * @return Document`rapidjson::Document`
     */
    rapidjson::Document parseJSONConfig(const std::string &config_filename);

    /**
     * Parses the file indicated by the "nodes" key in `config_doc` into a vector of `fea::Node`'s.
     *
     * @param config_doc `rapidjson::Document`. Document storing the file name containing the nodal coordinates.
     * @return Nodal coordinates. `std::vector<Node>`.
     */
    std::vector<Node> createNodeVecFromJSON(const rapidjson::Document &config_doc);

    /**
     * Parses the files indicated by the "elems" and "props" keys in `config_doc` into a vector of `fea::Elem`'s.
     *
     * @param config_doc `rapidjson::Document`. Document storing the file names of the csv files that contain
     *                    the node number designations for each element and elemental properties.
     * @return Elements. `std::vector<Elem>`.
     */
    std::vector<Elem> createElemVecFromJSON(const rapidjson::Document &config_doc);

    /**
     * Parses the file indicated by the "bcs" key in `config_doc` into a vector of `fea::BC`'s.
     *
     * @param config_doc `rapidjson::Document`. Document storing the file name containing the boundary conditions.
     * @return Boundary conditions. `std::vector<BC>`.
     */
    std::vector<BC> createBCVecFromJSON(const rapidjson::Document &config_doc);

    /**
     * Parses the file indicated by the "forces" key in `config_doc` into a vector of `fea::Forces`'s.
     *
     * @param config_doc `rapidjson::Document`. Document storing the file name containing the prescribed forces.
     * @return Boundary conditions. `std::vector<BC>`.
     */
    std::vector<Force> createForceVecFromJSON(const rapidjson::Document &config_doc);

    /**
     * Parses the file indicated by the "ties" key in `config_doc` into a vector of `fea::Tie`'s.
     *
     * @param config_doc `rapidjson::Document`. Document storing the file name containing the prescribed forces.
     * @return Prescribed forces. `std::vector<Force>`.
     */
    std::vector<Tie> createTieVecFromJSON(const rapidjson::Document &config_doc);

    /**
     * Creates vectors of `fea::Node`'s and `fea::Elem`'s from the files specified in `config_doc`. A
     * `fea::Job` is created from the node and element vectors and returned.
     *
     * @param config_doc `rapidjson::Document`. Document storing the file name containing the
     *                    nodes, elements, and properties.
     * @return Job. `fea::Job`.
     */
    Job createJobFromJSON(const rapidjson::Document &config_doc);

    /**
     * Creates an `fea::Options` object from the configuration document. Any options provided will override the
     * defaults.
     *
     * @param config_doc `rapidjson::Document`. Document containing the configuration for the current analysis.
     *                    This function will look for the "options" member in `config_document` for a json object
     *                    holding the user-supplied options.
     * @return Analysis options `fea::Options`.
     */
    Options createOptionsFromJSON(const rapidjson::Document &config_doc);
}

#endif // FEA_SETUP_H
