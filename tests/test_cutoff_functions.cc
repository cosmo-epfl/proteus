/**
 * @file   test_cutoff_functions.cc
 *
 * @author  Felix Musil <felix.musil@epfl.ch>
 *
 * @date   21 May 2019
 *
 * @brief Test the implementation of cutoff functions
 *
 * Copyright © 2019  Felix Musil, COSMO (EPFL), LAMMM (EPFL)
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

#include "test_math.hh"

#include "rascal/representations/cutoff_functions.hh"

#include <boost/test/unit_test.hpp>

namespace rascal {

  BOOST_AUTO_TEST_SUITE(cutoff_tests);

  /* ---------------------------------------------------------------------- */
  /**
   * Test the gradients of the shifted cosine cutoff function
   */
  BOOST_AUTO_TEST_CASE(shifted_cosine_gradient_test) {
    static const bool verbose{false};
    std::vector<json> fc_hypers{
        R"({
        "type": "ShiftedCosine",
        "cutoff": {"value": 3, "unit": "AA"},
        "smooth_width": {"value": 0.5, "unit": "AA"}
      })"_json,
    };
    using Cutoff_t =
        internal::CutoffFunction<internal::CutoffFunctionType::ShiftedCosine>;

    for (auto & fc_hyper : fc_hypers) {
      if (verbose) {
        std::cout << fc_hyper.dump(2) << std::endl;
      }

      Cutoff_t cutoff(fc_hyper);

      CutoffGradientProvider<Cutoff_t> cutoff_calculator(cutoff);

      GradientTestFixture fix{
          "reference_data/tests_only/cutoff_function_test.json"};
      test_gradients(cutoff_calculator, fix);
    }
  }

  /* ---------------------------------------------------------------------- */
  /**
   * Test the gradients of the radial scaling cutoff function
   */
  BOOST_AUTO_TEST_CASE(radial_scaling_gradient_test) {
    static const bool verbose{false};
    std::vector<json> fc_hypers{
        R"({
        "type": "RadialScaling",
        "cutoff": {"value": 3, "unit": "AA"},
        "smooth_width": {"value": 0.5, "unit": "AA"},
        "rate": {"value": 1, "unit": "AA"},
        "scale": {"value": 2, "unit": "AA"} ,
        "exponent": {"value": 3, "unit": ""}
      })"_json,
        R"({
        "type": "RadialScaling",
        "cutoff": {"value": 3, "unit": "AA"},
        "smooth_width": {"value": 0.5, "unit": "AA"},
        "rate": {"value": 0, "unit": "AA"},
        "scale": {"value": 2, "unit": "AA"} ,
        "exponent": {"value": 3, "unit": ""}
      })"_json,
        R"({
        "type": "RadialScaling",
        "cutoff": {"value": 3, "unit": "AA"},
        "smooth_width": {"value": 0.5, "unit": "AA"},
        "rate": {"value": 0, "unit": "AA"},
        "scale": {"value": 2, "unit": "AA"} ,
        "exponent": {"value": 0, "unit": ""}
      })"_json,
    };
    using Cutoff_t =
        internal::CutoffFunction<internal::CutoffFunctionType::RadialScaling>;

    for (auto & fc_hyper : fc_hypers) {
      if (verbose) {
        std::cout << fc_hyper.dump(2) << std::endl;
      }

      Cutoff_t cutoff(fc_hyper);

      CutoffGradientProvider<Cutoff_t> cutoff_calculator(cutoff);

      GradientTestFixture fix{
          "reference_data/tests_only/cutoff_function_test.json"};
      test_gradients(cutoff_calculator, fix);
    }
  }

  /* ---------------------------------------------------------------------- */
  /**
   * Test if the exponent is zero case (m = 0) is correctly identified in the
   * radial scaling cutoff function
   */
  BOOST_AUTO_TEST_CASE(radial_scaling_exponent_zero_test) {
    static const bool verbose{false};
    std::vector<json> fc_hypers{
        R"({
        "type": "RadialScaling",
        "cutoff": {"value": 3, "unit": "AA"},
        "smooth_width": {"value": 0.5, "unit": "AA"},
        "rate": {"value": 1, "unit": "AA"},
        "scale": {"value": 1, "unit": "AA"} ,
        "exponent": {"value": 0, "unit": ""}
      })"_json,
        R"({
        "type": "RadialScaling",
        "cutoff": {"value": 3, "unit": "AA"},
        "smooth_width": {"value": 0.5, "unit": "AA"},
        "rate": {"value": 0, "unit": "AA"},
        "scale": {"value": 0, "unit": "AA"} ,
        "exponent": {"value": 0, "unit": ""}
      })"_json,
        R"({
        "type": "RadialScaling",
        "cutoff": {"value": 3, "unit": "AA"},
        "smooth_width": {"value": 0.5, "unit": "AA"},
        "rate": {"value": 0, "unit": "AA"},
        "scale": {"value": 1, "unit": "AA"} ,
        "exponent": {"value": 0, "unit": ""}
      })"_json,
        R"({
        "type": "RadialScaling",
        "cutoff": {"value": 3, "unit": "AA"},
        "smooth_width": {"value": 0.5, "unit": "AA"},
        "rate": {"value": 1, "unit": "AA"},
        "scale": {"value": 0, "unit": "AA"} ,
        "exponent": {"value": 0, "unit": ""}
      })"_json,
    };
    using Cutoff_t =
        internal::CutoffFunction<internal::CutoffFunctionType::RadialScaling>;

    for (auto & fc_hyper : fc_hypers) {
      if (verbose) {
        std::cout << fc_hyper.dump(2) << std::endl;
      }

      Cutoff_t cutoff(fc_hyper);
      BOOST_CHECK_EQUAL(cutoff.value(2), 1);
      BOOST_CHECK_EQUAL(cutoff.grad(2), 0);
    }
  }

  /* ---------------------------------------------------------------------- */
  BOOST_AUTO_TEST_SUITE_END();

}  // namespace rascal
