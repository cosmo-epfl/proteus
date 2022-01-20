/**
 * @file   test_lattice.hh
 *
 * @author  Felix Musil <felix.musil@epfl.ch>
 *
 * @date   05 Apr 2018
 *
 * @brief Test implementation of lattice.cc
 *
 * Copyright  2018  Felix Musil, COSMO (EPFL), LAMMM (EPFL)
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

#ifndef TESTS_TEST_LATTICE_HH_
#define TESTS_TEST_LATTICE_HH_

#include "rascal/structure_managers/atomic_structure.hh"
#include "rascal/structure_managers/lattice.hh"

namespace rascal {

  // TODO(markus): make dimension dependent
  // short hands for types, which were deleted from basic types
  constexpr static auto Dim{3};
  using Vec3_t = Eigen::Matrix<double, Dim, 1>;
  using Cell_t = typename AtomicStructure<Dim>::Cell_t;

  struct ManagerFixtureLattice {
    ManagerFixtureLattice() {
      Cell_t cell;
      // clang-format off
      cell << 6.19, 2.41, 0.21,
              0.00, 6.15, 1.02,
              0.00, 0.00, 7.31;
      // clang-format on
      lattice.set_cell(cell);
    }

    ~ManagerFixtureLattice() {}

    Lattice<Dim> lattice{};
  };
}  // namespace rascal

#endif  // TESTS_TEST_LATTICE_HH_
