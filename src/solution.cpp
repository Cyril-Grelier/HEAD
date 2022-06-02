
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "solution.h"

using namespace std;

Solution::~Solution() {
    if (tColor) {
        delete[] tColor;
    }
}

void Solution::initRandom(int nbCol) {
    if (!graph) {
        cout << "Erreur : initRandom sans graph associe\n";
        exit(-1);
    }

    if (!tColor) {
        tColor = new int[graph->nb_vertices];
    }

    for (int i = 0; i < graph->nb_vertices; i++) {
        tColor[i] = static_cast<int>(rand() / static_cast<double>(RAND_MAX)) * nbCol;
    }

    nbIterations = nbIterationsFirst = nbNodesConflict = nbEdgesConflict = 999999;
}

Solution &Solution::operator=(const Solution &s) {
    nbIterations = s.nbIterations;
    nbIterationsFirst = s.nbIterationsFirst;
    nbEdgesConflict = s.nbEdgesConflict;
    nbNodesConflict = s.nbNodesConflict;

    int nbSommets1 = graph ? graph->nb_vertices : -1;
    int nbSommets2 = s.graph ? s.graph->nb_vertices : -1;

    graph = s.graph;

    if (nbSommets1 != nbSommets2) {
        if (nbSommets1 > 0) {
            delete[] tColor;
            tColor = NULL;
        }
        if (nbSommets2 > 0) {
            tColor = new int[nbSommets2];
        }
    }

    if (nbSommets2 > 0) {
        for (int i = 0; i < nbSommets2; i++) {
            tColor[i] = s.tColor[i];
        }
    }

    return *this;
}

void Solution::computeConflicts(int tConflicts[]) {
    if (!graph) {
        cout << "Erreur : computeConflicts sans graph associe\n";
        exit(-1);
    }

    int nb_vertices = graph->nb_vertices;

    nbNodesConflict = 0;
    nbEdgesConflict = 0;
    for (int i = 0; i < nb_vertices; i++)
        tConflicts[i] = 0;

    for (int i = 0; i < nb_vertices; i++) {
        for (int j = i; j < nb_vertices; j++) {
            if (graph->adjacency_matrix[i][j] && tColor[i] == tColor[j]) {
                tConflicts[i]++;
                tConflicts[j]++;
                nbEdgesConflict++;
                if (tConflicts[i] == 1)
                    nbNodesConflict++;
                if (tConflicts[j] == 1)
                    nbNodesConflict++;
            }
        }
    }
}

int Solution::computeConflicts() {
    if (!graph) {
        cout << "Erreur : computeConflicts sans graph associe\n";
        exit(-1);
    }

    int nb_vertices = graph->nb_vertices;
    vector<bool> vNodeConf(nb_vertices, false);

    nbNodesConflict = nbEdgesConflict = 0;

    for (int i = 0; i < nb_vertices - 1; i++) {
        for (int j = i + 1; j < nb_vertices; j++) {
            if (graph->adjacency_matrix[i][j] && tColor[i] == tColor[j]) {
                nbEdgesConflict++;
                if (!vNodeConf[i])
                    nbNodesConflict++;
                vNodeConf[i] = true;
                if (!vNodeConf[j])
                    nbNodesConflict++;
                vNodeConf[j] = true;
            }
        }
    }

    return nbEdgesConflict;
}

// determine la proximité de 2 individus (on identifier les meilleurs associations de
// couleur entre les 2 individus). si changeToBestMatching est vrai : on change les
// couleurs de l'individu courant pour avoir le meilleur matching
int Solution::proxi(Solution &sol, bool changeToBestMatching) {
    int proxi = 0;
    int nbColors = 0;

    for (int i = 0; i < graph->nb_vertices; i++) {
        if (tColor[i] > nbColors)
            nbColors = tColor[i];
        if (sol.tColor[i] > nbColors)
            nbColors = sol.tColor[i];
    }

    nbColors++;

    // pour identifier les meilleurs correspondance de couleurs
    std::vector<std::vector<int>> ttNbSameColor(nbColors, std::vector<int>(nbColors, 0));

    for (int i = 0; i < graph->nb_vertices; i++)
        ttNbSameColor[tColor[i]][sol.tColor[i]]++;

    vector<int> vCorrespondingColor(nbColors, 0);
    for (int c = 0; c < nbColors; c++) {
        int maxVal = -1, maxI = -1, maxJ = -1;
        for (int i = 0; i < nbColors; i++) {
            for (int j = 0; j < nbColors; j++) {
                if (ttNbSameColor[i][j] > maxVal) {
                    maxVal = ttNbSameColor[i][j];
                    maxI = i;
                    maxJ = j;
                }
            }
        }

        vCorrespondingColor[maxI] = maxJ;
        proxi += maxVal;

        for (int i = 0; i < nbColors; i++) {
            ttNbSameColor[maxI][i] = -1;
            ttNbSameColor[i][maxJ] = -1;
        }
    }

    if (changeToBestMatching) {
        for (int i = 0; i < graph->nb_vertices; i++)
            tColor[i] = vCorrespondingColor[tColor[i]];
    }

    return proxi;
}

// compte le nombre de sommets ayant la même couleur
int Solution::nbSameColor(Solution &sol) {
    int proxi = 0;
    for (int i = 0; i < graph->nb_vertices; i++) {
        if (tColor[i] == sol.tColor[i])
            proxi++;
    }

    return proxi;
}

