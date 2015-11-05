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

#include <boost/format.hpp>
#include <exception>
#include <fstream>
#include <iomanip>
#include "threed_beam_fea.h"

namespace fea {

    namespace {
        void writeStringToTxt(std::string filename, std::string data) {
            std::ofstream output_file;
            output_file.open(filename);

            if (!output_file.is_open()) {
                throw std::runtime_error(
                        (boost::format("Error opening file %s.") % filename).str()
                );
            }
            output_file << data;
            output_file.close();
        }
    }

    inline double norm(const Node &n1, const Node &n2) {
        const Node dn = n2 - n1;
        return dn.norm();
    }

    void GlobalStiffAssembler::calcKelem(unsigned int i, const Job &job) {
        // extract element properties
        const double EA = job.props[i].EA;
        const double EIz = job.props[i].EIz;
        const double EIy = job.props[i].EIy;
        const double GJ = job.props[i].GJ;

        // store node indices of current element
        const int nn1 = job.elems[i][0];
        const int nn2 = job.elems[i][1];

        // calculate the length of the element
        const double length = norm(job.nodes[nn1], job.nodes[nn2]);

        // store the entries in the (local) elemental stiffness matrix as temporary values to avoid recalculation
        const double tmpEA = EA / length;
        const double tmpGJ = GJ / length;

        const double tmp12z = 12.0 * EIz / (length * length * length);
        const double tmp6z = 6.0 * EIz / (length * length);
        const double tmp1z = EIz / length;

        const double tmp12y = 12.0 * EIy / (length * length * length);
        const double tmp6y = 6.0 * EIy / (length * length);
        const double tmp1y = EIy / length;

        // update local elemental stiffness matrix
        Klocal(0, 0) = tmpEA;
        Klocal(0, 6) = -tmpEA;
        Klocal(1, 1) = tmp12z;
        Klocal(1, 5) = tmp6z;
        Klocal(1, 7) = -tmp12z;
        Klocal(1, 11) = tmp6z;
        Klocal(2, 2) = tmp12y;
        Klocal(2, 4) = -tmp6y;
        Klocal(2, 8) = -tmp12y;
        Klocal(2, 10) = -tmp6y;
        Klocal(3, 3) = tmpGJ;
        Klocal(3, 9) = -tmpGJ;
        Klocal(4, 2) = -tmp6y;
        Klocal(4, 4) = 4.0 * tmp1y;
        Klocal(4, 8) = tmp6y;
        Klocal(4, 10) = 2.0 * tmp1y;
        Klocal(5, 1) = tmp6z;
        Klocal(5, 5) = 4.0 * tmp1z;
        Klocal(5, 7) = -tmp6z;
        Klocal(5, 11) = 2.0 * tmp1z;
        Klocal(6, 0) = -tmpEA;
        Klocal(6, 6) = tmpEA;
        Klocal(7, 1) = -tmp12z;
        Klocal(7, 5) = -tmp6z;
        Klocal(7, 7) = tmp12z;
        Klocal(7, 11) = -tmp6z;
        Klocal(8, 2) = -tmp12y;
        Klocal(8, 4) = tmp6y;
        Klocal(8, 8) = tmp12y;
        Klocal(8, 10) = tmp6y;
        Klocal(9, 3) = -tmpGJ;
        Klocal(9, 9) = tmpGJ;
        Klocal(10, 2) = -tmp6y;
        Klocal(10, 4) = 2.0 * tmp1y;
        Klocal(10, 8) = tmp6y;
        Klocal(10, 10) = 4.0 * tmp1y;
        Klocal(11, 1) = tmp6z;
        Klocal(11, 5) = 2.0 * tmp1z;
        Klocal(11, 7) = -tmp6z;
        Klocal(11, 11) = 4.0 * tmp1z;

        // calculate unit normal vector along local x-direction
        Eigen::Vector3d nx = job.nodes[nn2] - job.nodes[nn1];
        nx.normalize();

        // calculate unit normal vector along y-direction
        const Eigen::Vector3d ny = job.props[i].normal_vec.normalized();

        // update rotation matrices
        calcAelem(nx, ny);

        // update Kelem
        Kelem = AelemT * Klocal * Aelem;
    };

