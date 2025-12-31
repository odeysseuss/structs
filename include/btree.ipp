#include "btree.hpp"
#include <iostream>

template <typename T, int8_t order>
bool BtreeNode<T, order>::is_full() const {
    return num_keys == (2 * order - 1);
}

template <typename T, int8_t order>
void Btree<T, order>::split_root() {
    BtreeNode<T, order> *old_root = root;

    BtreeNode<T, order> *new_root = new BtreeNode<T, order>();
    BtreeNode<T, order> *left_node = new BtreeNode<T, order>();
    BtreeNode<T, order> *right_node = new BtreeNode<T, order>();

    // promote the median_key to new root
    // for order 3, max_keys will be 5 and median will be order/3 but
    // as keys array is 0 indexed median will be order - 1
    T median_key = old_root->keys[order - 1];
    new_root->keys[0] = median_key;
    new_root->num_keys = 1;

    // move the keys before median to newly created left node
    // move the keys after median to newly creted right nide
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

        // move the childrens 0-order to newly created left node
        // move the childrens order-end to newly created right node
        for (int8_t i = 0; i < order; i++) {
            left_node->childrens[i] = old_root->childrens[i];
            right_node->childrens[i] = old_root->childrens[i + order];
        }
        // clear pointers
        old_root->childrens.fill(nullptr);
    }

    // setup new root
    new_root->is_leaf = false;
    // add left and right node as new roots childrens
    new_root->childrens[0] = left_node;
    new_root->childrens[1] = right_node;

    delete old_root;
    root = new_root;
}

template <typename T, int8_t order>
void Btree<T, order>::split_child(BtreeNode<T, order> *parent, int8_t n) {
    // child to split (sounds creepy dude)
    BtreeNode<T, order> *child = parent->childrens[n];
    BtreeNode<T, order> *new_child = new BtreeNode<T, order>();

    new_child->is_leaf = child->is_leaf;

    // store median key
    T median_key = child->keys[order - 1];

    // new child gets the right keys after median
    for (int8_t i = 0; i < order - 1; i++) {
        new_child->keys[i] = child->keys[i + order];
        // clear these keys from child
        child->keys[i + order] = T();
    }

    child->keys[order - 1] = T(); // clear median
    child->num_keys = order - 1;
    new_child->num_keys = order - 1;

    // copy right half of childrens to new child
    if (!child->is_leaf) {
        for (int8_t i = 0; i < order; i++) {
            new_child->childrens[i] = child->childrens[i + order];
            child->childrens[i + order] = nullptr;
        }
    }

    // shift parent keys to right
    for (int8_t i = parent->num_keys; i > n; i--) {
        parent->keys[i] = parent->keys[i - 1];
    }

    // shift parent childrens to right
    for (int8_t i = parent->num_keys + 1; i > n + 1; i--) {
        parent->childrens[i] = parent->childrens[i - 1];
    }

    parent->keys[n] = median_key;
    parent->childrens[n + 1] = new_child;
    parent->num_keys++;
}

template <typename T, int8_t order>
void Btree<T, order>::insert(BtreeNode<T, order> *node, const T &key) {
    // position to insert
    int8_t i = node->num_keys - 1;

    if (node->is_leaf) {
        // shift keys to make space and insert
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->num_keys++;
    } else {
        // find child to descend into
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;

        if (node->childrens[i]->is_full()) {
            split_child(node, i);
            if (key > node->keys[i]) {
                i++;
            }
        }

        insert(node->childrens[i], key);
    }
}

template <typename T, int8_t order>
void Btree<T, order>::set(const T &key) {
    if (!root) {
        // if btree is empty create a new node with key
        root = new BtreeNode<T, order>();
        root->keys[0] = key;
        root->num_keys = 1;
        return;
    }

    if (root->is_full()) {
        split_root();
    }

    insert(root, key);
}

template <typename T, int8_t order>
BtreeNode<T, order> *Btree<T, order>::search(BtreeNode<T, order> *node,
                                             const T &key) const {
    if (!node)
        return nullptr;

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
    return search(node->childrens[i], key);
}

template <typename T, int8_t order>
const T *Btree<T, order>::get(const T &key) const {
    BtreeNode<T, order> *node = search(root, key);
    if (!node) {
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
    // print node header
    for (int j = 0; j < depth; j++)
        std::cout << "  ";
    std::cout << (node->is_leaf ? "Leaf" : "Internal") << "["
              << static_cast<int>(node->num_keys) << "]: ";

    // print keys
    for (int8_t i = 0; i < node->num_keys; i++) {
        std::cout << node->keys[i];
        if (i < node->num_keys - 1)
            std::cout << ", ";
    }
    std::cout << std::endl;

    // print children if not leaf
    if (!node->is_leaf) {
        for (int8_t i = 0; i <= node->num_keys; i++) {
            print_node(node->childrens[i], depth + 1);
        }
    }
}

template <typename T, int8_t order>
void Btree<T, order>::print() const {
    if (!root) {
        std::cout << "Empty B-tree (order " << static_cast<int>(order) << ")"
                  << std::endl;
        return;
    }

    print_node(root, 0);
}
