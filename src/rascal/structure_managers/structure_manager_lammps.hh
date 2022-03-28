/**
 * @file   rascal/structure_managers/structure_manager_lammps.hh
 *
 * @author Till Junge <till.junge@epfl.ch>
 *
 * @date   05 Apr 2018
 *
 * @brief Neighbourhood manager for lammps neighbourhood lists
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

#ifndef SRC_RASCAL_STRUCTURE_MANAGERS_STRUCTURE_MANAGER_LAMMPS_HH_
#define SRC_RASCAL_STRUCTURE_MANAGERS_STRUCTURE_MANAGER_LAMMPS_HH_

#include "rascal/structure_managers/structure_manager.hh"
#include "rascal/structure_managers/atomic_structure.hh"
#include "rascal/structure_managers/lattice.hh"

#include <stdexcept>
#include <vector>

namespace rascal {
  //! forward declaration for traits
  class StructureManagerLammps;

  /*
   * traits specialisation for Lammps manager The traits are used for vector
   * allocation and further down the processing chain to determine what
   * functionality the given StructureManager already contains to avoid
   * recomputation. See also the implementation of adaptors.
   */
  template <>
  struct StructureManager_traits<StructureManagerLammps> {
    constexpr static int Dim{3};
    constexpr static size_t MaxOrder{2};
    constexpr static AdaptorTraits::Strict Strict{AdaptorTraits::Strict::no};
    constexpr static bool HasDistances{false};
    constexpr static bool HasDirectionVectors{false};
    constexpr static bool HasCenterPair{false};
    constexpr static int StackLevel{0};
    using LayerByOrder = std::index_sequence<0, 0>;
    constexpr static AdaptorTraits::NeighbourListType NeighbourListType{
        AdaptorTraits::NeighbourListType::full};
    using PreviousManager_t = StructureManagerLammps;
  };

  /* ---------------------------------------------------------------------- */
  //! Definition of the new StructureManagerLammps class.
  class StructureManagerLammps
      : public StructureManager<StructureManagerLammps>,
        public std::enable_shared_from_this<StructureManagerLammps> {
   public:
    using traits = StructureManager_traits<StructureManagerLammps>;
    using PreviousManager_t = typename traits::PreviousManager_t;
    using Parent = StructureManager<StructureManagerLammps>;
    using Vector_ref = typename Parent::Vector_ref;
    using AtomRef_t = typename Parent::AtomRef;

    using Cell_t = AtomicStructure<traits::Dim>::Cell_t;
    using Cell_ref = AtomicStructure<traits::Dim>::Cell_ref;

    using PBC_t = AtomicStructure<traits::Dim>::PBC_t;
    using PBC_ref = AtomicStructure<traits::Dim>::PBC_ref;

    using ManagerImplementation_t = StructureManagerLammps;
    using ImplementationPtr_t = std::shared_ptr<StructureManagerLammps>;
    using ConstImplementationPtr_t =
        const std::shared_ptr<const StructureManagerLammps>;

    //! Default constructor
    StructureManagerLammps() = default;

    //! Copy constructor
    StructureManagerLammps(const StructureManagerLammps & other) = delete;

    //! Move constructor
    StructureManagerLammps(StructureManagerLammps && other) = delete;

    //! Destructor
    virtual ~StructureManagerLammps() = default;

    //! Copy assignment operator
    StructureManagerLammps &
    operator=(const StructureManagerLammps & other) = delete;

    //! Move assignment operator
    StructureManagerLammps &
    operator=(StructureManagerLammps && other) = delete;

    //! Updates the manager using the impl
    template <class... Args>
    void update(Args &&... arguments) {
      if (sizeof...(arguments) > 0) {
        // the structure has changed to tell it to the whole tree
        this->send_changed_structure_signal();
      }
      // update the underlying structure
      this->update_self(std::forward<Args>(arguments)...);
      this->set_update_status(true);

      // send the update signal to the tree
      this->update_children();
    }
    ////---------------------------------------------------
    /**
     * Returns a traits::Dim by traits::Dim matrix with the cell vectors of the
     * structure.
     */
    Cell_ref get_cell() { return Cell_ref(this->lattice.get_cell()); }

    Eigen::Array3d get_cell_lengths() {
      return this->lattice.get_cell_lengths().array();
    }
    
    double get_cell_volume() { return this->lattice.get_cell_volume(); }

    //! Returns a map of size traits::Dim with 0/1 for periodicity
    PBC_ref get_periodic_boundary_conditions() {
      return PBC_ref(this->pbc);
    }

    ////-----------------------------------------------

    //! return position vector of an atom given the atom tag
    Vector_ref get_position(int atom_tag) {
      auto * xval{this->x[atom_tag]};
      return Vector_ref(xval);
    }

    //! return position vector of an atom given the atom tag
    Vector_ref get_position(const AtomRef_t & atom) {
      return this->get_position(this->get_atom_index(atom.get_index()));
    }

    //! get const atom type reference given an atom_tag
    int get_atom_type(int atom_tag) const {
      // lammps atom types go from 1 to N, so we map it to rascal atom types
      // by mapping 0 to N-1 to the corresponding number
      return this->atom_types[this->type[atom_tag]-1];
    }

    //! return number of I atoms in the list
    size_t get_size() const { return this->inum; }

    //! return number of center and ghost atoms
    size_t get_size_with_ghosts() const { return this->tot_num; }

    //! Returns the number of neighbours of a given atom at a given TargetOrder
    //! Returns the number of pairs of a given center
    template <size_t TargetOrder, size_t Order, size_t Layer>
    typename std::enable_if_t<TargetOrder == 2, size_t>
    get_cluster_size_impl(const ClusterRefKey<Order, Layer> & cluster) const {
      return this->numneigh[this->get_atom_index(cluster.get_atom_tag())];
    }

    /**
     * return the atom_tag of the index-th atom in manager parent here is
     * dummy and is used for consistency in other words, atom_tag is the
     * global LAMMPS atom tag.
     */
    int get_neighbour_atom_tag(const Parent &, size_t cluster_index) const {
      //return this->atom_index_from_atom_tag_list.at(cluster_index);
      //return this->ilist[cluster_index];
      return this->atom_tag_list[cluster_index];
    }

    //! return the index-th neighbour of the last atom in a cluster with
    //! cluster_size = 1 (atoms) which can be used to construct pairs
    template <size_t Order, size_t Layer>
    int get_neighbour_atom_tag(const ClusterRefKey<Order, Layer> & cluster,
                               size_t index) const {
      static_assert(Order == traits::MaxOrder - 1,
                    "this implementation only handles atoms and identify its "
                    "index-th neighbour.");
      auto && i_atom_id{cluster.back()};
      return this->firstneigh[std::move(i_atom_id)][index];
    }
  
    int get_atom_index(int atom_tag) const {
      return this->atom_index_from_atom_tag_list[atom_tag];
    }

    /**
     * provided an atom, returns the cumulative numbers of pairs up to the first
     * pair in which the atom is the I atom this only works for atom
     */
    template <size_t Order>
    size_t get_offset_impl(const std::array<size_t, Order> & counters) const;

    /**
     * return the number of clusters of size cluster_size.  Can only handle
     * cluster_size 1 (atoms) and cluster_size 2 (pairs).
     */
    size_t get_nb_clusters(int order) const;

    bool is_not_masked() const { return true; }

    //! overload of update that does not change the underlying structure
    void update_self() {}
    /**
     * resetting is required every time the list changes. Here, this
     * is implemented without explicit dependency to lammps. The
     * signature could be simplified by including lammps as a
     * dependency, but it is unclear that the convenience would
     * outweigh the hassle of maintaining the dependency.
     *
     * @param inum Property `inum` in the lammps `NeighList` structure
     *
     * @param tot_num sum of the properties `nlocal` and `nghost` in the
     *                lammps `Atom` structure
     *
     * @param ilist Property `ilist` in the lammps `NeighList` structure
     *
     * @param numneigh Property `numneigh` in the lammps `NeighList` structure
     *
     * @param firstneigh Property `firstneigh` in the lammps `NeighList`
     * structure
     *
     * @param x Property `x` in the lammps `Atom` structure
     *
     * @param f Property `f` in the lammps `Atom` structure
     *
     * @param type Property `type` in the lammps `Atom` structure
     *
     * @param eatom per-atom energy
     *
     * @param vatom per-atom virial
     */
    void update_self(int inum, int tot_num, int * ilist, int * numneigh,
                     int ** firstneigh, double ** x, double ** f, int * type,
                     double * eatom, double ** vatom,
                     std::vector<int> atom_types,
                     int * lammps_atom_tag, double * lattice, int * pbc);

   protected:
    /**
     * Get a ptr of the previous manager, required for forwarding requests
     * downwards a stack. Since there is no last manager, the manager returns
     * itself.
     */
    ImplementationPtr_t get_previous_manager_impl() {
      return shared_from_this();
    }

    //! Get the manager used to build the instance
    ConstImplementationPtr_t get_previous_manager_impl() const {
      return shared_from_this();
    }

    int inum{};           //!< total numer of atoms 
    int tot_num{};        //!< total number, includes ghosts
    int * ilist{};        //!< lammps atomic indices / rascal atom tags, size is inum if lammps ghost flag is turned off which is default other wise size is tot_num
    int * numneigh{};     //!< number of neighbours per atom
    int ** firstneigh{};  //!< pointer to first neighbour
    double ** x{};        //!< atomic positions
    double ** f{};        //!< atomic forces
    int * type{};         //!< lammps atom types
    double * eatom{};     //!< energy of atoms
    double ** vatom{};    //!< virial stress of atoms
    int nb_pairs{};       //! number of clusters with cluster_size=2 (pairs)
    std::vector<int> atom_types{}; //!< lammps atom types to rascal, used as atom_types[type[atom_tag]-1]
    std::vector<int> atom_tag_list{};  //! stores i-atom and ghost atom tags
    std::vector<int> offsets{};  //! offset per atom to access neighbour list
    Lattice<traits::Dim> lattice{};
    PBC_t pbc{};

    // works as in adaptor neighbour list
    std::vector<size_t> atom_index_from_atom_tag_list{};

    //! Stores the number of neighbours for every atom
    //?? std::vector<size_t> nb_neigh{};

    //! Stores additional atom tags of current Order (only ghost atoms)
    //std::vector<int> ghost_atom_tag_list{};
    //! Stores neighbour's atom tag in a list in sequence of atoms
    //std::vector<int> neighbours_atom_tag{};

   private:
  };

  /**
   * provided an atom, returns the cumulative numbers of pairs up to the first
   * pair in which the atom is the I atom this only works for atom
   */
  template <size_t Order>
  size_t StructureManagerLammps::get_offset_impl(
      const std::array<size_t, Order> & counters) const {
    // The static assert with <= is necessary, because the template parameter
    // ``Order`` is one Order higher than the MaxOrder at the current level. The
    // return type of this function is used to build the next Order iteration.
    static_assert(Order <= traits::MaxOrder,
                  "this manager can only give the offset (= starting index)"
                  " for a pair iterator, given the i atom of the pair");
    return this->offsets[counters.front()];
  }
}  // namespace rascal

#endif  // SRC_RASCAL_STRUCTURE_MANAGERS_STRUCTURE_MANAGER_LAMMPS_HH_