    void GlobalStiffAssembler::calcAelem(const Eigen::Vector3d &nx, const Eigen::Vector3d &ny) {
        // calculate the unit normal vector in local z direction
        Eigen::Vector3d nz;
        nz = nx.cross(ny);
        const double dlz = nz.squaredNorm();
        nz /= dlz;

        // update rotation matrix
        Aelem(0, 0) = nx(0);
        Aelem(0, 1) = nx(1);
        Aelem(0, 2) = nx(2);
        Aelem(1, 0) = ny(0);
        Aelem(1, 1) = ny(1);
        Aelem(1, 2) = ny(2);
        Aelem(2, 0) = nz(0);
        Aelem(2, 1) = nz(1);
        Aelem(2, 2) = nz(2);

        Aelem(3, 3) = nx(0);
        Aelem(3, 4) = nx(1);
        Aelem(3, 5) = nx(2);
        Aelem(4, 3) = ny(0);
        Aelem(4, 4) = ny(1);
        Aelem(4, 5) = ny(2);
        Aelem(5, 3) = nz(0);
        Aelem(5, 4) = nz(1);
        Aelem(5, 5) = nz(2);

        Aelem(6, 6) = nx(0);
        Aelem(6, 7) = nx(1);
        Aelem(6, 8) = nx(2);
        Aelem(7, 6) = ny(0);
        Aelem(7, 7) = ny(1);
        Aelem(7, 8) = ny(2);
        Aelem(8, 6) = nz(0);
        Aelem(8, 7) = nz(1);
        Aelem(8, 8) = nz(2);

        Aelem(9, 9) = nx(0);
        Aelem(9, 10) = nx(1);
        Aelem(9, 11) = nx(2);
        Aelem(10, 9) = ny(0);
        Aelem(10, 10) = ny(1);
        Aelem(10, 11) = ny(2);
        Aelem(11, 9) = nz(0);
        Aelem(11, 10) = nz(1);
        Aelem(11, 11) = nz(2);

        // update transposed rotation matrix
        AelemT(0, 0) = nx(0);
        AelemT(0, 1) = ny(0);
        AelemT(0, 2) = nz(0);
        AelemT(1, 0) = nx(1);
        AelemT(1, 1) = ny(1);
        AelemT(1, 2) = nz(1);
        AelemT(2, 0) = nx(2);
        AelemT(2, 1) = ny(2);
        AelemT(2, 2) = nz(2);

        AelemT(3, 3) = nx(0);
        AelemT(3, 4) = ny(0);
        AelemT(3, 5) = nz(0);
        AelemT(4, 3) = nx(1);
        AelemT(4, 4) = ny(1);
        AelemT(4, 5) = nz(1);
        AelemT(5, 3) = nx(2);
        AelemT(5, 4) = ny(2);
        AelemT(5, 5) = nz(2);

        AelemT(6, 6) = nx(0);
        AelemT(6, 7) = ny(0);
        AelemT(6, 8) = nz(0);
        AelemT(7, 6) = nx(1);
        AelemT(7, 7) = ny(1);
        AelemT(7, 8) = nz(1);
        AelemT(8, 6) = nx(2);
        AelemT(8, 7) = ny(2);
        AelemT(8, 8) = nz(2);

        AelemT(9, 9) = nx(0);
        AelemT(9, 10) = ny(0);
        AelemT(9, 11) = nz(0);
        AelemT(10, 9) = nx(1);
        AelemT(10, 10) = ny(1);
        AelemT(10, 11) = nz(1);
        AelemT(11, 9) = nx(2);
        AelemT(11, 10) = ny(2);
        AelemT(11, 11) = nz(2);
    };

