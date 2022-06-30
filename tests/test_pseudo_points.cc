/**
 * @file test_sparse_points.cc
 *
 * @author Felix Musil <felix.musil@epfl.ch>
 *
 * @date   7 January 2020
 *
 * @brief test the implementation of sparse_points classes
 *
 * @section LICENSE
 *
 * Copyright  2020 Felix Musil, COSMO (EPFL), LAMMM (EPFL)
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

#include "test_adaptor.hh"
#include "test_calculator.hh"
#include "test_manager_collection.hh"

#include "rascal/models/sparse_points.hh"
#include "rascal/utils/json_io.hh"

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

namespace rascal {
  BOOST_AUTO_TEST_SUITE(sparse_points_test);

  using ManagerCollection_t =
      ManagerCollection<StructureManagerCenters, AdaptorNeighbourList,
                        AdaptorCenterContribution, AdaptorStrict>;

  template <class Representation, class ManagerCollection,
            template <class> class SparsePoints>
  struct SparsePointsFixture {
    using Representation_t = Representation;
    using ManagerCollection_t = ManagerCollection;
    using SparsePoints_t = SparsePoints<Representation>;

    SparsePointsFixture() {
      hypers["cutoff_function"] = fc_hypers;
      hypers["gaussian_density"] = sigma_hypers;
      hypers["radial_contribution"] = {{"type", "GTO"}};

      json ad1a{{"name", "AdaptorNeighbourList"},
                {"initialization_arguments", {{"cutoff", cutoff}}}};
      json ad1b{{"name", "AdaptorCenterContribution"},
                {"initialization_arguments", {}}};
      json ad2{{"name", "AdaptorStrict"},
               {"initialization_arguments", {{"cutoff", cutoff}}}};
      adaptors.emplace_back(ad1a);
      adaptors.emplace_back(ad1b);
      adaptors.emplace_back(ad2);
    }

    std::string filename{"reference_data/inputs/small_molecules-20.json"};

    json adaptors{};

    SparsePoints_t sparse_points{};

    double cutoff{3.};
    json hypers{{"max_radial", 1},
                {"max_angular", 1},
                {"compute_gradients", false},
                {"soap_type", "PowerSpectrum"},
                {"normalize", true},
                {"expansion_by_species_method", "environment wise"}};

    json fc_hypers{{"type", "ShiftedCosine"},
                   {"cutoff", {{"value", cutoff}, {"unit", "AA"}}},
                   {"smooth_width", {{"value", 0.5}, {"unit", "AA"}}}};
    json sigma_hypers{{"type", "Constant"},
                      {"gaussian_sigma", {{"value", 0.4}, {"unit", "AA"}}}};
  };

  using multiple_fixtures = boost::mpl::list<
      SparsePointsFixture<CalculatorSphericalInvariants, ManagerCollection_t,
                          SparsePointsBlockSparse>>;
  /**
   * Tests if the features extracted from a set of structure features actually
   * match them after extraction.
   */
  BOOST_FIXTURE_TEST_CASE_TEMPLATE(data_matching_test, Fix, multiple_fixtures,
                                   Fix) {
    const bool verbose{false};
    auto & sparse_points = Fix::sparse_points;

    typename Fix::ManagerCollection_t managers{Fix::adaptors};
    managers.add_structures(Fix::filename, 0, 3);
    typename Fix::Representation_t representation{Fix::hypers};

    representation.compute(managers);

    std::vector<std::vector<int>> selected_ids;
    std::random_device rd;
    std::mt19937 gen{rd()};
    auto dice{std::uniform_real_distribution<double>(0., 1.)};
    for (auto & manager : managers) {
      selected_ids.emplace_back();
      int ii{0};
      for (auto center : manager) {
        (void)center;  // to avoid compiler warning
        double roll{dice(gen)};
        if (roll < 0.85) {
          selected_ids.back().push_back(ii);
        } else if (verbose) {
          std::cout << "Center " << ii << " will not be considered."
                    << std::endl;
        }
        ++ii;
      }
    }

    sparse_points.push_back(representation, managers, selected_ids);

    auto feat_ref = managers.get_features(representation);

    auto feat_test = sparse_points.get_features();

    for (int i_row{0}; i_row < feat_test.rows(); i_row++) {
      auto diffs = (feat_ref.rowwise() - feat_test.row(i_row))
                       .rowwise()
                       .template lpNorm<1>();
      // check that one row of feat_ref matches the current row of feat_test
      BOOST_TEST((diffs.array() < 1e-16).count() == 1);

      if (verbose and (diffs.array() < 1e-16).count() != 1) {  // NOLINT
        std::cout << "Number of matching row " << i_row << " :"
                  << (diffs.array() < 1e-16).count() << std::endl;
        std::cout << feat_ref << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << feat_test.row(i_row) << std::endl;
        std::cout << "####################################" << std::endl;
      }
    }
  }

  /**
   * Tests the serialization works properly, i.e. the deserialized object is
   * effectively the same as the original one, and that trying to get a type
   * mismatched with the serialized sparse_points throws an error.
   */
  BOOST_FIXTURE_TEST_CASE_TEMPLATE(serialization_test, Fix, multiple_fixtures,
                                   Fix) {
    const bool verbose{false};
    auto & sparse_points = Fix::sparse_points;

    typename Fix::ManagerCollection_t managers{Fix::adaptors};
    managers.add_structures(Fix::filename, 0, 3);
    typename Fix::Representation_t representation{Fix::hypers};

    representation.compute(managers);

    std::vector<std::vector<int>> selected_ids;
    std::random_device rd;
    std::mt19937 gen{rd()};
    auto dice{std::uniform_real_distribution<double>(0., 1.)};
    for (auto & manager : managers) {
      selected_ids.emplace_back();
      int ii{0};
      for (auto center : manager) {
        (void)center;  // to avoid compiler warning
        double roll{dice(gen)};
        if (roll < 0.85) {
          selected_ids.back().push_back(ii);
        } else if (verbose) {
          std::cout << "Center " << ii << " will not be considered."
                    << std::endl;
        }
        ++ii;
      }
    }

    sparse_points.push_back(representation, managers, selected_ids);

    json j;
    j = sparse_points;
    if (verbose) {
      std::cout << j.dump() << std::endl;
    }

    auto sparse_points_b = j.template get<typename Fix::SparsePoints_t>();
    bool test{sparse_points == sparse_points_b};
    BOOST_TEST(test);

    BOOST_CHECK_THROW(
        j.template get<SparsePointsBlockSparse<CalculatorSphericalExpansion>>(),
        std::runtime_error);
  }

  BOOST_AUTO_TEST_SUITE_END();

}  // namespace rascal
