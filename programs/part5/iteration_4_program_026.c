#include <stdatomic.h>
#include <stdbool.h>

// Use atomic for thread-safe flag checking
atomic_bool test_flag = ATOMIC_VAR_INIT(false);

// Or if you need volatile for hardware access:
volatile int hardware_flag = 0;

void run_tests(void) {
    // For thread-safe flags
    if (atomic_load(&test_flag)) {
        test_invalid_address_of_bitfield();
        test_incomplete_sizeof();
    }
    
    // For hardware flags (with memory barrier if needed)
    if (hardware_flag) {
        // Insert memory barrier if needed for your architecture
        // __sync_synchronize(); // GCC built-in
        test_invalid_address_of_bitfield();
        test_incomplete_sizeof();
    }
}
