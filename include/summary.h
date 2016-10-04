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

#ifndef THREEDBEAMFEA_SUMMARY_H
#define THREEDBEAMFEA_SUMMARY_H

#include <string>
#include <vector>

namespace fea {

    /**
     * @brief Contains the results of an analysis after calling `fea::solve`.
     */
    struct Summary {

        /**
         * Default constructor
         */
        Summary();

        /**
         * @brief Returns a message containing the results of the analysis.
         */
        std::string FullReport() const;

        /**
         * The total time of the FE analysis.
         */
        long long total_time_in_ms;

        /**
         * The time it took to assemble the global stiffness matrix.
         */
        long long assembly_time_in_ms;

        /**
         * The time to reorder the nonzero elements of the the global stiffness matrix,
         * such that the factorization step creates less fill-in.
         */
        long long preprocessing_time_in_ms;

        /**
         * The time to compute the factors of the coefficient matrix.
         */
        long long factorization_time_in_ms;

        /**
         * The time to compute the solution of the linear system.
         */
        long long solve_time_in_ms;

        /**
         * The time to compute the nodal forces.
         */
        long long nodal_forces_solve_time_in_ms;

        /**
         * The time to compute the nodal forces.
         */
        long long tie_forces_solve_time_in_ms;

        /**
         * The time to compute the nodal forces.
         * Does not include the time to save the summary itself if `Options::save_report == true` since all data must be
         * computed prior to saving the report.
         */
        long long file_save_time_in_ms;

        /**
         * The number of nodes in the analysis.
         */
        unsigned long num_nodes;

        /**
         * The number of elements in the analysis.
         */
        unsigned long num_elems;

        /**
         * The number of boundary conditions in the analysis.
         */
        unsigned long num_bcs;

        /**
         * The number of prescribed forces in the analysis.
         */
        unsigned long num_forces;

        /**
         * The number of tie constraints in the analysis.
         */
        unsigned long num_ties;

        /**
         * The number of equation constraints in the analysis.
         */
        unsigned long num_eqns;

        /**
         * The resultant nodal displacement from the FE analysis.
         * `nodal_displacements` is a 2D vector where each row
         * correspond to a node, and the columns correspond to
         * `[d_x, d_y, d_z, theta_x, theta_y, theta_z]`
         */
        std::vector<std::vector<double> > nodal_displacements;

        /**
         * The resultant nodal forces from the FE analysis.
         * `nodal_displacements` is a 2D vector where each row
         * correspond to a node, and the columns correspond to
         * `[f_x, f_y, f_z, m_x, m_y, m_z]`
         */
        std::vector<std::vector<double> > nodal_forces;

        /**
         * The resultant forces associated with ties between nodes.
         * `tie_forces` is a 2D vector where each row
         * correspond to a tie, and the columns correspond to
         * `[f_x, f_y, f_z, f_rot_x, f_rot_y, f_rot_z]`
         */
        std::vector<std::vector<double> > tie_forces;

    };

} //namespace fea



#endif //THREEDBEAMFEA_SUMMARY_H
