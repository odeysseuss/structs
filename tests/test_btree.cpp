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

    t.print();

    const int *val = t.get(5);
    if (val) {
        printf("Found %d\n", *val);
    } else {
        printf("Not found\n");
    }

    val = t.get(100);
    if (val) {
        printf("Found %d\n", *val);
    } else {
        printf("Not found\n");
    }
}
