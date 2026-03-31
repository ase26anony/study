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
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124vnniw",
        "avx5124fmaps", "avx512vp2intersect", "avx512bf16", "avx512fp16",
        "fma", "fma4", "xop", "aes", "pclmul", "rdseed", "rdrand",
        "sha", "gfni", "vaes", "vpclmulqdq", "movbe", "fsgsbase",
        "bmi", "bmi2", "lzcnt", "fxsr", "xsave", "xsaveopt",
        "xsavec", "xsaves", "mwaitx", "clzero", "pku", "ospke",
        "waitpkg", "cldemote", "movdiri", "movdir64b", "enqcmd",
        "serialize", "tsxldtrk", "amx-bf16", "amx-tile", "amx-int8",
        "uintr", "hreset", "kl", "widekl", "avxvnni", "avx512fp16",
        "cmpccxadd", "wrmsrns", "msrlist", "amx-fp16", "prefetchi",
        "raoint", "avxifma", "avxvnniint8", "avxneconvert", "cmpccxadd",
        NULL
    };
    
    /* Check each feature */
    for (int i = 0; features[i]; i++) {
        cpu_features[i] = __builtin_cpu_supports(features[i]);
    }
    
    /* Query CPU models to trigger different cache descriptor paths */
    const char *models[] = {
        "intel", "amd", "atom", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "cooperlake", "tigerlake",
        "sapphirerapids", "alderlake", "rocketlake", "graniterapids",
        "graniterapids-d", "sierraforest", "grandridge", "clearwaterforest",
        "znver1", "znver2", "znver3", "znver4", NULL
    };
    
    for (int i = 0; models[i]; i++) {
        cpu_models[i] = __builtin_cpu_is(models[i]);
    }
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *array = malloc(1024 * 1024); /* 1MB */
    if (!array) return;
    
    /* Access with 64-byte stride (typical cache line) */
    for (int i = 0; i < 1024 * 1024; i += 64) {
        array[i] = i & 0xFF;
    }
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
    
    free((void*)array);
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *array = malloc(2 * 1024 * 1024); /* 2MB */
    if (!array) return;
    
    /* Access with 32-byte and 64-byte strides */
    for (int i = 0; i < 2 * 1024 * 1024; i += 32) {
        sink = array[i];
    }
    for (int i = 0; i < 2 * 1024 * 1024; i += 64) {
        array[i] = sink;
    }
    
    asm volatile("" ::: "memory");
    free((void*)array);
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *array = malloc(4 * 1024 * 1024); /* 4MB */
    if (!array) return;
    
    /* More complex access pattern */
    for (int j = 0; j < 100; j++) {
        for (int i = 0; i < 4 * 1024 * 1024; i += 128) {
            sink = array[i];
            array[i + 64] = sink + 1;
        }
    }
    
    asm volatile("" ::: "memory");
    free((void*)array);
}

__attribute__((target("arch=znver3")))
static void zen3_cache_test(void) {
    volatile char *array = malloc(8 * 1024 * 1024); /* 8MB */
    if (!array) return;
    
    /* Random-ish access pattern */
    unsigned int seed = 12345;
    for (int i = 0; i < 10000; i++) {
        unsigned int idx = (seed * 1103515245 + 12345) % (8 * 1024 * 1024);
        sink = array[idx];
        seed = idx;
    }
    
    asm volatile("" ::: "memory");
    free((void*)array);
}

/* Function to measure cache-sensitive timing */
static uint64_t measure_cache_sensitivity(int size_kb) {
    int size = size_kb * 1024;
    volatile char *array = malloc(size);
    if (!array) return 0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i & 0xFF;
    }
    
    uint64_t start, end;
    
    /* Use RDTSC for timing if available */
    if (__builtin_cpu_supports("rdtsc")) {
        start = __builtin_ia32_rdtsc();
        
        /* Access every cache line */
        for (int i = 0; i < size; i += 64) {
            sink = array[i];
        }
        
        end = __builtin_ia32_rdtsc();
    } else {
        /* Fallback to simple counter */
        start = 0;
        for (int i = 0; i < size; i += 64) {
            sink = array[i];
        }
        end = size / 64;
    }
    
    free((void*)array);
    return end - start;
}

/* Main function with cache-size dependent branching */
int main(void) {
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    skylake_cache_test();
    zen3_cache_test();
    
    /* Measure access times for different buffer sizes */
    uint64_t times[8];
    int sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024};
    
    for (int i = 0; i < 8; i++) {
        times[i] = measure_cache_sensitivity(sizes[i]);
    }
    
    /* Create branches based on timing ratios (simulating cache size detection) */
    int cache_hints = 0;
    
    if (times[1] * 2 < times[3]) { /* Rough L1 vs L2 timing */
        cache_hints |= 1;
    }
    
    if (times[3] * 2 < times[5]) { /* Rough L2 vs L3 timing */
        cache_hints |= 2;
    }
    
    /* Use CPU feature flags to create data-dependent computation */
    unsigned int checksum = 0;
    
    /* Mix in feature detection results */
    for (int i = 0; i < 64; i++) {
        if (cpu_features[i]) {
            checksum = (checksum << 1) | (checksum >> 31);
            checksum ^= i * 0x9e3779b9;
        }
    }
    
    /* Mix in model detection results */
    for (int i = 0; i < 16; i++) {
        if (cpu_models[i]) {
            checksum = (checksum << 3) | (checksum >> 29);
            checksum ^= i * 0x9e3779b9;
        }
    }
    
    /* Mix in timing results */
    for (int i = 0; i < 8; i++) {
        checksum ^= (times[i] & 0xFFFFFFFF);
        checksum = (checksum << 5) | (checksum >> 27);
    }
    
    /* Use cache hints */
    checksum ^= cache_hints;
    
    /* Prevent dead code elimination */
    volatile unsigned int result = checksum;
    
    /* Use result in a way that can't be optimized away */
    if (result == 0x12345678) {
        printf("Impossible branch\n");
    }
    
    printf("CPU detection test completed (checksum: 0x%08x)\n", checksum);
    
    /* Additional forced CPUID queries */
    #pragma GCC push_options
    #pragma GCC target("avx2")
    {
        if (__builtin_cpu_supports("avx2")) {
            /* Force AVX2 code generation */
            __m256i vec = _mm256_setzero_si256();
            sink = _mm256_extract_epi32(vec, 0);
        }
    }
    #pragma GCC pop_options
    
    #pragma GCC push_options
    #pragma GCC target("avx512f")
    {
        if (__builtin_cpu_supports("avx512f")) {
            /* Force AVX512 code generation */
            __m512i vec512 = _mm512_setzero_si512();
            sink = _mm512_extract_epi32(vec512, 0);
        }
    }
    #pragma GCC pop_options
    
    return 0;
}

/* Force inclusion of AVX/AVX512 headers if available */
#ifdef __AVX2__
#include <immintrin.h>
#endif

#ifdef __AVX512F__
#include <immintrin.h>
#endif