// determine la proximité des IS de 2 individus.
// pour chaque couleur détermine les independant set qui ont le + de sommets en commun
// renvoie pour chaque IS la meilleur proxi (vProxi) est l'ensemble des IS de ce niveau de
// proximité (couple solution, IS)
void Solution::proxiIS(vector<Solution> &vSolRef,
                       vector<int> &vProxi,
                       vector<vector<pair<int, int>>> &vClosestRefSolIS) {
    /// compte le nombre de couleurs
    int nbColors1 = 0, nbColors2 = 0;
    Solution &sol = vSolRef[0];
    for (int i = 0; i < graph->nb_vertices; i++) {
        if (tColor[i] > nbColors1)
            nbColors1 = tColor[i];
        if (sol.tColor[i] > nbColors2)
            nbColors2 = sol.tColor[i];
    }
    nbColors1++;
    nbColors2++;
    ///

    vProxi.clear();
    vProxi.resize(nbColors1, 0);
    vClosestRefSolIS.clear();
    vClosestRefSolIS.resize(nbColors1);

    vector<int> vClosestRefIS(nbColors1); // vecteur rempli par la seconde fonction
                                          // proxiIS pour chaque solution de référence
    vector<Solution>::iterator it;
    vector<int> vPrevProxi;

    for (it = vSolRef.begin(); it != vSolRef.end(); it++) {
        vPrevProxi = vProxi;
        proxiIS(*it, vProxi, vClosestRefIS, nbColors1, nbColors2);
        for (int i = 0; i < nbColors1; i++) {
            if (vClosestRefIS[i] >= 0) { /// la classe i a ete mise a jour
                if (vProxi[i] > vPrevProxi[i])
                    vClosestRefSolIS[i].clear();
                vClosestRefSolIS[i].push_back(
                    pair<int, int>(distance(vSolRef.begin(), it), vClosestRefIS[i]));
            }
        }
    }
}

// determine la proximité des IS de 2 individus.
// pour chaque couleur vérifie s'il existe une couleur de sol qui est plus proche
void Solution::proxiIS(Solution &solRef,
                       vector<int> &vProxi,
                       vector<int> &vClosestRefIS,
                       int nbColors1,
                       int nbColors2) {
    /// Determination si besoin du nombre de couleurs
    if (nbColors1 < 0) {
        nbColors1 = 0;
        for (int i = 0; i < graph->nb_vertices; i++)
            if (tColor[i] > nbColors1)
                nbColors1 = tColor[i];
        nbColors1++;
    }
    if (nbColors2 < 0) {
        nbColors2 = 0;
        for (int i = 0; i < graph->nb_vertices; i++)
            if (solRef.tColor[i] > nbColors2)
                nbColors2 = solRef.tColor[i];
        nbColors2++;
    }
    ///

    for (int i = 0; i < nbColors1; i++) {
        vClosestRefIS[i] = -1;
    }
    // pour identifier les meilleurs correspondance de couleurs
    std::vector<std::vector<int>> ttNbSameColor(nbColors1,
                                                std::vector<int>(nbColors2, 0));

    for (int i = 0; i < graph->nb_vertices; i++)
        ttNbSameColor[tColor[i]][solRef.tColor[i]]++;

    for (int i = 0; i < nbColors1; i++) {
        int maxVal = -1, maxId = -1;
        for (int j = 0; j < nbColors2; j++) {
            if (ttNbSameColor[i][j] > maxVal) {
                maxVal = ttNbSameColor[i][j];
                maxId = j;
            }
        }
        if (maxVal >= vProxi[i]) {
            vProxi[i] = maxVal;
            vClosestRefIS[i] = maxId;
        }
    }
}

void Solution::decresaseNbColors() {
    /// Determination du nombre de couleurs
    int nbColors = 0;
    for (int i = 0; i < graph->nb_vertices; i++)
        if (tColor[i] > nbColors)
            nbColors = tColor[i];
    // nbColors contient la couleur max courante, donc le nb de couleurs cible

    int nb_vertices = graph->nb_vertices;
    // int colToRemove = rand()/(double)RAND_MAX * (nbColors+1); // on eneleve une couleur
    // au hazard
    int colToRemove = nbColors; // on enleve la dernière couleur
    for (int i = 0; i < nb_vertices; i++) {
        if (tColor[i] == colToRemove)
            tColor[i] =
                static_cast<int>(rand() / static_cast<double>(RAND_MAX)) * nbColors;
        else if (tColor[i] > colToRemove)
            tColor[i]--;
    }
}

/** Permute les couleurs pour que les plus petites couleurs
    soient appribuées aux sommets de plus petit indice
*/
void Solution::breakSymmetry() {
    int *swap = new int[graph->nb_vertices];
    int color_curent = 0;
    for (int i = 0; i < graph->nb_vertices; i++)
        swap[i] = -1;
    for (int i = 0; i < graph->nb_vertices; i++) {
        if (swap[tColor[i]] == -1) {
            swap[tColor[i]] = color_curent;
            tColor[i] = color_curent;
            color_curent++;
        } else
            tColor[i] = swap[tColor[i]];
    }
    delete[] swap;
}

void Solution::print() {
    for (int i = 0; i < graph->nb_vertices; i++) {
        printf("%d ", tColor[i]);
    }
    printf("\n");
}

void Solution::save(string filename) {
    FILE *f;
    f = fopen(filename.c_str(), "a");

    for (int i = 0; i < graph->nb_vertices; i++) {
        fprintf(f, "%d\t", tColor[i]);
    }
    fprintf(f, "\n");

    fclose(f);
}
