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

#include "threed_beam_fea.h"
#include <gtest/gtest.h>

using namespace fea;

class beamFEATest : public testing::Test {
protected:
    beamFEATest() : JOB_L_BRACKET(), BCS_L_BRACKET(0), FORCES_L_BRACKET(0),
                    JOB_CANTILEVER(), BCS_CANTILEVER(0), FORCES_CANTILEVER(0),
                    assembleK3D() { };

    virtual void SetUp() {
        using namespace fea;

        // L-bracket setup
        std::vector<double> normal_vec = {0.0, 1.0, 0.0};
        Props PROPS1(10.0, 10.0, 10.0, 10.0, normal_vec);
        Props PROPS2(10.0, 1.0, 1.0, 10.0, normal_vec);

        std::vector<Node> NODES = {Node(0.0, 0.0, 0.0), Node(1.0, 0.0, 0.0), Node(2.0, 0.0, 0.0), Node(2.0, 0.0, 1.0)};
        std::vector<Elem> ELEMS = {Elem(0, 1, PROPS1), Elem(1, 2, PROPS1), Elem(2, 3, PROPS2)};

        JOB_L_BRACKET = Job(NODES, ELEMS);

        BC bc1(0, 0, 0.0);
        BC bc2(0, 1, 0.0);
        BC bc3(0, 2, 0.0);
        BC bc4(0, 3, 0.0);
        BC bc5(0, 4, 0.0);
        BC bc6(0, 5, 0.0);
        BC bc7(3, 1, 0.5);

        BCS_L_BRACKET = {bc1, bc2, bc3, bc4, bc5, bc6, bc7};

        // cantilever setup
        std::vector<double> normal_cantilever = {0.0, 0.0, 1.0};
        Props props_cantilever(1.0, 1.0, 1.0, 1.0, normal_cantilever);
        std::vector<Node> nodes_cantilever = {Node(0.0, 0.0, 0.0),
                                              Node(1.0, 0.0, 0.0)};

        std::vector<Elem> elems_cantilever = {Elem(0, 1, props_cantilever)};

        BCS_CANTILEVER = {BC(0, 0, 0.0),
                          BC(0, 1, 0.0),
                          BC(0, 2, 0.0),
                          BC(0, 3, 0.0),
                          BC(0, 4, 0.0),
                          BC(0, 5, 0.0)};

        FORCES_CANTILEVER = {Force(1, 1, 0.1)};

        JOB_CANTILEVER = Job(nodes_cantilever, elems_cantilever);
    }

    Job JOB_L_BRACKET;
    std::vector<BC> BCS_L_BRACKET;
    std::vector<Force> FORCES_L_BRACKET;

    Job JOB_CANTILEVER;
    std::vector<BC> BCS_CANTILEVER;
    std::vector<Force> FORCES_CANTILEVER;

    GlobalStiffAssembler assembleK3D;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
};

TEST_F(beamFEATest, TransformsLocalToGlobalCoords) {
    // test that identity matrix is recovered if local axes == global axes
    Eigen::Vector3d nx(1.0, 0.0, 0.0);
    Eigen::Vector3d nz(0.0, 0.0, 1.0);

    assembleK3D.calcAelem(nx, nz);

    LocalMatrix Aelem = assembleK3D.getAelem();

    LocalMatrix expected;
    expected.setIdentity();

    for (size_t i = 0; i < Aelem.rows(); ++i) {
        EXPECT_DOUBLE_EQ(expected(i), Aelem(i));
    }
}

