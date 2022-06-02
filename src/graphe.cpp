#include "graphe.h"

#include <fstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <fmt/printf.h>
#pragma GCC diagnostic pop

Graph::Graph(const std::string &name_,
             const int &nb_vertices_,
             const int &nb_edges_,
             const std::vector<std::pair<int, int>> &edges_list_,
             const std::vector<std::vector<bool>> &adjacency_matrix_,
             const std::vector<std::vector<int>> &neighborhood_,
             const std::vector<int> &degrees_)
    : name(name_),
      nb_vertices(nb_vertices_),
      nb_edges(nb_edges_),
      edges_list(edges_list_),
      adjacency_matrix(adjacency_matrix_),
      neighborhood(neighborhood_),
      degrees(degrees_) {
}

Graph *load_graph(const std::string &instance_name) {
    // load the edges and vertices of the graph
    std::ifstream file;
    file.open("../instances/gcp_reduced/" + instance_name + ".col");

    if (!file) {
        fmt::print(stderr,
                   "Didn't find {} in ../instances/wvcp_reduced/ or "
                   "../instances/gcp_reduced/ (if problem == gcp)\n"
                   "Did you run \n\n"
                   "git submodule init\n"
                   "git submodule update\n\n"
                   "before executing the program ?(import instances)\n"
                   "Otherwise check that you are in the build "
                   "directory before executing the program\n",
                   instance_name);
        exit(1);
    }
    int nb_vertices{0}, nb_edges{0}, n1{0}, n2{0};
    std::vector<std::pair<int, int>> edges_list;
    std::string first;
    file >> first;
    while (!file.eof()) {
        if (first == "e") {
            file >> n1 >> n2;
            edges_list.emplace_back(--n1, --n2);
        } else if (first == "p") {
            file >> first >> nb_vertices >> nb_edges;
            edges_list.reserve(nb_edges);
        } else {
            getline(file, first);
        }
        file >> first;
    }
    file.close();

    std::vector<std::vector<bool>> adjacency_matrix(
        nb_vertices, std::vector<bool>(nb_vertices, false));
    std::vector<std::vector<int>> neighborhood(nb_vertices, std::vector<int>(0));
    std::vector<int> degrees(nb_vertices, 0);
    // Init adjacency matrix and neighborhood of the vertices
    for (auto p : edges_list) {
        if (not adjacency_matrix[p.first][p.second]) {
            adjacency_matrix[p.first][p.second] = true;
            adjacency_matrix[p.second][p.first] = true;
            neighborhood[p.first].push_back(p.second);
            neighborhood[p.second].push_back(p.first);
            ++nb_edges;
        }
    }
    // Init degrees_ of the vertices
    for (int vertex{0}; vertex < nb_vertices; ++vertex) {
        degrees[vertex] = static_cast<int>(neighborhood[vertex].size());
    }
    return new Graph(instance_name,
                     nb_vertices,
                     nb_edges,
                     edges_list,
                     adjacency_matrix,
                     neighborhood,
                     degrees);
}
