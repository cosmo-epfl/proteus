import numpy as np
import json

from ..neighbourlist import AtomsList
from .base import CalculatorFactory
from itertools import starmap
from ..utils import BaseIO


class SortedCoulombMatrix(BaseIO):
    """
    Computes the Sorted Coulomb matrix representation [1].

    Attributes
    ----------
    cutoff : float

    central_decay : float
        The distance over which the the coulomb interaction decays from full to none.

    interaction_cutoff : float
        The distance between two non-central atom, where the coulomb interaction element will be zero.

    interaction_decay : float
        The distance over which the the coulomb interaction decays from full to none.

    size : int
        Larger or equal to the maximum number of neighbour an atom has in the structure.

    Methods
    -------
    transform(frames)
        Compute the representation for a list of ase.Atoms object.


    .. [1] Rupp, M., Tkatchenko, A., Müller, K.-R., & von Lilienfeld, O. A. (2011).
        Fast and Accurate Modeling of Molecular Atomization Energies with Machine Learning.
        Physical Review Letters, 108(5), 58301. https://doi.org/10.1103/PhysRevLett.108.058301
    """

    def __init__(
        self,
        cutoff,
        sorting_algorithm="row_norm",
        size=10,
        central_decay=-1,
        interaction_cutoff=10,
        interaction_decay=-1,
    ):
        self.name = "sortedcoulomb"
        self.size = size
        self.hypers = dict()
        self.update_hyperparameters(
            sorting_algorithm=sorting_algorithm,
            central_cutoff=cutoff,
            central_decay=central_decay,
            interaction_cutoff=interaction_cutoff,
            interaction_decay=interaction_decay,
            size=int(size),
        )

        self.nl_options = [
            dict(name="centers", args=dict()),
            dict(name="neighbourlist", args=dict(cutoff=cutoff)),
            dict(name="strict", args=dict(cutoff=cutoff)),
        ]

    def update_hyperparameters(self, **hypers):
        """Store the given dict of hyperparameters

        Also updates the internal json-like representation

        """
        allowed_keys = {
            "sorting_algorithm",
            "central_cutoff",
            "central_decay",
            "interaction_cutoff",
            "interaction_decay",
            "size",
        }
        hypers_clean = {key: hypers[key] for key in hypers if key in allowed_keys}
        self.hypers.update(hypers_clean)

    def transform(self, frames):
        """Compute the representation.

        Parameters
        ----------
        frames : list(ase.Atoms) or AtomsList
            List of atomic structures.

        Returns
        -------
           AtomsList : Object containing the representation

        """
        if not isinstance(frames, AtomsList):
            frames = AtomsList(frames, self.nl_options)

        self.size = self.get_size(frames.managers)
        self.update_hyperparameters(size=self.size)
        self.rep_options = dict(name=self.name, args=[self.hypers])
        self._representation = CalculatorFactory(self.rep_options)

        self._representation.compute(frames.managers)

        return frames

    def get_feature_size(self):
        return int(self.size * (self.size + 1) / 2)

    def get_size(self, managers):
        Nneigh = []
        for manager in managers:
            for center in manager:
                Nneigh.append(center.nb_pairs + 1)
        size = int(np.max(Nneigh))
        return size

    def _get_init_params(self):
        init_params = dict(
            cutoff=self.hypers["central_cutoff"],
            sorting_algorithm=self.hypers["sorting_algorithm"],
            size=self.hypers["size"],
            central_decay=self.hypers["central_decay"],
            interaction_cutoff=self.hypers["interaction_cutoff"],
            interaction_decay=self.hypers["interaction_decay"],
        )
        return init_params

    def _set_data(self, data):
        super()._set_data(data)

    def _get_data(self):
        return super()._get_data()
