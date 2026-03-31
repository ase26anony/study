/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
#include <stdint.h>

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static uint64_t multi_version_func(uint64_t a, uint64_t b) {
    /* Use operations that might benefit from different ISA extensions */
    uint64_t result = a;
    
    /* Mix of operations to prevent optimization */
    for (int i = 0; i < 4; i++) {
        result = (result * 1103515245 + 12345) ^ b;
        b = (b * 1664525 + 1013904223) ^ result;
    }
    
    /* Atomic operation that might need runtime support */
    __atomic_thread_fence(__ATOMIC_ACQ_REL);
    
    return result + b;
}

/* Force generation of another helper with explicit target attribute */
__attribute__((target("arch=x86-64-v4"), noinline, used))
static volatile uint64_t target_specific_helper(uint64_t x) {
    /* Volatile asm to prevent optimization */
    uint64_t y = x;
    asm volatile ("" : "+r" (y));
    
    /* Use __builtin_cpu_supports which may generate helper functions */
    if (__builtin_cpu_supports("avx512f")) {
        y = y * 7 + 1;
    }
    
    return y;
}

/* Public declaration to satisfy external linkage requirement */
extern uint64_t get_multi_version_result(uint64_t a, uint64_t b);

/* The actual implementation */
uint64_t get_multi_version_result(uint64_t a, uint64_t b) {
    /* Call both functions to ensure they're used */
    uint64_t r1 = multi_version_func(a, b);
    uint64_t r2 = target_specific_helper(r1);
    
    /* Prevent constant folding with runtime-dependent computation */
    if (r2 & 1) {
        return r1 + r2;
    } else {
        return r1 ^ r2;
    }
}
