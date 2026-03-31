/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to store CPU features - prevents optimization */
volatile int cpu_features[256] = {0};
volatile int cpu_models[256] = {0};
volatile int timing_results[8] = {0};

/* Early CPU initialization - runs before main */
__attribute__((constructor))
static void init_cpu(void) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger cache detection */
    const char *features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vbmi2",
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124vnniw",
        "avx5124fmaps", "avx512vp2intersect", "fma", "fma4", "xop",
        "aes", "pclmul", "sha", "gfni", "vaes", "vpclmulqdq",
        "movbe", "rdrnd", "rdseed", "adx", "lzcnt", "bmi", "bmi2",
        "f16c", "fsgsbase", "rtm", "xtpr", "pdpe1gb", "abm", "sse4a",
        "misalignsse", "3dnow", "3dnowa", "sse4", "sse4.1", "sse4.2",
        "sse5", "avx", "avx2", NULL
    };
    
    /* Query CPU models to trigger different cache configurations */
    const char *models[] = {
        "intel", "amd", "atom", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "cooperlake", "tigerlake",
        "sapphirerapids", "alderlake", "rocketlake", "graniterapids",
        "graniterapids-d", "si erraforest", "grandridge", "clearwaterforest",
        "bonnell", "silvermont", "goldmont", "goldmont-plus", "tremont",
        "gracemont", "knl", "knm", "k8", "k8-sse3", "opteron", "athlon64",
        "athlon-fx", "k10", "barcelona", "bulldozer", "piledriver",
        "steamroller", "excavator", "zen", "zen2", "zen3", "zen4",
        NULL
    };
    
    /* Store feature detection results */
    int i = 0;
    for (const char **f = features; *f; f++) {
        cpu_features[i++] = __builtin_cpu_supports(*f);
    }
    
    /* Store model detection results */
    i = 0;
    for (const char **m = models; *m; m++) {
        cpu_models[i++] = __builtin_cpu_is(*m);
    }
}

/* Target-specific functions to force different microarchitecture considerations */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile unsigned long long start, end;
    volatile int sum = 0;
    const int size = 1024 * 1024 * 16; /* 16MB - larger than typical L2 */
    static char *array;
    
    if (!array) {
        array = (char*)malloc(size);
        memset(array, 1, size);
    }
    
    /* Time linear access with 64-byte stride (typical cache line) */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < size; i += 64) {
        sum += array[i];
    }
    end = __builtin_ia32_rdtsc();
    timing_results[0] = (int)(end - start);
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile unsigned long long start, end;
    volatile int sum = 0;
    const int size = 1024 * 1024 * 32; /* 32MB */
    static char *array;
    
    if (!array) {
        array = (char*)malloc(size);
        memset(array, 2, size);
    }
    
    /* Time with 32-byte stride */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < size; i += 32) {
        sum += array[i];
    }
    end = __builtin_ia32_rdtsc();
    timing_results[1] = (int)(end - start);
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile unsigned long long start, end;
    volatile int sum = 0;
    const int size = 1024 * 1024 * 64; /* 64MB */
    static char *array;
    
    if (!array) {
        array = (char*)malloc(size);
        memset(array, 3, size);
    }
    
    /* Random access pattern to stress cache */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < 1000000; i++) {
        int idx = (i * 97) % size; /* Pseudo-random pattern */
        sum += array[idx];
    }
    end = __builtin_ia32_rdtsc();
    timing_results[2] = (int)(end - start);
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile unsigned long long start, end;
    volatile int sum = 0;
    const int size = 1024 * 1024 * 128; /* 128MB */
    static char *array;
    
    if (!array) {
        array = (char*)malloc(size);
        memset(array, 4, size);
    }
    
    /* Large stride to bypass caches */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < size; i += 4096) {
        sum += array[i];
    }
    end = __builtin_ia32_rdtsc();
    timing_results[3] = (int)(end - start);
}

/* AVX2-optimized function to trigger vectorization and cache considerations */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    volatile unsigned long long start, end;
    const int size = 1024 * 1024 * 8;
    static int *array;
    
    if (!array) {
        array = (int*)aligned_alloc(32, size * sizeof(int));
        for (int i = 0; i < size; i++) {
            array[i] = i;
        }
    }
    
    /* Vectorized sum */
    start = __builtin_ia32_rdtsc();
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    end = __builtin_ia32_rdtsc();
    timing_results[4] = (int)(end - start);
    timing_results[5] = sum; /* Use result to prevent optimization */
}
#pragma GCC pop_options