TEST_F(beamFEATest, AssemblesElementalStiffness) {

    assembleK3D.calcKelem(0, JOB_L_BRACKET);
    LocalMatrix Kelem = assembleK3D.getKelem();

    std::vector<std::vector<double>> expected =
            {{10.,  0.,    0.,    0.,   0.,   0.,   -10., 0.,    0.,    0.,   0.,   0.},
             {0.,   120.,  0.,    0.,   0.,   60.,  0.,   -120., 0.,    0.,   0.,   60.},
             {0.,   0.,    120.,  0.,   -60., 0.,   0.,   0.,    -120., 0.,   -60., 0.},
             {0.,   0.,    0.,    10.,  0.,   0.,   0.,   0.,    0.,    -10., 0.,   0.},
             {0.,   0.,    -60.,  0.,   40.,  0.,   0.,   0.,    60.,   0.,   20.,  0.},
             {0.,   60.,   0.,    0.,   0.,   40.,  0.,   -60.,  0.,    0.,   0.,   20.},
             {-10., 0.,    0.,    0.,   0.,   0.,   10.,  0.,    0.,    0.,   0.,   0.},
             {0.,   -120., 0.,    0.,   0.,   -60., 0.,   120.,  0.,    0.,   0.,   -60.},
             {0.,   0.,    -120., 0.,   60.,  0.,   0.,   0.,    120.,  0.,   60.,  0.},
             {0.,   0.,    0.,    -10., 0.,   0.,   0.,   0.,    0.,    10.,  0.,   0.},
             {0.,   0.,    -60.,  0.,   20.,  0.,   0.,   0.,    60.,   0.,   40.,  0.},
             {0.,   60.,   0.,    0.,   0.,   20.,  0.,   -60.,  0.,    0.,   0.,   40.}};

    for (size_t i = 0; i < expected.size(); i++) {
        for (size_t j = 0; j < expected[i].size(); j++) {
            EXPECT_DOUBLE_EQ(expected[i][j], Kelem(i, j));
        }
    }
}

