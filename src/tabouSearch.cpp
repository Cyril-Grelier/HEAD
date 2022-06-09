#include "tabouSearch.h"

#include <iostream>

#include "random_generator.h"

int tabu_search(Solution &best_solution, int nb_turn) {
    const int nb_vertices{Graph::g->nb_vertices};
    const int nb_colors{Graph::g->nb_colors};

    // solution qui évolue lors de la recherche Tabou
    Solution solution = best_solution;

    // contient pour chaque sommet le nombre de conflits
    std::vector<int> nb_conflicts(nb_vertices, 0);

    // TODO should be already computed in solution
    solution.computeConflicts(nb_conflicts);

    // contient pour chaque noeud ne nb de conflits en plus ou
    // moins si changement vers chaque couleur, suivi de la
    // meilleure couleur et meilleur gain associé
    std::vector<std::vector<int>> conflicts_colors(nb_vertices,
                                                   std::vector<int>(nb_colors, 0));

    /// determine les delta-conflits pour chaque transition de couleur
    for (int vertex = 0; vertex < nb_vertices; vertex++) {
        for (int color = 0; color < nb_colors; color++) {
            conflicts_colors[vertex][color] = -nb_conflicts[vertex];
        }
        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            conflicts_colors[vertex][solution._colors[neighbor]]++;
        }
    }

    // optimisation1 permettant de connaître la meilleure amélioration possible pour
    // chaque noeud

    // contient pour chaque sommet la meilleure valeur de transition
    std::vector<int> best_improve_conflicts(nb_vertices, 0);

    // contient pour chaque sommet la meilleure transition de
    // couleur (-1 si plusieurs équivalentes => stratégie classique)
    std::vector<std::vector<int>> best_improve_color(nb_vertices,
                                                     std::vector<int>(nb_colors, 0));

    // contient le nombre pour chaque sommet de couleurs fournissant
    // la meilleure amélioration
    std::vector<int> nb_best_improve(nb_vertices, 0);

    /// optimisation1: Determine les meilleures transition de chaque sommet
    for (int vertex = 0; vertex < nb_vertices; vertex++) {
        int best_conflicts = Graph::g->nb_edges + 1;
        int nb_best_value = 0;
        for (int color = 0; color < nb_colors; color++) {
            const int conflicts = conflicts_colors[vertex][color];
            if (conflicts < best_conflicts) {
                nb_best_value = 0;
                best_conflicts = conflicts;
            }
            if (conflicts <= best_conflicts) {
                best_improve_color[vertex][nb_best_value++] = color;
            }
        }
        best_improve_conflicts[vertex] = best_conflicts;
        nb_best_improve[vertex] = nb_best_value;
    }

    /// optimisation2: Determination des noeuds avec conflict (par défaut tous les
    /// noeuds)

    // optimisation2 permettant de parcourir uniquement les noeuds avec conflits

    // contient les noeuds avec conflit pour ne pas tout parcourir
    std::vector<int> conflicting_nodes(nb_vertices, 0); // TODO should be a set
    int nb_conflicting_nodes{0};
    // permet de ne pas ajouter 2 fois le meme noeud
    std::vector<bool> node_added(nb_vertices, false);

    for (int vertex = 0; vertex < nb_vertices; vertex++) {
        if (nb_conflicts[vertex] > 0) {
            conflicting_nodes[nb_conflicting_nodes++] = vertex;
            node_added[vertex] = true;
        }
    }

    // initTables end

    std::uniform_int_distribution<int> distribution_tabu(0, 10);

    // contient pour chaque noeud et chaque couleur la fin de la période taboue
    std::vector<std::vector<int>> tabu_matrix(nb_vertices,
                                              std::vector<int>(nb_colors, -1));
    int turn;
    for (turn = 0; turn < nb_turn && solution._penalty > 0; turn++) {

        solution.nbIterations = turn;

        int best_nb_conflicts = Graph::g->nb_edges + 1;
        int best_vertex = -1;
        int best_color = -1;
        int nb_best_nb_conflicts = 0;

        for (int ind = 0; ind < nb_conflicting_nodes; ind++) {
            int vertex = conflicting_nodes[ind];

            if (nb_conflicts[vertex] == 0 or
                best_improve_conflicts[vertex] > best_nb_conflicts) {
                continue;
            }

            const int current_color = solution._colors[vertex];
            const int current_best_improve = best_improve_conflicts[vertex];
            const int current_nb_best_improve = nb_best_improve[vertex];
            /// permet de savoir si on a réussi à ajouter une valeur (1=true)
            int added = 0;

            for (int ind_col = 0; ind_col < current_nb_best_improve; ind_col++) {
                int color = best_improve_color[vertex][ind_col];

                const bool vertex_not_tabu{(tabu_matrix[vertex][color] < turn)};
                const bool improve_best_solution{
                    (((current_best_improve + solution._penalty) <
                      best_solution._penalty))};
                // and (tabu_matrix[vertex][color] != nb_turn)

                if ((color == current_color) or
                    not(vertex_not_tabu or improve_best_solution)) {
                    continue;
                }

                added = 1;
                if (current_best_improve < best_nb_conflicts) {
                    best_nb_conflicts = current_best_improve;
                    best_vertex = vertex;
                    best_color = color;
                    nb_best_nb_conflicts = 1;
                } else {
                    nb_best_nb_conflicts++;
                    std::uniform_int_distribution<int> distribution(
                        0, nb_best_nb_conflicts - 1);
                    int val = distribution(rd::generator);
                    if (val == 0) {
                        best_vertex = vertex;
                        best_color = color;
                    }
                }
            }

            if (current_best_improve < best_nb_conflicts && added == 0) {
                /// on doit tout vérifier
                for (int color = 0; color < nb_colors; color++) {
                    if (color == current_color) {
                        continue;
                    }

                    const int conflicts = conflicts_colors[vertex][color];
                    const bool vertex_not_tabu{(tabu_matrix[vertex][color] < turn)};
                    const bool improve_best_solution{
                        (((conflicts + solution._penalty) < best_solution._penalty))};

                    if ((conflicts < best_nb_conflicts) and
                        (vertex_not_tabu or improve_best_solution)) {
                        best_nb_conflicts = conflicts;
                        best_vertex = vertex;
                        best_color = color;
                        nb_best_nb_conflicts = 1;
                    } else if ((conflicts == best_nb_conflicts) and
                               (vertex_not_tabu or improve_best_solution)) {
                        // on tire aléatoirement 1 des 2
                        nb_best_nb_conflicts++;
                        std::uniform_int_distribution<int> distribution(
                            0, nb_best_nb_conflicts - 1);
                        int val = distribution(rd::generator);
                        if (val == 0) {
                            best_nb_conflicts = conflicts;
                            best_vertex = vertex;
                            best_color = color;
                        }
                    }
                }
            }
        }
        if (best_vertex > -1) {

            int last_color = solution._colors[best_vertex];
            // solution.add_to_color(best_vertex, best_color);

            tabu_matrix[best_vertex][last_color] =
                turn + distribution_tabu(rd::generator) +
                static_cast<int>(0.6 * solution.nb_conflicting_vertices);

            // updateTables(int best_vertex, int best_color)
            solution._colors[best_vertex] = best_color;

            for (const auto &neighbor : Graph::g->neighborhood[best_vertex]) {
                /// répercutions sur les voisins
                if (solution._colors[neighbor] == last_color) {
                    nb_conflicts[neighbor]--;
                    nb_conflicts[best_vertex]--;
                    solution._penalty--;
                    if (nb_conflicts[neighbor] == 0)
                        solution.nb_conflicting_vertices--;
                    if (nb_conflicts[best_vertex] == 0)
                        solution.nb_conflicting_vertices--;

                    for (int color = 0; color < nb_colors; color++) {
                        conflicts_colors[neighbor][color]++;
                        conflicts_colors[best_vertex][color]++;
                    }
                    best_improve_conflicts[neighbor]++;
                    best_improve_conflicts[best_vertex]++;
                }

                conflicts_colors[neighbor][last_color]--;
            }
            for (const auto &neighbor : Graph::g->neighborhood[best_vertex]) {
                if (solution._colors[neighbor] == best_color) {
                    nb_conflicts[neighbor]++;
                    nb_conflicts[best_vertex]++;
                    solution._penalty++;
                    if (nb_conflicts[neighbor] == 1) {
                        solution.nb_conflicting_vertices++;
                        if (node_added[neighbor] != 1) {
                            node_added[neighbor] = 1;
                            conflicting_nodes[nb_conflicting_nodes++] = neighbor;
                        }
                    }
                    if (nb_conflicts[best_vertex] == 1) {
                        solution.nb_conflicting_vertices++;
                        if (node_added[best_vertex] != 1) {
                            node_added[best_vertex] = 1;
                            conflicting_nodes[nb_conflicting_nodes++] = best_vertex;
                        }
                    }
                    for (int color = 0; color < nb_colors; color++) {
                        conflicts_colors[neighbor][color]--;
                        conflicts_colors[best_vertex][color]--;
                    }
                    best_improve_conflicts[neighbor]--;
                    best_improve_conflicts[best_vertex]--;
                }
                conflicts_colors[neighbor][best_color]++;
            }

            for (const auto &neighbor : Graph::g->neighborhood[best_vertex]) {
                //// ajout pour garder la meilleur transition
                const int best_improve = best_improve_conflicts[neighbor];

                if (conflicts_colors[neighbor][last_color] < best_improve) {
                    best_improve_conflicts[neighbor]--;
                    best_improve_color[neighbor][0] = last_color;
                    nb_best_improve[neighbor] = 1;
                } else if (conflicts_colors[neighbor][last_color] == best_improve) {
                    best_improve_color[neighbor][nb_best_improve[neighbor]] = last_color;
                    nb_best_improve[neighbor]++;
                }

                if ((conflicts_colors[neighbor][best_color] - 1) == best_improve) {
                    // si c'était le meilleur
                    const int nbBestImprove = nb_best_improve[neighbor];
                    if (nbBestImprove > 1) {
                        nb_best_improve[neighbor]--;
                        int pos = 0;
                        bool found = false;
                        for (pos = 0; found != 1; pos++) {
                            if (best_improve_color[neighbor][pos] == best_color)
                                found = true;
                        }

                        for (pos = pos; pos < nbBestImprove; pos++) {
                            best_improve_color[neighbor][pos - 1] =
                                best_improve_color[neighbor][pos];
                        }
                    } else {
                        int bestVal_ = Graph::g->nb_edges + 1;
                        int nbBestVal_ = 0;
                        for (int color = 0; color < nb_colors; color++) {
                            const int conflicts = conflicts_colors[neighbor][color];
                            if (conflicts < bestVal_) {
                                bestVal_ = conflicts;
                                best_improve_color[neighbor][0] = color;
                                nbBestVal_ = 1;
                            } else if (conflicts == bestVal_) {
                                best_improve_color[neighbor][nbBestVal_] = color;
                                nbBestVal_++;
                            }
                        }
                        best_improve_conflicts[neighbor] = bestVal_;
                        nb_best_improve[neighbor] = nbBestVal_;
                    }
                }
            }

            if (turn % 100 == 0) {
                // arbitrairement tous les 100 on recalcule les noeuds avec conflit
                nb_conflicting_nodes = 0;
                std::fill(node_added.begin(), node_added.end(), false);

                for (int vertex = 0; vertex < nb_vertices; vertex++) {
                    if (nb_conflicts[vertex] > 0) {
                        conflicting_nodes[nb_conflicting_nodes++] = vertex;
                        node_added[vertex] = true;
                    }
                }
                // recalcule pour optimisation2
            }
        }

        if (solution._penalty <= best_solution._penalty) {
            //// si <= : dernière meilleure rencontrée ; si < : premiere meilleure
            if (solution._penalty < best_solution._penalty)
                solution.nbIterationsFirst = turn;
            best_solution = solution;
        }
    }
    return turn;
}
