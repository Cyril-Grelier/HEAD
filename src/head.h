#pragma once

#include <iostream>
#include <sys/time.h>

#include "tabouSearch.h"

class Head {
    Solution buildChild(std::vector<Solution> &vParents, int startParent);

  public:
    Head() {
    }

    void compute();

    Solution bestSol;

    unsigned long long nbIterations;
    int nbIterationsCross;

    int nbLocalSearch;
    int tauxAcceptWorst;

    int max_secondes{-1};
    time_t humanTime;
};
