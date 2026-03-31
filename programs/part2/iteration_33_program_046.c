/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[256];
static int cpu_models[256];
static volatile int sink;

/* Early CPU initialization - runs before main */
static void __attribute__((constructor)) init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide range of CPU features to trigger cache detection */
    const char *features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vbmi2",
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124vnniw",
        "avx5124fmaps", "avx512vp2intersect", "fma", "fma4", "xop",
        "f16c", "bmi", "bmi2", "lzcnt", "tbm", "aes", "pclmul",
        "sha", "gfni", "vaes", "vpclmulqdq", "rdrand", "rdseed",
        "fsgsbase", "ptwrite", "clflushopt", "clwb", "xsave",
        "xsaveopt", "xsavec", "xgetbv", "xsaves", "mwaitx", "clzero",
        "pku", "ospke", "waitpkg", "cldemote", "movdiri", "movdir64b",
        "enqcmd", "serialize", "tsxldtrk", "amx-bf16", "amx-tile",
        "amx-int8", "uintr", "hreset", "kl", "widekl", "avxvnni",
        "avx512fp16", "avxifma", "avxvnniint8", "avxneconvert",
        "cmpccxadd", "amx-fp16", "prefetchi", "raoint", "amx-complex",
        NULL
    };
    
    /* Query CPU models to trigger different cache table lookups */
    const char *models[] = {
        "intel", "amd", "atom", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "cooperlake", "tigerlake",
        "sapphirerapids", "alderlake", "rocketlake", "meteorlake",
        "graniterapids", "granitecove", "grandridge", "sierralakes",
        "clearwaterforest", "bonnell", "silvermont", "goldmont",
        "goldmont-plus", "tremont", "gracemont", "grandridge",
        "knl", "knm", "k8", "k8-sse3", "opteron", "athlon64",
        "athlon-fx", "barcelona", "shanghai", "istanbul", "amdfam10",
        "bdver1", "bdver2", "bdver3", "bdver4", "znver1", "znver2",
        "znver3", "znver4", "znver5", NULL
    };
    
    /* Store feature detection results */
    for (int i = 0; features[i]; i++) {
        cpu_features[i] = __builtin_cpu_supports(features[i]);
    }
    
    /* Store model detection results */
    for (int i = 0; models[i]; i++) {
        cpu_models[i] = __builtin_cpu_is(models[i]);
    }
}

/* Target-specific functions to force driver to consider different cache configs */
static void __attribute__((target("arch=core2"))) 
core2_cache_test(void) {
    volatile char *array = malloc(1024 * 1024);
    if (!array) return;
    
    /* Access pattern that depends on cache line size */
    for (int i = 0; i < 1024 * 1024; i += 64) {
        array[i] = i & 0xFF;
    }
    
    sink = array[0];
    free((void*)array);
}

static void __attribute__((target("arch=sandybridge"))) 
sandybridge_cache_test(void) {
    volatile char *array = malloc(2 * 1024 * 1024);
    if (!array) return;
    
    /* Different stride for different cache architectures */
    for (int i = 0; i < 2 * 1024 * 1024; i += 32) {
        array[i] = i & 0xFF;
    }
    
    sink = array[0];
    free((void*)array);
}

static void __attribute__((target("arch=haswell"))) 
haswell_cache_test(void) {
    volatile char *array = malloc(4 * 1024 * 1024);
    if (!array) return;
    
    /* Mix of strides to trigger different cache logic */
    for (int i = 0; i < 4 * 1024 * 1024; i += 128) {
        array[i] = i & 0xFF;
    }
    
    sink = array[0];
    free((void*)array);
}

static void __attribute__((target("arch=skylake"))) 
skylake_cache_test(void) {
    volatile char *array = malloc(8 * 1024 * 1024);
    if (!array) return;
    
    /* Large array to potentially exceed L2 */
    for (int i = 0; i < 8 * 1024 * 1024; i += 256) {
        array[i] = i & 0xFF;
    }
    
    sink = array[0];
    free((void*)array);
}

static void __attribute__((target("arch=znver1"))) 
zen_cache_test(void) {
    volatile char *array = malloc(16 * 1024 * 1024);
    if (!array) return;
    
    /* Very large array with varying access pattern */
    for (int i = 0; i < 16 * 1024 * 1024; i += 512) {
        array[i] = i & 0xFF;
    }
    
    sink = array[0];
    free((void*)array);
}

