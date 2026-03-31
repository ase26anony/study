/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[256];
static int cpu_models[32];
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
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124fmaps",
        "avx5124vnniw", "avx512vp2intersect", "avx512bf16", "avx512fp16",
        "fma", "fma4", "xop", "aes", "pclmul", "sha", "gfni", "vaes",
        "vpclmulqdq", "movbe", "rdrnd", "rdseed", "adx", "lzcnt",
        "bmi", "bmi2", "f16c", "fsgsbase", "rtm", "xtpr", "mwait",
        "clwb", "pku", "ospke", "waitpkg", "cldemote", "movdiri",
        "movdir64b", "enqcmd", "serialize", "tsxldtrk", "amx-bf16",
        "amx-tile", "amx-int8", "uintr", "hreset", "kl", "widekl",
        "avxvnni", "avx512fp16", "avxifma", "avxvnniint8", "avxneconvert",
        "cmpccxadd", "amx-fp16", "prefetchi", "raoint", "amx-complex"
    };
    
    const char *models[] = {
        "intel", "amd", "athlon", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "cooperlake", "tigerlake",
        "sapphirerapids", "alderlake", "rocketlake", "graniterapids",
        "graniterapids-d", "sierraforest", "grandridge", "clearwaterforest",
        "znver1", "znver2", "znver3", "znver4", "znver5"
    };
    
    /* Store feature detection results */
    for (size_t i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        cpu_features[i] = __builtin_cpu_supports(features[i]);
    }
    
    /* Store model detection results */
    for (size_t i = 0; i < sizeof(models)/sizeof(models[0]); i++) {
        cpu_models[i] = __builtin_cpu_is(models[i]);
    }
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void test_core2_cache(void) {
    volatile char *array = malloc(1024 * 1024); /* 1MB */
    if (!array) return;
    
    /* Access with 64-byte stride (typical cache line) */
    for (int i = 0; i < 1024 * 1024; i += 64) {
        array[i] = i;
    }
    
    sink = array[0];
    free((void*)array);
}

__attribute__((target("arch=sandybridge")))
static void test_sandybridge_cache(void) {
    volatile char *array = malloc(2 * 1024 * 1024); /* 2MB */
    if (!array) return;
    
    /* Access with 32-byte and 64-byte strides */
    for (int i = 0; i < 2 * 1024 * 1024; i += 32) {
        array[i] = i;
    }
    for (int i = 0; i < 2 * 1024 * 1024; i += 64) {
        sink = array[i];
    }
    
    free((void*)array);
}

__attribute__((target("arch=skylake")))
static void test_skylake_cache(void) {
    volatile char *array = malloc(4 * 1024 * 1024); /* 4MB */
    if (!array) return;
    
    /* Mixed access patterns */
    for (int i = 0; i < 4 * 1024 * 1024; i += 128) {
        array[i] = i;
    }
    for (int i = 64; i < 4 * 1024 * 1024; i += 128) {
        sink = array[i];
    }
    
    free((void*)array);
}

__attribute__((target("arch=znver2")))
static void test_zen2_cache(void) {
    volatile char *array = malloc(8 * 1024 * 1024); /* 8MB */
    if (!array) return;
    
    /* Large stride to potentially miss caches */
    for (int i = 0; i < 8 * 1024 * 1024; i += 256) {
        array[i] = i;
    }
    
    sink = array[1024];
    free((void*)array);
}

/* Function with pragma to trigger AVX2 path */
#pragma GCC push_options
#pragma GCC target("avx2")
static void test_avx2_cache_sensitive(void) {
    /* Use AVX2 instructions that might trigger cache size queries */
    volatile int *array = malloc(1024 * 1024 * sizeof(int));
    if (!array) return;
    
    for (int i = 0; i < 1024 * 1024; i++) {
        array[i] = i;
    }
    
    /* Simple reduction to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 1024 * 1024; i += 64) {
        sum += array[i];
    }
    
    sink = sum;
    free((void*)array);
}
#pragma GCC pop_options

/* Timing-based cache detection */
static void measure_cache_effects(void) {
#define ARRAY_SIZE (4 * 1024 * 1024) /* 4MB */
    volatile char *array = malloc(ARRAY_SIZE);
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i & 0xFF;
    }
    
    /* Measure time for sequential access (should hit caches) */
    uint64_t start, end;
    
    /* Use rdtsc for timing */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < ARRAY_SIZE; i += 64) {
        sink = array[i];
    }
    end = __builtin_ia32_rdtsc();
    uint64_t seq_time = end - start;
    
    /* Measure time for random access (more cache misses) */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < 100000; i++) {
        int idx = (i * 97) % ARRAY_SIZE; /* Pseudo-random pattern */
        sink = array[idx];
    }
    end = __builtin_ia32_rdtsc();
    uint64_t rand_time = end - start;
    
    /* Use timing ratio to branch - may affect driver optimizations */
    if (rand_time > seq_time * 3) {
        /* Likely larger cache */
        cpu_features[0] |= 1;
    } else {
        /* Likely smaller cache */
        cpu_features[0] &= ~1;
    }
    
    free((void*)array);
}

int main(void) {
    /* Call all target-specific functions */
    test_core2_cache();
    test_sandybridge_cache();
    test_skylake_cache();
    test_zen2_cache();
    test_avx2_cache_sensitive();
    
    /* Measure cache effects */
    measure_cache_effects();
    
    /* Compute checksum from feature flags to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= cpu_features[i] * (i + 1);
    }
    for (int i = 0; i < 32; i++) {
        checksum ^= cpu_models[i] * (i + 257);
    }
    
    /* Use checksum to affect output */
    printf("CPU feature checksum: %d\n", checksum);
    
    /* Additional CPUID queries in main to ensure driver paths are taken */
    if (__builtin_cpu_supports("avx")) {
        checksum += 1;
    }
    if (__builtin_cpu_supports("avx2")) {
        checksum += 2;
    }
    if (__builtin_cpu_supports("avx512f")) {
        checksum += 4;
    }
    
    /* Check specific models that might have unique cache configurations */
    if (__builtin_cpu_is("intel")) {
        checksum += 8;
    }
    if (__builtin_cpu_is("amd")) {
        checksum += 16;
    }
    if (__builtin_cpu_is("core2")) {
        checksum += 32;
    }
    if (__builtin_cpu_is("skylake")) {
        checksum += 64;
    }
    if (__builtin_cpu_is("znver2")) {
        checksum += 128;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
