/*!
* \file threed_beam_fea.h
*
* \author Ryan Latture
* \date 8-12-15
*
* Contains declarations for 3D beam FEA functions.
*/

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

#ifndef THREED_BEAM_FEA_H
#define THREED_BEAM_FEA_H

#ifdef EIGEN_USE_MKL_ALL
#include <Eigen/PardisoSupport>
#else
#include <Eigen/SparseLU>
#endif

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/SparseCore>

#include "containers.h"
#include "options.h"
#include "summary.h"
#include "csv_parser.h"

namespace fea {

    /**
     * Dense global stiffness matrix
     */
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> GlobalStiffMatrix;

    /**
     * An elemental matrix in local coordinates. Will either be the elemental stiffness matrix or the global-to-local rotation matrix
     */
    typedef Eigen::Matrix<double, 12, 12, Eigen::RowMajor> LocalMatrix;

    /**
     * Vector that stores the nodal forces, i.e. the variable \f$[F]\f$ in \f$[K][Q]=[F]\f$,
     * where \f$[K]\f$ is the global stiffness matrix and \f$[Q]\f$ contains the nodal displacements
     */
    typedef Eigen::Matrix<double, Eigen::Dynamic, 1> ForceVector;

    /**
     * Sparse matrix that is used internally to hold the global stiffness matrix
     */
    typedef Eigen::SparseMatrix<double> SparseMat;

    /**
     * @brief Calculates the distance between 2 nodes.
     * @details Calculates the original Euclidean distance between 2 nodes in the x-y plane.
     *
     * @param[in] n1 `fea::Node`. Nodal coordinates of first point.
     * @param[in] n2 `fea::Node`. Nodal coordinates of second point.
     *
     * @return <B>Distance</B> `double`. The distance between the nodes.
     */
    inline double norm(const Node &n1, const Node &n2);

    /**
     * @brief Assembles the global stiffness matrix.
     */
    class GlobalStiffAssembler {

    public:

        /**
         * @brief Default constructor.
         * @details Initializes all entries in member matrices to 0.0.
         */
        GlobalStiffAssembler() {
            Kelem.setZero();
            Klocal.setZero();
            Aelem.setZero();
            AelemT.setZero();
            SparseKelem.resize(12, 12);
            SparseKelem.reserve(40);
        };

        /**
         * @brief Assembles the global stiffness matrix.
         * @details The input stiffness matrix is modified in place to contain the correct values for the given job.
         * Assumes that the input stiffness matrix has the correct dimensions and all values are initially set to zero.
         *
         * @param Kg `fea::GlobalStiffMatrix`. Modified in place. After evaluation, Kg contains the correct values for
         *                                the global stiffness matrix due to the input Job.
         * @param[in] job `fea::Job`. Current Job to analyze contains node, element, and property lists.
         * @param[in] ties `std::vector<fea::Tie>`. Vector of ties that apply to attach springs of specified stiffness to
         *                                     all nodal degrees of freedom between each set of nodes indicated.
         */
        void operator()(SparseMat &Kg, const Job &job, const std::vector<Tie> &ties);

        /**
         * @brief Updates the elemental stiffness matrix for the `ith` element.
         *
         * @param[in] i `unsigned int`. Specifies the ith element for which the elemental stiffness matrix is calculated.
         * @param[in] job `Job`. Current `fea::Job` to analyze contains node, element, and property lists.
         */
        void calcKelem(unsigned int i, const Job &job);

        /**
         * @brief Updates the rotation and transposed rotation matrices.
         * @details The rotation matrices `Aelem` and `AelemT` are updated based on the 2 specified unit
         * normal vectors along the local x and y directions.
         *
         * @param[in] nx `Eigen::Matrix3d`. Unit normal vector in global space parallel to the beam element's local x-direction.
         * @param[in] ny `Eigen::Matrix3d`. Unit normal vector in global space parallel to the beam element's local y-direction.
         */
        void calcAelem(const Eigen::Vector3d &nx, const Eigen::Vector3d &nz);

        /**
         * @brief Returns the currently stored elemental stiffness matrix.
         * @return <B>Elemental stiffness matrix</B> `fea::LocalMatrix`.
         */
        LocalMatrix getKelem() {
            return Kelem;
        }

        /**
         * @brief Returns the currently stored rotation matrix.
         * @return <B>Rotation matrix</B> `fea::LocalMatrix`.
         */
        LocalMatrix getAelem() {
            return Aelem;
        }

