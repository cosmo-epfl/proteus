/**
 * @file   performance/profiles/profile_kernel.cc
 *
 * @author Felix Musil <felix.musil@epfl.ch>
 *
 * @date   12 December 2019
 *
 * @brief  Example for profiling the spherical invariants (SOAP)
 *
 * Copyright © 2019 Felix Musil, COSMO (EPFL), LAMMM (EPFL)
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

#include "rascal/atomic_structure.hh"
#include "rascal/basic_types.hh"
#include "rascal/models/kernels.hh"
#include "rascal/representations/calculator_sorted_coulomb.hh"
#include "rascal/representations/calculator_spherical_expansion.hh"
#include "rascal/representations/calculator_spherical_invariants.hh"
#include "rascal/structure_managers/adaptor_center_contribution.hh"
#include "rascal/structure_managers/adaptor_neighbour_list.hh"
#include "rascal/structure_managers/adaptor_strict.hh"
#include "rascal/structure_managers/make_structure_manager.hh"
#include "rascal/structure_managers/structure_manager_centers.hh"
#include "rascal/structure_managers/structure_manager_collection.hh"
#include "rascal/utils.hh"

#include <chrono>
#include <cmath>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <list>
#include <string>

// using namespace std;
using namespace rascal;  // NOLINT

const int N_ITERATIONS = 100;

using ManagerTypeHolder_t =
    StructureManagerTypeHolder<StructureManagerCenters, AdaptorNeighbourList,
                               AdaptorCenterContribution, AdaptorStrict>;
using ManagerTypeList_t = typename ManagerTypeHolder_t::type_list;
using Manager_t = typename ManagerTypeHolder_t::type;
using ManagerCollection_t =
    typename TypeHolderInjector<ManagerCollection, ManagerTypeList_t>::type;
using Representation_t = CalculatorSphericalInvariants;

int main() {
  std::string filename{"../reference_data/inputs/small_molecules-20.json"};

  double cutoff{3.5};

  json hypers{{"max_radial", 8},
              {"max_angular", 6},
              {"compute_gradients", false},
              {"soap_type", "PowerSpectrum"},
              {"normalize", true},
              {"expansion_by_species_method", "user defined"},
              {"global_species", {1, 6, 7, 8}}};

  json fc_hypers{{"type", "ShiftedCosine"},
                 {"cutoff", {{"value", cutoff}, {"unit", "AA"}}},
                 {"smooth_width", {{"value", 0.5}, {"unit", "AA"}}}};
  json sigma_hypers{{"type", "Constant"},
                    {"gaussian_sigma", {{"value", 0.4}, {"unit", "AA"}}}};

  hypers["cutoff_function"] = fc_hypers;
  hypers["gaussian_density"] = sigma_hypers;
  hypers["radial_contribution"] = {{"type", "GTO"}};

  json kernel_hypers{
      {"zeta", 2}, {"target_type", "Structure"}, {"name", "Cosine"}};

  json structure{{"filename", filename}};
  json adaptors;
  json ad1{{"name", "AdaptorNeighbourList"},
           {"initialization_arguments", {{"cutoff", cutoff}, {"skin", 0.}}}};
  json ad1b{{"name", "AdaptorCenterContribution"},
            {"initialization_arguments", {}}};
  json ad2{{"name", "AdaptorStrict"},
           {"initialization_arguments", {{"cutoff", cutoff}}}};
  adaptors.emplace_back(ad1);
  adaptors.emplace_back(ad1b);
  adaptors.emplace_back(ad2);

  ManagerCollection_t collection{adaptors};
  collection.add_structures(filename, 0, 20);

  Representation_t soap{hypers};

  soap.compute(collection);

  Kernel kernel{kernel_hypers};

  std::cout << "structure filename: " << filename << std::endl;

  std::chrono::duration<double> elapsed{};

  math::Matrix_t aa{20, 20};
  auto start = std::chrono::high_resolution_clock::now();
  // This is the part that should get profiled
  for (size_t looper{0}; looper < N_ITERATIONS + 600; looper++) {
    auto kk = kernel.compute(soap, collection, collection);
    aa += kk;
  }
  auto finish = std::chrono::high_resolution_clock::now();

  elapsed = finish - start;
  std::cout << "Kernel with user defined species"
            << " elapsed: " << elapsed.count() / (N_ITERATIONS + 600)
            << " seconds" << std::endl;

  hypers["expansion_by_species_method"] = "structure wise";
  Representation_t soap_sw{hypers};

  soap_sw.compute(collection);

  start = std::chrono::high_resolution_clock::now();
  for (size_t looper{0}; looper < N_ITERATIONS; looper++) {
    auto kk = kernel.compute(soap_sw, collection, collection);
    aa += kk;
  }
  finish = std::chrono::high_resolution_clock::now();

  elapsed = finish - start;
  std::cout << "Kernel with structure wise species"
            << " elapsed: " << elapsed.count() / N_ITERATIONS << " seconds"
            << std::endl;

  hypers["expansion_by_species_method"] = "environment wise";
  Representation_t soap_ew{hypers};

  soap_ew.compute(collection);

  start = std::chrono::high_resolution_clock::now();
  for (size_t looper{0}; looper < N_ITERATIONS; looper++) {
    auto kk = kernel.compute(soap_ew, collection, collection);
    aa += kk;
  }
  finish = std::chrono::high_resolution_clock::now();

  elapsed = finish - start;
  std::cout << "Kernel with environment wise species"
            << " elapsed: " << elapsed.count() / N_ITERATIONS << " seconds"
            << std::endl;
}
