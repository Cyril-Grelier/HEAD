#pragma once

#include <limits>

#include "graphe.h"

class Solution {
    Graph *graph;

  public:
    Solution() {
        graph = NULL;
        tColor = NULL;
        nbEdgesConflict = nbNodesConflict = std::numeric_limits<int>::max();
        nbIterations = nbIterationsFirst = std::numeric_limits<unsigned long long>::max();
    }
    Solution(Graph *gr, int nbCol) {
        graph = gr;
        tColor = NULL;
        initRandom(nbCol);
    }
    Solution(const Solution &s) {
        graph = NULL;
        tColor = NULL;
        *this = s;
    }
    ~Solution();

    void initRandom(int nbCol);

    Solution &operator=(const Solution &s);
    void
    computeConflicts(int tConflicts[]); // calcule le nb de conflits et remplit le tableau
    int computeConflicts();
    int proxi(Solution &sol, bool changeToBestMatching = false);
    int nbSameColor(Solution &sol);
    void proxiIS(std::vector<Solution> &vSolRef,
                 std::vector<int> &vProxi,
                 std::vector<std::vector<std::pair<int, int>>> &vClosestRefSolIS);
    void proxiIS(Solution &solRef,
                 std::vector<int> &vProxi,
                 std::vector<int> &vClosestRefIS,
                 int nbColors1 = -1,
                 int nbColors2 = -1);
    void decresaseNbColors();
    void breakSymmetry();
    void print();
    void save(std::string filename);

    int *tColor; // contient pour chaque noeud sa couleur
    int nbEdgesConflict;
    int nbNodesConflict;
    unsigned long long nbIterations; // nb d'iterations locales pour la trouver
    unsigned long long
        nbIterationsFirst; // nb d'iterations de la première fois qu'on la trouve
};
