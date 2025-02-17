#include "solution.h"
#include "random_generator.h"

Solution::Solution() : _colors(Graph::g->nb_vertices, -1) {
}

void Solution::init_random() {
    std::uniform_int_distribution<int> distribution(0, Graph::g->nb_colors - 1);
    for (int i = 0; i < Graph::g->nb_vertices; i++) {
        _colors[i] = distribution(rd::generator);
    }
}

std::vector<int> Solution::compute_conflicts() {
    const int nb_vertices = Graph::g->nb_vertices;
    nb_conflicting_vertices = 0;
    _penalty = 0;
    std::vector<int> nb_conflicts(nb_vertices, 0);

    for (int vertex1 = 0; vertex1 < nb_vertices; vertex1++) {
        for (int vertex2 = vertex1; vertex2 < nb_vertices; vertex2++) {
            if (Graph::g->adjacency_matrix[vertex1][vertex2] and
                _colors[vertex1] == _colors[vertex2]) {
                nb_conflicts[vertex1]++;
                nb_conflicts[vertex2]++;
                _penalty++;
                if (nb_conflicts[vertex1] == 1)
                    nb_conflicting_vertices++;
                if (nb_conflicts[vertex2] == 1)
                    nb_conflicting_vertices++;
            }
        }
    }
    return nb_conflicts;
}

int Solution::proxi(const Solution &sol) {
    int proxi = 0;
    int nbColors = 0;

    for (int i = 0; i < Graph::g->nb_vertices; i++) {
        if (_colors[i] > nbColors)
            nbColors = _colors[i];
        if (sol._colors[i] > nbColors)
            nbColors = sol._colors[i];
    }
    nbColors++;
    // pour identifier les meilleurs correspondance de couleurs
    std::vector<std::vector<int>> ttNbSameColor(nbColors, std::vector<int>(nbColors, 0));

    for (int i = 0; i < Graph::g->nb_vertices; i++)
        ttNbSameColor[_colors[i]][sol._colors[i]]++;

    std::vector<int> vCorrespondingColor(nbColors, 0);
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
    return proxi;
}
