/**
 * file   test_nl.cc
 *
 * @author Felix Musil <felix.musil@epfl.ch>
 *
 * @date   2018
 *
 * @brief Example for Neighbour list
 *
 * Copyright © 2018 Felix Musil, COSMO (EPFL), LAMMM (EPFL)
 *
 * Rascal is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3, or (at
 * your option) any later version.
 *
 * Rascal is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software; see the file LICENSE. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "structure_managers/structure_manager_centers.hh"
#include "structure_managers/adaptor_strict.hh"
#include "structure_managers/adaptor_neighbour_list.hh"
#include "structure_managers/make_structure_manager.hh"
#include "rascal_utility.hh"
#include "representations/representation_manager_sorted_coulomb.hh"
#include "basic_types.hh"

#include <Eigen/StdVector>

#include <iostream>
#include <basic_types.hh>
#include <cmath>
#include <list>

//using namespace std;
using namespace rascal; // NOLINT

//using Manager_t = StructureManagerCenters;
constexpr static int dim{3};
using Vector_t = Eigen::Matrix<double, dim, 1>;

using Representation_t = RepresentationManagerSortedCoulomb<
              AdaptorStrict<AdaptorNeighbourList<StructureManagerCenters>>>;


template<class StructureManager>
struct MultipleStrictStructureManager {
  using Manager1_t = StructureManager;
  using Manager2_t = AdaptorNeighbourList<Manager1_t>;
  using Manager_t = AdaptorStrict<Manager2_t>;

  MultipleStrictStructureManager() {
    std::vector<std::string> filenames{
      {"alanine-X.json"}
                                        };
    std::vector<double> cutoffs{{3, 4}};
    for (auto filename : filenames) {
      for (auto cutoff : cutoffs) {
        auto manager = make_structure_manager_stack<
              StructureManager,AdaptorNeighbourList,AdaptorStrict>
                (filename, std::make_tuple(cutoff), std::make_tuple(cutoff));
        this->managers.emplace_back(
                manager
                );
      }
    }
  }

  ~MultipleStrictStructureManager() {}

  // std::list<std::shared_ptr<Manager1_t>> managers1{};
  // std::list<std::shared_ptr<Manager2_t>> managers2{};
  std::list<std::shared_ptr<Manager_t>> managers{};
};

template<template<class> class ...AdaptorImplementation>
struct AdaptorTypeHolder;

template<typename MI, template<class> class ...AdaptorImplementation>
struct TypeHolder {
  using type = std::tuple<MI,AdaptorTypeHolder<AdaptorImplementation...>>;
};


// template<template<class...>class U, typename MI, typename typeholder>
// struct test;

// template<template<class...>class U, typename MI, template<class> class ...T>
// struct test<U, MI, AdaptorTypeHolder<T...>> {
//   template<typename ...Args>
//   static decltype(auto) func(Args ...args) {
//     return U<MI, T...>::apply(args...);
//   }
// };



// template<typename TemplateTypeHolder>
// struct call_with_typeholders;

// template<typename ...T>
// struct call_with_typeholders<std::tuple<T...>> {

//   template<typename ...Args>
//   static decltype(auto) make_manager_stack(Args ...args) {
//     return TypeExtractor<T...>::apply(args...);
//   }

//  protected:

//   template<typename MI, typename TemplateTypeHolder_>
//   struct TypeExtractor;

//   template<typename MI, template<class> class ...Ti>
//   struct TypeExtractor<MI, AdaptorTypeHolder<Ti...>> {
//     using Manager_t = typename internal::AdaptorTypeStacker<MI,Ti...>::type;
//     using ManagerPtr_t = std::shared_ptr<Manager_t>;

//     template<typename ...Args>
//     static ManagerPtr_t apply(Args ...args) {
//       return helper(args...);
//     }

//     template<typename ...Args>
//     static ManagerPtr_t helper(Args ...args) {
//       return make_structure_manager_stack<MI, Ti...>(args...);
//     }
//   };
// };

template<typename TemplateTypeHolder, typename ArgsTypeHolder>
struct call_with_typeholders;

template<typename ...T, typename ...Args>
struct call_with_typeholders<std::tuple<T...>, std::tuple<Args...>> {

  static decltype(auto) make_manager_stack(std::tuple<Args...> tuple) {
    return TypeExtractor<T...>::apply(tuple);
  }

 protected:

  template<typename MI, typename TemplateTypeHolder_>
  struct TypeExtractor;

  template<typename MI, template<class> class ...Ti>
  struct TypeExtractor<MI, AdaptorTypeHolder<Ti...>> {
    using Manager_t = typename internal::AdaptorTypeStacker<MI,Ti...>::type;
    using ManagerPtr_t = std::shared_ptr<Manager_t>;

    static ManagerPtr_t apply(std::tuple<Args...> tuple) {
      return helper(tuple, std::index_sequence_for<Args...>());
    }
    template<std::size_t... Is>
    static ManagerPtr_t helper(std::tuple<Args...> tuple, std::index_sequence<Is...>) {
      return make_structure_manager_stack<MI, Ti...>(std::get<Is>(tuple)...);
    }
  };
};

  // template<typename MI, typename TemplateTypeHolder_>
  // struct TypeExtractor;

  // template<typename MI, template<class> class ...Ti>
  // struct TypeExtractor<MI, AdaptorTypeHolder<Ti...>> {
  //   using Manager_t = typename internal::AdaptorTypeStacker<MI,Ti...>::type;
  //   using ManagerPtr_t = std::shared_ptr<Manager_t>;

  //   template<typename ...Args_>
  //   static ManagerPtr_t apply(std::tuple<Args_...> tuple) {
  //     return helper(tuple, std::index_sequence_for<Args_...>());
  //   }

  //   template<typename ...Args_, std::size_t... Is>
  //   static ManagerPtr_t helper(std::tuple<Args_...> tuple, std::index_sequence<Is...>) {
  //     return make_structure_manager_stack<MI, Ti...>(std::get<Is>(tuple)...);
  //   }
  // };


  // template<typename T>
  // struct call_with_tuple;

  // template<typename ...T>
  // struct call_with_tuple<std::tuple<T...>> {
  // static ManagerPtr_t make_manager_stack(std::tuple<T...> tuple) {
  //   return helper(tuple, std::index_sequence_for<T...>());
  // }

  // template<std::size_t... Is>
  // static ManagerPtr_t helper(std::tuple<T...> tuple, std::index_sequence<Is...>) {
  //   ManagerPtr_t manager{make_structure_manager_stack<StructureManager, AdaptorImplementationPack...>(std::get<Is>(tuple)...)};
  //   return manager;
  // }






int main() {
  bool verbose{false};
  bool verbose_rep{false};
  double cutoff{2.};
  std::string filename{"alanine-X.json"};
  auto a1 = std::make_tuple(cutoff);

  // using AdaptorTypeHolder_t = AdaptorTypeHolder<AdaptorNeighbourList, AdaptorStrict>;
  using Factory_t = std::tuple<std::string,std::tuple<double>,std::tuple<double>>;
  using AdaptorTypeHolder_t = typename TypeHolder<StructureManagerCenters, AdaptorNeighbourList, AdaptorStrict>::type;
  auto aa = std::make_tuple(filename, a1, a1);
  // auto man{call_with_typeholders<AdaptorTypeHolder_t>::make_manager_stack(filename, a1, a1)};
  auto man{call_with_typeholders<AdaptorTypeHolder_t,Factory_t>::make_manager_stack(aa)};
  // auto man{call_with_typeholders<Factory_t, AdaptorTypeHolder_t>::make_structure_manager_stack(aa)};
  std::cout << man->get_name()<< std::endl;


  //"reference_data/CaCrP2O7_mvc-11955_symmetrized_.json",

  // std::ifstream reader("alanine-X.json");
  // std::cout << reader.is_open() << std::endl;
  // std::string str((std::istreambuf_iterator<char>(reader)),
  //                std::istreambuf_iterator<char>());
  // std::cout << str;

  // json j;

  // reader >> j;

  // std::cout << j.dump(2);

  // Eigen::MatrixXd positions(22, 3);
  // Eigen::VectorXi atom_types(22);
  // Eigen::MatrixXd cell(3, 3);
  // std::array<int, 3> pbc{{true, true, true}};
  // cell << 6.19, 2.41, 0.21, 0.00, 6.15, 1.02, 0.00, 0.00, 7.31;

  // // clang-format off
  // positions << 3.689540159937393, 5.123016813620886, 1.994119731169116,
  //     6.818437242389163, 2.630056617829216, 6.182500355729062,
  //     2.114977334498767, 6.697579639059512, 1.392155450018263,
  //     7.420401523540017, 2.432242071439904, 6.380314902118375,
  //     1.112656394115962, 7.699900579442317, 3.569715877854675,
  //     5.242841095703604, 3.122826344932127, 5.689730628626151,
  //     3.248684682453303, 5.563872291104976, 2.608353462112637,
  //     6.204203511445642, 5.035681855581504, 2.134827911489532,
  //     0.946910011088814, 6.223599755982222, 4.168634519120968,
  //     3.001875247950068, 1.980327734683430, 5.190182032387606,
  //     2.943861424421339, 4.226648342649697, 5.457161501166098,
  //     1.713348265904937, 1.501663178733906, 5.668846588337130,
  //     5.208365510425203, 1.962144256645833, 2.728127406527150,
  //     4.442382360543885, 2.839975217222644, 4.330534549848392,
  //     0.744216089807768, 6.426293677263268, 4.643695520786083,
  //     2.662204050783991, 1.250682335857938, 6.055217235712136,
  //     0.860905287815103, 6.444994283754972, 4.536108843695142,
  //     2.769790727874932, 5.609177455068640, 1.696722116501434,
  //     6.703053268421970, 0.602846303148105, 3.487609972580834,
  //     3.818289598989240, 1.436734374347541, 5.869165197222533,
  //     1.054504320562138, 6.251395251007936, 3.998423858825871,
  //     3.307475712744203, 5.323662899811682, 1.982236671758393;
  // // clang-format on
  // positions.transposeInPlace();
  // atom_types << 20, 20, 24, 24, 15, 15, 15, 15, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  //     8, 8, 8, 8, 8;

  // using PBC_t = Eigen::Map<Eigen::Matrix<int, 3, 1>>;
  // manager->update(positions, atom_types, cell, PBC_t{pbc.data()});





  MultipleStrictStructureManager<StructureManagerCenters> meta{};


    // for (auto& manager : meta.managers1) {
    //   std::cout << "#################################"<< std::endl;
    //   std::cout << manager->nb_clusters(1) << std::endl;
    //   std::cout << "#################################"<< std::endl;
    //   for (auto&& center : *manager) {
    //   //for (auto center = manager->begin(); center!=manager->end(); ++center)

    //     std::cout << center.get_atom_type()<< std::endl;
    //   }
    // }

  for (auto&& manager : meta.managers) {
    // manager->update("alanine-X.json");
    // manager->update(positions, atom_types, cell, PBC_t{pbc.data()});
    if (verbose) {
      std::cout << "################################# 1"<< std::endl;
      std::cout << manager->size() << std::endl;
    }
    auto lower_manager = extract_underlying_manager<-2>(manager);
    std::cout << lower_manager->get_name()<< std::endl;

    for (auto&& center : manager) {
      if (verbose) {
        std::cout << center.get_atom_type() << std::endl;
        std::cout << "################################# 2"<< std::endl;
      }
      for (auto neigh : center) {
        if (verbose) {
          std::cout << neigh.get_atom_type() << std::endl;
        }
      }
    }
  }

  for (auto& manager : meta.managers) {
    // double central_decay{10};
    // double interaction_cutoff{10};
    // double interaction_decay{10};
    // size_t size{50};
    json hypers{
      {"central_decay", 10},
      {"interaction_cutoff", 10},
      {"interaction_decay", 10},
      {"size", 50},
      {"sorting_algorithm", "distance"}
    };
    Representation_t representation{manager, hypers};
    representation.compute();

    auto rep = representation.get_representation_full();
    if (verbose_rep) {
        std::cout << rep.size() <<", "<< rep.cols() <<", "
                                    << rep.rows() << std::endl;
        for (auto ii{0}; ii < rep.cols(); ++ii) {
            for (auto jj{0}; jj < rep.rows(); ++jj) {
                std::cout << rep(jj, ii) << ", ";
            }
            std::cout << std::endl;
        }
    }
  }

  // Manager_t manager{};
  // size_t Natom{22};
  // Eigen::MatrixXd positions(22,3);
  // Eigen::Matrix<int, Eigen::Dynamic, 1> numbers(Natom);
  // Eigen::MatrixXd cell(3, 3);
  // std::array<int, 3> pbc{{true,true,true}};
  // bool verbose{false};
  // bool verbose_rep{false};

  // // cell << 2., 0., 0.,
  // //       0., 2., 0.,
  // //       0., 0., 2.;

  // // positions << 0.4, 1.4, 0.4, 1.4, 0.4, 1.4, 0.4, 1.4,
  // //   0.4, 0.4, 1.4, 1.4, 0.4, 0.4, 1.4, 1.4,
  // //   0.4, 0.4, 0.4, 0.4, 1.4, 1.4, 1.4, 1.4;

  // // numbers << 1,2,3,4,5,6,7,8;
  // cell <<
  //   6.19, 2.41, 0.21,
  //   0.00, 6.15, 1.02,
  //   0.00, 0.00, 7.31;
  // positions <<
  //   3.689540159937393, 5.123016813620886, 1.994119731169116,
  //   6.818437242389163, 2.630056617829216, 6.182500355729062,
  //   2.114977334498767, 6.697579639059512, 1.392155450018263,
  //   7.420401523540017, 2.432242071439904, 6.380314902118375,
  //   1.112656394115962, 7.699900579442317, 3.569715877854675,
  //   5.242841095703604, 3.122826344932127, 5.689730628626151,
  //   3.248684682453303, 5.563872291104976, 2.608353462112637,
  //   6.204203511445642, 5.035681855581504, 2.134827911489532,
  //   0.946910011088814, 6.223599755982222, 4.168634519120968,
  //   3.001875247950068, 1.980327734683430, 5.190182032387606,
  //   2.943861424421339, 4.226648342649697, 5.457161501166098,
  //   1.713348265904937, 1.501663178733906, 5.668846588337130,
  //   5.208365510425203, 1.962144256645833, 2.728127406527150,
  //   4.442382360543885, 2.839975217222644, 4.330534549848392,
  //   0.744216089807768, 6.426293677263268, 4.643695520786083,
  //   2.662204050783991, 1.250682335857938, 6.055217235712136,
  //   0.860905287815103, 6.444994283754972, 4.536108843695142,
  //   2.769790727874932, 5.609177455068640, 1.696722116501434,
  //   6.703053268421970, 0.602846303148105, 3.487609972580834,
  //   3.818289598989240, 1.436734374347541, 5.869165197222533,
  //   1.054504320562138, 6.251395251007936, 3.998423858825871,
  //   3.307475712744203, 5.323662899811682, 1.982236671758393;
  // numbers << 20, 20, 24, 24, 15, 15, 15, 15,  8,  8,  8,
  //   8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8;
  // positions.transposeInPlace();

  // // setting up the manager
  // manager->update(positions, numbers, cell,
  //                Eigen::Map<Eigen::Matrix<int, 3, 1>>{pbc.data()});

  // double cut_off{2};

  // int mult{4};
  // double rc_max{mult*0.5 + cut_off};
  // // build a neighbourlist
  // rascal::AdaptorNeighbourList<rascal::StructureManagerCenters>
  //   pair_manager{manager, rc_max };
  // // update to execute the build
  // pair_manager->update();

  // for (auto i{0}; i < mult; ++i) {
  //   auto cutoff_tmp = i*0.5 + cut_off;
  //   std::vector<std::vector<int>> neigh_ids;
  //   std::vector<std::vector<double>> neigh_dist;

  //   std::vector<std::vector<int>> neigh_ids_strict;
  //   std::vector<std::vector<double>> neigh_dist_strict;

  //   std::cout << "Setting up strict manager with rc="
                          // <<cutoff_tmp << std::endl;
  //   // make strict neighbour list
  //   rascal::AdaptorStrict<
  //     rascal::AdaptorNeighbourList<rascal::StructureManagerCenters>>
  //     adaptor_strict{pair_manager, cutoff_tmp};
  //   // execute
  //   adaptor_strict->update();


  //   if (verbose) std::cout << "Setting get adaptor_strict info" << std::endl;
  //   for (auto center : adaptor_strict) {
  //     auto icenter{center.get_index()};
  //     std::vector<int> indices_{};
  //     std::vector<double> distances_{};

  //     if (verbose) {
  //       // get_index returns iteration index
  //       std::cout << "strict atom out " << center.get_index();
  //       // get_atom_index returns index from
  //       std::cout << " " << center.get_atom_index() << " " ;

  //       for (int ii{0};ii<3;++ii){
  //         std::cout << center.get_position()[ii] << " ";
  //       }
  //       std::cout << " " << center.get_atom_type() << std::endl;
  //     }

  //     int Nneigh{0};
  //     for (auto neigh : center) {
  //       double distance{(center.get_position()
  //                        - neigh.get_position()).norm()};
  //       Nneigh += 1;
  //       indices_.push_back(neigh.get_atom_index());
  //       distances_.push_back(adaptor_strict->get_distance(neigh));

  //       if (verbose) {
  //         std::cout << "strict neigh out " << neigh.get_index();
  //         std::cout << " " << neigh.get_atom_index() << "\t " ;

  //         for (int ii{0};ii<3;++ii){
  //           std::cout << neigh.get_position()[ii] << ", ";
  //         }
  //         std::cout << "\t dist=" << distance;
  //         std::cout << "\t " << neigh.get_atom_type() << std::endl;
  //       }

  //     }

  //     std::cout << "Number of Neighbourg: " << Nneigh << std::endl;
  //     neigh_ids_strict.push_back(indices_);
  //     neigh_dist_strict.push_back(distances_);
  //     if (icenter > 1) break;
  //   }
  // }




  return(0);
}
