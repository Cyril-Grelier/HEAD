#pragma once

#include <limits>

#include "graph.h"
#include "utils.h"

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

    void add_to_color(const int vertex,
                      int color,
                      std::vector<int> &nb_conflicts,
                      std::vector<int> &conflicting_nodes,
                      std::vector<std::vector<int>> &delta_conflicts_colors) {
        _colors[vertex] = color;
        // affect of the vertex entering in the new color
        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            if (_colors[neighbor] == color) {
                nb_conflicts[neighbor]++;
                nb_conflicts[vertex]++;
                _penalty++;
                if (nb_conflicts[neighbor] == 1) {
                    nb_conflicting_vertices++;
                    if (not contains(conflicting_nodes, neighbor)) {
                        insert_sorted(conflicting_nodes, neighbor);
                    }
                }
                if (nb_conflicts[vertex] == 1) {
                    nb_conflicting_vertices++;
                    if (not contains(conflicting_nodes, vertex)) {
                        insert_sorted(conflicting_nodes, vertex);
                    }
                }
                for (int color_ = 0; color_ < Graph::g->nb_colors; color_++) {
                    delta_conflicts_colors[neighbor][color_]--;
                    delta_conflicts_colors[vertex][color_]--;
                }
            }
            delta_conflicts_colors[neighbor][color]++;
        }
    }

    int delete_from_color(const int vertex,
                          std::vector<int> &nb_conflicts,
                          std::vector<int> &conflicting_nodes,
                          std::vector<std::vector<int>> &delta_conflicts_colors) {
        const int old_color = _colors[vertex];

        // affect of the vertex leaving the old color
        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            /// répercutions sur les voisins
            if (_colors[neighbor] == old_color) {
                nb_conflicts[neighbor]--;
                nb_conflicts[vertex]--;
                _penalty--;
                if (nb_conflicts[neighbor] == 0) {
                    nb_conflicting_vertices--;
                    erase_sorted(conflicting_nodes, neighbor);
                }
                if (nb_conflicts[vertex] == 0) {
                    nb_conflicting_vertices--;
                    erase_sorted(conflicting_nodes, vertex);
                }

                for (int color = 0; color < Graph::g->nb_colors; color++) {
                    delta_conflicts_colors[neighbor][color]++;
                    delta_conflicts_colors[vertex][color]++;
                }
            }
            delta_conflicts_colors[neighbor][old_color]--;
        }
        return old_color;
    }

    std::vector<int> compute_conflicts();

    // determine la proximité de 2 individus (on identifier les meilleurs associations de
    // couleur entre les 2 individus). si changeToBestMatching est vrai : on change les
    // couleurs de l'individu courant pour avoir le meilleur matching
    int proxi(const Solution &sol);
};
