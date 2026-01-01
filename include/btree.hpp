#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

/// min children -> order
/// max children -> 2 * order
/// min keys -> order - 1
/// max keys ->  2 * order - 1
template <typename T, int8_t order>
class BtreeNode {
private:
    std::array<T, ((2 * order) - 1)> keys;
    std::array<std::unique_ptr<BtreeNode>,
               (static_cast<std::size_t>(2 * order))>
        childrens;
    int8_t num_keys = 0;
    bool is_leaf = true;

public:
    BtreeNode() = default;
    [[nodiscard]] auto is_full() const -> bool;

    template <typename U, int8_t O>
    friend class Btree;
};

template <typename T, int8_t order>
class Btree {
private:
    std::unique_ptr<BtreeNode<T, order>> root = nullptr;

    /// split root
    void split_root();
    /// splits nth child node
    void split_child(BtreeNode<T, order> *parent, int8_t n);
    /// insert in key in node
    void insert(BtreeNode<T, order> *node, const T &key);
    /// search for key
    auto search(BtreeNode<T, order> *node, const T &key) const
        -> BtreeNode<T, order> *;
    void print_node(BtreeNode<T, order> *node, int depth) const;

public:
    Btree() = default;
    auto get(const T &key) const -> const T *;
    void set(const T &key);
    void print() const;
};
