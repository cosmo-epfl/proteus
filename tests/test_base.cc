/**
 * @file   test_base.cc
 *
 * @author Till Junge <till.junge@epfl.ch>
 *
 * @date   01 Mar 2018
 *
 * @brief  description
 *
 * @section LICENSE
 *
 * Copyright  2018 Till Junge, COSMO (EPFL), LAMMM (EPFL)
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

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

namespace rascal {

  BOOST_AUTO_TEST_SUITE(base_tests);

  BOOST_AUTO_TEST_CASE(base_test) { BOOST_CHECK_EQUAL(1, 2 - 1); }

  template <int Dim>
  struct DemoTestFixture {
    static constexpr int dim() { return Dim; }
    DemoTestFixture() : val{Dim} {}

    int val;
  };

  using fixtures = boost::mpl::list<DemoTestFixture<2>, DemoTestFixture<3>>;

  BOOST_FIXTURE_TEST_CASE_TEMPLATE(templated_basic_fixture_test, Fix, fixtures,
                                   Fix) {
    BOOST_CHECK_EQUAL(Fix::val, Fix::dim());
  }

  BOOST_AUTO_TEST_SUITE_END();
}  // namespace rascal
