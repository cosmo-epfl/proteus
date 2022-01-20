/**
 * @file test_sparse_kernels.cc
 *
 * @author Felix Musil <felix.musil@epfl.ch>
 *
 * @date   18 Jan 2020
 *
 * @brief test the implementation of sparse similarity kernel classes
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

#include "test_sparse_kernels.hh"

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

namespace rascal {
  BOOST_AUTO_TEST_SUITE(sparse_kernels_test);

  using multiple_fixtures =
      boost::mpl::list<SparseKernelFixture<StrictNLSparseKernelFixture>>;

  /**
   * Tests if the compute functionality matches the size of atoms/structures
   * given as input.
   */
  BOOST_FIXTURE_TEST_CASE_TEMPLATE(multiple_kernel_compute_test, Fix,
                                   multiple_fixtures, Fix) {
    using Calculator_t = typename Fix::Calculator_t;
    auto & kernels = Fix::kernels;
    auto & representations = Fix::representations;
    auto & collections = Fix::collections;

    // use all the features as sparse point
    std::vector<std::vector<std::vector<int>>> selected_ids;
    for (auto & collection : collections) {
      selected_ids.emplace_back();
      for (auto & manager : collection) {
        selected_ids.back().emplace_back();
        int ii{0};
        for (auto center : manager) {
          (void)center;
          selected_ids.back().back().push_back(ii);
          ++ii;
        }
      }
    }

    int i_collection{0};
    for (auto & collection : collections) {
      for (auto & representation : representations) {
        SparsePointsBlockSparse<Calculator_t> sparse_points{};
        sparse_points.push_back(representation, collection,
                                selected_ids[i_collection]);
        for (auto & kernel : kernels) {
          auto mat = kernel.compute(representation, collection, sparse_points);

          if (Fix::verbose) {
            std::cout << "target_type=" << static_cast<int>(kernel.target_type)
                      << " mat.size=" << mat.size() << std::endl;
          }

          if (kernel.target_type == internal::TargetType::Structure) {
            BOOST_CHECK_EQUAL(mat.size(),
                              collection.size() * sparse_points.size());

          } else if (kernel.target_type == internal::TargetType::Atom) {
            int n_centers{0};
            for (auto & manager : collection) {
              n_centers += manager->size();
            }
            BOOST_CHECK_EQUAL(mat.size(), n_centers * sparse_points.size());
          }
        }
      }
    }
  }

  /* ---------------------------------------------------------------------- */
  /**
   * Utility fixture used to compare representations with sparsification
   */

  struct SparseKernelGradFixture {
    using ManagerTypeHolder_t =
        StructureManagerTypeHolder<StructureManagerCenters,
                                   AdaptorNeighbourList,
                                   AdaptorCenterContribution, AdaptorStrict>;
    using Manager_t = typename ManagerTypeHolder_t::type;
    using ManagerCollection_t =
        typename TypeHolderInjector<ManagerCollection,
                                    ManagerTypeHolder_t::type_list>::type;
    using Structure_t = AtomicStructure<3>;
    using Representation_t = CalculatorSphericalInvariants;
    using SparsePoints_t = SparsePointsBlockSparse<Representation_t>;
    using Kernel_t = SparseKernel;
    using Prop_t = typename Representation_t::template Property_t<Manager_t>;
    using PropGrad_t =
        typename Representation_t::template PropertyGradient_t<Manager_t>;

    SparseKernelGradFixture() {}

    ~SparseKernelGradFixture() = default;
  };

  using sparse_grad_fixtures = boost::mpl::list<SparseKernelGradFixture>;

  /**
   * Test the analytical kernel gradients against numerical kernel gradients.
   */
  BOOST_FIXTURE_TEST_CASE_TEMPLATE(grad_test, Fix, sparse_grad_fixtures, Fix) {
    using ManagerCollection_t = typename Fix::ManagerCollection_t;
    using Manager_t = typename ManagerCollection_t::Manager_t;
    using Representation_t = typename Fix::Representation_t;
    using Kernel_t = typename Fix::Kernel_t;
    using SparsePoints_t = typename Fix::SparsePoints_t;
    json inputs{};
    inputs =
        json_io::load("reference_data/tests_only/sparse_kernel_inputs.json");

    const bool verbose{true};
    // relative error threshold
    const double delta{1e-3};
    // range of zero
    const double epsilon{1e-14};
    const bool compute_stress{false};

    for (const auto & input : inputs) {
      // extract inputs
      std::string filename{input.at("filename").template get<std::string>()};
      json adaptors_input = input.at("adaptors").template get<json>();
      json calculator_input = input.at("calculator").template get<json>();
      json kernel_input = input.at("kernel").template get<json>();
      auto selected_ids = input.at("selected_ids")
                              .template get<std::vector<std::vector<int>>>();
      // initialize classes
      Kernel_t kernel{kernel_input};
      kernel_input.at("target_type") = "Atom";
      Kernel_t kernel_num{kernel_input};
      ManagerCollection_t managers{adaptors_input};
      SparsePoints_t sparse_points{};
      Representation_t representation{calculator_input};
      // load structures, compute representation and fill sparse points
      managers.add_structures(filename, 0,
                              input.at("n_structures").template get<int>());
      representation.compute(managers);
      sparse_points.push_back(representation, managers, selected_ids);
      calculator_input["compute_gradients"] = false;
      Representation_t representation_{calculator_input};

      // compute kernel gradients
      auto KNM_der{kernel.compute_derivative(representation, managers,
                                             sparse_points, compute_stress)};
      auto KNM_num_der{compute_numerical_kernel_gradients(
          kernel_num, representation_, managers, sparse_points,
          input.at("h").template get<double>(), compute_stress)};
      auto diff = math::relative_error(KNM_der, KNM_num_der, delta, epsilon);
      int col_max{0}, row_max{0};
      double max_rel_diff{diff.maxCoeff(&row_max, &col_max)};
      BOOST_TEST(max_rel_diff < delta);
      if (verbose and max_rel_diff > delta) {
        std::cout << filename << std::endl;
        std::cout << adaptors_input.dump() << std::endl;
        std::cout << calculator_input.dump() << std::endl;
        std::cout << kernel_input.dump() << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << diff.row(row_max) << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << KNM_der.row(row_max) << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << KNM_num_der.row(row_max) << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << KNM_der << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << KNM_num_der << std::endl;
      }

      // test that prediction routine and predictions with kernel are equal
      math::Vector_t weights{sparse_points.size()};
      weights.setConstant(1.);

      math::Matrix_t gradients_k = KNM_der * weights.transpose();
      std::string force_name = compute_sparse_kernel_gradients(
          representation, kernel, managers, sparse_points, weights);
      size_t i_center{0};
      for (auto manager : managers) {
        auto && gradients{*manager->template get_property<
            Property<double, 1, Manager_t, 1, ThreeD>>(force_name, true)};
        math::Matrix_t ff = Eigen::Map<const math::Matrix_t>(
            gradients.view().data(), manager->size() * ThreeD, 1);
        math::Matrix_t ff_r =
            gradients_k.block(i_center, 0, manager->size() * ThreeD, 1);
        math::Matrix_t force_diff =
            math::relative_error(ff, ff_r, delta, epsilon);
        double gradients_max_rel_diff{force_diff.maxCoeff(&row_max, &col_max)};
        BOOST_TEST(gradients_max_rel_diff < delta);
        i_center += manager->size() * ThreeD;
      }
    }
  }

  /**
   * Test the analytical kernel stress against numerical kernel stress.
   */
  BOOST_FIXTURE_TEST_CASE_TEMPLATE(grad_stress_test, Fix, sparse_grad_fixtures,
                                   Fix) {
    using ManagerCollection_t = typename Fix::ManagerCollection_t;
    using Manager_t = typename ManagerCollection_t::Manager_t;
    using Representation_t = typename Fix::Representation_t;
    using Kernel_t = typename Fix::Kernel_t;
    using SparsePoints_t = typename Fix::SparsePoints_t;

    json inputs{};
    inputs =
        json_io::load("reference_data/tests_only/sparse_kernel_inputs.json");

    const bool verbose{true};
    // relative error threshold
    const double delta{6e-3};
    // range of zero
    const double epsilon{1e-11};

    const bool compute_stress{true};

    for (const auto & input : inputs) {
      // extract inputs
      std::string filename{input.at("filename").template get<std::string>()};
      json adaptors_input = input.at("adaptors").template get<json>();
      json calculator_input = input.at("calculator").template get<json>();
      json kernel_input = input.at("kernel").template get<json>();
      auto selected_ids = input.at("selected_ids")
                              .template get<std::vector<std::vector<int>>>();
      // initialize classes
      Kernel_t kernel{kernel_input};
      kernel_input.at("target_type") = "Atom";
      Kernel_t kernel_num{kernel_input};
      ManagerCollection_t managers{adaptors_input};
      SparsePoints_t sparse_points{};
      Representation_t representation{calculator_input};
      // load structures, compute representation and fill sparse points
      managers.add_structures(filename, 0,
                              input.at("n_structures").template get<int>());
      representation.compute(managers);
      sparse_points.push_back(representation, managers, selected_ids);
      calculator_input["compute_gradients"] = false;
      Representation_t representation_{calculator_input};
      // compute kernel gradients
      auto KNM_der{kernel.compute_derivative(representation, managers,
                                             sparse_points, compute_stress)};
      auto KNM_num_der{compute_numerical_kernel_gradients(
          kernel_num, representation_, managers, sparse_points,
          input.at("h").template get<double>(), compute_stress)};

      int n_stress_rows{static_cast<int>(managers.size() * 6)};
      math::Matrix_t KNM_stress{KNM_der.block(KNM_der.rows() - n_stress_rows, 0,
                                              n_stress_rows, KNM_der.cols())};
      math::Matrix_t KNM_stress_num{
          KNM_num_der.block(KNM_num_der.rows() - n_stress_rows, 0,
                            n_stress_rows, KNM_num_der.cols())};
      auto diff =
          math::relative_error(KNM_stress, KNM_stress_num, delta, epsilon);
      int col_max{0}, row_max{0};
      double max_rel_diff{diff.maxCoeff(&row_max, &col_max)};
      BOOST_TEST(max_rel_diff < delta);
      if (verbose and max_rel_diff > delta) {
        std::cout << filename << std::endl;
        std::cout << adaptors_input.dump() << std::endl;
        std::cout << calculator_input.dump() << std::endl;
        std::cout << kernel_input.dump() << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << diff.row(row_max) << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << KNM_stress.row(row_max) << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << KNM_stress_num.row(row_max) << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << KNM_stress << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << KNM_stress_num << std::endl;
      }

      // test that prediction routine and predictions with kernel are equal
      math::Vector_t weights{sparse_points.size()};
      weights.setConstant(1.);

      math::Matrix_t neg_stress_k = KNM_stress * weights.transpose();
      std::string neg_stress_name = compute_sparse_kernel_neg_stress(
          representation, kernel, managers, sparse_points, weights);
      size_t i_center{0};
      for (auto manager : managers) {
        auto && neg_stress{
            *manager->template get_property<Property<double, 0, Manager_t, 6>>(
                neg_stress_name, true)};
        math::Matrix_t ff =
            Eigen::Map<const math::Matrix_t>(neg_stress.view().data(), 6, 1);
        math::Matrix_t ff_r = neg_stress_k.block(i_center, 0, 6, 1);
        math::Matrix_t neg_stress_diff =
            math::relative_error(ff, ff_r, delta, epsilon);
        double gradients_max_rel_diff{
            neg_stress_diff.maxCoeff(&row_max, &col_max)};
        BOOST_TEST(gradients_max_rel_diff < delta);
        i_center += 6;
      }
    }
  }

  BOOST_AUTO_TEST_SUITE_END();

}  // namespace rascal
