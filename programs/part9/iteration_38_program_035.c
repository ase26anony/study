/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
#include <stdint.h>

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static uint64_t multi_version_func(uint64_t a, uint64_t b) {
    /* Complex enough to not be optimized away, but simple for demonstration */
    uint64_t result = a;
    
    /* Use operations that might benefit from different ISA extensions */
    for (int i = 0; i < 64; i++) {
        result = (result >> 1) ^ (b & 1 ? 0x9E3779B97F4A7C15ULL : 0);
        b >>= 1;
    }
    
    /* Use atomic operation that might require helper function */
    __atomic_thread_fence(__ATOMIC_ACQ_REL);
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
extern uint64_t avx512_specific_func(uint64_t x) {
    /* Force use of AVX512 instructions */
    uint64_t result = x;
    
    /* This pattern might trigger compiler to use internal helpers */
    for (int i = 0; i < 8; i++) {
        result = (result * 0x5DEECE66DULL + 0xB) & ((1ULL << 48) - 1);
    }
    
    /* Memory barrier that might need special handling */
    __sync_synchronize();
    
    return result;
}

/* Force declaration of builtin helper */
uint64_t use_builtin_helpers(void) {
    /* Use builtins that may require runtime checking or helpers */
    if (__builtin_cpu_supports("avx2")) {
        return 1;
    }
    
    /* Atomic compare exchange with large type might need helper */
    __int128 atomic_var = 0;
    __int128 expected = 0;
    __int128 desired = 1;
    
    __atomic_compare_exchange(&atomic_var, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return atomic_var;
}
