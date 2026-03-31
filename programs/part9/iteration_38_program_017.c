/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
#include <stdint.h>

/* Function with target clones - compiler may generate internal resolver */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static uint64_t target_specific_compute(uint64_t a, uint64_t b) {
    /* Use architecture-specific operations */
    uint64_t result = a;
    
    /* Force different code generation per target */
    for (int i = 0; i < 64; i++) {
        if (b & (1ULL << i)) {
            result ^= (result << i) | (result >> (64 - i));
        }
    }
    
    /* Use atomic operation that might need helper */
    __atomic_thread_fence(__ATOMIC_ACQ_REL);
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("arch=core-avx2"), 
               noinline,
               noclone,
               used))
uint64_t avx2_optimized_hash(uint64_t x) {
    /* Complex enough to prevent optimization */
    uint64_t h = x;
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    
    /* Force potential helper generation */
    if (__builtin_cpu_supports("avx2")) {
        /* This might trigger internal function generation */
        __atomic_compare_exchange_n(&h, &x, h, 0, 
                                   __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    }
    
    return h;
}

/* Force generation of exception handling helpers (if using C++) */
#ifdef __cplusplus
__attribute__((target("default"), noinline, noclone))
void throw_helper(int x) {
    if (x > 100) {
        throw x;  /* May generate exception handling helpers */
    }
}
#endif