    void GlobalStiffAssembler::operator()(SparseMat &Kg, const Job &job, const std::vector<Tie> &ties) {
        int nn1, nn2;
        unsigned int row, col;
        const unsigned int dofs_per_elem = DOF::NUM_DOFS;

        // form vector to hold triplets that will be used to assemble global stiffness matrix
        std::vector<Eigen::Triplet<double> > triplets;
        triplets.reserve(40 * job.elems.size() + 4 * dofs_per_elem * ties.size());

        for (unsigned int i = 0; i < job.elems.size(); ++i) {
            // update Kelem with current elemental stiffness matrix
            calcKelem(i, job);

            // get sparse representation of the current elemental stiffness matrix
            SparseKelem = Kelem.sparseView();

            // pull out current node indices
            nn1 = job.elems[i][0];
            nn2 = job.elems[i][1];

            for (unsigned int j = 0; j < SparseKelem.outerSize(); ++j) {
                for (SparseMat::InnerIterator it(SparseKelem, j); it; ++it) {
                    row = it.row();
                    col = it.col();

                    // check position in local matrix and update corresponding global position
                    if (row < 6) {
                        // top left
                        if (col < 6) {
                            triplets.push_back(Eigen::Triplet<double>(dofs_per_elem * nn1 + row,
                                                                      dofs_per_elem * nn1 + col,
                                                                      it.value()));
                        }
                            // top right
                        else {
                            triplets.push_back(Eigen::Triplet<double>(dofs_per_elem * nn1 + row,
                                                                      dofs_per_elem * (nn2 - 1) + col,
                                                                      it.value()));
                        }
                    }
                    else {
                        // bottom left
                        if (col < 6) {
                            triplets.push_back(Eigen::Triplet<double>(dofs_per_elem * (nn2 - 1) + row,
                                                                      dofs_per_elem * nn1 + col,
                                                                      it.value()));
                        }
                            // bottom right
                        else {
                            triplets.push_back(Eigen::Triplet<double>(dofs_per_elem * (nn2 - 1) + row,
                                                                      dofs_per_elem * (nn2 - 1) + col,
                                                                      it.value()));
                        }
                    }
                }
            }
        }

        loadTies(triplets, ties);

        Kg.setFromTriplets(triplets.begin(), triplets.end());
    };

    void loadBCs(SparseMat &Kg, SparseMat &force_vec, const std::vector<BC> &BCs, unsigned int num_nodes) {
        unsigned int bc_idx;
        const unsigned int dofs_per_elem = DOF::NUM_DOFS;
        // calculate the index that marks beginning of Lagrange multiplier coefficients
        const unsigned int global_add_idx = dofs_per_elem * num_nodes;

        for (size_t i = 0; i < BCs.size(); ++i) {
            bc_idx = dofs_per_elem * BCs[i].node + BCs[i].dof;

            // update global stiffness matrix
            Kg.insert(bc_idx, global_add_idx + i) = 1;
            Kg.insert(global_add_idx + i, bc_idx) = 1;

            // update force vector. All values are already zero. Only update if BC if non-zero.
            if (std::abs(BCs[i].value) > std::numeric_limits<double>::epsilon()) {
                force_vec.insert(global_add_idx + i, 0) = BCs[i].value;
            }
        }
    };

    void loadTies(std::vector<Eigen::Triplet<double> > &triplets, const std::vector<Tie> &ties) {
        const unsigned int dofs_per_elem = DOF::NUM_DOFS;
        unsigned int nn1, nn2;
        double lmult, rmult, spring_constant;

        for (size_t i = 0; i < ties.size(); ++i) {
            nn1 = ties[i].node_number_1;
            nn2 = ties[i].node_number_2;
            lmult = ties[i].lmult;
            rmult = ties[i].rmult;

            for (unsigned int j = 0; j < dofs_per_elem; ++j) {
                // first 3 DOFs are linear DOFs, second 2 are rotational, last is torsional
                spring_constant = j < 3 ? lmult : rmult;

                triplets.push_back(Eigen::Triplet<double>(dofs_per_elem * nn1 + j,
                                                          dofs_per_elem * nn1 + j,
                                                          spring_constant));

                triplets.push_back(Eigen::Triplet<double>(dofs_per_elem * nn2 + j,
                                                          dofs_per_elem * nn2 + j,
                                                          spring_constant));

                triplets.push_back(Eigen::Triplet<double>(dofs_per_elem * nn1 + j,
                                                          dofs_per_elem * nn2 + j,
                                                          -spring_constant));

                triplets.push_back(Eigen::Triplet<double>(dofs_per_elem * nn2 + j,
                                                          dofs_per_elem * nn1 + j,
                                                          -spring_constant));
            }
        }
    };

