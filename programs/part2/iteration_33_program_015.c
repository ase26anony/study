/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink; /* Prevent optimization */

/* Early CPU initialization - runs before main */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* Force driver to execute CPUID and cache detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger different cache paths */
    cpu_features[0] = __builtin_cpu_supports("sse");
    cpu_features[1] = __builtin_cpu_supports("sse2");
    cpu_features[2] = __builtin_cpu_supports("sse3");
    cpu_features[3] = __builtin_cpu_supports("ssse3");
    cpu_features[4] = __builtin_cpu_supports("sse4.1");
    cpu_features[5] = __builtin_cpu_supports("sse4.2");
    cpu_features[6] = __builtin_cpu_supports("avx");
    cpu_features[7] = __builtin_cpu_supports("avx2");
    cpu_features[8] = __builtin_cpu_supports("avx512f");
    cpu_features[9] = __builtin_cpu_supports("avx512vl");
    cpu_features[10] = __builtin_cpu_supports("avx512bw");
    cpu_features[11] = __builtin_cpu_supports("avx512dq");
    cpu_features[12] = __builtin_cpu_supports("fma");
    cpu_features[13] = __builtin_cpu_supports("aes");
    cpu_features[14] = __builtin_cpu_supports("pclmul");
    cpu_features[15] = __builtin_cpu_supports("rdrand");
    cpu_features[16] = __builtin_cpu_supports("rdseed");
    cpu_features[17] = __builtin_cpu_supports("sha");
    cpu_features[18] = __builtin_cpu_supports("xsave");
    cpu_features[19] = __builtin_cpu_supports("xsaveopt");
    cpu_features[20] = __builtin_cpu_supports("xsavec");
    cpu_features[21] = __builtin_cpu_supports("xsaves");
    
    /* Query CPU models to trigger different cache descriptor tables */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1"); /* AMD Zen */
}

/* Target-specific functions to force driver to consider different microarchitectures */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char buffer[32768]; /* 32KB - typical L1 cache size */
    for (int i = 0; i < sizeof(buffer); i += 64) { /* 64-byte stride */
        sink = buffer[i];
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char buffer[262144]; /* 256KB - typical L2 cache size */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char buffer[1048576]; /* 1MB - typical L3 cache slice */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char buffer[2097152]; /* 2MB */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

__attribute__((target("arch=znver1")))
static void zen_cache_test(void) {
    volatile char buffer[524288]; /* 512KB L2 per core */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sink = buffer[i];
    }
}

/* Cache size detection through timing */
static uint64_t detect_cache_timing(void) {
    const size_t size = 8 * 1024 * 1024; /* 8MB - larger than most L3 caches */
    char *buffer = malloc(size);
    uint64_t start, end;
    
    if (!buffer) return 0;
    
    /* Initialize */
    memset(buffer, 1, size);
    
    /* Time sequential access (cache friendly) */
    start = __builtin_ia32_rdtsc();
    for (size_t i = 0; i < size; i += 64) {
        sink = buffer[i];
    }
    end = __builtin_ia32_rdtsc();
    uint64_t seq_time = end - start;
    
    /* Time random access (cache unfriendly) */
    /* Use a simple pseudo-random pattern */
    start = __builtin_ia32_rdtsc();
    size_t pos = 0;
    for (size_t i = 0; i < size / 64; i++) {
        pos = (pos * 1103515245 + 12345) % (size / 64);
        sink = buffer[pos * 64];
    }
    end = __builtin_ia32_rdtsc();
    uint64_t rand_time = end - start;
    
    free(buffer);
    
    /* Return ratio - different cache sizes will give different ratios */
    return (seq_time > 0) ? (rand_time * 100 / seq_time) : 0;
}

/* Vectorized test with pragma to trigger AVX paths */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    volatile int buffer[16384]; /* 64KB with 4-byte ints */
    for (int i = 0; i < 16384; i += 16) { /* Process 16 ints at a time */
        sink = buffer[i];
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    volatile double buffer[8192]; /* 64KB with 8-byte doubles */
    for (int i = 0; i < 8192; i += 8) { /* Process 8 doubles at a time */
        sink = (int)buffer[i];
    }
}
#pragma GCC pop_options

int main(void) {
    uint64_t cache_ratio;
    int checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    zen_cache_test();
    
    /* Call vectorized tests */
    avx2_cache_test();
    if (cpu_features[7]) { /* If AVX2 is supported */
        avx512_cache_test();
    }
    
    /* Perform cache timing detection */
    cache_ratio = detect_cache_timing();
    
    /* Compute checksum from CPU features and timing results */
    for (int i = 0; i < 22; i++) {
        checksum ^= cpu_features[i] << (i % 16);
    }
    for (int i = 0; i < 8; i++) {
        checksum ^= cpu_models[i] << (i % 16);
    }
    checksum ^= (int)(cache_ratio & 0xFFFFFFFF);
    checksum ^= (int)(cache_ratio >> 32);
    
    /* Use checksum to prevent dead code elimination */
    sink = checksum;
    
    /* Print minimal output to satisfy test framework */
    printf("CPU detection test completed. Checksum: %d\n", checksum);
    
    return 0;
}

/* { dg-final { scan-assembler "cpuid" } } */
/* { dg-final { scan-tree-dump "cache" "optimized" } } */
