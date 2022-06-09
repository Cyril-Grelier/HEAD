#include "head.h"
#include "random_generator.h"

/// Crossover operator: GPX algorithm
/// Parents are colors (couleur de chaque sommet)
Solution Head::buildChild(std::vector<Solution> &vParents, int startParent) {

    Solution child;

    std::vector<std::vector<int>> nb_vertices_per_colors(
        2, std::vector<int>(Graph::g->nb_colors, 0));

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < Graph::g->nb_vertices; j++) {
            nb_vertices_per_colors[i][vParents[i]._colors[j]]++;
        }
    }

    std::uniform_int_distribution<int> distribution_colors(0, Graph::g->nb_colors - 1);

    for (int color = 0; color < Graph::g->nb_colors; color++) {
        int indice = (startParent + color) % 2;
        const auto &parent = vParents[indice];
        const auto &nb_vertices_per_colors_ = nb_vertices_per_colors[indice];
        int valMax = -1;
        int colorMax = -1;

        const int startColor = distribution_colors(rd::generator);

        if (color < 0) {
            // pick a random non empty color
            for (int j = 0; j < Graph::g->nb_colors; j++) {
                int color_ = (startColor + j) % Graph::g->nb_colors;
                int currentVal = nb_vertices_per_colors_[color_];
                if (currentVal > 0) {
                    valMax = currentVal;
                    colorMax = color_;
                    break;
                }
            }
        } else {
            // for the first iteration, pick the largest group of color
            for (int j = 0; j < Graph::g->nb_colors; j++) {
                int color_ = (startColor + j) % Graph::g->nb_colors;
                int currentVal = nb_vertices_per_colors_[color_];
                if (currentVal > valMax) {
                    valMax = currentVal;
                    colorMax = color_;
                }
            }
        }
        //
        for (int j = 0; j < Graph::g->nb_vertices; j++) {
            if (parent._colors[j] == colorMax and child._colors[j] < 0) {
                child._colors[j] = color;

                for (int k = 0; k < 2; k++) {
                    nb_vertices_per_colors[k][vParents[k]._colors[j]]--;
                }
            }
        }
    }

    // greedy to complete the solution
    for (int i = 0; i < Graph::g->nb_vertices; i++) {
        if (child._colors[i] < 0) {
            child._colors[i] = distribution_colors(rd::generator);
        }
    }
    return child;
}

void Head::compute() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    humanTime = static_cast<time_t>(static_cast<double>(tv.tv_sec) +
                                    static_cast<double>(tv.tv_usec) / 1000000.0);

    // 2 pour la population courante, 2 pour l'archive
    std::vector<Solution> vPopulation(4);

    int bestSolNbIterationsCross = 0;
    nbIterations = nbIterationsCross = 0;

    // random initialization
    vPopulation[0].init_random();
    vPopulation[1].init_random();
    vPopulation[2].init_random(); ///  individu elite1
    vPopulation[3].init_random(); ///  individu elite2

    bestSol = vPopulation[0];

    std::vector<Solution> vFils(2);

    int seuil = static_cast<int>(0.99 * Graph::g->nb_vertices);
    int proximity = 0;
    bool found = false;
    int currentElite = 0;
    int swapIter = -1;

    std::uniform_int_distribution<int> distribution(0, 99);

    while (!found and proximity < seuil and
           (difftime(time(NULL), humanTime) < max_secondes or max_secondes < 0)) {

        nbIterationsCross++;

        vFils[0] = buildChild(vPopulation, 0);
        vFils[1] = buildChild(vPopulation, 1);

        std::vector<int> nb_iters(2, 0);
#pragma omp parallel for
        for (int i = 0; i < 2; i++) {
            nb_iters[i] = tabu_search(vFils[i], nbLocalSearch);
        }

        for (int i = 0; i < 2; i++) {
            nbIterations += nb_iters[i];
            if (vFils[i]._penalty == 0)
                found = true;
        }
        for (int i = 0; i < 2; i++) {
            if (vFils[i]._penalty < bestSol._penalty) {
                bestSol = vFils[i];
                bestSolNbIterationsCross = nbIterationsCross;
            }
            const int rand_number = distribution(rd::generator);
            if (vFils[i]._penalty <= vPopulation[i]._penalty or
                rand_number < tauxAcceptWorst)
                vPopulation[i] = vFils[i];
            else {
                std::cout << rand_number << std::endl;
            }
            if (vFils[i]._penalty <= vPopulation[2 + currentElite]._penalty)
                vPopulation[2 + currentElite] = vFils[i];
        }

        proximity = vPopulation[0].proxi(vPopulation[1]);

        std::cout << "k-" << Graph::g->nb_colors << " " << nbIterationsCross << ": "
                  << "fitness=" << vPopulation[0]._penalty << " "
                  << vPopulation[1]._penalty << " (" << vPopulation[2]._penalty << " "
                  << vPopulation[3]._penalty << ")\t"
                  << "\tprox=" << proximity << " best=" << bestSol._penalty << "(it"
                  << bestSolNbIterationsCross << ")" << std::endl;

        if (swapIter > 0 and (nbIterationsCross % swapIter == 0 or proximity >= seuil)) {
            currentElite = (currentElite + 1) % 2;

            int indivToReplace = static_cast<int>(static_cast<double>(rand()) /
                                                  static_cast<double>(RAND_MAX)) *
                                 2;
            if (vPopulation[(indivToReplace + 1) % 2].proxi(
                    vPopulation[2 + currentElite]) >=
                seuil) /// if elite and indivToReplace are too similar, choose the
                       /// other individual for the replacement
                indivToReplace = (indivToReplace + 1) % 2;

            vPopulation[indivToReplace] = vPopulation[2 + currentElite];
            vPopulation[2 + currentElite]._penalty = 999999;

            proximity = vPopulation[0].proxi(vPopulation[1]);

            printf("\nSwap\n");
        }
    }
}