    std::vector<std::vector<double> > computeTieForces(const std::vector<Tie> &ties,
                                                       const std::vector<std::vector<double> > &nodal_displacements) {
        const unsigned int dofs_per_elem = DOF::NUM_DOFS;
        unsigned int nn1, nn2;
        double lmult, rmult, spring_constant, delta1, delta2;

        std::vector<std::vector<double> > tie_forces(ties.size(), std::vector<double>(dofs_per_elem));

        for (size_t i = 0; i < ties.size(); ++i) {
            nn1 = ties[i].node_number_1;
            nn2 = ties[i].node_number_2;
            lmult = ties[i].lmult;
            rmult = ties[i].rmult;

            for (unsigned int j = 0; j < dofs_per_elem; ++j) {
                // first 3 DOFs are linear DOFs, second 2 are rotational, last is torsional
                spring_constant = j < 3 ? lmult : rmult;
                delta1 = nodal_displacements[nn1][j];
                delta2 = nodal_displacements[nn2][j];
                tie_forces[i][j] = spring_constant * (delta2 - delta1);
            }
        }
        return tie_forces;
    }

    void loadForces(SparseMat &force_vec, const std::vector<Force> &forces) {
        const unsigned int dofs_per_elem = DOF::NUM_DOFS;
        unsigned int idx;

        for (size_t i = 0; i < forces.size(); ++i) {
            idx = dofs_per_elem * forces[i].node + forces[i].dof;
            force_vec.insert(idx, 0) = forces[i].value;
        }
    };

