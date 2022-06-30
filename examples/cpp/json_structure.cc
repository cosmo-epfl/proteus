/**
 * @file   examples/json_structure.cc
 *
 * @author Felix Musil <felix.musil@epfl.ch>
 *
 * @date   14 Aug 2019
 *
 * @brief Example for constructing atomic structures and StructureManagers from
 *        JSON
 *
 * Copyright  2019 Felix Musil, COSMO (EPFL), LAMMM (EPFL)
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

#include "rascal/representations/calculator_sorted_coulomb.hh"
#include "rascal/representations/calculator_spherical_expansion.hh"
#include "rascal/representations/calculator_spherical_invariants.hh"
#include "rascal/structure_managers/adaptor_neighbour_list.hh"
#include "rascal/structure_managers/adaptor_strict.hh"
#include "rascal/structure_managers/make_structure_manager.hh"
#include "rascal/structure_managers/structure_manager_centers.hh"
#include "rascal/utils/basic_types.hh"
#include "rascal/utils/json_io.hh"
#include "rascal/utils/utils.hh"

#include <cmath>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <list>
#include <string>

using namespace rascal;  // NOLINT

using Representation_t = CalculatorSphericalInvariants;

using LayerByOrder = std::index_sequence<0, 0, 1>;

int main() {
  double cutoff{2.};
  json adaptors;
  json ad1{{"name", "AdaptorNeighbourList"},
           {"initialization_arguments", {{"cutoff", cutoff}}}};
  json ad2{{"name", "AdaptorStrict"},
           {"initialization_arguments", {{"cutoff", cutoff}}}};
  adaptors.emplace_back(ad1);
  adaptors.emplace_back(ad2);

  // Convert a JSON nested list to an Eigen matrix
  json j1{{1, 23, 4, 6, 7}, {1, 23, 4, 6, 9}, {4, 23, 4, 6, 9}};
  std::cout << "JSON list:" << std::endl;
  std::cout << j1.dump() << std::endl;
  std::cout << "Converted Eigen matrix:" << std::endl;
  auto mat1 = j1.get<Eigen::MatrixXd>();
  std::cout << mat1 << std::endl;

  // Read in an atomic structure from a JSON file...
  AtomicStructure<3> structure{};
  // std::string
  // filename{"reference_data/inputs/CaCrP2O7_mvc-11955_symmetrized.json"};
  std::string filename{"../reference_data/inputs/alanine-center-select.json"};
  structure.set_structure(filename);

  std::cout << "Structure arrays for " << filename << ":" << std::endl;
  std::cout << structure.atom_types.rows() << ", "
            << structure.atom_types.cols() << std::endl;
  std::cout << structure.cell.rows() << ", " << structure.cell.cols()
            << std::endl;
  std::cout << structure.positions.rows() << ", " << structure.positions.cols()
            << std::endl;
  std::cout << structure.pbc.rows() << ", " << structure.pbc.cols()
            << std::endl;
  std::cout << structure.center_atoms_mask.transpose() << std::endl;
  std::cout << structure.cell << std::endl;
  std::cout << structure.positions << std::endl;

  // ...wrap it in a StructureManagerCenters to enable iteration over centers...
  auto manager = make_structure_manager<StructureManagerCenters>();
  manager->update(structure);

  std::cout << "Number of center atoms: " << manager->get_size() << std::endl;
  std::cout << "Number of atoms in total (centers and ghosts): "
            << manager->get_size_with_ghosts() << std::endl;
  std::cout << "manager iteration over atoms" << std::endl;
  for (auto atom : manager) {
    std::cout << "atom " << atom.get_atom_tag() << " global index "
              << atom.get_global_index() << std::endl;
  }

  // ...and wrap that adaptor in a strict NeighbourList to enable iteration
  // over neighbours.
  auto man = stack_adaptors<StructureManagerCenters, AdaptorNeighbourList,
                            AdaptorStrict>(manager, adaptors);
  std::cout << "manager iteration over (strict) pairs" << std::endl;
  for (auto atom : man) {
    for (auto pair : atom.pairs()) {
      std::cout << "strict pair (" << atom.get_atom_tag() << ", "
                << pair.get_atom_tag() << ") global index "
                << pair.get_global_index() << std::endl;
    }
  }

  std::cout << man->get_name() << std::endl;
  std::cout << extract_underlying_manager<2>(man)->get_name() << std::endl;
  std::cout << extract_underlying_manager<1>(man)->get_name() << std::endl;
  auto man1 = extract_underlying_manager<0>(man);
  std::cout << man1->get_name() << std::endl;

  return (0);
}
