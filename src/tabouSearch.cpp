#include "tabouSearch.h"

#include <iostream>
#include <set>

#include "random_generator.h"
#include "utils.h"

int tabu_search(Solution &best_solution, int nb_turn) {
    const int nb_vertices{Graph::g->nb_vertices};
    const int nb_colors{Graph::g->nb_colors};

    // solution qui évolue lors de la recherche Tabou
    Solution solution = best_solution;

    // contient pour chaque sommet le nombre de conflits
    auto nb_conflicts = solution.compute_conflicts();

    // contient pour chaque noeud ne nb de conflits en plus ou
    // moins si changement vers chaque couleur, suivi de la
    // meilleure couleur et meilleur gain associé
    std::vector<std::vector<int>> delta_conflicts_colors(nb_vertices,
                                                         std::vector<int>(nb_colors, 0));

    /// determine les delta-conflits pour chaque transition de couleur
    for (int vertex = 0; vertex < nb_vertices; vertex++) {
        std::fill(delta_conflicts_colors[vertex].begin(),
                  delta_conflicts_colors[vertex].end(),
                  -nb_conflicts[vertex]);
        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            delta_conflicts_colors[vertex][solution._colors[neighbor]]++;
        }
    }

    // optimisation1 permettant de connaître la meilleure amélioration possible pour
    // chaque noeud

    // Pour chaque sommet, les meilleures couleurs possibles
    std::vector<std::vector<int>> best_improve_color(nb_vertices);

    // Pour chaque sommet la meilleure valeur de transition
    std::vector<int> best_improve_conflicts(nb_vertices, 0);

    // optimisation1: Determine les meilleures transition de chaque sommet
    for (int vertex = 0; vertex < nb_vertices; vertex++) {
        int best_conflicts = std::numeric_limits<int>::max();
        for (int color = 0; color < nb_colors; color++) {
            const int conflicts = delta_conflicts_colors[vertex][color];
            if (conflicts > best_conflicts) {
                continue;
            }
            if (conflicts < best_conflicts) {
                best_improve_color[vertex].clear();
                best_conflicts = conflicts;
            }
            insert_sorted(best_improve_color[vertex], color);
        }
        best_improve_conflicts[vertex] = best_conflicts;
    }

    // optimisation2 permettant de parcourir uniquement les noeuds avec conflits

    // contient les noeuds avec conflit pour ne pas tout parcourir
    std::vector<int> conflicting_nodes;

    for (int vertex = 0; vertex < nb_vertices; vertex++) {
        if (nb_conflicts[vertex] > 0) {
            insert_sorted(conflicting_nodes, vertex);
        }
    }

    std::uniform_int_distribution<int> distribution_tabu(0, 10);

    // contient pour chaque noeud et chaque couleur la fin de la période taboue
    std::vector<std::vector<int>> tabu_matrix(nb_vertices,
                                              std::vector<int>(nb_colors, -1));
    int turn;
    long stop{0};
    long recompute{0};
    for (turn = 0; turn < nb_turn && solution._penalty > 0; ++turn) {

        solution.nbIterations = turn;

        int best_nb_conflicts = std::numeric_limits<int>::max();
        std::vector<Coloration> best_colorations;

        for (const auto &vertex : conflicting_nodes) {
            const int current_best_improve = best_improve_conflicts[vertex];
            if (current_best_improve > best_nb_conflicts) {
                continue;
            }

            const int current_color = solution._colors[vertex];
            /// permet de savoir si on a réussi à ajouter une valeur (1=true)
            bool added = false;

            for (const auto &color : best_improve_color[vertex]) {
                const bool vertex_not_tabu{(tabu_matrix[vertex][color] < turn)};
                const bool improve_best_solution{
                    (((current_best_improve + solution._penalty) <
                      best_solution._penalty))};
                // and (tabu_matrix[vertex][color] != nb_turn) // for blocked vertices

                if ((color == current_color) or
                    not(vertex_not_tabu or improve_best_solution)) {
                    continue;
                }

                added = true;
                if (current_best_improve < best_nb_conflicts) {
                    best_nb_conflicts = current_best_improve;
                    best_colorations.clear();
                }
                best_colorations.emplace_back(Coloration{vertex, color});
            }

            if (added or (current_best_improve >= best_nb_conflicts)) {
                stop++;
                continue;
            }
            recompute++;
            // on doit tout vérifier
            for (int color = 0; color < nb_colors; color++) {
                if (color == current_color) {
                    continue;
                }

                const int conflicts = delta_conflicts_colors[vertex][color];
                const bool vertex_not_tabu{(tabu_matrix[vertex][color] < turn)};
                const bool improve_best_solution{
                    (((conflicts + solution._penalty) < best_solution._penalty))};

                if (conflicts > best_nb_conflicts or
                    not(vertex_not_tabu or improve_best_solution)) {
                    continue;
                }

                if ((conflicts < best_nb_conflicts)) {
                    best_nb_conflicts = conflicts;
                    best_colorations.clear();
                }
                best_colorations.emplace_back(Coloration{vertex, color});
            }
        }
        if (best_colorations.empty()) {
            continue;
        }

        const auto best_move{rd::choice(best_colorations)};

        const int old_color = solution.delete_from_color(
            best_move.vertex, nb_conflicts, conflicting_nodes, delta_conflicts_colors);

        solution.add_to_color(best_move.vertex,
                              best_move.color,
                              nb_conflicts,
                              conflicting_nodes,
                              delta_conflicts_colors);

        for (const auto &neighbor : Graph::g->neighborhood[best_move.vertex]) {
            if (solution._colors[neighbor] == old_color) {
                best_improve_conflicts[neighbor]++;
                best_improve_conflicts[best_move.vertex]++;
            }
            if (solution._colors[neighbor] == best_move.color) {
                best_improve_conflicts[neighbor]--;
                best_improve_conflicts[best_move.vertex]--;
            }
            //// ajout pour garder la meilleur transition
            const int best_improve = best_improve_conflicts[neighbor];

            if (delta_conflicts_colors[neighbor][old_color] < best_improve) {
                best_improve_conflicts[neighbor]--;
                best_improve_color[neighbor].clear();
                insert_sorted(best_improve_color[neighbor], old_color);
            } else if (delta_conflicts_colors[neighbor][old_color] == best_improve) {
                insert_sorted(best_improve_color[neighbor], old_color);
            }

            if ((delta_conflicts_colors[neighbor][best_move.color] - 1) == best_improve) {
                // si c'était le meilleur
                if (best_improve_color[neighbor].size() > 1) {
                    erase_sorted(best_improve_color[neighbor], best_move.color);
                } else {
                    int best_conflicts = std::numeric_limits<int>::max();
                    for (int color = 0; color < nb_colors; color++) {
                        const int conflicts = delta_conflicts_colors[neighbor][color];
                        if (conflicts > best_conflicts) {
                            continue;
                        }
                        if (conflicts < best_conflicts) {
                            best_improve_color[neighbor].clear();
                            best_conflicts = conflicts;
                        }
                        insert_sorted(best_improve_color[neighbor], color);
                    }
                    best_improve_conflicts[neighbor] = best_conflicts;
                }
            }
        }

        // update tabu matrix
        tabu_matrix[best_move.vertex][old_color] =
            turn + distribution_tabu(rd::generator) +
            static_cast<int>(0.6 * solution.nb_conflicting_vertices);

        if (solution._penalty <= best_solution._penalty) {
            //// si <= : dernière meilleure rencontrée ; si < : premiere meilleure
            if (solution._penalty < best_solution._penalty)
                solution.nbIterationsFirst = turn;
            best_solution = solution;
        }
    }
    std::cout << "nb stop " << stop << " nb recompute " << recompute << std::endl;
    return turn;
}