    Summary solve(const Job &job,
                  const std::vector<BC> &BCs,
                  const std::vector<Force> &forces,
                  const std::vector<Tie> &ties,
                  const Options &options) {
        auto initial_start_time = std::chrono::high_resolution_clock::now();

        Summary summary;
        summary.num_nodes = job.nodes.size();
        summary.num_elems = job.elems.size();
        summary.num_bcs = BCs.size();
        summary.num_ties = ties.size();

        const unsigned int dofs_per_elem = DOF::NUM_DOFS;

        // calculate size of global stiffness matrix and force vector
        const int size = dofs_per_elem * job.nodes.size() + BCs.size();

        // construct global stiffness matrix and force vector
        SparseMat Kg(size, size);
        SparseMat force_vec(size, 1);
        force_vec.reserve(forces.size() + BCs.size());

        // construct global assembler object and assemble global stiffness matrix
        auto start_time = std::chrono::high_resolution_clock::now();
        GlobalStiffAssembler assembleK3D = GlobalStiffAssembler();
        assembleK3D(Kg, job, ties);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        summary.assembly_time_in_ms = delta_time;

        if (options.verbose)
            std::cout << "Global stiffness matrix assembled in "
            << delta_time
            << " ms.\nNow preprocessing factorization..." << std::endl;

        // load prescribed boundary conditions into stiffness matrix and force vector
        loadBCs(Kg, force_vec, BCs, job.nodes.size());

        // load prescribed forces into force vector
        if (forces.size() > 0) {
            loadForces(force_vec, forces);
        }

        // compress global stiffness matrix since all non-zero values have been added.
        Kg.makeCompressed();

        // initialize solver based on whether MKL should be used
#ifdef EIGEN_USE_MKL_ALL
        Eigen::PardisoLU<SparseMat> solver;
#else
        Eigen::SparseLU<SparseMat> solver;
#endif

        //Compute the ordering permutation vector from the structural pattern of Kg
        start_time = std::chrono::high_resolution_clock::now();
        solver.analyzePattern(Kg);
        end_time = std::chrono::high_resolution_clock::now();
        delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        summary.preprocessing_time_in_ms = delta_time;

        if (options.verbose)
            std::cout << "Preprocessing step of factorization completed in "
            << delta_time
            << " ms.\nNow factorizing global stiffness matrix..." << std::endl;

        // Compute the numerical factorization
        start_time = std::chrono::high_resolution_clock::now();
        solver.factorize(Kg);
        end_time = std::chrono::high_resolution_clock::now();

        delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        summary.factorization_time_in_ms = delta_time;

        if (options.verbose)
            std::cout << "Factorization completed in "
            << delta_time
            << " ms. Now solving system..." << std::endl;

        //Use the factors to solve the linear system
        start_time = std::chrono::high_resolution_clock::now();
        SparseMat dispSparse = solver.solve(force_vec);
        end_time = std::chrono::high_resolution_clock::now();
        delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        summary.solve_time_in_ms = delta_time;

        if (options.verbose)
            std::cout << "System was solved in "
            << delta_time
            << " ms.\n" << std::endl;

        // convert to dense matrix
        Eigen::VectorXd disp(dispSparse);

        // convert from Eigen vector to std vector
        std::vector<std::vector<double> > disp_vec(job.nodes.size(), std::vector<double>(dofs_per_elem));
        for (size_t i = 0; i < disp_vec.size(); ++i) {
            for (unsigned int j = 0; j < dofs_per_elem; ++j)
                // round all values close to 0.0
                disp_vec[i][j] =
                        std::abs(disp(dofs_per_elem * i + j)) < options.epsilon ? 0.0 : disp(dofs_per_elem * i + j);
        }
        summary.nodal_displacements = disp_vec;

        // [calculate nodal forces
        start_time = std::chrono::high_resolution_clock::now();

        Kg = Kg.topLeftCorner(dofs_per_elem * job.nodes.size(), dofs_per_elem * job.nodes.size());
        dispSparse = dispSparse.topRows(dofs_per_elem * job.nodes.size());

        SparseMat nodal_forces_sparse = Kg * dispSparse;

        Eigen::VectorXd nodal_forces_dense(nodal_forces_sparse);

        std::vector<std::vector<double> > nodal_forces_vec(job.nodes.size(), std::vector<double>(dofs_per_elem));
        for (size_t i = 0; i < nodal_forces_vec.size(); ++i) {
            for (unsigned int j = 0; j < dofs_per_elem; ++j)
                // round all values close to 0.0
                nodal_forces_vec[i][j] = std::abs(nodal_forces_dense(dofs_per_elem * i + j)) < options.epsilon ? 0.0
                                                                                                               : nodal_forces_dense(
                                dofs_per_elem * i + j);
        }
        summary.nodal_forces = nodal_forces_vec;

        end_time = std::chrono::high_resolution_clock::now();

        summary.nodal_forces_solve_time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time).count();
        //]

        // [ calculate forces associated with ties
        if (ties.size() > 0) {
            start_time = std::chrono::high_resolution_clock::now();
            summary.tie_forces = computeTieForces(ties, disp_vec);
            end_time = std::chrono::high_resolution_clock::now();
            summary.tie_forces_solve_time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count();
        }
        // ]

        // [save files specified in options
        CSVParser csv;
        start_time = std::chrono::high_resolution_clock::now();
        if (options.save_nodal_displacements) {
            csv.write(options.nodal_displacements_filename, disp_vec, options.csv_precision, options.csv_delimiter);
        }

        if (options.save_nodal_forces) {
            csv.write(options.nodal_forces_filename, nodal_forces_vec, options.csv_precision, options.csv_delimiter);
        }

        if (options.save_tie_forces) {
            csv.write(options.tie_forces_filename, summary.tie_forces, options.csv_precision, options.csv_delimiter);
        }

        end_time = std::chrono::high_resolution_clock::now();
        summary.file_save_time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time).count();
        // ]

        auto final_end_time = std::chrono::high_resolution_clock::now();

        delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(final_end_time - initial_start_time).count();
        summary.total_time_in_ms = delta_time;

        if (options.save_report) {
            writeStringToTxt(options.report_filename, summary.FullReport());
        }

        if (options.verbose)
            std::cout << summary.FullReport();

        return summary;
    };

} // namespace fea
