
#include "head.h"
#include "random_generator.h"

int main() {

    /* Options, changed by command-line arguments. */

    // "../instances/gcp_reduced/flat1000_50_0.col"
    // nbColor = 55
    // Mean CPU time   : 0,48 min
    // Mean humain time: 0,22 min
    // Mean iterations : 0,570 (x10.6)
    // Mean crossover  : 10

    // Mean CPU time   : 0,81 min
    // Mean humain time: 0,36 min
    // Mean iterations : 1,023 (x10.6)
    // Mean crossover  : 18

    // Mean CPU time   : 0,54 min
    // Mean humain time: 0,25 min
    // Mean iterations : 0,659 (x10.6)
    // Mean crossover  : 11

    std::string instance = "flat1000_50_0";
    const int nb_colors = 55;
    Graph::init_graph(load_graph(instance, nb_colors));

    rd::generator.seed(time(nullptr));

    Head solver = Head();

    // >> Solver.parameters
    solver.nbLocalSearch = 30000;
    solver.tauxAcceptWorst = 100;
    solver.max_secondes = -1;
    // << solver.parameters

    double totalCpuTime = 0;
    double totalHumanTime = 0;
    unsigned long long totalIterations = 0;
    unsigned long long totalIterationsCross = 0;
    unsigned long long totalIterationsCrossWithWrongRun = 0;
    int nbFound = 0;

    int nbRun = 1;
    //// >>>>>>>>>  affichage de l'heure au debut
    time_t raw_time;
    struct tm *time_info;
    time(&raw_time);
    time_info = localtime(&raw_time);
    printf("Start at :  %s", asctime(time_info));
    //// <<<<<<<<<  affichage de l'heure de debut

    int runId;
    for (runId = 0; (runId < nbRun) || ((nbRun == -1) && (nbFound == 0)); runId++) {
        clock_t startTime = clock();
        struct timeval tv;
        gettimeofday(&tv, NULL);
        double humanTime =
            static_cast<double>(tv.tv_sec) + static_cast<double>(tv.tv_usec) / 1000000.0;

        //// LANCEMENT DU CALCUL /////////////////
        solver.compute();

        double elapsedTime = static_cast<double>(clock() - startTime) /
                             static_cast<double>(CLOCKS_PER_SEC) / 60.0;
        gettimeofday(&tv, NULL);
        humanTime = (static_cast<double>(tv.tv_sec) +
                     static_cast<double>(tv.tv_usec) / 1000000.0 - humanTime) /
                    60.0;

        printf("nb conflicted edges: %d\n", solver.bestSol._penalty);

        printf("best coloring: ");
        // break Symmetry
        std::vector<int> swap(Graph::g->nb_vertices, -1);
        int color_curent = 0;
        for (int i = 0; i < Graph::g->nb_vertices; i++) {
            if (swap[solver.bestSol._colors[i]] == -1) {
                swap[solver.bestSol._colors[i]] = color_curent;
                solver.bestSol._colors[i] = color_curent;
                color_curent++;
            } else
                solver.bestSol._colors[i] = swap[solver.bestSol._colors[i]];
        }

        // print best sol
        for (int i = 0; i < Graph::g->nb_vertices; i++) {
            printf("%d ", solver.bestSol._colors[i]);
        }
        printf("\n");

        /////// Affichage de l'heure
        time(&raw_time);
        time_info = localtime(&raw_time);

        printf("\tFinished :  %s  (cpu time: %fmin, humain time : %fmin)\n",
               asctime(time_info),
               elapsedTime,
               humanTime);
        fflush(stdout);
        ///////

        totalIterationsCrossWithWrongRun += solver.nbIterationsCross;
        if (solver.bestSol._penalty == 0) {
            // solver.save(elapsedTime);
            nbFound++;
            totalCpuTime += elapsedTime;
            totalHumanTime += humanTime;
            totalIterations += solver.nbIterations;
            totalIterationsCross += solver.nbIterationsCross;
        }

        fflush(stdout);
    }
    printf("\n#Success / #Runs : %d / %d\n", nbFound, runId);

    if (nbFound > 0) {
        printf("Mean CPU time   : %0.2f min\n", totalCpuTime / nbFound);
        printf("Mean humain time: %0.2f min\n", totalHumanTime / nbFound);
        printf("Mean iterations : %0.3f (x10.6)\n",
               static_cast<double>(totalIterations) / static_cast<double>(nbFound) /
                   1000000.0);
        printf("Mean crossover  : %llu \n", totalIterationsCross / nbFound);
    }

    printf("End\n");
    fflush(stdout);
    return 0;
}