TEST_F(beamFEATest, AssemblesGlobalStiffness) {

    unsigned int dofs_per_elem = 6;

    // calculate size of global stiffness matrix and force vector
    size_t size = dofs_per_elem * JOB_L_BRACKET.nodes.size() + FORCES_L_BRACKET.size();

    // construct global stiffness matrix and force vector
    SparseMat Kg(size, size);

    std::vector<Tie> ties;

    assembleK3D(Kg, JOB_L_BRACKET, ties);

    GlobalStiffMatrix KgDense(Kg);

    std::vector<std::vector<double> > expected =
            {{10.,  0.,    0.,    0.,   0.,   0.,   -10., 0.,    0.,    0.,   0.,   0.,   0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,   0.,   0.,  0.,  0.},
             {0.,   120.,  0.,    0.,   0.,   60.,  0.,   -120., 0.,    0.,   0.,   60.,  0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,   0.,   0.,  0.,  0.},
             {0.,   0.,    120.,  0.,   -60., 0.,   0.,   0.,    -120., 0.,   -60., 0.,   0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,   0.,   0.,  0.,  0.},
             {0.,   0.,    0.,    10.,  0.,   0.,   0.,   0.,    0.,    -10., 0.,   0.,   0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,   0.,   0.,  0.,  0.},
             {0.,   0.,    -60.,  0.,   40.,  0.,   0.,   0.,    60.,   0.,   20.,  0.,   0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,   0.,   0.,  0.,  0.},
             {0.,   60.,   0.,    0.,   0.,   40.,  0.,   -60.,  0.,    0.,   0.,   20.,  0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,   0.,   0.,  0.,  0.},
             {-10., 0.,    0.,    0.,   0.,   0.,   20.,  0.,    0.,    0.,   0.,   0.,   -10., 0.,    0.,    0.,   0.,   0.,   0.,   0.,   0.,   0.,  0.,  0.},
             {0.,   -120., 0.,    0.,   0.,   -60., 0.,   240.,  0.,    0.,   0.,   0.,   0.,   -120., 0.,    0.,   0.,   60.,  0.,   0.,   0.,   0.,  0.,  0.},
             {0.,   0.,    -120., 0.,   60.,  0.,   0.,   0.,    240.,  0.,   0.,   0.,   0.,   0.,    -120., 0.,   -60., 0.,   0.,   0.,   0.,   0.,  0.,  0.},
             {0.,   0.,    0.,    -10., 0.,   0.,   0.,   0.,    0.,    20.,  0.,   0.,   0.,   0.,    0.,    -10., 0.,   0.,   0.,   0.,   0.,   0.,  0.,  0.},
             {0.,   0.,    -60.,  0.,   20.,  0.,   0.,   0.,    0.,    0.,   80.,  0.,   0.,   0.,    60.,   0.,   20.,  0.,   0.,   0.,   0.,   0.,  0.,  0.},
             {0.,   60.,   0.,    0.,   0.,   20.,  0.,   0.,    0.,    0.,   0.,   80.,  0.,   -60.,  0.,    0.,   0.,   20.,  0.,   0.,   0.,   0.,  0.,  0.},
             {0.,   0.,    0.,    0.,   0.,   0.,   -10., 0.,    0.,    0.,   0.,   0.,   22.,  0.,    0.,    0.,   6.,   0.,   -12., 0.,   0.,   0.,  6.,  0.},
             {0.,   0.,    0.,    0.,   0.,   0.,   0.,   -120., 0.,    0.,   0.,   -60., 0.,   132.,  0.,    -6.,  0.,   -60., 0.,   -12., 0.,   -6., 0.,  0.},
             {0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,    -120., 0.,   60.,  0.,   0.,   0.,    130.,  0.,   60.,  0.,   0.,   0.,   -10., 0.,  0.,  0.},
             {0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,    0.,    -10., 0.,   0.,   0.,   -6.,   0.,    14.,  0.,   0.,   0.,   6.,   0.,   2.,  0.,  0.},
             {0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,    -60.,  0.,   20.,  0.,   6.,   0.,    60.,   0.,   44.,  0.,   -6.,  0.,   0.,   0.,  2.,  0.},
             {0.,   0.,    0.,    0.,   0.,   0.,   0.,   60.,   0.,    0.,   0.,   20.,  0.,   -60.,  0.,    0.,   0.,   50.,  0.,   0.,   0.,   0.,  0.,  -10.},
             {0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,    0.,    0.,   0.,   0.,   -12., 0.,    0.,    0.,   -6.,  0.,   12.,  0.,   0.,   0.,  -6., 0.},
             {0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,    0.,    0.,   0.,   0.,   0.,   -12.,  0.,    6.,   0.,   0.,   0.,   12.,  0.,   6.,  0.,  0.},
             {0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,    -10.,  0.,   0.,   0.,   0.,   0.,   10.,  0.,  0.,  0.},
             {0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,    0.,    0.,   0.,   0.,   0.,   -6.,   0.,    2.,   0.,   0.,   0.,   6.,   0.,   4.,  0.,  0.},
             {0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,    0.,    0.,   0.,   0.,   6.,   0.,    0.,    0.,   2.,   0.,   -6.,  0.,   0.,   0.,  4.,  0.},
             {0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,    0.,    0.,   0.,   0.,   0.,   0.,    0.,    0.,   0.,   -10., 0.,   0.,   0.,   0.,  0.,  10.}};

    for (size_t i = 0; i < expected.size(); i++) {
        for (size_t j = 0; j < expected[i].size(); j++) {
            EXPECT_DOUBLE_EQ(expected[i][j], KgDense(i, j));
        }
    }
}

TEST_F(beamFEATest, CorrectNodalDisplacementsNoTies) {
    std::vector<Tie> ties;
    Summary summary = solve(JOB_L_BRACKET, BCS_L_BRACKET, FORCES_L_BRACKET, ties, Options());

    // the first 4 rows check nodal displacements
    // the last row is associated with the reaction
    // forces due to enforcing the BCs.
    std::vector<std::vector<double> > expected = {{0., 0.,                  0., 0.,      0., 0.},
                                                  {0., 0.0520833333333333,  0., -0.0625, 0., 0.09375},
                                                  {0., 0.16666666666666666, 0., -0.125,  0., 0.125},
                                                  {0., 0.5,                 0., -0.4375, 0., 0.125},
                                                  {0., 0.625,               0., -0.625,  0., 1.25, -0.625}};

    for (size_t i = 0; i < summary.nodal_displacements.size(); ++i) {
        for (size_t j = 0; j < summary.nodal_displacements[i].size(); ++j) {
            EXPECT_NEAR(expected[i][j], summary.nodal_displacements[i][j], 1.e-10);
        }
    }
}