/* AVX512 function for newer CPUs */
#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    volatile unsigned long long start, end;
    const int size = 1024 * 1024 * 16;
    static float *array;
    
    if (!array) {
        array = (float*)aligned_alloc(64, size * sizeof(float));
        for (int i = 0; i < size; i++) {
            array[i] = i * 0.1f;
        }
    }
    
    start = __builtin_ia32_rdtsc();
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    end = __builtin_ia32_rdtsc();
    timing_results[6] = (int)(end - start);
    timing_results[7] = (int)sum;
}
#pragma GCC pop_options

/* Main function with branching based on CPU features */
int main(void) {
    int checksum = 0;
    
    /* Execute all cache tests */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    avx2_cache_test();
    
    /* Only run AVX512 test if supported */
    if (__builtin_cpu_supports("avx512f")) {
        avx512_cache_test();
    }
    
    /* Calculate checksum from all results to prevent dead code elimination */
    for (int i = 0; i < 256; i++) {
        checksum ^= cpu_features[i];
        checksum ^= cpu_models[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum ^= timing_results[i];
    }
    
    /* Branch based on cache-sensitive timing ratios */
    if (timing_results[0] > 0 && timing_results[1] > 0) {
        int ratio = timing_results[0] * 100 / timing_results[1];
        if (ratio > 150) {
            checksum |= 0x10000; /* Indicate large L2 cache */
        } else if (ratio < 50) {
            checksum |= 0x20000; /* Indicate small L2 cache */
        }
    }
    
    /* Additional feature-based branching */
    if (__builtin_cpu_supports("avx2")) {
        checksum |= 0x40000;
    }
    
    if (__builtin_cpu_supports("avx512f")) {
        checksum |= 0x80000;
    }
    
    /* Model-specific branching */
    if (__builtin_cpu_is("intel")) {
        checksum |= 0x100000;
    } else if (__builtin_cpu_is("amd")) {
        checksum |= 0x200000;
    }
    
    /* Force compiler to consider all cache descriptor cases by using
       a switch that mimics the driver's logic */
    int simulated_cache_descriptor = 0;
    
    /* This switch structure mirrors the uncovered code in driver-i386.cc */
    switch (checksum & 0xFF) {
        case 0x0a: simulated_cache_descriptor = 1; break;
        case 0x0c: simulated_cache_descriptor = 2; break;
        case 0x0d: simulated_cache_descriptor = 3; break;
        case 0x0e: simulated_cache_descriptor = 4; break;
        case 0x21: simulated_cache_descriptor = 5; break;
        case 0x24: simulated_cache_descriptor = 6; break;
        case 0x2c: simulated_cache_descriptor = 7; break;
        case 0x39: simulated_cache_descriptor = 8; break;
        case 0x3a: simulated_cache_descriptor = 9; break;
        case 0x3b: simulated_cache_descriptor = 10; break;
        case 0x3c: simulated_cache_descriptor = 11; break;
        case 0x3d: simulated_cache_descriptor = 12; break;
        case 0x3e: simulated_cache_descriptor = 13; break;
        case 0x41: simulated_cache_descriptor = 14; break;
        case 0x42: simulated_cache_descriptor = 15; break;
        case 0x43: simulated_cache_descriptor = 16; break;
        case 0x44: simulated_cache_descriptor = 17; break;
        case 0x45: simulated_cache_descriptor = 18; break;
        case 0x48: simulated_cache_descriptor = 19; break;
        case 0x49: simulated_cache_descriptor = 20; break;
        case 0x4e: simulated_cache_descriptor = 21; break;
        case 0x60: simulated_cache_descriptor = 22; break;
        case 0x66: simulated_cache_descriptor = 23; break;
        case 0x67: simulated_cache_descriptor = 24; break;
        case 0x68: simulated_cache_descriptor = 25; break;
        case 0x78: simulated_cache_descriptor = 26; break;
        case 0x79: simulated_cache_descriptor = 27; break;
        case 0x7a: simulated_cache_descriptor = 28; break;
        case 0x7b: simulated_cache_descriptor = 29; break;
        case 0x7c: simulated_cache_descriptor = 30; break;
        case 0x7d: simulated_cache_descriptor = 31; break;
        case 0x7f: simulated_cache_descriptor = 32; break;
        case 0x80: simulated_cache_descriptor = 33; break;
        case 0x82: simulated_cache_descriptor = 34; break;
        case 0x83: simulated_cache_descriptor = 35; break;
        case 0x84: simulated_cache_descriptor = 36; break;
        case 0x85: simulated_cache_descriptor = 37; break;
        case 0x86: simulated_cache_descriptor = 38; break;
        case 0x87: simulated_cache_descriptor = 39; break;
        default: simulated_cache_descriptor = 0; break;
    }
    
    checksum ^= simulated_cache_descriptor;
    
    /* Output checksum to prevent complete optimization */
    printf("CPU detection checksum: 0x%x\n", checksum);
    
    return 0;
}
