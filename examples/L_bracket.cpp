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

#include <iostream>
#include "threed_beam_fea.h"

int main(int argc, char *argv[])
{
    using namespace fea;

    // define the vector perpendicular to the beam elements
    // in general you will need 1 normal vector per element
    std::vector<double> normal_vec = {0.0, 1.0, 0.0};

    // set up the properties for the elements
    double EA = 10.0; // extensional stiffness
    double EIz = 10.0; // bending stiffness along z-axis
    double EIy = 10.0; // bending stiffness along y-axis
    double GJ = 10.0; // torsional stiffness

    Props props1(EA, EIz, EIy, GJ, normal_vec);

    // make the second element's bending stiffness lower
    Props props2(EA, 0.1 * EIz, 0.1 * EIy, GJ, normal_vec);

    // define the (x, y, z) coordinate of the nodes
    std::vector<Node> nodes = {Node(0.0, 0.0, 0.0), Node(1.0, 0.0, 0.0), Node(2.0, 0.0, 0.0), Node(2.0, 0.0, 1.0)};

    // define which nodes are connected to form elements
    std::vector<Elem> elems = {Elem(0, 1, props1), Elem(1, 2, props1), Elem(2, 3, props2)};

    // assemble nodes and elements into a Job for analysis
    Job job(nodes, elems);

    // define boundary conditions for the mesh
    BC bc1(0, 0, 0.0);
    BC bc2(0, 1, 0.0);
    BC bc3(0, 2, 0.0);
    BC bc4(0, 3, 0.0);
    BC bc5(0, 4, 0.0);
    BC bc6(0, 5, 0.0);
    BC bc7(3, 1, 0.5);

    std::vector<BC> bcs = {bc1, bc2, bc3, bc4, bc5, bc6, bc7};

    // initialize empty vector of ties
    std::vector<Tie> ties;

    // initialize empty vector of equation constraints
    std::vector<Equation> equations;

    // initialize vector of prescribed forces
    std::vector<Force> forces;

    // use default options
    Options opts;

    // solve for nodal displacements
    Summary summary = solve(job, bcs, forces, ties, equations, opts);

    // write report to terminal
    std::cout << summary.FullReport() << std::endl;
	return 0;
}