// In this test I create the same mesh as above with 1 exception
// there are redundant nodes at (1,0,0) that I tie with extremely
// stiff linear and rotational springs. If the tie constraints are
// functioning properly, the ties should behave the same as having
// rigid nodal joints, at least to several decimal places.
TEST_F(beamFEATest, CorrectNodalDisplacementsWithStiffTies) {
    std::vector<double> normal_vec = {0.0, 1.0, 0.0};
    Props props1(10.0, 10.0, 10.0, 10.0, normal_vec);
    Props props2(10.0, 1.0, 1.0, 10.0, normal_vec);

    std::vector<Node> nodes = {Node(0.0, 0.0, 0.0),
                               Node(1.0, 0.0, 0.0),
                               Node(1.0, 0.0, 0.0),
                               Node(2.0, 0.0, 0.0),
                               Node(2.0, 0.0, 1.0)};
    std::vector<Elem> elems = {Elem(0, 1, props1), Elem(2, 3, props1), Elem(3, 4, props2)};

    Job job_tie(nodes, elems);

    BC bc1(0, 0, 0.0);
    BC bc2(0, 1, 0.0);
    BC bc3(0, 2, 0.0);
    BC bc4(0, 3, 0.0);
    BC bc5(0, 4, 0.0);
    BC bc6(0, 5, 0.0);
    BC bc7(4, 1, 0.5);

    std::vector<BC> bcs = {bc1, bc2, bc3, bc4, bc5, bc6, bc7};

    std::vector<Tie> ties = {Tie(1, 2, 1.e8, 1.e8)};

    std::vector<Force> forces;

    Summary summary = solve(job_tie, bcs, forces, ties, Options());

    // The verification program used to generate expected values outputs data as floats
    std::vector<std::vector<double> > expected = {{0., 0.,                  0., 0.,      0., 0.},
                                                  {0., 0.0520833333333333,  0., -0.0625, 0., 0.09375},
                                                  {0., 0.0520833333333333,  0., -0.0625, 0., 0.09375},
                                                  {0., 0.16666666666666666, 0., -0.125,  0., 0.125},
                                                  {0., 0.5,                 0., -0.4375, 0., 0.125},
                                                  {0., 0.625,               0., -0.625,  0., 1.25, -0.625}};

    for (size_t i = 0; i < summary.nodal_displacements.size(); ++i) {
        for (size_t j = 0; j < summary.nodal_displacements[i].size(); ++j) {
            EXPECT_NEAR(expected[i][j], summary.nodal_displacements[i][j], 1.e-7);
        }


    }
}

// This is simple cantilever beam with a point load at the
// free end. This checks the end displacements match the analytical results.
TEST_F(beamFEATest, CorrectTipDisplacementCantileverBeam) {

    std::vector<Tie> ties;

    Summary summary = solve(JOB_CANTILEVER, BCS_CANTILEVER, FORCES_CANTILEVER, ties, Options());

    // the first row checks nodal displacements of fixed node are zero
    // the second row checks the analytical result for tip displacement
    // for the given load in the y-direction of 0.01 yields the correct
    // displacement and rotation
    std::vector<std::vector<double> > expected = {{0., 0.,                   0., 0.,  0.,  0.},
                                                  {0., 0.033333333333333333, 0., 0.0, 0.0, 0.05}};

    for (size_t i = 0; i < summary.nodal_displacements.size(); ++i) {
        for (size_t j = 0; j < summary.nodal_displacements[i].size(); ++j)
                EXPECT_DOUBLE_EQ(expected[i][j], summary.nodal_displacements[i][j]);
    }
}

// This test displaces a cantilever beam axially and
// transverse to the beam axis. Nodal forces are
// compared to the analytical result.
TEST_F(beamFEATest, CorrectTipForcesCantileverBeam) {

    std::vector<Tie> ties;
    std::vector<Force> forces;
    std::vector<BC> bcs = BCS_CANTILEVER;
    bcs.push_back(BC(1, 0, 0.1));
    bcs.push_back(BC(1, 1, 0.1));

    Options opts;
    opts.save_report = true;
    opts.save_nodal_forces = true;
    opts.save_nodal_displacements = true;

    Summary summary = solve(JOB_CANTILEVER, bcs, forces, ties, opts);

    std::vector<std::vector<double> > expected = {{-0.1, -0.3, 0., 0.,  0.,  -0.3},
                                                  {0.1,  0.3,  0., 0.0, 0.0, 0.0}};

    for (size_t i = 0; i < summary.nodal_forces.size(); ++i) {
        for (size_t j = 0; j < summary.nodal_forces[i].size(); ++j)
                EXPECT_DOUBLE_EQ(expected[i][j], summary.nodal_forces[i][j]);
    }
}


