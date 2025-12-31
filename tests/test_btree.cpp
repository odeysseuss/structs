#include "btree.ipp"

int main() {
    Btree<int, 3> t;
    t.set(10);
    t.set(20);
    t.set(20);
    t.set(5);
    t.set(15);
    t.set(25);
    t.set(35);
}
