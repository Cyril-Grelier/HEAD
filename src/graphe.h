#pragma once

#include <memory>
#include <string>
#include <vector>

struct Graph {
    const std::string name;
    const int nb_vertices;
    const int nb_edges;
    const std::vector<std::pair<int, int>> edges_list;
    const std::vector<std::vector<bool>> adjacency_matrix;
    const std::vector<std::vector<int>> neighborhood;
    const std::vector<int> degrees;
    const std::vector<int> weights;

    explicit Graph(const std::string &name_,
                   const int &nb_vertices_,
                   const int &nb_edges_,
                   const std::vector<std::pair<int, int>> &edges_list_,
                   const std::vector<std::vector<bool>> &adjacency_matrix_,
                   const std::vector<std::vector<int>> &neighborhood_,
                   const std::vector<int> &degrees_);
};

Graph *load_graph(const std::string &instance_name);
