/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
#include <stdint.h>

/* Function with target_clones attribute - this often triggers creation of 
   internal resolver functions with the exact flags from the uncovered lines */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static uint64_t target_specific_computation(uint64_t a, uint64_t b) {
    /* Use architecture-specific intrinsics to ensure different code paths */
    uint64_t result = a + b;
    
    /* Add some computation that could benefit from vectorization */
    for (int i = 0; i < 4; i++) {
        result = (result << 5) ^ (result >> 3);
    }
    
    return result;
}

/* Force generation of atomic helper function - often creates artificial decls */
__attribute__((noinline, noclone))
static uint64_t atomic_helper(uint64_t *ptr) {
    uint64_t expected = *ptr;
    uint64_t desired = expected + 1;
    
    /* __atomic_compare_exchange with large type may need helper function */
    __atomic_compare_exchange(ptr, &expected, &desired, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return desired;
}

/* Public interface function */
uint64_t compute_with_features(uint64_t a, uint64_t b) {
    /* Mix both functions to ensure both are considered */
    uint64_t r1 = target_specific_computation(a, b);
    uint64_t r2 = atomic_helper(&r1);
    return r1 ^ r2;
}
