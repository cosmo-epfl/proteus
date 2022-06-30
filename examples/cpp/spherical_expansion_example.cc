/**
 * @file   examples/spherical_expansion_example.cc
 *
 * @author Max Veit <max.veit@epfl.ch>
 *
 * @date   26 June 2019
 *
 * @brief  Example for computing the spherical expansion
 *
 * Copyright © 2018 Max Veit, Felix Musil, COSMO (EPFL), LAMMM (EPFL)
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
#include "rascal/structure_managers/adaptor_center_contribution.hh"
#include "rascal/structure_managers/adaptor_neighbour_list.hh"
#include "rascal/structure_managers/adaptor_strict.hh"
#include "rascal/structure_managers/atomic_structure.hh"
#include "rascal/structure_managers/make_structure_manager.hh"
#include "rascal/structure_managers/structure_manager_centers.hh"
#include "rascal/utils/basic_types.hh"
#include "rascal/utils/utils.hh"

#include <chrono>
#include <cmath>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <list>
#include <string>

using namespace rascal;  // NOLINT

using Representation_t = CalculatorSphericalExpansion;
using Manager_t = AdaptorStrict<
    AdaptorCenterContribution<AdaptorNeighbourList<StructureManagerCenters>>>;
using Prop_t = typename CalculatorSphericalInvariants::Property_t<Manager_t>;
using PropGrad_t =
    typename CalculatorSphericalInvariants::PropertyGradient_t<Manager_t>;

int main(int argc, char * argv[]) {
  if (argc < 2) {
    std::cerr << "Must provide atomic structure json filename as argument";
    std::cerr << std::endl;
    return -1;
  }

  std::string filename{argv[1]};

  double cutoff{4.};
  json hypers{
      {"max_radial", 3}, {"max_angular", 0}, {"compute_gradients", true}};
  // {"soap_type", "PowerSpectrum"},
  // {"normalize", true}};

  json fc_hypers{{"type", "ShiftedCosine"},
                 {"cutoff", {{"value", cutoff}, {"unit", "AA"}}},
                 {"smooth_width", {{"value", 0.5}, {"unit", "AA"}}}};
  json sigma_hypers{{"type", "Constant"},
                    {"gaussian_sigma", {{"value", 0.4}, {"unit", "AA"}}}};
  std::map<std::string, std::vector<std::vector<std::vector<double>>>>
      projection_matrices{{"1", {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}}}};
  json projection_matrices_hypers{{"projection_matrices", projection_matrices}};
  json radial_dim_reduction_hypers{"RadialDimReduction",
                                   projection_matrices_hypers};
  json spline_accuracy_hypers{{"accuracy", 1e-8}};
  json spline_hypers{"Spline", spline_accuracy_hypers};

  json optimization_hypers{spline_hypers, radial_dim_reduction_hypers};

  hypers["cutoff_function"] = fc_hypers;
  hypers["gaussian_density"] = sigma_hypers;
  hypers["radial_contribution"] = {{"type", "GTO"},
                                   {"optimization", optimization_hypers}};

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
  auto manager =
      make_structure_manager_stack<StructureManagerCenters,
                                   AdaptorNeighbourList,
                                   AdaptorCenterContribution, AdaptorStrict>(
          structure, adaptors);

  Representation_t representation{hypers};
  representation.compute(manager);

  constexpr size_t n_centers_print{4};
  constexpr size_t n_neigh_print{1};

  // Print the first few elements and gradients, so we know we're getting
  // something
  std::cout << "Expansion of first " << n_centers_print << " centers:";
  std::cout << std::endl;
  std::cout << "Note that the coefficients are printed with species along the "
               "columns and n-l-m along the rows."
            << std::endl;
  std::cout << "Gradients are printed with: First Cartesian component, "
               "then species, along the rows; n-l-m along the columns.";
  std::cout << std::endl;

  auto && expansions_coefficients{
      *manager->template get_property<Prop_t>(representation.get_name())};
  auto && expansions_coefficients_gradient{
      *manager->template get_property<PropGrad_t>(
          representation.get_gradient_name())};

  size_t center_count{0};
  for (auto center : manager) {
    if (center_count >= n_centers_print) {
      break;
    }
    auto ii_pair = center.get_atom_ii();
    size_t n_species_center{expansions_coefficients.get_keys(center).size()};
    std::cout << "============================" << std::endl;
    std::cout << "Center " << center.get_index();
    std::cout << " of type " << center.get_atom_type() << std::endl;
    std::cout << expansions_coefficients.get_dense_row(center);
    std::cout << std::endl;
    std::cout << "Gradient of this expansion wrt center pos: " << std::endl;
    std::cout << Eigen::Map<Eigen::MatrixXd>(
        expansions_coefficients_gradient.get_dense_row(ii_pair).data(),
        3 * n_species_center, expansions_coefficients_gradient.get_nb_comp());
    std::cout << std::endl;
    size_t neigh_count{0};
    for (auto neigh : center.pairs()) {
      if (neigh_count >= n_neigh_print) {
        break;
      }
      auto keys_neigh = expansions_coefficients_gradient[neigh].get_keys();
      std::cout << "Neighbour keys: ";
      for (auto key : keys_neigh) {
        std::cout << "(";
        for (auto key_sp : key) {
          std::cout << key_sp << ", ";
        }
        std::cout << "\b\b) ";
      }
      std::cout << std::endl;
      std::cout << "Gradient of the above wrt atom " << neigh.back();
      std::cout << " of type " << neigh.get_atom_type() << std::endl;
      std::cout << Eigen::Map<Eigen::MatrixXd>(
          expansions_coefficients_gradient.get_dense_row(neigh).data(),
          3 * n_species_center, expansions_coefficients_gradient.get_nb_comp());
      std::cout << std::endl;
      ++neigh_count;
    }
    ++center_count;
  }
}
