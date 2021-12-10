/**
 * @file   examples/alanine_chain.cc
 *
 * @author Markus Stricker <markus.stricker@epfl.ch>
 *
 * @date   18 Jun 2018
 *
 * @brief Example for NeighbourhoodManagerChain
 *
 * Copyright  2018 Markus Stricker, COSMO (EPFL), LAMMM (EPFL)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "rascal/external/json.hpp"
#include "rascal/structure_managers/adaptor_neighbour_list.hh"
#include "rascal/structure_managers/make_structure_manager.hh"
#include "rascal/structure_managers/property.hh"
#include "rascal/structure_managers/structure_manager_centers.hh"

#include <iostream>
#include <vector>

using Manager_t = rascal::StructureManagerCenters;
using PairManager_t = rascal::AdaptorNeighbourList<Manager_t>;

int main() {
  // integer lists containing the definition of the quadruplets for the
  // calculation of the dihedral angles in the alanine-unit.
  std::vector<int> phi{4, 3, 19, 1};
  std::vector<int> psi{19, 1, 0, 18};
  // put them in a container for iteration
  std::vector<std::vector<int>> quadruplets{};
  quadruplets.push_back(phi);
  quadruplets.push_back(psi);

  // solution vector for the dihedral angles
  std::vector<double> dihedral_angles{};

  // initialize the manager
  auto manager{rascal::make_structure_manager<Manager_t>()};
  double cutoff{2.0};
  /*
   * TODO(markus) bug with cutoff==1.0
   * in adaptor_neighbour_list.hh:992, at some point idx is assigned a
   * coordinate that does not exist leading to a seg fault when trying to
   * access it.
   */

  // read atomic structure from the JSON file
  // manager.read_structure_from_json("../reference_data/inputs/alanine-X-examples.json");
  // manager.update("../reference_data/inputs/alanine-X-examples.json");
  auto pair_manager{rascal::make_adapted_manager<rascal::AdaptorNeighbourList>(
      manager, cutoff)};
  std::string filename{"../reference_data/inputs/alanine-X.json"};
  pair_manager->update(filename);

  // Loop over the defined quadruplets and calculate the respective
  // angles with atan2 and cosine definition.
  for (auto q : quadruplets) {
    auto pos0 = pair_manager->get_position(q[0]);
    auto pos1 = pair_manager->get_position(q[1]);
    auto pos2 = pair_manager->get_position(q[2]);
    auto pos3 = pair_manager->get_position(q[3]);

    auto b1 = pos1 - pos0;
    auto b2 = pos1 - pos2;
    auto b3 = pos3 - pos2;

    auto na = b1.cross(b2);
    auto nb = b2.cross(b3);

    auto arg1 = ((b1.cross(b2)).cross(b2.cross(b3))).dot(b2 / b2.norm());
    auto arg2 = (b1.cross(b2)).dot(b2.cross(b3));

    auto angle = atan2(arg1, arg2);

    dihedral_angles.push_back(angle);

    std::cout << "atan2 " << angle << std::endl;
    std::cout << "cos " << acos(na.dot(nb) / na.norm() / nb.norm())
              << std::endl;
  }

  std::cout << "Dihedral angles in alanine data\n";
  for (auto a : dihedral_angles) {
    std::cout << a << " ";
  }
  std::cout << std::endl;
}  // end main
