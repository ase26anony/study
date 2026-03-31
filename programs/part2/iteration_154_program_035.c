#include <stdatomic.h>
atomic_int g = 0;

void test() {
    int local = 5;
    int current = atomic_load(&g);
    if (current > local) {
        atomic_store(&g, local);
    }
}
