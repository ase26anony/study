#include <stdatomic.h>

atomic_int g = 0;  // Use atomic type

void test() {
    int local = 5;
    int current_g = atomic_load(&g);  // Atomic read
    if (current_g > local) {
        atomic_store(&g, local);      // Atomic write
    }
}
