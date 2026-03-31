/* Force generation of target-specific clones with hidden visibility */
#include <stdint.h>

/* Function with multiple target clones - compiler will generate resolver */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               used, 
               visibility("hidden")))
uint64_t target_specific_compute(uint64_t a, uint64_t b) {
    /* Complex enough to not be optimized away, but simple for demonstration */
    uint64_t result = a;
    
    /* Use operations that might benefit from vectorization */
    for (int i = 0; i < 64; i++) {
        if (b & (1ULL << i)) {
            result ^= (result << i) | (result >> (64 - i));
        }
    }
    
    /* Add some arithmetic that could use different ISA extensions */
    result = result * 6364136223846793005ULL;
    result = result ^ (result >> 32);
    
    return result;
}

/* Another approach: Use __builtin_cpu_supports which may generate helpers */
__attribute__((target("arch=x86-64-v3"), noinline, used))
uint64_t avx2_version(uint64_t x) {
    /* Force potential use of AVX2 operations through intrinsics */
    uint64_t arr[4] = {x, x, x, x};
    uint64_t sum = 0;
    
    /* This pattern might encourage vectorization */
    for (int i = 0; i < 4; i++) {
        arr[i] = arr[i] * 0x9e3779b97f4a7c15ULL;
        sum += arr[i];
    }
    
    return sum;
}

/* Exception handling in C++ can also generate artificial helpers */
#ifdef __cplusplus
__attribute__((noinline, target("default")))
int throwing_function(int x) {
    if (x < 0) {
        throw x;  /* May generate exception handling helpers */
    }
    return x * 2;
}
#endif
