#include "btree.hpp"

template <typename T, int8_t order>
bool BtreeNode<T, order>::is_full() const {
    return num_keys == (2 * order - 1);
}

template <typename T, int8_t order>
void Btree<T, order>::split_root() {
    BtreeNode<T, order> *new_root = new BtreeNode<T, order>();
    BtreeNode<T, order> *left_node = new BtreeNode<T, order>();
    BtreeNode<T, order> *right_node = new BtreeNode<T, order>();

    // promote the median_key to new root
    // for order 3, max_keys will be 5 and median will be order/3 but
    // as keys array is 0 indexed median will be order - 1
    T median_key = root->keys.at(order - 1);
    new_root->keys.at(0) = median_key;

    // move the keys before median to newly created left node
    // move the keys after median to newly creted right nide
    for (int8_t i = 0; i < order - 1; i++) {
        left_node->keys.at(i) = root->keys.at(i);
        right_node->keys.at(i) = root->keys.at(i + order);
    }

    // increment num keys for left and right node
    left_node->num_keys = order - 1;
    right_node->num_keys = order - 1;

    if (!root->is_leaf) {
        left_node->is_leaf = false;
        right_node->is_leaf = false;

        // move the childrens 0-order to newly created left node
        // move the childrens order-end to newly created right node
        for (int8_t i = 0; i < order; i++) {
            left_node->childrens[i] = root->childrens[i];
            right_node->childrens[i] = root->childrens[i + order];
        }
    }

    // add left and right node as new roots childrens
    new_root->childrens.at(0) = left_node;
    new_root->childrens.at(1) = right_node;

    new_root->is_leaf = false;
    new_root->num_keys = 1;
    root = new_root;
}
