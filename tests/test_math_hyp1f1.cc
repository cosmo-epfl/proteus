/**
 * @file   test_math_math.cc
 *
 * @author  Felix Musil <felix.musil@epfl.ch>
 *
 * @date   21 May 2019
 *
 * @brief Test the implementation of Hyp1f1 against mpmath
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

#include <boost/test/unit_test.hpp>

namespace rascal {

  BOOST_AUTO_TEST_SUITE(MathHyp1f1Tests);

  /* ---------------------------------------------------------------------- */
  /**
   * Check the implementation of hyp1f1 against mpmath v1.1.0 for most of the
   * possible values of n, l and z that we will reasonably have to handdle.
   * see generate_soap_ref_data.py for details
   */
  BOOST_FIXTURE_TEST_CASE(math_hyp1f1_test, Hyp1F1RefFixture) {
    for (auto & data : this->ref_data) {
      double a{data["a"]}, b{data["b"]}, z{data["z"]}, hyp1f1_ref{data["val"]},
          hyp1f1_der_ref{data["der"]};
      math::Hyp1f1 func{a, b, 200, 1e-15};
      double val{func.calc(z)};
      double der{func.calc(z, true)};

      double h{1e-5};
      double hyp1f1_num_der{func.calc_numerical_derivative(z, h)};

      // check if hyp1f1 is consistent with the
      // mpmath reference
      double rel_error{std::abs((hyp1f1_ref - val) / hyp1f1_ref)};
      if (rel_error > 15 * math::DBL_FTOL and this->verbose) {
        std::cout << " a=" << a << " b=" << b << " z=" << z
                  << " ref=" << hyp1f1_ref << " impl=" << val << std::endl;
      }
      BOOST_CHECK_LE(rel_error, 15 * math::DBL_FTOL);

      // check if the analytical derivatives are consistent with the
      // mpmath reference
      double rel_der_error{std::abs((hyp1f1_der_ref - der) / hyp1f1_der_ref)};
      if (rel_der_error > 15 * math::DBL_FTOL and this->verbose) {
        std::cout << "Derivative a=" << a << " b=" << b << " z=" << z
                  << " ref=" << hyp1f1_der_ref << " impl=" << der
                  << " rel_err=" << rel_der_error << std::endl;
      }
      BOOST_CHECK_LE(rel_der_error, 15 * math::DBL_FTOL);

      // check if the numerical derivatives are consistent with the
      // analytical ones
      double der_consistency_rel_error{
          std::abs((hyp1f1_num_der - der) / hyp1f1_num_der)};
      if (der_consistency_rel_error > 1e5 * math::DBL_FTOL and this->verbose) {
        std::cout << "Derivative consistency a=" << a << " b=" << b
                  << " z=" << z << " num_der=" << hyp1f1_num_der
                  << " impl=" << der
                  << " rel_diff=" << der_consistency_rel_error << std::endl;
      }

      BOOST_CHECK_LE(der_consistency_rel_error, 2e6 * math::DBL_FTOL);
    }
  }

  /**
   * Check the acurracy of the the recurrence relations of 1F1 compared to
   * direct evaluation.
   */
  BOOST_FIXTURE_TEST_CASE(math_hyp1f1_spherical_expansion_test,
                          Hyp1f1SphericalExpansionFixture) {
    for (size_t i_rc{0}; i_rc < this->rcs.size(); ++i_rc) {
      auto & rc{this->rcs[i_rc]};
      auto & fac_b{this->facs_b[i_rc]};
      for (auto & r_ij : this->r_ijs) {
        if (r_ij >= rc) {
          continue;
        }
        for (auto & fac_a : this->fac_as) {
          for (size_t ii{0}; ii < this->hyp1f1.size(); ++ii) {
            hyp1f1[ii].calc(r_ij, fac_a, fac_b[ii], true);
            hyp1f1_recursion[ii].calc(r_ij, fac_a, fac_b[ii], true);
            auto hyp1f1_val{hyp1f1[ii].get_values()};
            auto hyp1f1_recursion_val{hyp1f1_recursion[ii].get_values()};
            auto diff_val{(hyp1f1_val - hyp1f1_recursion_val).array().abs() /
                          hyp1f1_val.array()};
            auto hyp1f1_der{hyp1f1[ii].get_derivatives()};
            auto hyp1f1_recursion_der{hyp1f1_recursion[ii].get_derivatives()};
            auto diff_der{(hyp1f1_der - hyp1f1_recursion_der).array().abs() /
                          hyp1f1_der.array()};
            BOOST_CHECK_LE(diff_val.mean(), 3 * math::DBL_FTOL);
            BOOST_CHECK_LE(diff_der.mean(), 3 * math::DBL_FTOL);
            if (verbose) {
              std::cout << "diff_val= " << diff_val.mean()
                        << " diff_der=" << diff_der.mean() << std::endl;
              for (int ii{0}; ii < diff_val.rows(); ++ii) {
                for (int jj{0}; jj < diff_val.cols(); ++jj) {
                  std::cout << diff_val(ii, jj) << ", ";
                }
              }
            }
          }
        }
      }
    }
  }

  BOOST_AUTO_TEST_CASE(hyp1f1_gradient_test) {
    const size_t max_radial = 4;
    const size_t max_angular = 2;
    // I couldn't care less about how these were computed
    Eigen::VectorXd fac_b(max_radial);
    fac_b << 0.888889, 0.888889, 0.444444, 0.296296;
    const double fac_a{0.5 * std::pow(0.2, -2)};
    Hyp1f1GradientProvider hyp1f1_calculator{max_radial, max_angular, fac_a,
                                             fac_b};
    GradientTestFixture fix{
        "reference_data/tests_only/radial_derivative_test.json"};
    test_gradients(hyp1f1_calculator, fix);
  }

  /* ---------------------------------------------------------------------- */
  BOOST_AUTO_TEST_SUITE_END();

}  // namespace rascal
