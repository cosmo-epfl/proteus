/**
 * @file   performance/profiles/profile_spherical_expansion.cc
 *
 * @author Max Veit <max.veit@epfl.ch>
 *
 * @date   7 May 2019
 *
 * @brief  Example for profiling the spherical expansion
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

// using namespace std;
using namespace rascal;  // NOLINT

const int N_ITERATIONS = 20;

using Representation_t = CalculatorSphericalInvariants;
using Manager_t = AdaptorStrict<
    AdaptorCenterContribution<AdaptorNeighbourList<StructureManagerCenters>>>;
using Prop_t = typename CalculatorSphericalInvariants::Property_t<Manager_t>;
using PropGrad_t =
    typename CalculatorSphericalInvariants::PropertyGradient_t<Manager_t>;
bool VERBOSE{false};
int main(int argc, char * argv[]) {
  if (argc < 2) {
    std::cerr << "Must provide atomic structure json filename as argument";
    std::cerr << std::endl;
    return -1;
  }

  // TODO(max) put these in a file so they can be varied systematically
  // maybe together with the filename and iteration count
  std::string filename{argv[1]};

  double cutoff{5.};
  json hypers{{"max_radial", 8},
              {"max_angular", 6},
              {"soap_type", "PowerSpectrum"},
              {"normalize", true},
              {"compute_gradients", false}};
  json fc_hypers{{"type", "ShiftedCosine"},
                 {"cutoff", {{"value", cutoff}, {"unit", "AA"}}},
                 {"smooth_width", {{"value", 0.5}, {"unit", "AA"}}}};
  json sigma_hypers{{"type", "Constant"},
                    {"gaussian_sigma", {{"value", 0.4}, {"unit", "AA"}}}};

  hypers["cutoff_function"] = fc_hypers;
  hypers["gaussian_density"] = sigma_hypers;
  hypers["radial_contribution"] = {{"type", "GTO"}};

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

  AtomicStructure<3> ast{};
  ast.set_structure(filename);

  std::cout << "structure filename: " << filename << std::endl;

  std::chrono::duration<double> elapsed{};

  auto start = std::chrono::high_resolution_clock::now();
  // This is the part that should get profiled
  for (size_t looper{0}; looper < N_ITERATIONS; looper++) {
    manager->update(ast);
  }
  auto finish = std::chrono::high_resolution_clock::now();

  elapsed = finish - start;
  std::cout << "Neighbour List"
            << " elapsed: " << elapsed.count() / N_ITERATIONS << " seconds"
            << std::endl;

  Representation_t representation{hypers};

  start = std::chrono::high_resolution_clock::now();
  // This is the part that should get profiled
  for (size_t looper{0}; looper < N_ITERATIONS; looper++) {
    representation.compute(manager);
  }
  finish = std::chrono::high_resolution_clock::now();

  elapsed = finish - start;
  std::cout << "Compute representation"
            << " elapsed: " << elapsed.count() / N_ITERATIONS << " seconds"
            << std::endl;

  if (VERBOSE) {
    auto expn = manager
                    ->template get_property<PropGrad_t>(
                        representation.get_gradient_name())
                    ->get_features();
    std::cout << "Sample SphericalExpansion elements " << std::endl
              << expn(0, 0) << " " << expn(0, 1) << " " << expn(0, 2) << "\n"
              << expn(1, 0) << " " << expn(1, 1) << " " << expn(1, 2) << "\n"
              << expn(2, 0) << " " << expn(2, 1) << " " << expn(2, 2) << "\n";
  }

  // Profile again, this time with gradients
  hypers["compute_gradients"] = true;
  Representation_t representation_gradients{hypers};
  start = std::chrono::high_resolution_clock::now();
  // This is the part that should get profiled
  for (size_t looper{0}; looper < N_ITERATIONS; looper++) {
    representation_gradients.compute(manager);
  }
  finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_grad{};
  elapsed_grad = finish - start;
  std::cout << "Compute representation with gradients"
            << " elapsed: " << elapsed_grad.count() / N_ITERATIONS << " seconds"
            << std::endl;
  std::cout << "Ratio (with gradients / without gradients): "
            << elapsed_grad.count() / elapsed.count() << std::endl;

  if (VERBOSE) {
    auto expn2 = manager
                     ->template get_property<PropGrad_t>(
                         representation_gradients.get_gradient_name())
                     ->get_features();
    std::cout << "Sample SphericalExpansion elements (should be identical) "
              << std::endl
              << expn2(0, 0) << " " << expn2(0, 1) << " " << expn2(0, 2) << "\n"
              << expn2(1, 0) << " " << expn2(1, 1) << " " << expn2(1, 2) << "\n"
              << expn2(2, 0) << " " << expn2(2, 1) << " " << expn2(2, 2)
              << "\n";
  }
  // TODO(max) print out analogous gradient components, for now see
  // spherical_expansion_example
}
