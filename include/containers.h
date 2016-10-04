/*!
* \file containers.h
*
* \author Ryan Latture
* \date 8-12-15
*
* Contains the structs used to organize the job, BCs, and ties for 3D beam FEA.
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

#ifndef FEA_CONTAINERS_H
#define FEA_CONTAINERS_H

#include <vector>
#include <Eigen/Core>

namespace fea {

    /**
     * @brief A node that describes a mesh. Uses Eigen's predefined Vector class for added functionality.
     * @details See the Eigen documentation on the Vector3d class for more options of what can be done with `Nodes`. \n
     * Examples of constucting a `Node` at \f$(x, y, z)=(0,1,2)\f$:
     * @code
     * // specify values on constuction
     * fea::Node n1(1.0, 2.0, 3.0);
     *
     * // construct a Node then insert values
     * fea::Node n2;
     * n2 << 0.0, 1.0, 2.0;
     * @endcode
     */
    typedef Eigen::Vector3d Node;

    /**
     * @brief A boundary condition to enforce.
     * @details Set by specifying the node to constrain, which degree of freedom, and the value to hold the node at.
     *
     * @code
     * // define the node number to constrain
     * unsigned int nn1 = 0;
     * // define the value to hold the nodal DOF at
     * double value = 0.0;
     * fea::BC bc(nn1, fea::DOF::DISPLACEMENT_X, value);
     * @endcode
     */
    struct BC {
        unsigned int node;/**<The index of the node to constrain*/

        /**
         * The index of the dof to constrain. The fea::DOF enum can be used for specification or the integer
         * values can be used directly `0==d_x`, `1==d_y`, ...
         */
        unsigned int dof;

        double value;/**<The value to hold the dof at.*/

        /**
         * @brief Default Constructor
         * @details All values are set to zero.
         */
        BC() : node(0), dof(0), value(0) { };

        /**
         * @brief Constructor
         * @param[in] node `unsigned int`. The index of the node.
         * @param[in] dof `unsigned int`. Degree of freedom to constrain (See fea::DOF).
         * @param[in] value `double`. The prescribed value for the boundary condition.
         */
        BC(unsigned int _node, unsigned int _dof, double _value) : node(_node), dof(_dof), value(_value) { };
    };

    /**
     * @brief A nodal force to enforce.
     * @details Set by specifying the node where the force will be applied, which degree of freedom will be affected,
     * and the value to apply.
     *
     * @code
     * // define the node number to constrain
     * unsigned int nn1 = 0;
     * // define the value to hold the nodal DOF at
     * double value = 0.0;
     * fea::Force force(nn1, fea::DOF::DISPLACEMENT_X, value);
     * @endcode
    */
    struct Force {
        unsigned int node;/**<The index of the node to apply the force*/

        /**
         * The index of the dof to constrain. The fea::DOF enum can be used for specification or the integer
         * values can be used directly `0==d_x`, `1==d_y`, ...
         */
        unsigned int dof;

        double value;/**<The value of the force to apply.*/

        /**
        * @brief Default Constructor
        * @details All values are set to zero.
        */
        Force() : node(0), dof(0), value(0) { };

        /**
         * @brief Constructor
         * @param[in] node `unsigned int`. The index of the node.
         * @param[in] dof `unsigned int`. Degree of freedom to constrain (See fea::DOF).
         * @param[in] value `double`. The prescribed value for the force.
         */
        Force(unsigned int _node, unsigned int _dof, double _value) : node(_node), dof(_dof), value(_value) { };
    };

    /**
     * @brief The set of properties associated with an element.
     * @details The properties must define the extensional stiffness, \f$EA\f$, bending stiffness parallel to the local z-axis \f$EI_{z}\f$,
     * bending stiffness parallel to the local y-axis\f$EI_{y}\f$, the torsional stiffness, \f$GJ\f$, and a vector pointing along the beam elements local y-axis.
     *
     * @code
     * double EA = 1000.0;
     * double EIz = 100.0;
     * double EIy = 100.0;
     * double GJ = 200.0;
     * std::vector<double> normal_vec = {0.0, 0.0, 1.0};
     * fea::Props props(EA, EIz, EIy, GJ, normal_vec);
     * @endcode
     */
    struct Props {
        double EA;/**<Extensional stiffness.*/
        double EIz;/**<Bending stiffness parallel to local z-axis.*/
        double EIy;/**<Bending stiffness parallel to local y-axis.*/
        double GJ;/**<Torsional stiffness.*/
        Eigen::Vector3d normal_vec;/**<Vector normal to element (size==3). Direction should be parallel to the beam element's local y-axis.*/

        Props() : EA(0), EIz(0), EIy(0), GJ(0) { };/**<Default constuctor*/

        /**
         * @brief Constructor
         * @details Allows properties to be set upon initialization.
         *
         * @param[in] EA double. Extensional stiffness.
         * @param[in] EIz double. Bending stiffness parallel to z-axis.
         * @param[in] EIy double. Bending stiffness parallel to y-axis.
         * @param[in] GJ double. Torsional stiffness.
         * @param[in] normal_vec std::vector<double>. Vector normal to element (`normal_vec.size()==3`). Direction should be parallel to the beam element's local y-axis.
         */
        Props(double _EA, double _EIz, double _EIy, double _GJ, const std::vector<double> &_normal_vec)
                : EA(_EA), EIz(_EIz), EIy(_EIy), GJ(_GJ) {
            normal_vec << _normal_vec[0], _normal_vec[1], _normal_vec[2];
        };
    };

    /**
     * @brief Places linear springs between all degrees of freedom of 2 nodes.
     * @details To form a tie specify the 2 nodes that will be linked as well as the spring constants for translational and rotational degrees of freedom.
     * All translational degrees of freedom will be assigned the same spring constant.
     * The same is true for rotational degrees of freedom, although the spring constant does not have to be the same as that used for the translational DOFs.
     * @code
     * // create the tie between node2 and node3
     * unsigned int nn1 = 1; // i.e. the second node in the node list
     * unsigned int nn2 = 2; // i.e. the third node in the node list
     *
     * // define the spring constant for x, y, and z translational DOFs
     * double lmult = 100.0;
     *
     * // define the spring constant for x, y, and z rotational DOFs
     * double rmult = 100.0;
     *
     * // form the tie
     * fea::Tie tie1(nn1, nn2, lmult, rmult);
     * @endcode
     */
    struct Tie {
        unsigned int node_number_1;/**<The first element's index involved in the constraint.*/
        unsigned int node_number_2;/**<The second element's index involved in the constraint.*/
        double lmult;/**<multiplier for the linear spring.*/
        double rmult; /**<multiplier for the rotational spring.*/

        /**
        * @brief Default Constructor
        * @details All member variables are set to 0.
        */
        Tie() : node_number_1(0), node_number_2(0), lmult(0), rmult(0) { };

        /**
         * @brief Constructor
         * @param[in] node_number_1 `unsigned int`. Index of the first node.
         * @param[in] node_number_2 `unsigned int`. Index of the second node.
         * @param[in] lmult `double`. Spring constant for the translational degrees of freedom.
         * @param[in] rmult `double`. Spring constant for the rotational degrees of freedom.
         */
        Tie(unsigned int _node_number_1, unsigned int _node_number_2, double _lmult, double _rmult) :
                node_number_1(_node_number_1), node_number_2(_node_number_2), lmult(_lmult), rmult(_rmult) { };
    };

    /**
     * @brief A linear multipoint constraint.
     * @details Equation constraints are defined by a series of terms that
     * sum to zero, e.g. `t1 + t2 + t3 ... = 0`, where `tn` is the `n`th term.
     * Each term specifies the node number, degree of freedom and coefficient.
     * The node number and degree of freedom specify which nodal variable
     * (either nodal displacement or rotation) is involved with the equation
     * constraint, and coefficient is multiplied by the specified nodal variable
     * when forming the equation. Note, the equation sums to zero, so in order
     * to specify that 2 nodal degrees of freedom are equal their coefficients
     * should be equal and opposite.
     *
     * @code
     * // Create an empty equation
     * fea::Equation eqn;
     *
     * // Stipulate that the x and y displacement for the first node must be equal
     * unsigned int node_number = 0;
     * eqn.terms.push_back(fea::Equation::Term(node_number, fea::DOF::DISPLACEMENT_X, 1.0));
     * eqn.terms.push_back(fea::Equation::Term(node_number, fea::DOF::DISPLACEMENT_Y, -1.0);
     * @endcode
     */
    struct Equation {

      /**
       * @brief A single term in the equation constraint.
       * @details Each term defines the node number, degree of freedom and
       * coefficient.
       */
      struct Term {
        unsigned int node_number;/**<Index of the node in the node list.*/
        unsigned int dof;/**<Degree of freedom. @sa fea::DOF*/
        double coefficient;/**<Coefficient to multiply the nodal variable by.*/

        /**
         * Default constructor
         */
        Term() : node_number(0), dof(0), coefficient(0) {};

        /**
         * @brief Constructor
         * @param node_number. `unsigned int`. Index of the node within the node list.
         * @param dof. `unsigned int`. Degree of freedom for the specified node.
         * @param coefficient. `double`. coefficient to multiply the corresponding nodal variable by.
         */
        Term(unsigned int _node_number, unsigned int _dof, double _coefficient) :
            node_number(_node_number), dof(_dof), coefficient(_coefficient) {};
      };

      std::vector<Term> terms;/**<A list of terms that sum to zero.*/

      /**
       * Default constructor
       */
      Equation() : terms(0) {};

      /**
       * @brief Constructor
       * @param terms. `std::vector<Term>`. A list of terms that sum to zero.k
       */
      Equation(const std::vector<Term> &_terms) : terms(_terms) {};
    };

    /**
     * @brief An element of the mesh. Contains the indices of the two `fea::Node`'s that form the element as well
     * as the properties of the element given by the `fea::Props` struct.
     */
    struct Elem {
        Eigen::Vector2i node_numbers;/**<The indices of the node list that define the element.*/
        Props props;/**<The set of properties to associate with the element.*/

        /**
        * @brief Default Constructor
        */
        Elem() { };

        /**
         * @brief Constructor
         * @details Constructor if the node indices are passed independently. Assumes 2 nodes per element.
         *
         * @param[in] node1 unsigned int. The indices of first node associate with the element.
         * @param[in] node2 unsigned int. The indices of second node associate with the element.
         * @param[in] props Props. The element's properties.
         */
        Elem(unsigned int node1, unsigned int node2, const Props &_props) : props(_props) {
            node_numbers << node1, node2;
        }
    };

    /**
     * @brief Contains a node list, element list, and the properties of each element.
     */
    struct Job {
        std::vector<Node> nodes;/**<A vector of Node objects that define the mesh.*/
        std::vector<Eigen::Vector2i> elems;/**<A 2D vector of ints that defines the connectivity of the node list.*/
        std::vector<Props> props;/**<A vector that contains the properties of each element.*/

        /**
         * @brief Default constructor
         */
        Job() : nodes(0), elems(0), props(0) { };

        /**
         * @brief Constructor
         * @details Takes an list of nodes and elements as inputs. The elements are deconstructed into
         * an array holding the connectivity list and an array holding the properties.
         *
         * @param[in] nodes std::vector<Node>. The node list that defines the mesh.
         * @param[in] elems std::vector<Elem>. The elements that define the mesh.
         *                  An element is defined by the connectivity list and the associated properties.
         */
        Job(const std::vector<Node> &_nodes, const std::vector<Elem> _elems) : nodes(_nodes) {
            unsigned int num_elems = _elems.size();
            elems.reserve(num_elems);
            props.reserve(num_elems);

            for (unsigned int i = 0; i < num_elems; i++) {
                elems.push_back(_elems[i].node_numbers);
                props.push_back(_elems[i].props);
            }
        };
    };

    /**
     * @brief Convenience enumerator for specifying the active degree of freedom in a constraint.
     */
    enum DOF {
        /**
         * Displacement along the global x-axis.
         */
        DISPLACEMENT_X,

        /**
         * Displacement along the global y-axis.
         */
        DISPLACEMENT_Y,

        /**
         * Displacement along the global z-axis.
         */
        DISPLACEMENT_Z,

        /**
         * Rotation about the global x-axis.
         */
        ROTATION_X,

        /**
         * Rotation about the global y-axis.
         */
        ROTATION_Y,

        /**
         * Rotation about the global z-axis.
         */
        ROTATION_Z,
        /**
         * Number of degrees of freedom per node.
         */
        NUM_DOFS
    };

} // namespace fea

#endif // FEA_CONTAINERS_H
