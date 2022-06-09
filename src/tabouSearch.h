#pragma once

#include "solution.h"

struct Coloration {
    int vertex;
    int color;
};

int tabu_search(Solution &sol, int nbLocalSearch_);