/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */
/* Test program to trigger GCC x86 driver cache detection logic */
/* This targets the switch cases for cache descriptor bytes in driver-i386.cc */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[256];
static int cpu_models[256];
static volatile int sink; /* Prevent optimization */

/* Early CPU initialization - runs before main */
static void __attribute__((constructor)) init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger cache detection */
    const char *features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512dq",
        "avx512cd", "avx512bw", "avx512vl", "avx512vbmi", "avx512vbmi2",
        "avx512vnni", "avx512bitalg", "avx512vpopcntdq", "avx5124vnniw",
        "avx5124fmaps", "avx512vp2intersect", "avx512bf16", "avx512fp16",
        "fma", "fma4", "xop", "aes", "pclmul", "sha", "gfni",
        "vaes", "vpclmulqdq", "movbe", "rdrnd", "rdseed", "adx",
        "bmi", "bmi2", "lzcnt", "fxsr", "xsave", "xsaveopt",
        "xsavec", "xsaves", "mwaitx", "clzero", "pku", "ospke",
        "waitpkg", "cldemote", "movdiri", "movdir64b", "enqcmd",
        "serialize", "tsxldtrk", "amx-bf16", "amx-tile", "amx-int8",
        "uintr", "hreset", "kl", "widekl", "avxifma", "avxvnni",
        "avxvnniint8", "avx-ne-convert", "cmpccxadd", "amx-fp16",
        "prefetchi", "raoint", "amx-complex", NULL
    };
    
    const char *models[] = {
        "intel", "amd", "atom", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake-client",
        "icelake-server", "cascadelake", "cooperlake", "tigerlake",
        "sapphirerapids", "alderlake", "rocketlake", "graniterapids",
        "graniterapids-d", "sierraforest", "grandridge", "clearwaterforest",
        "athlon", "athlon-4", "k8", "k8-sse3", "opteron", "opteron-sse3",
        "barcelona", "shanghai", "istanbul", "amdfam10", "bdver1", "bdver2",
        "bdver3", "bdver4", "znver1", "znver2", "znver3", "znver4",
        "znver5", "bonnell", "silvermont", "goldmont", "goldmont-plus",
        "tremont", "gracemont", "sierraforest", "grandridge", NULL
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
    volatile int sum = 0;
    const int size = 1024 * 1024; /* 1MB */
    static int array[1024 * 1024];
    
    /* Access pattern sensitive to cache line size */
    for (int i = 0; i < size; i += 64) { /* 64-byte stride */
        sum += array[i];
    }
    sink = sum;
}

static void __attribute__((target("arch=sandybridge"))) 
sandybridge_cache_test(void) {
    volatile int sum = 0;
    const int size = 2 * 1024 * 1024; /* 2MB */
    static int array[2 * 1024 * 1024];
    
    /* Different stride for different cache line assumption */
    for (int i = 0; i < size; i += 32) { /* 32-byte stride */
        sum += array[i];
    }
    sink = sum;
}

static void __attribute__((target("arch=haswell"))) 
haswell_cache_test(void) {
    volatile int sum = 0;
    const int size = 4 * 1024 * 1024; /* 4MB */
    static int array[4 * 1024 * 1024];
    
    /* Mix of strides */
    for (int i = 0; i < size; i += 128) {
        sum += array[i];
    }
    sink = sum;
}

static void __attribute__((target("arch=znver2"))) 
zen2_cache_test(void) {
    volatile int sum = 0;
    const int size = 8 * 1024 * 1024; /* 8MB */
    static int array[8 * 1024 * 1024];
    
    /* Large stride for L3 cache */
    for (int i = 0; i < size; i += 256) {
        sum += array[i];
    }
    sink = sum;
}

/* AVX-optimized function to trigger AVX-specific paths */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_sensitive(void) {
    volatile long long sum = 0;
    const int size = 16 * 1024 * 1024; /* 16MB */
    static long long array[16 * 1024 * 1024 / 8];
    
    /* Time the access using RDTSC */
    unsigned long long start, end;
    start = __builtin_ia32_rdtsc();
    
    for (int i = 0; i < size / 8; i += 8) { /* 64-byte cache lines */
        sum += array[i];
    }
    
    end = __builtin_ia32_rdtsc();
    
    /* Branch based on timing - forces compiler to consider cache */
    if ((end - start) < 1000000) {
        sink = 1; /* Likely hitting cache */
    } else {
        sink = 2; /* Likely cache miss */
    }
}
#pragma GCC pop_options

/* AVX-512 optimized function */
#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_sensitive(void) {
    volatile long long sum = 0;
    const int size = 32 * 1024 * 1024; /* 32MB */
    static long long array[32 * 1024 * 1024 / 8];
    
    unsigned long long start, end;
    start = __builtin_ia32_rdtsc();
    
    /* 512-bit accesses - 64 bytes per access */
    for (int i = 0; i < size / 8; i += 8) {
        sum += array[i];
    }
    
    end = __builtin_ia32_rdtsc();
    
    /* More timing-based branches */
    if ((end - start) < 2000000) {
        sink = 3;
    } else if ((end - start) < 5000000) {
        sink = 4;
    } else {
        sink = 5;
    }
}
#pragma GCC pop_options

/* Function that uses all target-specific variants */
static void run_all_cache_tests(void) {
    /* Call each target-specific function */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    zen2_cache_test();
    avx2_cache_sensitive();
    avx512_cache_sensitive();
}

/* Main function with extensive feature usage */
int main(void) {
    int result = 0;
    
    /* Use feature detection results to control flow */
    if (__builtin_cpu_supports("sse4.2")) {
        result |= 1;
    }
    if (__builtin_cpu_supports("avx2")) {
        result |= 2;
    }
    if (__builtin_cpu_supports("avx512f")) {
        result |= 4;
    }
    
    /* Check CPU model */
    if (__builtin_cpu_is("intel")) {
        result |= 8;
    }
    if (__builtin_cpu_is("amd")) {
        result |= 16;
    }
    if (__builtin_cpu_is("core2")) {
        result |= 32;
    }
    if (__builtin_cpu_is("skylake")) {
        result |= 64;
    }
    if (__builtin_cpu_is("znver3")) {
        result |= 128;
    }
    
    /* Run cache-sensitive tests */
    run_all_cache_tests();
    
    /* Create a checksum from feature flags to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        if (cpu_features[i]) checksum ^= i;
        if (cpu_models[i]) checksum ^= (i << 8);
    }
    
    checksum ^= result;
    checksum ^= sink;
    
    /* Output result to prevent optimization */
    printf("CPU detection checksum: %d\n", checksum);
    printf("Result flags: %d\n", result);
    
    return checksum == 0 ? 0 : 1;
}

/* Additional constructor to force early initialization */
static void __attribute__((constructor)) early_init(void) {
    /* Force some CPUID queries even earlier */
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int is_intel = __builtin_cpu_is("intel");
    (void)has_sse;
    (void)has_sse2;
    (void)is_intel;
}