// This tests the tie constraints correctly deform.
// I have extremely stiff elements and apply a
// displacement to the end node. It is expected
// that the tie will accommodate all the deformation.
TEST_F(beamFEATest, CorrectDisplacementWeakTies) {

    std::vector<double> normal_vec = {0.0, 1.0, 0.0};

    Props props(1.e9, 1.e9, 1.e9, 1.e9, normal_vec);


    std::vector<Node> nodes = {Node(0.0, 0.0, 0.0),
                               Node(1.0, 0.0, 0.0),
                               Node(1.0, 0.0, 0.0),
                               Node(2.0, 0.0, 0.0)};

    std::vector<Elem> elems = {Elem(0, 1, props), Elem(2, 3, props)};

    Job job_tie(nodes, elems);

    BC bc1(0, 0, 0.0);
    BC bc2(0, 1, 0.0);
    BC bc3(0, 2, 0.0);
    BC bc4(0, 3, 0.0);
    BC bc5(0, 4, 0.0);
    BC bc6(0, 5, 0.0);
    BC bc7(3, 0, 0.5);

    std::vector<BC> bcs = {bc1, bc2, bc3, bc4, bc5, bc6, bc7};

    std::vector<Tie> ties = {Tie(1, 2, 0.01, 0.01)};

    std::vector<Force> forces;

    Options opts;
    opts.epsilon = 1e-10;

    Summary summary = solve(job_tie, bcs, forces, ties, opts);

    std::vector<std::vector<double> > expected = {{0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                                  {0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                                  {0.5, 0.0, 0.0, 0.0, 0.0, 0.0},
                                                  {0.5, 0.0, 0.0, 0.0, 0.0, 0.0}};

    for (size_t i = 0; i < summary.nodal_displacements.size(); ++i) {
        for (size_t j = 0; j < summary.nodal_displacements[i].size(); ++j) {
            EXPECT_NEAR(expected[i][j], summary.nodal_displacements[i][j], 1e-10);
        }
    }
}

TEST_F(beamFEATest, CorrectForcesWeakTies) {

    std::vector<double> normal_vec = {0.0, 1.0, 0.0};

    Props props(1.e9, 1.e9, 1.e9, 1.e9, normal_vec);


    std::vector<Node> nodes = {Node(0.0, 0.0, 0.0),
                               Node(1.0, 0.0, 0.0),
                               Node(1.0, 0.0, 0.0),
                               Node(2.0, 0.0, 0.0)};

    std::vector<Elem> elems = {Elem(0, 1, props), Elem(2, 3, props)};

    Job job_tie(nodes, elems);

    BC bc1(0, 0, 0.0);
    BC bc2(0, 1, 0.0);
    BC bc3(0, 2, 0.0);
    BC bc4(0, 3, 0.0);
    BC bc5(0, 4, 0.0);
    BC bc6(0, 5, 0.0);
    BC bc7(3, 0, 0.5);
    BC bc8(2, 3, 0.5);

    std::vector<BC> bcs = {bc1, bc2, bc3, bc4, bc5, bc6, bc7, bc8};

    std::vector<Tie> ties = {Tie(1, 2, 0.01, 0.01)};

    std::vector<Force> forces;

    Options opts;
    opts.epsilon = 1e-10;

    Summary summary = solve(job_tie, bcs, forces, ties, opts);

    std::vector<std::vector<double> > expected = {{0.005, 0.0, 0.0, 0.005, 0.0, 0.0}};

    for (size_t i = 0; i < summary.tie_forces.size(); ++i) {
        for (size_t j = 0; j < summary.tie_forces[i].size(); ++j) {
            EXPECT_NEAR(expected[i][j], summary.tie_forces[i][j], 1e-13);
        }
    }
}
