#pragma once

#include <limits>

#include "graph.h"

struct Solution {
  public:
    // contient pour chaque noeud sa couleur
    std::vector<int> _colors{};
    int _penalty{std::numeric_limits<int>::max()};
    int nb_conflicting_vertices{std::numeric_limits<int>::max()};
    // nb d'iterations locales pour la trouver
    unsigned long long nbIterations{std::numeric_limits<unsigned long long>::max()};
    // nb d'iterations de la première fois qu'on la trouve
    unsigned long long nbIterationsFirst{std::numeric_limits<unsigned long long>::max()};

    Solution();

    void init_random();

    std::vector<int> compute_conflicts();

    // determine la proximité de 2 individus (on identifier les meilleurs associations de
    // couleur entre les 2 individus). si changeToBestMatching est vrai : on change les
    // couleurs de l'individu courant pour avoir le meilleur matching
    int proxi(const Solution &sol);
};
