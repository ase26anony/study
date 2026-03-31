/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[50];
static int cpu_models[20];
static volatile int sink;

/* Early CPU initialization - runs before main */
__attribute__((constructor))
static void init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger cache detection */
    const char *features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vbmi2",
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124vnniw",
        "avx5124fmaps", "avx512vp2intersect", "fma", "fma4", "xop",
        "aes", "pclmul", "sha", "gfni", "vaes", "vpclmulqdq",
        "movbe", "rdrand", "rdseed", "adx", "bmi", "bmi2", "lzcnt",
        "f16c", "fsgsbase", "rtm", "xtpr", "mwaitx", "clzero",
        "pku", "ospke", "waitpkg", "cldemote", "movdiri", "movdir64b"
    };
    
    for (int i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        cpu_features[i] = __builtin_cpu_supports(features[i]);
    }
    
    /* Query CPU models to trigger different cache table lookups */
    const char *models[] = {
        "intel", "amd", "atom", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "cooperlake", "tigerlake",
        "sapphirerapids", "alderlake", "rocketlake", "graniterapids"
    };
    
    for (int i = 0; i < sizeof(models)/sizeof(models[0]); i++) {
        cpu_models[i] = __builtin_cpu_is(models[i]);
    }
}

/* Target-specific functions to force driver to consider different architectures */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *array = malloc(256 * 1024); /* 256KB - typical L2 size */
    if (!array) return;
    
    /* Access with 64-byte stride (typical cache line) */
    for (int i = 0; i < 256 * 1024; i += 64) {
        array[i] = i;
    }
    
    sink = array[0];
    free((void*)array);
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *array = malloc(1024 * 1024); /* 1MB */
    if (!array) return;
    
    /* 32-byte stride for AVX */
    for (int i = 0; i < 1024 * 1024; i += 32) {
        array[i] = i;
    }
    
    sink = array[0];
    free((void*)array);
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *array = malloc(2048 * 1024); /* 2MB */
    if (!array) return;
    
    /* 64-byte stride */
    for (int i = 0; i < 2048 * 1024; i += 64) {
        array[i] = i;
    }
    
    sink = array[0];
    free((void*)array);
}

__attribute__((target("arch=znver1")))
static void zen_cache_test(void) {
    volatile char *array = malloc(512 * 1024); /* 512KB */
    if (!array) return;
    
    /* Mixed stride pattern */
    for (int i = 0; i < 512 * 1024; i += 32) {
        array[i] = i;
    }
    for (int i = 0; i < 512 * 1024; i += 64) {
        array[i] = i;
    }
    
    sink = array[0];
    free((void*)array);
}

/* Function to perform cache-sensitive timing */
static uint64_t time_cache_access(size_t size, int stride) {
    volatile char *array = malloc(size);
    if (!array) return 0;
    
    /* Initialize array */
    for (size_t i = 0; i < size; i++) {
        array[i] = (char)i;
    }
    
    uint64_t start, end;
    
    /* Use rdtsc for timing */
    start = __builtin_ia32_rdtsc();
    
    /* Access pattern sensitive to cache size */
    for (size_t i = 0; i < size; i += stride) {
        sink = array[i];
    }
    
    end = __builtin_ia32_rdtsc();
    
    free((void*)array);
    return end - start;
}

/* AVX2-optimized function with pragma */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_sensitive(void) {
    volatile int *array = aligned_alloc(32, 1024 * 1024);
    if (!array) return;
    
    /* AVX2 operations that might trigger cache queries */
    for (int i = 0; i < 1024 * 1024 / sizeof(int); i += 8) {
        /* Simulate some computation */
        array[i] = i * 2;
    }
    
    sink = array[0];
    free((void*)array);
}
#pragma GCC pop_options

/* AVX512-optimized function */
#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_sensitive(void) {
    volatile int *array = aligned_alloc(64, 2048 * 1024);
    if (!array) return;
    
    /* Larger working set for AVX512 */
    for (int i = 0; i < 2048 * 1024 / sizeof(int); i += 16) {
        array[i] = i * 3;
    }
    
    sink = array[0];
    free((void*)array);
}
#pragma GCC pop_options

int main(void) {
    uint64_t checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    skylake_cache_test();
    zen_cache_test();
    avx2_cache_sensitive();
    avx512_cache_sensitive();
    
    /* Perform cache timing tests with different sizes */
    uint64_t timings[8];
    
    /* Test different cache sizes that correspond to uncovered cases */
    timings[0] = time_cache_access(8 * 1024, 32);    /* 8KB L1 */
    timings[1] = time_cache_access(16 * 1024, 32);   /* 16KB L1 */
    timings[2] = time_cache_access(32 * 1024, 64);   /* 32KB L1 */
    timings[3] = time_cache_access(128 * 1024, 64);  /* 128KB L2 */
    timings[4] = time_cache_access(256 * 1024, 64);  /* 256KB L2 */
    timings[5] = time_cache_access(512 * 1024, 64);  /* 512KB L2 */
    timings[6] = time_cache_access(1024 * 1024, 64); /* 1MB L2 */
    timings[7] = time_cache_access(2048 * 1024, 64); /* 2MB L2 */
    
    /* Use timing ratios to branch (simulating cache detection) */
    if (timings[3] > timings[2] * 3) {
        /* Likely L2 cache miss pattern */
        checksum += 1;
    }
    
    if (timings[6] > timings[5] * 2) {
        /* Likely larger cache boundary */
        checksum += 2;
    }
    
    /* Add CPU feature results to checksum */
    for (int i = 0; i < 50; i++) {
        checksum += cpu_features[i];
    }
    
    for (int i = 0; i < 20; i++) {
        checksum += cpu_models[i];
    }
    
    /* Add timing results */
    for (int i = 0; i < 8; i++) {
        checksum += timings[i] & 0xFF;
    }
    
    /* Prevent dead code elimination */
    printf("CPU checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
