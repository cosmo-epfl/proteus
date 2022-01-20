/**
 * file   test_rascal_utils.cc
 *
 * @author  Alexander Goscinski <alexander.goscinski@epfl.ch>
 *
 * @date   14 Oct 2019
 *
 * @brief Tests the implementation of interpolator
 *
 * Copyright © 2019  Alexander Goscinski, COSMO (EPFL), LAMMM (EPFL)
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

#include "rascal/representations/calculator_spherical_expansion.hh"
#include "rascal/utils/utils.hh"

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

namespace rascal {

  // TODO(all) not sure about the naming convention of tests, camelcase ...
  BOOST_AUTO_TEST_SUITE(RascalUtilityTests);

  using internal::AtomicSmearingType;
  using internal::OptimizationType;
  using internal::RadialBasisType;

  /*
   * enum class RadialBasisType { GTO, DVR, End_ };
   * enum class AtomicSmearingType { Constant, PerSpecies, Radial, End_ };
   * enum class OptimizationType { None, Spline, End_ };
   *
   * enum value tuple  ->  combined key
   *    (0,0,0)                 0
   *    (1,0,0)                 1
   *    (0,1,0)                 2
   *    (1,1,0)                 3
   *    (0,2,0)                 4
   *    (1,2,0)                 5
   *    (0,0,1)                 6
   *    (1,0,1)                 7
   *    (0,1,1)                 8
   *    (1,1,1)                 9
   *    (0,2,1)                10
   *    (1,2,1)                11
   */
  BOOST_AUTO_TEST_CASE(combine_enums_test) {
    BOOST_CHECK_EQUAL(combine_enums(RadialBasisType::GTO,
                                    AtomicSmearingType::Constant,
                                    OptimizationType::None),
                      0);

    BOOST_CHECK_EQUAL(combine_enums(RadialBasisType::DVR,
                                    AtomicSmearingType::PerSpecies,
                                    OptimizationType::None),
                      3);
    BOOST_CHECK_EQUAL(combine_enums(RadialBasisType::DVR,
                                    AtomicSmearingType::Constant,
                                    OptimizationType::Spline),
                      7);
    BOOST_CHECK_EQUAL(combine_enums(RadialBasisType::DVR,
                                    AtomicSmearingType::Radial,
                                    OptimizationType::Spline),
                      11);
  }
  BOOST_AUTO_TEST_SUITE_END();
}  // namespace rascal
