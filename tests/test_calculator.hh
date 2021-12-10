/**
 * @file   test_calculator.hh
 *
 * @author Musil Felix <musil.felix@epfl.ch>
 * @author Max Veit <max.veit@epfl.ch>
 *
 * @date   14 September 2018
 *
 * @brief  test representation managers
 *
 * Copyright  2018 Musil Felix, COSMO (EPFL), LAMMM (EPFL)
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

#ifndef TESTS_TEST_CALCULATOR_HH_
#define TESTS_TEST_CALCULATOR_HH_

#include "test_adaptor.hh"
#include "test_math.hh"
#include "test_structure.hh"

#include "rascal/representations/calculator_base.hh"
#include "rascal/representations/calculator_sorted_coulomb.hh"
#include "rascal/representations/calculator_spherical_covariants.hh"
#include "rascal/representations/calculator_spherical_expansion.hh"
#include "rascal/representations/calculator_spherical_invariants.hh"
#include "rascal/structure_managers/atomic_structure.hh"
#include "rascal/structure_managers/cluster_ref_key.hh"
#include "rascal/structure_managers/structure_manager_collection.hh"
#include "rascal/utils/json_io.hh"
#include "rascal/utils/utils.hh"

#include <memory>
#include <tuple>

namespace rascal {

  struct TestData {
    using ManagerTypeHolder_t =
        StructureManagerTypeHolder<StructureManagerCenters,
                                   AdaptorNeighbourList,
                                   AdaptorCenterContribution, AdaptorStrict>;
    TestData() = default;

    void get_ref(const std::string & ref_filename) {
      this->ref_data =
          json::from_ubjson(internal::read_binary_file(ref_filename));
      auto filenames =
          this->ref_data.at("filenames").get<std::vector<std::string>>();
      auto cutoffs = this->ref_data.at("cutoffs").get<std::vector<double>>();

      for (auto && filename : filenames) {
        for (auto && cutoff : cutoffs) {
          json parameters;
          json structure{{"filename", filename}};
          json adaptors;
          json ad1{{"name", "AdaptorNeighbourList"},
                   {"initialization_arguments", {{"cutoff", cutoff}}}};
          json ad1b{{"name", "AdaptorCenterContribution"},
                    {"initialization_arguments", {}}};
          json ad2{{"name", "AdaptorStrict"},
                   {"initialization_arguments", {{"cutoff", cutoff}}}};
          adaptors.emplace_back(ad1);
          adaptors.emplace_back(ad1b);
          adaptors.emplace_back(ad2);

          parameters["structure"] = structure;
          parameters["adaptors"] = adaptors;

          this->factory_args.emplace_back(parameters);
        }
      }
    }

    ~TestData() = default;

    json ref_data{};
    json factory_args{};
  };

  template <typename MultipleStructureFixture>
  struct MultipleStructureSphericalInvariants : MultipleStructureFixture {
    using Parent = MultipleStructureFixture;
    using ManagerTypeHolder_t = typename Parent::ManagerTypeHolder_t;
    using Representation_t = CalculatorSphericalInvariants;

    MultipleStructureSphericalInvariants() : Parent{} {
      for (auto & ri_hyp : this->radial_contribution_hypers) {
        for (auto & fc_hyp : this->fc_hypers) {
          for (auto & sig_hyp : this->density_hypers) {
            for (auto & coef_sub_hyp : this->coef_sub_hypers) {
              for (auto & rep_hyp : this->rep_hypers) {
                rep_hyp["cutoff_function"] = fc_hyp;
                rep_hyp["gaussian_density"] = sig_hyp;
                rep_hyp["radial_contribution"] = ri_hyp;
                if (not(coef_sub_hyp.is_null())) {
                  rep_hyp["coefficient_subselection"] = coef_sub_hyp;
                }
                this->representation_hypers.push_back(rep_hyp);
              }
            }
          }
        }
      }
    };

    ~MultipleStructureSphericalInvariants() = default;

    std::vector<json> representation_hypers{};

    std::vector<json> coef_sub_hypers{
        {}, {{"a", {1}}, {"b", {1}}, {"n1", {0}}, {"n2", {0}}, {"l", {0}}}};
    std::vector<json> fc_hypers{
        {{"type", "ShiftedCosine"},
         {"cutoff", {{"value", 3.0}, {"unit", "AA"}}},
         {"smooth_width", {{"value", 0.5}, {"unit", "AA"}}}}};

    std::vector<json> density_hypers{
        {{"type", "Constant"},
         {"gaussian_sigma", {{"value", 0.2}, {"unit", "AA"}}}}};
    std::vector<json> radial_contribution_hypers{{{"type", "GTO"}}};

    std::vector<json> rep_hypers{{{"max_radial", 3},
                                  {"max_angular", 0},
                                  {"soap_type", "RadialSpectrum"},
                                  {"normalize", true}},
                                 {{"max_radial", 3},
                                  {"max_angular", 0},
                                  {"soap_type", "RadialSpectrum"},
                                  {"normalize", true}},
                                 {{"max_radial", 3},
                                  {"max_angular", 3},
                                  {"soap_type", "PowerSpectrum"},
                                  {"normalize", true}},
                                 {{"max_radial", 4},
                                  {"max_angular", 3},
                                  {"soap_type", "PowerSpectrum"},
                                  {"normalize", true}},
                                 {{"max_radial", 3},
                                  {"max_angular", 1},
                                  {"soap_type", "BiSpectrum"},
                                  {"inversion_symmetry", true},
                                  {"normalize", true}},
                                 {{"max_radial", 3},
                                  {"max_angular", 1},
                                  {"soap_type", "BiSpectrum"},
                                  {"inversion_symmetry", false},
                                  {"normalize", true}}};
  };

  template <typename MultipleStructureFixture>
  struct MultipleStructureSphericalCovariants : MultipleStructureFixture {
    using Parent = MultipleStructureFixture;
    using ManagerTypeHolder_t = typename Parent::ManagerTypeHolder_t;
    using Representation_t = CalculatorSphericalCovariants;

    MultipleStructureSphericalCovariants() : Parent{} {
      for (auto & ri_hyp : this->radial_contribution_hypers) {
        for (auto & fc_hyp : this->fc_hypers) {
          for (auto & sig_hyp : this->density_hypers) {
            for (auto & rep_hyp : this->rep_hypers) {
              rep_hyp["cutoff_function"] = fc_hyp;
              rep_hyp["gaussian_density"] = sig_hyp;
              rep_hyp["radial_contribution"] = ri_hyp;
              this->representation_hypers.push_back(rep_hyp);
            }
          }
        }
      }
    };

    ~MultipleStructureSphericalCovariants() = default;

    std::vector<json> representation_hypers{};

    std::vector<json> fc_hypers{
        {{"type", "ShiftedCosine"},
         {"cutoff", {{"value", 2.0}, {"unit", "AA"}}},
         {"smooth_width", {{"value", 1.0}, {"unit", "AA"}}}}};

    std::vector<json> density_hypers{
        {{"type", "Constant"},
         {"gaussian_sigma", {{"value", 0.2}, {"unit", "AA"}}}},
        {{"type", "Constant"},
         {"gaussian_sigma", {{"value", 0.4}, {"unit", "AA"}}}}};
    std::vector<json> radial_contribution_hypers{{{"type", "GTO"}}};
    std::vector<json> rep_hypers{{{"max_radial", 1},
                                  {"max_angular", 2},
                                  {"soap_type", "LambdaSpectrum"},
                                  {"covariant_lambda", 2},
                                  {"inversion_symmetry", true},
                                  {"normalize", true}},
                                 {{"max_radial", 2},
                                  {"max_angular", 2},
                                  {"soap_type", "LambdaSpectrum"},
                                  {"covariant_lambda", 2},
                                  {"inversion_symmetry", false},
                                  {"normalize", true}}};
  };

  struct SphericalInvariantsTestData : TestData {
    using Parent = TestData;
    using ManagerTypeHolder_t = typename Parent::ManagerTypeHolder_t;
    using Representation_t = CalculatorSphericalInvariants;
    SphericalInvariantsTestData() : Parent{} {
      this->get_ref(this->ref_filename);
    }
    ~SphericalInvariantsTestData() = default;
    bool verbose{false};
    std::string ref_filename{
        "reference_data/tests_only/spherical_invariants_reference.ubjson"};
  };

  struct SphericalCovariantsTestData : TestData {
    using Parent = TestData;
    using ManagerTypeHolder_t = typename Parent::ManagerTypeHolder_t;
    using Representation_t = CalculatorSphericalCovariants;
    SphericalCovariantsTestData() : Parent{} {
      this->get_ref(this->ref_filename);
    }
    ~SphericalCovariantsTestData() = default;
    bool verbose{false};
    std::string ref_filename{
        "reference_data/tests_only/spherical_covariants_reference.ubjson"};
  };

  template <class MultipleStructureFixture>
  struct MultipleStructureSphericalExpansion : MultipleStructureFixture {
    using Parent = MultipleStructureFixture;
    using ManagerTypeHolder_t = typename Parent::ManagerTypeHolder_t;
    using Representation_t = CalculatorSphericalExpansion;

    MultipleStructureSphericalExpansion() : Parent{} {
      for (auto & ri_hyp : this->radial_contribution_hypers) {
        for (auto & fc_hyp : this->fc_hypers) {
          for (auto & sig_hyp : this->density_hypers) {
            for (auto & rep_hyp : this->rep_hypers) {
              rep_hyp["cutoff_function"] = fc_hyp;
              rep_hyp["gaussian_density"] = sig_hyp;
              rep_hyp["radial_contribution"] = ri_hyp;
              this->representation_hypers.push_back(rep_hyp);
            }
          }
        }
      }
    };
    ~MultipleStructureSphericalExpansion() = default;

    std::vector<json> representation_hypers{};

    std::vector<json> fc_hypers{
        {{"type", "ShiftedCosine"},
         {"cutoff", {{"value", 3.0}, {"unit", "AA"}}},
         {"smooth_width", {{"value", 0.5}, {"unit", "AA"}}}},
        {{"type", "ShiftedCosine"},
         {"cutoff", {{"value", 2.0}, {"unit", "AA"}}},
         {"smooth_width", {{"value", 1.0}, {"unit", "AA"}}}},
        {{"type", "RadialScaling"},
         {"cutoff", {{"value", 4.0}, {"unit", "AA"}}},
         {"smooth_width", {{"value", 0.5}, {"unit", "AA"}}},
         {"rate", {{"value", .0}, {"unit", "AA"}}},
         {"exponent", {{"value", 4}, {"unit", ""}}},
         {"scale", {{"value", 2.5}, {"unit", "AA"}}}},
        {{"type", "RadialScaling"},
         {"cutoff", {{"value", 4.0}, {"unit", "AA"}}},
         {"smooth_width", {{"value", 0.5}, {"unit", "AA"}}},
         {"rate", {{"value", 1.}, {"unit", "AA"}}},
         {"exponent", {{"value", 3}, {"unit", ""}}},
         {"scale", {{"value", 2.}, {"unit", "AA"}}}}};

    std::vector<json> radial_contribution_hypers{{{"type", "GTO"}},
                                                 {{"type", "DVR"}}};

    std::vector<json> density_hypers{
        {{"type", "Constant"},
         {"gaussian_sigma", {{"value", 0.5}, {"unit", "AA"}}}}};

    std::vector<json> rep_hypers{{{"max_radial", 2}, {"max_angular", 2}}};
  };

  /**
   * Simplified version of MultipleStructureManagerNLStrictFixture
   *  that uses only one structure, cutoff, and adaptor set
   *
   *  Useful if we just need a StructureManager to test relatively isolated
   *  functionality on a single structure, but using the rest of the testing
   *  machinery
   */
  struct SimpleStructureManagerNLCCStrictFixture {
    using ManagerTypeHolder_t =
        StructureManagerTypeHolder<StructureManagerCenters,
                                   AdaptorNeighbourList,
                                   AdaptorCenterContribution, AdaptorStrict>;
    using Structure_t = AtomicStructure<3>;

    SimpleStructureManagerNLCCStrictFixture() {
      json parameters;
      json structure{{"filename", filename}};
      json adaptors;
      json ad1{{"name", "AdaptorNeighbourList"},
               {"initialization_arguments",
                {{"cutoff", cutoff}, {"skin", cutoff_skin}}}};
      json ad1b{{"name", "AdaptorCenterContribution"},
                {"initialization_arguments", {}}};
      json ad2{{"name", "AdaptorStrict"},
               {"initialization_arguments", {{"cutoff", cutoff}}}};
      adaptors.emplace_back(ad1);
      adaptors.push_back(ad1b);
      adaptors.emplace_back(ad2);

      parameters["structure"] = structure;
      parameters["adaptors"] = adaptors;

      this->factory_args.emplace_back(parameters);
    }

    ~SimpleStructureManagerNLCCStrictFixture() = default;

    const std::string filename{
        "reference_data/inputs/CaCrP2O7_mvc-11955_symmetrized.json"};
    const double cutoff{3.};
    const double cutoff_skin{0.};

    json factory_args{};
    std::vector<Structure_t> structures{};
  };

  struct MultipleHypersSphericalExpansion
      : SimpleStructureManagerNLCCStrictFixture {
    using Parent = SimpleStructureManagerNLCCStrictFixture;
    using ManagerTypeHolder_t = typename Parent::ManagerTypeHolder_t;
    using Representation_t = CalculatorSphericalExpansion;

    MultipleHypersSphericalExpansion() : Parent{} {
      for (auto & ri_hyp : this->radial_contribution_hypers) {
        for (auto & fc_hyp : this->fc_hypers) {
          for (auto & sig_hyp : this->density_hypers) {
            for (auto & rep_hyp : this->rep_hypers) {
              rep_hyp["cutoff_function"] = fc_hyp;
              rep_hyp["gaussian_density"] = sig_hyp;
              rep_hyp["radial_contribution"] = ri_hyp;
              this->representation_hypers.push_back(rep_hyp);
            }
          }
        }
      }
    };

    ~MultipleHypersSphericalExpansion() = default;

    std::vector<json> representation_hypers{};
    std::vector<json> fc_hypers{
        {{"type", "ShiftedCosine"},
         {"cutoff", {{"value", 3.0}, {"unit", "AA"}}},
         {"smooth_width", {{"value", 0.5}, {"unit", "AA"}}}},
        {{"type", "ShiftedCosine"},
         {"cutoff", {{"value", 2.0}, {"unit", "AA"}}},
         {"smooth_width", {{"value", 1.0}, {"unit", "AA"}}}}};

    std::vector<json> density_hypers{
        {{"type", "Constant"},
         {"gaussian_sigma", {{"value", 0.2}, {"unit", "AA"}}}},
        {{"type", "Constant"},
         {"gaussian_sigma", {{"value", 0.4}, {"unit", "AA"}}}}};
    // nested json definitions dont work properly in this case:
    // if not defined seperately, radial dimension reduction_hypers
    // are interpreted as arrays
    // https://github.com/nlohmann/json/issues/737
    // shape (max_angular+1, max_radial, max_radial)  = (4, 3, 3)
    std::map<std::string, std::vector<std::vector<std::vector<double>>>>
        projection_matrices{
            {"8",
             {
                 {{0.5, 0.1, 0.3}, {0.1, 1.5, 0.1}, {0.5, 0.1, 1.9}},
                 {{0.9, 0.2, 0.6}, {0.9, 1.1, 0.4}, {0.5, 0.5, 1.1}},
                 {{0.4, 0.8, 0.6}, {0.8, 1.2, 0.5}, {0.1, 0.4, 1.9}},
                 {{0.1, 0.3, 0.1}, {0.6, 1.1, 0.4}, {0.5, 0.6, 1.5}},
             }}};
    json projection_matrices_hypers{
        {"projection_matrices", projection_matrices}};
    json radial_dim_reduction_spline_hypers{
        {"Spline", {{"accuracy", 1e-8}}},
        {"RadialDimReduction", projection_matrices_hypers}};
    std::vector<json> radial_contribution_hypers{
        {{"type", "GTO"}, {"optimization", {}}},
        {{"type", "DVR"}, {"optimization", {}}},
        {{"type", "GTO"},
         {"optimization", {{"Spline", {{"accuracy", 1e-12}}}}}},
        {{"type", "DVR"}, {"optimization", {{"Spline", {{"accuracy", 1e-5}}}}}},
        {{"type", "GTO"}, {"optimization", radial_dim_reduction_spline_hypers}},
        {{"type", "DVR"}, {"optimization", radial_dim_reduction_spline_hypers}},
    };
    std::vector<json> rep_hypers{
        {{"max_radial", 3}, {"max_angular", 3}, {"compute_gradients", true}}};
  };

  /** Contains some simple periodic structures for testing complicated things
   *  like gradients
   */
  struct SimplePeriodicNLCCStrictFixture {
    using ManagerTypeHolder_t =
        StructureManagerTypeHolder<StructureManagerCenters,
                                   AdaptorNeighbourList,
                                   AdaptorCenterContribution, AdaptorStrict>;
    using Structure_t = AtomicStructure<3>;

    SimplePeriodicNLCCStrictFixture() {
      for (auto && filename : filenames) {
        json parameters;
        json structure{{"filename", filename}};
        json adaptors;
        json ad1{{"name", "AdaptorNeighbourList"},
                 {"initialization_arguments",
                  {{"cutoff", cutoff}, {"skin", cutoff_skin}}}};
        json ad1b{{"name", "AdaptorCenterContribution"},
                  {"initialization_arguments", {}}};
        json ad2{{"name", "AdaptorStrict"},
                 {"initialization_arguments", {{"cutoff", cutoff}}}};
        adaptors.emplace_back(ad1);
        adaptors.push_back(ad1b);
        adaptors.emplace_back(ad2);

        parameters["structure"] = structure;
        parameters["adaptors"] = adaptors;

        this->factory_args.emplace_back(parameters);
      }
    }

    ~SimplePeriodicNLCCStrictFixture() = default;

    const std::vector<std::string> filenames{
        "reference_data/inputs/diamond_2atom.json",
        "reference_data/inputs/diamond_2atom_distorted.json",
        "reference_data/inputs/diamond_cubic_distorted.json",
        "reference_data/inputs/SiC_moissanite.json",
        "reference_data/inputs/SiCGe_wurtzite_like.json",
        "reference_data/inputs/SiC_moissanite_supercell.json",
        "reference_data/inputs/methane.json",
        "reference_data/inputs/small_molecule.json",
        "reference_data/inputs/CaCrP2O7_mvc-11955_symmetrized.json"};

    const double cutoff{2.5};
    const double cutoff_skin{0.};

    json factory_args{};
    std::vector<Structure_t> structures{};
  };

  /** Contains some simple periodic structures for testing complicated things
   *  like gradients
   *  Should match the
   */
  struct SimplePeriodicNLHalfCCStrictFixture {
    using ManagerTypeHolder_t =
        StructureManagerTypeHolder<StructureManagerCenters,
                                   AdaptorNeighbourList, AdaptorHalfList,
                                   AdaptorCenterContribution, AdaptorStrict>;
    using Structure_t = AtomicStructure<3>;

    SimplePeriodicNLHalfCCStrictFixture() {
      for (auto && filename : filenames) {
        json parameters;
        json structure{{"filename", filename}};
        json adaptors;
        json ad1a{{"name", "AdaptorNeighbourList"},
                  {"initialization_arguments", {{"cutoff", cutoff}}}};
        json ad1b{{"name", "AdaptorHalfList"},
                  {"initialization_arguments", {}}};
        json ad1c{{"name", "AdaptorCenterContribution"},
                  {"initialization_arguments", {}}};
        json ad2{{"name", "AdaptorStrict"},
                 {"initialization_arguments", {{"cutoff", cutoff}}}};

        adaptors.emplace_back(ad1a);
        adaptors.emplace_back(ad1b);
        adaptors.emplace_back(ad1c);
        adaptors.emplace_back(ad2);

        parameters["structure"] = structure;
        parameters["adaptors"] = adaptors;

        this->factory_args.emplace_back(parameters);
      }
    }

    ~SimplePeriodicNLHalfCCStrictFixture() = default;

    const std::vector<std::string> filenames{
        "reference_data/inputs/diamond_2atom.json",
        "reference_data/inputs/diamond_2atom_distorted.json",
        "reference_data/inputs/diamond_cubic_distorted.json",
        "reference_data/inputs/SiC_moissanite.json",
        "reference_data/inputs/SiCGe_wurtzite_like.json",
        "reference_data/inputs/SiC_moissanite_supercell.json",
        "reference_data/inputs/methane.json"};
    const double cutoff{2.5};
    const double cutoff_skin{0.};

    json factory_args{};
    std::vector<Structure_t> structures{};
  };

  /** Contains a multi species periodic structure to test the sparsity of the
   * gradient keys
   */
  struct ComplexPeriodicNLCCStrictFixture {
    using ManagerTypeHolder_t =
        StructureManagerTypeHolder<StructureManagerCenters,
                                   AdaptorNeighbourList,
                                   AdaptorCenterContribution, AdaptorStrict>;
    using Structure_t = AtomicStructure<3>;

    ComplexPeriodicNLCCStrictFixture() {
      for (auto && filename : filenames) {
        json parameters;
        json structure{{"filename", filename}};
        json adaptors;
        json ad1{{"name", "AdaptorNeighbourList"},
                 {"initialization_arguments",
                  {{"cutoff", cutoff}, {"skin", cutoff_skin}}}};
        json ad1b{{"name", "AdaptorCenterContribution"},
                  {"initialization_arguments", {}}};
        json ad2{{"name", "AdaptorStrict"},
                 {"initialization_arguments", {{"cutoff", cutoff}}}};
        adaptors.emplace_back(ad1);
        adaptors.push_back(ad1b);
        adaptors.emplace_back(ad2);

        parameters["structure"] = structure;
        parameters["adaptors"] = adaptors;

        this->factory_args.emplace_back(parameters);
      }
    }

    ~ComplexPeriodicNLCCStrictFixture() = default;

    const std::vector<std::string> filenames{
        "reference_data/inputs/CaCrP2O7_mvc-11955_symmetrized.json"};

    const double cutoff{3.5};
    const double cutoff_skin{0.};

    json factory_args{};
    std::vector<Structure_t> structures{};
  };

  template <typename DataFixture>
  struct SingleHypersSphericalExpansion : DataFixture {
    using Parent = DataFixture;
    using ManagerTypeHolder_t = typename Parent::ManagerTypeHolder_t;
    using Representation_t = CalculatorSphericalExpansion;

    SingleHypersSphericalExpansion() : Parent{} {
      for (auto & ri_hyp : this->radial_contribution_hypers) {
        for (auto & fc_hyp : this->fc_hypers) {
          for (auto & sig_hyp : this->density_hypers) {
            for (auto & rep_hyp : this->rep_hypers) {
              rep_hyp["cutoff_function"] = fc_hyp;
              rep_hyp["gaussian_density"] = sig_hyp;
              rep_hyp["radial_contribution"] = ri_hyp;
              this->representation_hypers.push_back(rep_hyp);
            }
          }
        }
      }
    };

    ~SingleHypersSphericalExpansion() = default;

    std::vector<json> representation_hypers{};
    std::vector<json> fc_hypers{
        {{"type", "ShiftedCosine"},
         {"cutoff", {{"value", 2.5}, {"unit", "AA"}}},
         {"smooth_width", {{"value", 0.5}, {"unit", "AA"}}}}};

    std::vector<json> density_hypers{
        {{"type", "Constant"},
         {"gaussian_sigma", {{"value", 0.4}, {"unit", "AA"}}}}};
    std::map<std::string, std::vector<std::vector<std::vector<double>>>>
        projection_matrices{
            {"1",
             {
                 {{0.5, 0.1}, {0.1, 1.5}},
                 {{0.9, 0.2}, {0.9, 1.1}},
                 {{0.4, 0.8}, {0.8, 1.2}},
             }},
            {"6",
             {
                 {{0.5, 0.1}, {0.1, 1.5}},
                 {{0.9, 0.2}, {0.9, 1.1}},
                 {{0.4, 0.8}, {0.8, 1.2}},
             }},
            {"7",
             {
                 {{0.5, 0.1}, {0.1, 1.5}},
                 {{0.9, 0.2}, {0.9, 1.1}},
                 {{0.4, 0.8}, {0.8, 1.2}},
             }},
            {"8",
             {
                 {{0.5, 0.1}, {0.1, 1.5}},
                 {{0.9, 0.2}, {0.9, 1.1}},
                 {{0.4, 0.8}, {0.8, 1.2}},
             }},
            {"14",
             {
                 {{0.5, 0.1}, {0.1, 1.5}},
                 {{0.9, 0.2}, {0.9, 1.1}},
                 {{0.4, 0.8}, {0.8, 1.2}},
             }},
            {"15",
             {
                 {{0.5, 0.1}, {0.1, 1.5}},
                 {{0.9, 0.2}, {0.9, 1.1}},
                 {{0.4, 0.8}, {0.8, 1.2}},
             }},
            {"20",
             {
                 {{0.5, 0.1}, {0.1, 1.5}},
                 {{0.9, 0.2}, {0.9, 1.1}},
                 {{0.4, 0.8}, {0.8, 1.2}},
             }},
            {"24",
             {
                 {{0.5, 0.1}, {0.1, 1.5}},
                 {{0.9, 0.2}, {0.9, 1.1}},
                 {{0.4, 0.8}, {0.8, 1.2}},
             }},
            {"32",
             {
                 {{0.5, 0.1}, {0.1, 1.5}},
                 {{0.9, 0.2}, {0.9, 1.1}},
                 {{0.4, 0.8}, {0.8, 1.2}},
             }},
        };
    json projection_matrices_hypers{
        {"projection_matrices", projection_matrices}};
    json radial_dim_reduction_spline_hypers{
        {"Spline", {{"accuracy", 1e-8}}},
        {"RadialDimReduction", projection_matrices_hypers}};
    std::vector<json> radial_contribution_hypers{
        {{"type", "GTO"}, {"optimization", {}}},
        {{"type", "DVR"}, {"optimization", {}}},
        {{"type", "GTO"}, {"optimization", {{"Spline", {{"accuracy", 1e-8}}}}}},
        {{"type", "GTO"},
         {"optimization", radial_dim_reduction_spline_hypers}}};
    // if new hypers are added or current ones changed there will be problems
    // with the projection_matrices defined above since their size depend on
    // max_radial and max_angular
    std::vector<json> rep_hypers{
        {{"max_radial", 2}, {"max_angular", 2}, {"compute_gradients", true}}};
  };

  template <typename DataFixture>
  struct SingleHypersSphericalInvariants : DataFixture {
    using Parent = DataFixture;
    using ManagerTypeHolder_t = typename Parent::ManagerTypeHolder_t;
    using Representation_t = CalculatorSphericalInvariants;

    SingleHypersSphericalInvariants() : Parent{} {
      for (auto & ri_hyp : this->radial_contribution_hypers) {
        for (auto & fc_hyp : this->fc_hypers) {
          for (auto & sig_hyp : this->density_hypers) {
            for (auto & rep_hyp : this->rep_hypers) {
              rep_hyp["cutoff_function"] = fc_hyp;
              rep_hyp["gaussian_density"] = sig_hyp;
              rep_hyp["radial_contribution"] = ri_hyp;
              this->representation_hypers.push_back(rep_hyp);
            }
          }
        }
      }
    };

    ~SingleHypersSphericalInvariants() = default;

    std::vector<json> representation_hypers{};

    std::vector<json> fc_hypers{
        {{"type", "ShiftedCosine"},
         {"cutoff", {{"value", 2.5}, {"unit", "AA"}}},
         {"smooth_width", {{"value", 0.5}, {"unit", "AA"}}}}};

    std::vector<json> density_hypers{
        {{"type", "Constant"},
         {"gaussian_sigma", {{"value", 0.4}, {"unit", "AA"}}}}};
    std::vector<json> radial_contribution_hypers{{{"type", "GTO"}}};
    std::vector<json> rep_hypers{{{"max_radial", 2},
                                  {"max_angular", 2},
                                  {"normalize", true},
                                  {"soap_type", "PowerSpectrum"},
                                  {"compute_gradients", true}},
                                 {{"max_radial", 2},
                                  {"max_angular", 0},
                                  {"normalize", true},
                                  {"soap_type", "RadialSpectrum"},
                                  {"compute_gradients", true}}};
  };

  struct ComplexHypersSphericalInvariants : ComplexPeriodicNLCCStrictFixture {
    using Parent = ComplexPeriodicNLCCStrictFixture;
    using ManagerTypeHolder_t = typename Parent::ManagerTypeHolder_t;
    using Representation_t = CalculatorSphericalInvariants;

    ComplexHypersSphericalInvariants() : Parent{} {
      for (auto & ri_hyp : this->radial_contribution_hypers) {
        for (auto & fc_hyp : this->fc_hypers) {
          for (auto & sig_hyp : this->density_hypers) {
            for (auto & rep_hyp : this->rep_hypers) {
              rep_hyp["cutoff_function"] = fc_hyp;
              rep_hyp["gaussian_density"] = sig_hyp;
              rep_hyp["radial_contribution"] = ri_hyp;
              this->representation_hypers.push_back(rep_hyp);
            }
          }
        }
      }
    };

    ~ComplexHypersSphericalInvariants() = default;

    std::vector<json> representation_hypers{};
    std::vector<json> fc_hypers{
        {{"type", "ShiftedCosine"},
         {"cutoff", {{"value", 3.5}, {"unit", "AA"}}},
         {"smooth_width", {{"value", 1.0}, {"unit", "AA"}}}}};

    std::vector<json> density_hypers{
        {{"type", "Constant"},
         {"gaussian_sigma", {{"value", 0.4}, {"unit", "AA"}}}}};
    std::vector<json> radial_contribution_hypers{{{"type", "GTO"}}};
    std::vector<json> rep_hypers{{{"max_radial", 2},
                                  {"max_angular", 2},
                                  {"normalize", false},
                                  {"soap_type", "PowerSpectrum"},
                                  {"compute_gradients", true}},
                                 {{"max_radial", 2},
                                  {"max_angular", 2},
                                  {"normalize", true},
                                  {"soap_type", "PowerSpectrum"},
                                  {"compute_gradients", true}}};
  };

  struct SphericalExpansionTestData : TestData {
    using Parent = TestData;
    using ManagerTypeHolder_t = typename Parent::ManagerTypeHolder_t;
    using Representation_t = CalculatorSphericalExpansion;

    SphericalExpansionTestData() : Parent{} {
      this->get_ref(this->ref_filename);
    }
    ~SphericalExpansionTestData() = default;
    bool verbose{false};
    std::string ref_filename{
        "reference_data/tests_only/spherical_expansion_reference.ubjson"};
  };

  /**
   * Calculator specialized to testing the derivative of the RadialIntegral
   * in the definition of the SphericalExpansion representation.
   */
  template <class RadialIntegral, class ClusterRef>
  struct SphericalExpansionRadialDerivative {
    SphericalExpansionRadialDerivative(std::shared_ptr<RadialIntegral> ri,
                                       ClusterRef & pair_in)
        : radial_integral{ri}, pair{pair_in}, max_radial{ri->max_radial},
          max_angular{ri->max_angular} {}

    ~SphericalExpansionRadialDerivative() = default;

    Eigen::Array<double, 1, Eigen::Dynamic>
    f(const Eigen::Matrix<double, 1, 1> & input_v) {
      Eigen::ArrayXXd result(this->max_radial, this->max_angular + 1);
      result = this->radial_integral->template compute_neighbour_contribution(
          input_v(0), this->pair, this->pair.get_atom_type());
      Eigen::Map<Eigen::Array<double, 1, Eigen::Dynamic>> result_flat(
          result.data(), 1, result.size());
      return result_flat;
    }

    Eigen::Array<double, 1, Eigen::Dynamic>
    grad_f(const Eigen::Matrix<double, 1, 1> & input_v) {
      Eigen::ArrayXXd result(this->max_radial, this->max_angular + 1);
      result = this->radial_integral->template compute_neighbour_derivative(
          input_v(0), this->pair, this->pair.get_atom_type());
      Eigen::Map<Eigen::Array<double, 1, Eigen::Dynamic>> result_flat(
          result.data(), 1, result.size());
      return result_flat;
    }

    std::shared_ptr<RadialIntegral> radial_integral;
    ClusterRef & pair;
    size_t max_radial{6};
    size_t max_angular{4};
  };

  template <class BaseFixture, internal::RadialBasisType RadialType,
            internal::AtomicSmearingType SmearingType,
            internal::OptimizationType OptType>
  struct RadialIntegralHandlerFixture : MultipleStructureFixture<BaseFixture> {
    using Parent = MultipleStructureFixture<BaseFixture>;
    using Manager_t = typename Parent::Manager_t;
    using RadialIntegral_t =
        internal::RadialContributionHandler<RadialType, SmearingType, OptType>;

    RadialIntegralHandlerFixture() : Parent{} {
      // filter out the hypers that don't correspond to the current RadialType,
      // SmearingType or OptType
      std::vector<json> hypers_temp;

      for (const auto & hyper : this->representation_hypers) {
        // This block is to ignore hypers which do not agree with the type of
        // the fixture. This way we do not have to create a fixture for each
        // type while not using the wrong templated RadialIntegralHandler
        // constructor
        auto radial_contribution_hypers =
            hyper.at("radial_contribution").template get<json>();
        auto radial_contribution_type_name =
            radial_contribution_hypers.at("type").template get<std::string>();
        auto density_hypers = hyper.at("gaussian_density").template get<json>();
        auto smearing_type_name =
            density_hypers.at("type").template get<std::string>();
        auto optimization_hypers =
            radial_contribution_hypers.at("optimization").template get<json>();
        bool using_spline = optimization_hypers.count("Spline");
        bool using_radial_dim_reduction =
            optimization_hypers.count("RadialDimReduction");

        internal::RadialBasisType radial_contribution_type{};
        internal::AtomicSmearingType smearing_type{};
        internal::OptimizationType optimization_type{};
        if (radial_contribution_type_name == "GTO") {
          radial_contribution_type = internal::RadialBasisType::GTO;
        } else if (radial_contribution_type_name == "DVR") {
          radial_contribution_type = internal::RadialBasisType::DVR;
        } else {
          throw std::runtime_error(
              "Wrong radial basis type for RadialIntegralHandler tests");
        }

        if (smearing_type_name == "Constant") {
          smearing_type = internal::AtomicSmearingType::Constant;
        } else {
          throw std::runtime_error(
              "Wrong smearing type for RadialIntegralHandler tests");
        }

        if (using_spline && using_radial_dim_reduction) {
          optimization_type =
              internal::OptimizationType::RadialDimReductionSpline;
        } else if (using_spline) {
          optimization_type = internal::OptimizationType::Spline;
        } else if (using_radial_dim_reduction) {
          throw std::runtime_error(
              "RadialDimReduction without Spline is not supported.");
        } else {
          optimization_type = internal::OptimizationType::None;
        }
        auto hypers_radial_contribution_handler_type{internal::combine_enums(
            radial_contribution_type, smearing_type, optimization_type)};

        auto radial_contribution_handler_type{
            internal::combine_enums(RadialType, SmearingType, OptType)};

        if (hypers_radial_contribution_handler_type ==
            radial_contribution_handler_type) {
          hypers_temp.push_back(hyper);
        }
      }
      this->representation_hypers.clear();
      this->representation_hypers = std::move(hypers_temp);
    }
    ~RadialIntegralHandlerFixture() = default;
  };

  /**
   * Gradient provider specialized to testing the gradient of a Calculator
   *
   * The gradient is tested center-by-center, by iterating over each center and
   * doing finite displacements on its position.  This iteration should normally
   * be done by the RepresentationCalculatorGradientFixture class.
   *
   * In the case of periodic structures, the gradient is accumulated only onto
   * _real_ atoms, but the motion of all _images_ of the "moving" atom (the one
   * with respect to which the gradient is being taken) is taken into account.
   *
   * Initialize with a Calculator, a StructureManager, and an
   * AtomicStructure representing the original structure (before modifying with
   * finite-difference displacements).  The gradient of the representation with
   * respect to the center position can then be tested, as usual, with
   * test_gradients() (defined in test_math.hh).
   */
  template <typename Calculator, class StructureManager>
  class RepresentationCalculatorGradientProvider {
   public:
    using Structure_t = AtomicStructure<3>;
    using Key_t = typename Calculator::Key_t;
    static const size_t n_arguments = 3;

    using PairRef_t =
        typename Calculator::template ClusterRef_t<StructureManager, 2>;

    using PairRefKey_t = typename PairRef_t::ThisParentClass;

    // type of the data structure holding the representation and its gradients
    using Prop_t = typename Calculator::template Property_t<StructureManager>;
    using PropGrad_t =
        typename Calculator::template PropertyGradient_t<StructureManager>;

    template <typename T, class V>
    friend class RepresentationCalculatorGradientFixture;

    RepresentationCalculatorGradientProvider(
        Calculator & representation,
        std::shared_ptr<StructureManager> structure_manager,
        Structure_t atomic_structure)
        : representation{representation}, structure_manager{structure_manager},
          atomic_structure{atomic_structure}, center_it{
                                                  structure_manager->begin()} {
      for (auto center : this->structure_manager) {
        this->n_neighbors.push_back(center.pairs().size());
      }
    }

    ~RepresentationCalculatorGradientProvider() = default;

    Eigen::Array<double, 1, Eigen::Dynamic>
    f(const Eigen::Ref<const Eigen::Vector3d> & center_position) {
      auto center = *center_it;
      Structure_t modified_structure{this->atomic_structure};
      modified_structure.positions.col(center.get_index()) = center_position;
      modified_structure.wrap();
      this->structure_manager->update(modified_structure);
      int i_center{0};
      for (auto center : this->structure_manager) {
        if (this->n_neighbors[i_center] != center.pairs().size()) {
          throw std::runtime_error(
              R"(The number of neighbors has changed when making finite
              displacements. This happens because a neighbor is almost at the
              cutoff boundary so please change the structure or the cutoff to
              avoid this.)");
        }
        ++i_center;
      }

      this->representation.compute(this->structure_manager);

      auto && data_sparse{*structure_manager->template get_property<Prop_t>(
          representation.get_name())};
      auto && gradients_sparse{
          *structure_manager->template get_property<PropGrad_t>(
              representation.get_gradient_name())};
      auto ii_pair = center.get_atom_ii();
      auto & data_center{data_sparse[ii_pair]};
      auto keys_center = gradients_sparse.get_keys(ii_pair);
      Key_t center_key{center.get_atom_type()};
      size_t n_entries_per_key{static_cast<size_t>(data_sparse.get_nb_comp())};
      size_t n_entries_center{n_entries_per_key * keys_center.size()};
      size_t n_entries_neighbours{0};
      // Count all the keys in the sparse gradient structure where the gradient
      // is nonzero (i.e. where the key has an entry in the structure)
      for (auto neigh : center.pairs()) {
        if (this->structure_manager->is_ghost_atom(neigh)) {
          // Don't compute gradient contributions onto ghost atoms
          continue;
        }
        auto swapped_ref{std::move(swap_pair_ref(neigh).front())};
        n_entries_neighbours +=
            (gradients_sparse[swapped_ref].get_keys().size() *
             n_entries_per_key);
      }
      // Packed array containing: The center coefficients (all species) and
      // the neighbour coefficients (only same species as center)
      Eigen::ArrayXd data_pairs(n_entries_center + n_entries_neighbours);

      size_t result_idx{0};
      for (auto & key : keys_center) {
        Eigen::Map<Eigen::RowVectorXd> data_flat(data_center[key].data(),
                                                 n_entries_per_key);
        data_pairs.segment(result_idx, n_entries_per_key) = data_flat;
        result_idx += n_entries_per_key;
      }
      for (auto neigh : center.pairs()) {
        if (this->structure_manager->is_ghost_atom(neigh)) {
          // Don't compute gradient contributions onto ghost atoms
          continue;
        }
        auto & data_neigh{data_sparse[neigh]};
        // The neighbour gradient (i =/= j) only contributes to certain species
        // channels (keys), in the case of SOAP and SphExpn those keys
        // containing the species of the center (the atom wrt the derivative is
        // being taken)
        // The nonzero gradient keys are already indicated in the sparse
        // gradient structure
        auto swapped_ref{std::move(swap_pair_ref(neigh).front())};
        auto keys_neigh{gradients_sparse[swapped_ref].get_keys()};
        for (auto & key : keys_neigh) {
          Eigen::Map<Eigen::ArrayXd> data_flat(data_neigh[key].data(),
                                               n_entries_per_key);
          data_pairs.segment(result_idx, n_entries_per_key) = data_flat;
          result_idx += n_entries_per_key;
        }
      }

      // Reset the atomic structure for the next iteration
      this->structure_manager->update(this->atomic_structure);
      return data_pairs.transpose();
    }

    Eigen::Array<double, 3, Eigen::Dynamic>
    grad_f(const Eigen::Ref<const Eigen::Vector3d> & /*center_position*/) {
      using Matrix3Xd_RowMaj_t =
          Eigen::Matrix<double, 3, Eigen::Dynamic, Eigen::RowMajor>;
      // Assume f() was already called and updated the position
      // center_it->position() = center_position;
      // representation.compute();
      auto center = *center_it;

      auto && data_sparse{*structure_manager->template get_property<Prop_t>(
          representation.get_name())};
      auto && gradients_sparse{
          *structure_manager->template get_property<PropGrad_t>(
              representation.get_gradient_name())};
      auto ii_pair = center.get_atom_ii();
      auto & gradients_center{gradients_sparse[ii_pair]};
      auto keys_center = gradients_center.get_keys();
      size_t n_entries_per_key{static_cast<size_t>(data_sparse.get_nb_comp())};
      size_t n_entries_center{n_entries_per_key * keys_center.size()};
      size_t n_entries_neighbours{0};
      for (auto neigh : center.pairs()) {
        if (this->structure_manager->is_ghost_atom(neigh)) {
          // Don't compute gradient contributions onto ghost atoms
          continue;
        }
        auto swapped_ref{std::move(swap_pair_ref(neigh).front())};
        n_entries_neighbours +=
            (gradients_sparse[swapped_ref].get_keys().size() *
             n_entries_per_key);
      }
      Eigen::Matrix<double, 3, Eigen::Dynamic, Eigen::RowMajor>
          grad_coeffs_pairs(3, n_entries_center + n_entries_neighbours);
      grad_coeffs_pairs.setZero();

      // Use the exact same iteration pattern as in f()  to guarantee that the
      // gradients appear in the same place as their corresponding data
      size_t result_idx{0};
      for (auto & key : keys_center) {
        // Here the 'flattening' retains the 3 Cartesian dimensions as rows,
        // since they vary the slowest within each key
        Eigen::Map<Matrix3Xd_RowMaj_t> grad_coeffs_flat(
            gradients_center[key].data(), 3, n_entries_per_key);
        grad_coeffs_pairs.block(0, result_idx, 3, n_entries_per_key) =
            grad_coeffs_flat;
        result_idx += n_entries_per_key;
      }
      for (auto neigh : center.pairs()) {
        if (this->structure_manager->is_ghost_atom(neigh)) {
          // Don't compute gradient contributions onto ghost atoms
          continue;
        }
        // We need grad_i c^{ji} -- using just 'neigh' would give us
        // grad_j c^{ij}, hence the swap
        auto neigh_swap_images{swap_pair_ref(neigh)};
        auto & gradients_neigh_first{
            gradients_sparse[neigh_swap_images.front()]};
        // The set of species keys should be the same for all images of i
        auto keys_neigh{gradients_neigh_first.get_keys()};
        for (auto & key : keys_neigh) {
          // For each key, accumulate gradients over periodic images of the atom
          // that moves in the finite-difference step
          for (auto & neigh_swap : neigh_swap_images) {
            auto & gradients_neigh{gradients_sparse[neigh_swap]};
            Eigen::Map<Matrix3Xd_RowMaj_t> grad_coeffs_flat(
                gradients_neigh[key].data(), 3, n_entries_per_key);
            grad_coeffs_pairs.block(0, result_idx, 3, n_entries_per_key) +=
                grad_coeffs_flat;
          }
          result_idx += n_entries_per_key;
        }
      }
      return grad_coeffs_pairs;
    }

   private:
    Calculator & representation;
    std::shared_ptr<StructureManager> structure_manager;
    Structure_t atomic_structure;
    typename StructureManager::iterator center_it;
    //! count the number of neighbours of each centers
    std::vector<size_t> n_neighbors{};

    void advance_center() { ++this->center_it; }

    /**
     * Swap a ClusterRef<order=2> (i, j) so it refers to (j, i) instead
     *
     * @return std::vector of ClusterRefKeys or order 2 (pair keys) of all pairs
     *         (j, i') where i' is either i or any of its periodic images within
     *         the cutoff of j. The atom j, on the other hand, must be a real
     *         atom (not a ghost or periodic image).
     */
    std::vector<PairRefKey_t> swap_pair_ref(const PairRef_t & pair_ref) {
      auto center_manager{extract_underlying_manager<0>(structure_manager)};
      auto atomic_structure{center_manager->get_atomic_structure()};
      // Get the atom index to the corresponding atom tag
      size_t access_index{structure_manager->get_atom_index(pair_ref.back())};
      auto new_center_it{structure_manager->get_iterator_at(access_index)};
      // Return cluster ref at which the iterator is currently pointing
      auto && new_center{*new_center_it};
      size_t i_index{structure_manager->get_atom_index(pair_ref.front())};

      // Find all (j, i') pairs
      std::vector<PairRefKey_t> new_pairs;
      for (auto new_pair : new_center.pairs()) {
        size_t i_trial_index{
            structure_manager->get_atom_index(new_pair.back())};
        // Is this the i (old center) atom or any of its images?
        if (i_trial_index == i_index) {
          new_pairs.emplace_back(std::move(new_pair));
        }
      }
      if (new_pairs.size() == 0) {
        std::stringstream err_str{};
        err_str << "Didn't find any pairs for pair (i=" << pair_ref.front()
                << ", j=" << pair_ref.back()
                << "); access index for j = " << access_index;
        throw std::range_error(err_str.str());
      }
      return new_pairs;
    }
  };

  /**
   * Test fixture holding the gradient calculator and structure manager
   *
   * Holds data (function values, gradient directions, verbosity) and iterates
   * through the list of centers
   */
  template <typename Calculator, class StructureManager>
  class RepresentationCalculatorGradientFixture : public GradientTestFixture {
   public:
    using StdVector2Dim_t = std::vector<std::vector<double>>;
    using Provider_t =
        RepresentationCalculatorGradientProvider<Calculator, StructureManager>;

    static const size_t n_arguments = 3;
    /**
     * Increased error tolerance because some representations have quite large
     * finite-difference truncation errors (and possibly numerical issues for
     * very small displacements)
     */
    double fd_error_tol{1E-4};

    /**
     * Initialize a gradient test fixture
     *
     * @param filename JSON file holding gradient test parameters, format
     *                 documented in GradientTestFixture
     *
     * @param structure StructureManager on which to test
     *
     * @param calc RepresentationCalculator whose gradient is being tested
     */
    RepresentationCalculatorGradientFixture(
        std::string filename, std::shared_ptr<StructureManager> structure,
        Provider_t & calc)
        : structure{structure}, center_it{structure->begin()}, provider{calc} {
      json input_data = json_io::load(filename);

      this->function_inputs = this->get_function_inputs();
      this->displacement_directions =
          this->get_displacement_directions(input_data, this->n_arguments);
      this->verbosity = get_verbosity(input_data);
      if (input_data.find("fd_error_tol") != input_data.end()) {
        this->fd_error_tol = input_data["fd_error_tol"].get<double>();
      }
    }

    ~RepresentationCalculatorGradientFixture() = default;

    const Provider_t & get_provider() { return provider; }

    /**
     * Go to the next center in the structure
     *
     * Not (yet) implemented as iterator because that overcomplicates things
     */
    void advance_center() {
      ++this->center_it;
      this->provider.advance_center();
      if (this->has_next()) {
        this->function_inputs = get_function_inputs();
      }
    }

    bool has_next() { return (this->center_it != structure->end()); }

   private:
    StdVector2Dim_t get_function_inputs() {
      StdVector2Dim_t inputs_new{};
      auto center_pos = (*center_it).get_position();
      inputs_new.emplace_back(center_pos.data(),
                              center_pos.data() + center_pos.size());
      return inputs_new;
    }

    std::shared_ptr<StructureManager> structure;
    typename StructureManager::iterator center_it;
    Provider_t & provider;
  };

  template <class MultipleStructureFixture>
  struct MultipleStructureSortedCoulomb : MultipleStructureFixture {
    using Parent = MultipleStructureFixture;
    using ManagerTypeHolder_t = typename Parent::ManagerTypeHolder_t;
    using Representation_t = CalculatorSortedCoulomb;
    MultipleStructureSortedCoulomb() : Parent{} {};
    ~MultipleStructureSortedCoulomb() = default;

    std::vector<json> representation_hypers{
        {{"central_cutoff", 3.},
         {"central_decay", 0.5},
         {"interaction_cutoff", 10.},
         {"interaction_decay", 0.5},
         {"size", 120},
         {"sorting_algorithm", "distance"}},
        {{"central_cutoff", 3.},
         {"central_decay", 0.5},
         {"interaction_cutoff", 10.},
         {"interaction_decay", 0.5},
         {"size", 120},
         {"sorting_algorithm", "row_norm"}}};
  };

  struct SortedCoulombTestData {
    using ManagerTypeHolder_t =
        StructureManagerTypeHolder<StructureManagerCenters,
                                   AdaptorNeighbourList, AdaptorStrict>;
    using Representation_t = CalculatorSortedCoulomb;
    SortedCoulombTestData() { this->get_ref(this->ref_filename); }
    ~SortedCoulombTestData() = default;

    void get_ref(const std::string & ref_filename) {
      this->ref_data =
          json::from_ubjson(internal::read_binary_file(ref_filename));
      auto filenames =
          this->ref_data.at("filenames").get<std::vector<std::string>>();
      auto cutoffs = this->ref_data.at("cutoffs").get<std::vector<double>>();

      for (auto && filename : filenames) {
        for (auto && cutoff : cutoffs) {
          // std::cout << filename << " " << cutoff << std::endl;
          json parameters;
          json structure{{"filename", filename}};
          json adaptors;
          json ad1{{"name", "AdaptorNeighbourList"},
                   {"initialization_arguments", {{"cutoff", cutoff}}}};
          json ad2{{"name", "AdaptorStrict"},
                   {"initialization_arguments", {{"cutoff", cutoff}}}};
          adaptors.emplace_back(ad1);
          adaptors.emplace_back(ad2);

          parameters["structure"] = structure;
          parameters["adaptors"] = adaptors;

          this->factory_args.emplace_back(parameters);
        }
      }
    }

    json ref_data{};
    json factory_args{};

    std::string ref_filename{
        "reference_data/tests_only/sorted_coulomb_reference.ubjson"};
    bool verbose{false};
  };

  template <class BaseFixture>
  struct CalculatorFixture : MultipleStructureFixture<BaseFixture> {
    using Parent = MultipleStructureFixture<BaseFixture>;
    using Manager_t = typename Parent::Manager_t;
    using Representation_t = typename BaseFixture::Representation_t;
    using Property_t =
        typename Representation_t::template Property_t<Manager_t>;

    CalculatorFixture() : Parent{} {}
    ~CalculatorFixture() = default;

    std::vector<Representation_t> representations{};
  };

  /* ---------------------------------------------------------------------- */

}  // namespace rascal

#endif  // TESTS_TEST_CALCULATOR_HH_
