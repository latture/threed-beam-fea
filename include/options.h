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

#ifndef THREEDBEAMFEA_OPTIONS_H
#define THREEDBEAMFEA_OPTIONS_H

#include <string>

namespace fea {
    /**
     * @brief Provides a method for customizing the finite element analysis.
     */
    struct Options {
        /**
         * @brief Default constructor
         * @details This tries to set up reasonable defaults for analysis. It is
         * recommended that the user read and overwrite the defaults based on the information
         * desired from the analysis.
         */
        Options() {
            epsilon = 1e-14;

            csv_precision = 14;
            csv_delimiter = ",";

            save_nodal_displacements = false;
            save_nodal_forces = false;
            save_tie_forces = false;
            verbose = false;
            save_report = false;

            nodal_displacements_filename = "nodal_displacements.csv";
            nodal_forces_filename = "nodal_forces.csv";
            tie_forces_filename = "tie_forces.csv";
            report_filename = "report.txt";
        }

        /**
         * Values of forces and nodal displacements which have a magnitude less than `epsilon` will be rounded to 0.0.
         * Default = `1e-14`. This is a simple way to deal with machine precision when doing calculations.
         */
        double epsilon;

        /**
         * Number of decimal places to use when saving nodal forces and displacements if either
         * `save_nodal_displacements` or `save_nodal_forces` is set to `true`. Default = 14.
         */
        unsigned int csv_precision;

        /**
         * Delimiter to use when saving nodal forces and displacements if either
         * `save_nodal_displacements` or `save_nodal_forces` is set to `true`. Default = ",".
         */
        std::string csv_delimiter;

        /**
         * Specifies if the nodal displacements should be saved to a file. Default = `false`.
         * If `true` the nodal displacements will be saved to the file indicated by `nodal_displacements_filename`.
         */
        bool save_nodal_displacements;

        /**
         * Specifies if the nodal forces should be saved to a file. Default = `false`.
         * If `true` the nodal forces will be saved to the file indicated by `nodal_forces_filename`.
         */
        bool save_nodal_forces;

        /**
         * Specifies if the forces associated with tie elements should be saved to a file. Default = `false`.
         * If `true` the tie forces will be saved to the file indicated by `tie_forces_filename`.
         */
        bool save_tie_forces;

        /**
         * Specifies if progress of the analysis should be written to std::cout. Default = `false`.
         * If `true` information on current step and time taken per step will be reported.
         */
        bool verbose;

        /**
         * Specifies if a text file should be written detailing information on the analysis. Default = `false`.
         * If `true` the a report will be saved to the file indicated by `report_filename`.
         */
        bool save_report;

        /**
         * File name to save the nodal displacements to when `save_nodal_displacements == true`.
         */
        std::string nodal_displacements_filename;

        /**
         * File name to save the nodal forces to when `save_nodal_forces == true`.
         */
        std::string nodal_forces_filename;

        /**
         * File name to save the nodal forces to when `save_tie_forces == true`.
         */
        std::string tie_forces_filename;

        /**
         * File name to save the nodal forces to when `save_report == true`.
         */
        std::string report_filename;

    };

} // namespace fea

#endif //THREEDBEAMFEA_OPTIONS_H
