/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC's CPUID leaf 2 cache detection
 * logic for various x86 microarchitectures. It uses conditional compilation
 * and function multi-versioning to target specific CPUs that should trigger
 * the uncovered case statements in driver-i386.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent aggressive optimizations */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache thrashing benchmark function */
static void cache_thrash(size_t size_kb, int iterations) {
    /* Allocate buffer larger than L2 cache to ensure memory traffic */
    size_t elements = (size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    volatile int result = 0;
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed for %zu KB buffer\n", size_kb);
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < elements; i++) {
        seed = seed * 1103515245 + 12345;
        buffer[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    /* Cache thrashing benchmark */
    for (int iter = 0; iter < iterations; iter++) {
        /* Access pattern designed to stress cache associativity */
        for (size_t i = 0; i < elements; i += 64) {  /* 64-byte stride */
            size_t idx = (i * 17) % elements;  /* Pseudo-random but deterministic */
            buffer[idx] = buffer[idx] * 3 + 1;
        }
        
        MEMORY_BARRIER();
        
        /* Another pattern with different stride */
        for (size_t i = 0; i < elements; i += 128) {
            size_t idx = (i * 13) % elements;
            result += buffer[idx];
        }
        
        MEMORY_BARRIER();
    }
    
    /* Use result to prevent dead code elimination */
    printf("Cache thrash result: %d\n", result);
    
    free(buffer);
}

/* ============================================================================
 * Architecture-specific implementations using GCC target attributes
 * ============================================================================ */

/* Target: Pentium III (should trigger cases 0x0a, 0x0c, 0x0d, 0x0e) */
#ifdef TEST_PENTIUM3
__attribute__((target("arch=pentium3")))
void benchmark_pentium3(void) {
    printf("Target: Pentium III\n");
    /* Pentium III typically has 16-32KB L1, 256-512KB L2 */
    cache_thrash(512, 100);  /* Larger than typical L2 */
}
#endif

/* Target: Pentium 4 (should trigger cases 0x21, 0x24, 0x39-0x3e, 0x41-0x45) */
#ifdef TEST_PENTIUM4
__attribute__((target("arch=pentium4")))
void benchmark_pentium4(void) {
    printf("Target: Pentium 4\n");
    /* Pentium 4 has 8KB L1, 256-2048KB L2 */
    cache_thrash(2048, 50);  /* Up to 2MB L2 */
}
#endif

/* Target: Xeon MP (for case 0x49 with xeon_mp=true) */
#ifdef TEST_XEON_MP
__attribute__((target("arch=nocona")))  /* Nocona is Xeon DP/MP */
void benchmark_xeon_mp(void) {
    printf("Target: Xeon MP (Nocona)\n");
    /* Xeon MP has large caches, case 0x49 should break */
    cache_thrash(4096, 30);  /* 4MB L2/L3 */
}
#endif

/* Target: Xeon DP (for case 0x49 with xeon_mp=false) */
#ifdef TEST_XEON_DP
__attribute__((target("arch=prescott")))  /* Prescott has 2MB L2 */
void benchmark_xeon_dp(void) {
    printf("Target: Xeon DP (Prescott)\n");
    /* Should trigger case 0x49 assignment */
    cache_thrash(4096, 30);
}
#endif

/* Target: AMD K8 (Athlon64/Opteron) (should trigger cases 0x40, 0x78-0x87) */
#ifdef TEST_AMD_K8
__attribute__((target("arch=k8")))
void benchmark_amd_k8(void) {
    printf("Target: AMD K8\n");
    /* K8 has 64KB L1, 512-1024KB L2 */
    cache_thrash(1024, 80);
}
#endif

/* Target: AMD K10 (Phenom) (should trigger cases 0x48, 0x4e) */
#ifdef TEST_AMD_K10
__attribute__((target("arch=amdfam10")))
void benchmark_amd_k10(void) {
    printf("Target: AMD K10\n");
    /* K10 has 512KB L2 per core, up to 6MB L3 */
    cache_thrash(6144, 20);  /* 6MB L3 */
}
#endif

/* Target: Core 2 (should trigger cases 0x66, 0x67, 0x68) */
#ifdef TEST_CORE2
__attribute__((target("arch=core2")))
void benchmark_core2(void) {
    printf("Target: Core 2\n");
    /* Core 2 has 32-64KB L1, 2-6MB L2 */
    cache_thrash(6144, 25);
}
#endif

/* Target: Nehalem (should trigger cases 0x7a-0x7d) */
#ifdef TEST_NEHALEM
__attribute__((target("arch=nehalem")))
void benchmark_nehalem(void) {
    printf("Target: Nehalem\n");
    /* Nehalem has 256KB L2 per core, up to 8MB L3 */
    cache_thrash(8192, 20);
}
#endif

/* ============================================================================
 * Multi-versioned function using target_clones
 * This creates multiple versions for different architectures
 * ============================================================================ */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2, nehalem")))
void multiversion_benchmark(void) {
    printf("Multi-versioned benchmark\n");
    cache_thrash(4096, 40);
}
#endif

/* ============================================================================
 * Main function with conditional compilation
 * ============================================================================ */
int main(int argc, char **argv) {
    printf("Cache Detection Test Program\n");
    printf("============================\n\n");
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Run architecture-specific benchmarks based on compile-time defines */
    
#ifdef TEST_PENTIUM3
    benchmark_pentium3();
#endif

#ifdef TEST_PENTIUM4
    benchmark_pentium4();
#endif

#ifdef TEST_XEON_MP
    benchmark_xeon_mp();
#endif

#ifdef TEST_XEON_DP
    benchmark_xeon_dp();
#endif

#ifdef TEST_AMD_K8
    benchmark_amd_k8();
#endif

#ifdef TEST_AMD_K10
    benchmark_amd_k10();
#endif

#ifdef TEST_CORE2
    benchmark_core2();
#endif

#ifdef TEST_NEHALEM
    benchmark_nehalem();
#endif

#ifdef USE_MULTIVERSIONING
    multiversion_benchmark();
#endif

    /* If no specific test defined, run generic cache test */
#if !defined(TEST_PENTIUM3) && !defined(TEST_PENTIUM4) && \
    !defined(TEST_XEON_MP) && !defined(TEST_XEON_DP) && \
    !defined(TEST_AMD_K8) && !defined(TEST_AMD_K10) && \
    !defined(TEST_CORE2) && !defined(TEST_NEHALEM) && \
    !defined(USE_MULTIVERSIONING)
    
    printf("Generic cache test (using -march=native)\n");
    cache_thrash(8192, 20);
    
#endif

    printf("\nTest completed.\n");
    return 0;
}
