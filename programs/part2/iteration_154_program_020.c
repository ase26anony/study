#include <stdatomic.h>

atomic_int g = 0;  // C11 atomic

void test() {
    int local = 5;
    int current_g = atomic_load(&g);
    if (current_g > local) {
        atomic_store(&g, local);
    }
}
