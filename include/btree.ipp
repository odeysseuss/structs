#include "btree.hpp"
#include <iostream>
#include <memory>

template <typename T, int8_t order>
auto BtreeNode<T, order>::is_full() const -> bool {
    return num_keys == ((2 * order) - 1);
}

template <typename T, int8_t order>
void Btree<T, order>::split_root() {
    auto old_root = std::move(root);

    auto new_root = std::make_unique<BtreeNode<T, order>>();
    auto left_node = std::make_unique<BtreeNode<T, order>>();
    auto right_node = std::make_unique<BtreeNode<T, order>>();

    // promote the median_key to new root
    T median_key = old_root->keys[order - 1];
    new_root->keys[0] = median_key;
    new_root->num_keys = 1;

    // move the keys before median to newly created left node
    // move the keys after median to newly created right node
    for (int8_t i = 0; i < order - 1; i++) {
        left_node->keys[i] = old_root->keys[i];
        right_node->keys[i] = old_root->keys[i + order];
    }

    // increment num keys for left and right node
    left_node->num_keys = order - 1;
    right_node->num_keys = order - 1;

    if (!old_root->is_leaf) {
        left_node->is_leaf = false;
        right_node->is_leaf = false;

        // move the children 0-order to newly created left node
        // move the children order-end to newly created right node
        for (int8_t i = 0; i < order; i++) {
            left_node->childrens[i] = std::move(old_root->childrens[i]);
            right_node->childrens[i] =
                std::move(old_root->childrens[i + order]);
        }
    }

    // setup new root
    new_root->is_leaf = false;
    // add left and right node as new root's children
    new_root->childrens[0] = std::move(left_node);
    new_root->childrens[1] = std::move(right_node);

    // old_root will be automatically deleted when it goes out of scope
    root = std::move(new_root);
}

template <typename T, int8_t order>
void Btree<T, order>::split_child(BtreeNode<T, order> *parent, int8_t n) {
    // get reference to the child to split
    std::unique_ptr<BtreeNode<T, order>> &child_ref = parent->childrens[n];
    auto new_child = std::make_unique<BtreeNode<T, order>>();

    new_child->is_leaf = child_ref->is_leaf;

    // store median key
    T median_key = child_ref->keys[order - 1];

    // new child gets the right keys after median
    for (int8_t i = 0; i < order - 1; i++) {
        new_child->keys[i] = std::move(child_ref->keys[i + order]);
    }

    // update key counts
    child_ref->num_keys = order - 1;
    new_child->num_keys = order - 1;

    // copy right half of children to new child
    if (!child_ref->is_leaf) {
        for (int8_t i = 0; i < order; i++) {
            new_child->childrens[i] =
                std::move(child_ref->childrens[i + order]);
        }
    }

    // shift parent keys to right
    for (int8_t i = parent->num_keys; i > n; i--) {
        parent->keys[i] = std::move(parent->keys[i - 1]);
    }

    // shift parent children to right
    for (int8_t i = parent->num_keys + 1; i > n + 1; i--) {
        parent->childrens[i] = std::move(parent->childrens[i - 1]);
    }

    parent->keys[n] = std::move(median_key);
    parent->childrens[n + 1] = std::move(new_child);
    parent->num_keys++;
}

template <typename T, int8_t order>
void Btree<T, order>::insert(BtreeNode<T, order> *node, const T &key) {
    // position to insert
    int8_t pos = node->num_keys - 1;

    if (node->is_leaf) {
        // shift keys to make space and insert
        while (pos >= 0 && key < node->keys[pos]) {
            node->keys[pos + 1] = std::move(node->keys[pos]);
            pos--;
        }
        node->keys[pos + 1] = key;
        node->num_keys++;
    } else {
        // find child to descend into
        while (pos >= 0 && key < node->keys[pos]) {
            pos--;
        }
        pos++;

        if (node->childrens[pos]->is_full()) {
            split_child(node, pos);
            if (key > node->keys[pos]) {
                pos++;
            }
        }

        insert(node->childrens[pos].get(), key);
    }
}

template <typename T, int8_t order>
void Btree<T, order>::set(const T &key) {
    if (root == nullptr) {
        // if btree is empty create a new node with key
        root = std::make_unique<BtreeNode<T, order>>();
        root->keys[0] = key;
        root->num_keys = 1;
        return;
    }

    if (root->is_full()) {
        split_root();
    }

    insert(root.get(), key);
}

template <typename T, int8_t order>
auto Btree<T, order>::search(BtreeNode<T, order> *node, const T &key) const
    -> BtreeNode<T, order> * {
    if (node == nullptr) {
        return nullptr;
    }

    // find the first key >= search key
    int8_t i = 0;
    while (i < node->num_keys && key > node->keys[i]) {
        i++;
    }

    // check if key is found
    if (i < node->num_keys && key == node->keys[i]) {
        return node;
    }

    // if leaf and not found, return nullptr
    if (node->is_leaf) {
        return nullptr;
    }

    // otherwise search in the appropriate child
    return search(node->childrens[i].get(), key);
}

template <typename T, int8_t order>
auto Btree<T, order>::get(const T &key) const -> const T * {
    BtreeNode<T, order> *node = search(root.get(), key);
    if (node == nullptr) {
        return nullptr;
    }

    // find the exact key in the node
    for (int8_t i = 0; i < node->num_keys; i++) {
        if (node->keys[i] == key) {
            return &node->keys[i];
        }
    }

    return nullptr;
}

template <typename T, int8_t order>
void Btree<T, order>::print_node(BtreeNode<T, order> *node, int depth) const {
    if (node == nullptr) {
        return;
    }

    // print node header
    for (int j = 0; j < depth; j++) {
        std::cout << "  ";
    }
    std::cout << (node->is_leaf ? "Leaf" : "Internal") << "["
              << static_cast<int>(node->num_keys) << "]: ";

    // print keys
    for (int8_t i = 0; i < node->num_keys; i++) {
        std::cout << node->keys[i];
        if (i < node->num_keys - 1) {
            std::cout << ", ";
        }
    }
    std::cout << '\n';

    // print children if not leaf
    if (!node->is_leaf) {
        for (int8_t i = 0; i <= node->num_keys; i++) {
            if (node->childrens[i]) {
                print_node(node->childrens[i].get(), depth + 1);
            }
        }
    }
}

template <typename T, int8_t order>
void Btree<T, order>::print() const {
    if (root == nullptr) {
        std::cout << "Empty B-tree (order " << static_cast<int>(order) << ")"
                  << '\n';
        return;
    }

    print_node(root.get(), 0);
}