/* Function to perform cache-sensitive timing */
static uint64_t cache_sensitive_test(size_t size, int stride) {
    volatile char *array = malloc(size);
    if (!array) return 0;
    
    uint64_t start, end;
    
    /* Initialize array */
    for (size_t i = 0; i < size; i += stride) {
        array[i] = (char)(i & 0xFF);
    }
    
    /* Time the access pattern */
    start = __builtin_ia32_rdtsc();
    
    for (size_t i = 0; i < size; i += stride) {
        sink = array[i];
    }
    
    end = __builtin_ia32_rdtsc();
    
    free((void*)array);
    return end - start;
}

/* Main test function */
int main(void) {
    uint64_t timing_results[8];
    int checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    zen_cache_test();
    
    /* Perform cache-sensitive timing tests with different parameters */
    timing_results[0] = cache_sensitive_test(8 * 1024, 32);      /* Likely fits in L1 */
    timing_results[1] = cache_sensitive_test(64 * 1024, 64);     /* Likely fits in L1/L2 */
    timing_results[2] = cache_sensitive_test(256 * 1024, 128);   /* Likely exceeds L1 */
    timing_results[3] = cache_sensitive_test(1024 * 1024, 256);  /* Likely exceeds L2 */
    timing_results[4] = cache_sensitive_test(4 * 1024 * 1024, 512); /* Large */
    timing_results[5] = cache_sensitive_test(8 * 1024 * 1024, 1024); /* Very large */
    
    /* Use timing ratios to make branching decisions */
    if (timing_results[1] > timing_results[0] * 2) {
        checksum |= 1;
    }
    if (timing_results[3] > timing_results[1] * 4) {
        checksum |= 2;
    }
    if (timing_results[5] > timing_results[2] * 8) {
        checksum |= 4;
    }
    
    /* Use CPU feature flags to create a final checksum */
    for (int i = 0; i < 64; i++) {
        if (cpu_features[i]) {
            checksum ^= (1 << (i % 16));
        }
    }
    
    /* Use CPU model flags */
    for (int i = 0; i < 32; i++) {
        if (cpu_models[i]) {
            checksum ^= (i * 37);
        }
    }
    
    /* Force output to prevent optimization */
    printf("CPU detection checksum: %d\n", checksum);
    printf("Timing ratios: %lu %lu %lu %lu %lu %lu\n",
           timing_results[0], timing_results[1], timing_results[2],
           timing_results[3], timing_results[4], timing_results[5]);
    
    /* Additional feature queries to ensure driver paths are taken */
    #pragma GCC target("avx2")
    {
        if (__builtin_cpu_supports("avx2")) {
            checksum += 1000;
        }
    }
    
    #pragma GCC target("avx512f")
    {
        if (__builtin_cpu_supports("avx512f")) {
            checksum += 2000;
        }
    }
    
    #pragma GCC target("sse4.2")
    {
        if (__builtin_cpu_supports("sse4.2")) {
            checksum += 4000;
        }
    }
    
    return checksum == 0 ? 0 : 1;
}

/* Additional test cases for specific cache descriptor values */
#ifdef __SELF_TEST__
/* This would be compiled with special flags to test specific cache configs */
static void test_specific_cache_codes(void) {
    /* Force consideration of specific cache descriptor bytes by 
       querying features associated with CPUs that have those cache configs */
    
    /* 0x0a: 8KB L1, 2-way, 32B line - Early Pentium III/Celeron */
    if (__builtin_cpu_is("pentium3")) {
        sink = 0x0a;
    }
    
    /* 0x2c: 32KB L1, 8-way, 64B line - Pentium 4 with HT */
    if (__builtin_cpu_is("pentium4")) {
        sink = 0x2c;
    }
    
    /* 0x78: 1MB L2, 4-way, 64B line - Various Intel CPUs */
    if (__builtin_cpu_is("core2") || __builtin_cpu_is("nehalem")) {
        sink = 0x78;
    }
    
    /* 0x49: 4MB L2, 16-way, 64B line - Xeon (non-MP) */
    if (__builtin_cpu_is("xeon")) {
        sink = 0x49;
    }
    
    /* 0x4e: 6MB L2, 24-way, 64B line - Large cache Xeons */
    if (__builtin_cpu_supports("avx512f")) {
        /* Some AVX-512 Xeons have large caches */
        sink = 0x4e;
    }
}
#endif
