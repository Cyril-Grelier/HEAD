#pragma once
#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

/**
 * @brief Search for the element in the vector
 *
 * @tparam T value type
 * @param vector container
 * @param v value to find
 * @return true the value is in the vector
 * @return false the value is not in the vector
 */
template <class T> bool contains(const std::vector<T> &vector, const T &v) {
    auto it =
        std::lower_bound(vector.begin(), vector.end(), v, [](const T &l, const T &r) {
            return l < r;
        });
    return it != vector.end() && *it == v;
}

/**
 * @brief Insert element in sorted vector
 *
 * @tparam T value type
 * @param vector container
 * @param value value to add
 */
template <typename T> void insert_sorted(std::vector<T> &vector, T const &value) {
    if (vector.empty()) {
        vector.emplace_back(value);
    } else {
        vector.insert(std::upper_bound(std::begin(vector), std::end(vector), value),
                      value);
    }
}

/**
 * @brief Erase element from sorted vector
 *
 * @tparam T value type
 * @param vector container
 * @param value value to delete
 */
template <typename T> void erase_sorted(std::vector<T> &vector, T const &value) {
    auto lb = std::lower_bound(std::begin(vector), std::end(vector), value);
    if (lb != std::end(vector) and *lb == value) {
        vector.erase(lb);
    }
}
