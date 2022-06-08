#include "head.h"

/// Crossover operator: GPX algorithm
/// Parents are colors (couleur de chaque sommet)
Solution Head::buildChild(std::vector<Solution> &vParents, int startParent) {

    Solution res;

    int nbParents = 2;
    std::vector<std::vector<double>> tSizeOfColors(
        nbParents, std::vector<double>(Graph::g->nb_colors, 0));

    for (int i = 0; i < nbParents; i++) {
        for (int j = 0; j < Graph::g->nb_vertices; j++) {
            tSizeOfColors[i][vParents[i]._colors[j]]++;
        }
    }

    for (int i = 0; i < Graph::g->nb_vertices; i++)
        res._colors[i] = -1;

    double valMax;
    int colorMax;
    for (int i = 0; i < Graph::g->nb_colors; i++) {
        int indice = (startParent + i) % nbParents;
        Solution &currentParent = vParents[indice];
        std::vector<double> currentSizeOfColors = tSizeOfColors[indice];
        valMax = -1;
        colorMax = -1;

        if (i < 0) {
            int startColor = rand() / (double)RAND_MAX * Graph::g->nb_colors;
            for (int j = 0; j < Graph::g->nb_colors && colorMax < 0; j++) {
                int color = (startColor + j) % Graph::g->nb_colors;
                double currentVal = currentSizeOfColors[color];
                if (currentVal > 0) {
                    valMax = currentVal;
                    colorMax = color;
                }
            }
        } else {
            int startColor = rand() / (double)RAND_MAX * Graph::g->nb_colors;
            for (int j = 0; j < Graph::g->nb_colors; j++) {
                int color = (startColor + j) % Graph::g->nb_colors;
                double currentVal = currentSizeOfColors[color];

                if (currentVal > valMax) {
                    valMax = currentVal;
                    colorMax = color;
                }
            }
        }

        for (int j = 0; j < Graph::g->nb_vertices; j++) {
            if (currentParent._colors[j] == colorMax && res._colors[j] < 0) {
                res._colors[j] = i;

                for (int k = 0; k < nbParents; k++) {
                    tSizeOfColors[k][vParents[k]._colors[j]]--;
                }
            }
        }
    }

    int nbNotAttribute = 0;
    for (int i = 0; i < Graph::g->nb_vertices; i++) {
        if (res._colors[i] < 0) {
            nbNotAttribute++;
            res._colors[i] = (rand() / (double)RAND_MAX) * Graph::g->nb_colors;
        }
    }
    return res;
}

void Head::compute() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    humanTime =
        static_cast<double>(tv.tv_sec) + static_cast<double>(tv.tv_usec) / 1000000.0;

    // initRandSeed
    unsigned int rdSeed = time(NULL);
    srand(rdSeed);
    unsigned int randSeed[2] = {rdSeed, rdSeed + 1};
    // initRandSeed() end

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

    int seuil = 0.99 * Graph::g->nb_vertices;
    int proximity = 0;
    bool found = false;
    int currentElite = 0;
    int swapIter = -1;

    while (!found && proximity < seuil and
           (difftime(time(NULL), humanTime) < max_secondes || max_secondes < 0)) {

        nbIterationsCross++;

        vFils[0] = buildChild(vPopulation, 0);
        vFils[1] = buildChild(vPopulation, 1);

        std::vector<int> nb_iters(2, 0);
#pragma omp parallel for
        for (int i = 0; i < 2; i++) {
            nb_iters[i] = tabu_search(vFils[i], nbLocalSearch, randSeed[i]);
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
            if (vFils[i]._penalty <= vPopulation[i]._penalty ||
                rand() / (double)RAND_MAX < tauxAcceptWorst)
                vPopulation[i] = vFils[i];
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

        if (swapIter > 0 && (nbIterationsCross % swapIter == 0 || proximity >= seuil)) {
            currentElite = (currentElite + 1) % 2;

            int indivToReplace = rand() / (double)RAND_MAX * 2;
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
