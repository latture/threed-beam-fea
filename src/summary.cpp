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

#include <iomanip>
#include <algorithm>
#include <limits>
#include "summary.h"
#include "boost/format.hpp"

namespace fea {

    namespace {

        typedef std::pair<std::string, unsigned int> fe_param_pair;
        typedef std::pair<std::string, long long> timing_param_pair;

        struct Location2D {
            Location2D() : row(0), col(0) { };

            Location2D(size_t _row, size_t _col) : row(_row), col(_col) { };

            size_t row;
            size_t col;
        };

        template<typename T>
        std::pair<Location2D, Location2D> findMinMax2D(std::vector<std::vector<T>> input) {
            Location2D min_elem, max_elem;
            T min_val = std::numeric_limits<T>::max();
            T max_val = -std::numeric_limits<T>::max();

            const size_t num_cols = input[0].size();

            for (size_t i = 0; i < input.size(); ++i) {
                for (size_t j = 0; j < num_cols; ++j) {
                    if (input[i][j] > max_val) {
                        max_elem.row = i;
                        max_elem.col = j;
                        max_val = input[i][j];
                    }
                    if (input[i][j] < min_val) {
                        min_elem.row = i;
                        min_elem.col = j;
                        min_val = input[i][j];
                    }
                }
            }
            return std::pair<Location2D, Location2D>(min_elem, max_elem);
        }

        template<class T>
        int numDigits(T number) {
            int digits = 0;
            if (number < 0) digits = 1; // remove this line if '-' counts as a digit
            while (number) {
                number /= 10;
                digits++;
            }
            return digits;
        }
    }

    Summary::Summary()
            : total_time_in_ms(0),
              assembly_time_in_ms(0),
              preprocessing_time_in_ms(0),
              factorization_time_in_ms(0),
              solve_time_in_ms(0),
              nodal_forces_solve_time_in_ms(0),
              tie_forces_solve_time_in_ms(0),
              file_save_time_in_ms(0),
              num_nodes(0),
              num_elems(0),
              num_bcs(0),
              num_forces(0),
              num_ties(0),
              num_eqns(0),
              nodal_displacements(0),
              nodal_forces(0),
              tie_forces(0) {

    }

    std::string Summary::FullReport() const {
        std::string report = std::string("\nFinite Element Analysis Summary\n\nModel parameters\n");
        boost::format fe_params_fmt = boost::format("\t%s : %d\n");

        std::vector<fe_param_pair> fe_params(6);
        fe_params[0] = fe_param_pair("Nodes", num_nodes);
        fe_params[1] = fe_param_pair("Elements", num_elems);
        fe_params[2] = fe_param_pair("BCs", num_bcs);
        fe_params[3] = fe_param_pair("Ties", num_ties);
        fe_params[4] = fe_param_pair("Forces ", num_forces);
        fe_params[5] = fe_param_pair("Equations ", num_eqns);

        // get the maximum number of digits in the model parameters for formatting
        int max_digits = 1;
        int num_digits;
        for (std::vector<fe_param_pair>::iterator it = fe_params.begin(); it != fe_params.end(); ++it) {
            num_digits = numDigits(it->second);
            if (num_digits > max_digits)
                max_digits = num_digits;
        }

        // set format for key, value pairs
        fe_params_fmt.modify_item(1, boost::io::group(std::setw(20), std::left));
        fe_params_fmt.modify_item(2, boost::io::group(std::setw(max_digits), std::right));

        // append all fe parameter key, value pairs to the report
        for (std::vector<fe_param_pair>::iterator it = fe_params.begin(); it != fe_params.end(); ++it) {
            report.append(
                    (fe_params_fmt % it->first % it->second).str()
            );
        }

        // write the total time the analysis took
        report.append((boost::format("\n%s %dms\n") % "Total time" % total_time_in_ms).str());

        // define timing data to write to report
        std::vector<timing_param_pair> timing_params;
        timing_params.reserve(7);
        timing_params.push_back(timing_param_pair("Assembly time", assembly_time_in_ms));
        timing_params.push_back(timing_param_pair("Preprocessesing time", preprocessing_time_in_ms));
        timing_params.push_back(timing_param_pair("Factorization time", factorization_time_in_ms));
        timing_params.push_back(timing_param_pair("Linear solve time", solve_time_in_ms));
        timing_params.push_back(timing_param_pair("Forces solve time", nodal_forces_solve_time_in_ms));
        if (num_ties > 0) {
            timing_params.push_back(timing_param_pair("Ties solve time", tie_forces_solve_time_in_ms));
        }
        timing_params.push_back(timing_param_pair("File save time", file_save_time_in_ms));

        max_digits = 1;
        for (std::vector<timing_param_pair>::iterator it = timing_params.begin(); it != timing_params.end(); ++it) {
            num_digits = numDigits(it->second);
            if (num_digits > max_digits)
                max_digits = num_digits;
        }

        // create generic format for timing data
        boost::format timing_params_fmt = boost::format("\t%s : %dms\n");
        timing_params_fmt.modify_item(1, boost::io::group(std::setw(30), std::left));
        timing_params_fmt.modify_item(2, boost::io::group(std::setw(max_digits), std::right));

        // append timing data to report
        for (std::vector<timing_param_pair>::iterator it = timing_params.begin(); it != timing_params.end(); ++it) {
            report.append(
                    (timing_params_fmt % it->first % it->second).str()
            );
        }

        auto minmax = findMinMax2D(nodal_displacements);

        report.append(
                (boost::format(
                        "\nNodal displacements\n\tMinimum : Node %d\tDOF %d\tValue %.3f\n\tMaximum : Node %d\tDOF %d\tValue %.3f\n")
                 % minmax.first.row % minmax.first.col % nodal_displacements[minmax.first.row][minmax.first.col]
                 % minmax.second.row % minmax.second.col %
                 nodal_displacements[minmax.second.row][minmax.second.col]).str()
        );

        minmax = findMinMax2D(nodal_forces);

        report.append(
                (boost::format(
                        "\nNodal Forces\n\tMinimum : Node %d\tDOF %d\tValue %.3f\n\tMaximum : Node %d\tDOF %d\tValue %.3f\n")
                 % minmax.first.row % minmax.first.col % nodal_forces[minmax.first.row][minmax.first.col]
                 % minmax.second.row % minmax.second.col % nodal_forces[minmax.second.row][minmax.second.col]).str()
        );

        if (num_ties > 0) {
            minmax = findMinMax2D(tie_forces);

            report.append(
                    (boost::format(
                            "\nTie Forces\n\tMinimum : Tie %d\tDOF %d\tValue %.3f\n\tMaximum : Tie %d\tDOF %d\tValue %.3f\n")
                     % minmax.first.row % minmax.first.col % tie_forces[minmax.first.row][minmax.first.col]
                     % minmax.second.row % minmax.second.col % tie_forces[minmax.second.row][minmax.second.col]).str()
            );
        }
        return report;
    }

} // namespace fea
