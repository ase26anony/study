/* Function designed to trigger target hook for multi-versioning */
#include <stdint.h>

/* This function should trigger the creation of artificial helper/resolver functions
   with the exact flags from the uncovered lines:
   TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_THIS_VOLATILE=1,
   TREE_NOTHROW=1, DECL_ARTIFICIAL=1, DECL_IGNORED_P=1,
   DECL_VISIBILITY_SPECIFIED=1, DECL_VISIBILITY=VISIBILITY_HIDDEN */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2"),
               noinline,
               used,
               visibility("hidden")))
static uint64_t target_specific_computation(uint64_t a, uint64_t b) {
    /* Use architecture-specific intrinsics to ensure different code paths */
    uint64_t result = a;
    
    /* Mix of operations that might be optimized differently per target */
    for (int i = 0; i < 32; i++) {
        result = (result ^ b) + (result << 3);
        b = (b ^ result) + (b >> 2);
    }
    
    /* Use __builtin_cpu_supports to potentially trigger internal helpers */
    if (__builtin_cpu_supports("avx2")) {
        result ^= 0xAAAAAAAAAAAAAAAAULL;
    }
    
    return result;
}

/* Force generation of atomic helper with large type */
__attribute__((noinline, target("default")))
static void atomic_operation(uint64_t *ptr) {
    __atomic_fetch_add(ptr, 1, __ATOMIC_SEQ_CST);
}

/* Public interface function */
__attribute__((visibility("default")))
uint64_t compute_hash(uint64_t seed) {
    uint64_t counter = 0;
    
    /* Call target-specific function - compiler may create internal clones */
    uint64_t hash = target_specific_computation(seed, 0xDEADBEEF);
    
    /* Force atomic operation that might need helper */
    atomic_operation(&counter);
    
    return hash ^ counter;
}
