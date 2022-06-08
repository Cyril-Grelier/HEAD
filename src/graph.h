#pragma once

#include <memory>
#include <string>
#include <vector>

struct Graph {
    /** @brief Graph used for the search, refer as Graph::g*/
    static std::unique_ptr<const Graph> g;

    const std::string name;
    const int nb_vertices;
    const int nb_edges;
    const std::vector<std::pair<int, int>> edges_list;
    const std::vector<std::vector<bool>> adjacency_matrix;
    const std::vector<std::vector<int>> neighborhood;
    const std::vector<int> degrees;
    const std::vector<int> weights;
    const int nb_colors;

    /** @brief Set the Graph for the search*/
    static void init_graph(std::unique_ptr<const Graph> graph_);
    explicit Graph(const std::string &name_,
                   const int &nb_vertices_,
                   const int &nb_edges_,
                   const std::vector<std::pair<int, int>> &edges_list_,
                   const std::vector<std::vector<bool>> &adjacency_matrix_,
                   const std::vector<std::vector<int>> &neighborhood_,
                   const std::vector<int> &degrees_,
                   const int &nb_colors_);
};

const std::unique_ptr<const Graph> load_graph(const std::string &instance_name,
                                              const int &nb_colors);
