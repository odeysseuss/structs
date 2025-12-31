#pragma once

#include <array>
#include <cstdint>

/// min children -> order
/// max children -> 2 * order
/// min keys -> order - 1
/// max keys ->  2 * order - 1
template <typename T, int8_t order>
class BtreeNode {
public:
    std::array<T, (2 * order - 1)> keys;
    std::array<BtreeNode *, (2 * order)> childrens;
    int8_t num_keys = 0;
    bool is_leaf = true;

    BtreeNode() = default;
    bool is_full() const;
};

template <typename T, int8_t order>
class Btree {
private:
    BtreeNode<T, order> *root = nullptr;
    /// split root
    void split_root();
    /// splits nth child node
    void split(BtreeNode<T, order> *parent, int8_t n);

public:
    Btree() = default;
    T get(const T key) const;
    void set(const T key);
    void print();
};
