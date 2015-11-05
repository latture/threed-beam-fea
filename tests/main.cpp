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

#include <tclap/CmdLine.h>
#include <rapidjson/document.h>
#include "threed_beam_fea.h"
#include "setup.h"

fea::Summary runAnalysis(const rapidjson::Document &config_doc) {
    fea::Job job = fea::createJobFromJSON(config_doc);

    std::vector<fea::Tie> ties;
    if (config_doc.HasMember("ties")) {
        ties = fea::createTieVecFromJSON(config_doc);
    }

    std::vector<fea::BC> bcs;
    if (config_doc.HasMember("bcs")) {
        bcs = fea::createBCVecFromJSON(config_doc);
    }

    std::vector<fea::Force> forces;
    if (config_doc.HasMember("forces")) {
        forces = fea::createForceVecFromJSON(config_doc);
    }

    fea::Options options = fea::createOptionsFromJSON(config_doc);

    return fea::solve(job, bcs, forces, ties, options);
}

int main(int argc, char *argv[]) {
    try
    {
        TCLAP::CmdLine cmd("3D Euler-Bernoulli beam element FEA. "
                           "Use the -c [--config] flag to point to the configuration file for the current analysis.", ' ', "1.0");
        TCLAP::ValueArg<std::string> configArg("c",
                                               "config",
                                               "Finite element configuration file (json format). "
                                               "Must have \"nodes\", \"elems\", and \"props\" members pointing the associated files. "
                                               "Optionally, any boundary conditions should be in the file pointed to by "
                                               "the \"bcs\" member variable of the config file. Likewise, prescribed "
                                               "forces are set using the \"forces\" variable, and ties are set via the "
                                               "\"ties\" variable. Please refer to the documentation for the file format "
                                               "of each variable. Override the default options using the \"options\" "
                                               "member variable the itself is a nested json object. Refer to the "
                                               "fea::Options documentation the possible configurations that can be set.",
                                               true,
                                               "config.json",
                                               "string");
        cmd.add( configArg );
        cmd.parse( argc, argv );
        std::string config_filename = configArg.getValue();
        rapidjson::Document config_doc = fea::parseJSONConfig(config_filename);

        runAnalysis(config_doc);
    }
    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    return 0;
}