    private:
        LocalMatrix Kelem;
        /**<Elemental stiffness matrix in global coordinate system.*/
        LocalMatrix Klocal;
        /**<Elemental stiffness matrix in local coordinate system (used as temporary variable).*/
        LocalMatrix Aelem;
        /**<Rotation matrix. Transforms `Klocal` to global coorinate system (`Kelem`).*/
        LocalMatrix AelemT;
        /**<Transposed rotation matrix.*/
        SparseMat SparseKelem;/**<Sparse representation of elemental stiffness matrix.*/
    };

    /**
    * @brief Loads the boundary conditions into the global stiffness matrix and force vector.
    * @details Boundary conditions are enforced via Lagrange multipliers. The reaction force
    * due to imposing the boundary condition will be appended directly onto the returned
    * nodal displacements in the order the boundary conditions were specified.
    *
    * @param Kg `fea::GlobalStiffnessMatrix`. Coefficients are modified in place to reflect Lagrange multipliers.
    *                                   Assumes `Kg` has the correct dimensions.
    * @param force_vec `fea::ForceVector`. Right hand side of the \f$[K][Q]=[F]\f$ equation of the FE analysis.
    * @param[in] BCs `std::vector<fea::BC>`. Vector of `BC`'s to apply to the current analysis.
    * @param[in] num_nodes `unsigned int`. The number of nodes in the current job being analyzed.
    *                                  Used to calculate the position to insert border coefficients associated
    *                                  with enforcing boundary conditions via Langrange multipliers.
    */
    void loadBCs(SparseMat &Kg, ForceVector &force_vec, const std::vector<BC> &BCs, unsigned int num_nodes);

    void loadEquations(SparseMat &Kg, const std::vector<Equation> &equations, unsigned int num_nodes, unsigned int num_bcs);

    /**
     * @brief Loads any tie constraints into the set of triplets that will become the global stiffness matrix.
     * @details Tie constraints are enforced via linear springs between the 2 specified
     * nodes. The `lmult` member variable is used as the spring constant for displacement degrees
     * of freedom, e.g. 0, 1, and 2. `rmult` is used for rotational degrees of freedom, e.g. 3, 4, and 5.
     *
     * @param triplets `std::vector< Eigen::Triplet< double > >`. A vector of triplets that store data in the
     *                  form (i, j, value) that will be become the sparse global stiffness matrix.
     * @param[in] ties `std::vector<fea::Tie>`. Vector of `Tie`'s to apply to the current analysis.
     */
    void loadTies(std::vector<Eigen::Triplet<double> > &triplets, const std::vector<Tie> &ties);


    /**
     * @brief Computes the forces in the tie elements based on the nodal displacements of the FE
     * analysis and the spring constants provided in `ties`.
     *
     * @param[in] ties `std::vector<Tie>`. Vector of `fea::Tie`'s to applied to the current analysis.
     * @param[in] nodal_displacements `std::vector < std::vector < double > >`. The resultant nodal displacements of the analysis.
     * @return Tie forces. `std::vector < std::vector < double > >`
     */
    std::vector<std::vector<double> > computeTieForces(const std::vector<Tie> &ties,
                                                       const std::vector<std::vector<double> > &nodal_displacements);

    /**
     * @brief Loads the prescribed forces into the force vector.
     *
     * @param force_vec `ForceVector`. Right hand side of the \f$[K][Q]=[F]\f$ equation of the FE analysis.
     * @param[in] forces std::vector<Force>. Vector of prescribed forces to apply to the current analysis.
     */
    void loadForces(SparseMat &force_vec, const std::vector<Force> &forces);

    /**
     * @brief Solves the finite element analysis defined by the input Job, boundary conditions, and prescribed nodal forces.
     * @details Solves \f$[K][Q]=[F]\f$ for \f$[Q]\f$, where \f$[K]\f$ is the global stiffness matrix,
     * \f$[Q]\f$ contains the nodal displacements, and \f$[Q]\f$ contains the nodal forces.
     *
     * @param[in] job `fea::Job`. Contains the node, element, and property lists for the mesh.
     * @param[in] BCs `std::vector<fea::BC>`. Vector of boundary conditions to apply to the nodal degrees of freedom contained in the job.
     * @param[in] forces `std::vector<fea::Force>`. Vector of prescribed forces to apply to the nodal degrees of freedom contained in the job.
     * @param[in] ties `std::vector<fea::Tie>`. Vector of ties that apply to attach springs of specified stiffness to
     *                                          all nodal degrees of freedom between each set of nodes indicated.
     *
     * @return <B>Summary</B> `fea::Summary`. Summary containing the results of the analysis.
     */
    Summary solve(const Job &job,
                  const std::vector<BC> &BCs,
                  const std::vector<Force> &forces,
                  const std::vector<Tie> &ties,
                  const std::vector<Equation> &equations,
                  const Options &options);
} // namespace fea

#endif // THREED_BEAM_FEA_H
