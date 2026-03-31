/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[256];
static int cpu_models[256];
static volatile int timing_results[4];

/* Early CPU initialization - runs before main */
__attribute__((constructor(101)))
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
        "fma", "fma4", "xop", "aes", "pclmul", "sha", "gfni",
        "vaes", "vpclmulqdq", "movbe", "rdrnd", "rdseed", "adx",
        "clflushopt", "clwb", "xsavec", "xsaveopt", "xsaves",
        "rtm", "xtest", "mwaitx", "clzero", "pku", "ospke",
        "rdpid", "sgx", "cldemote", "movdiri", "movdir64b",
        "enqcmd", "serialize", "tsxldtrk", "amx-bf16", "amx-tile",
        "amx-int8", "avx-vnni", "avx512vp2intersect", "avxifma",
        "avxvnni", "avxvnniint8", "avxneconvert", "cmpccxadd",
        "amx-fp16", "prefetchi", "raoint", "amx-complex",
        "uintr", "hreset", "kl", "widekl", "avxvnniint16",
        "sha512", "sm3", "sm4", "tsx", NULL
    };
    
    const char *models[] = {
        "intel", "amd", "athlon", "core2", "nehalem",
        "westmere", "sandybridge", "ivybridge", "haswell",
        "broadwell", "skylake", "skylake-avx512", "cannonlake",
        "icelake-client", "icelake-server", "cascadelake",
        "cooperlake", "tigerlake", "sapphirerapids", "alderlake",
        "rocketlake", "graniterapids", "sierraforest", "grandridge",
        "atom", "bonnell", "silvermont", "goldmont", "goldmont-plus",
        "tremont", "gracemont", "generic", NULL
    };
    
    /* Store feature detection results */
    int i = 0;
    for (const char **f = features; *f; f++) {
        cpu_features[i++] = __builtin_cpu_supports(*f);
    }
    
    i = 0;
    for (const char **m = models; *m; m++) {
        cpu_models[i++] = __builtin_cpu_is(*m);
    }
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void cache_test_core2(void) {
    volatile int sum = 0;
    const int size = 1024 * 1024 * 16; /* 16MB - larger than typical L2 */
    static char *array = NULL;
    
    if (!array) {
        array = (char*)malloc(size);
        if (!array) return;
        memset(array, 1, size);
    }
    
    /* Access pattern sensitive to cache line size */
    for (int i = 0; i < size; i += 64) { /* 64-byte cache line */
        sum += array[i];
    }
    
    timing_results[0] = sum;
}

__attribute__((target("arch=sandybridge")))
static void cache_test_sandybridge(void) {
    volatile int sum = 0;
    const int size = 1024 * 1024 * 32; /* 32MB */
    static char *array = NULL;
    
    if (!array) {
        array = (char*)malloc(size);
        if (!array) return;
        memset(array, 2, size);
    }
    
    /* Different stride to potentially trigger different cache logic */
    for (int i = 0; i < size; i += 128) {
        sum += array[i];
    }
    
    timing_results[1] = sum;
}

__attribute__((target("arch=skylake")))
static void cache_test_skylake(void) {
    volatile int sum = 0;
    const int size = 1024 * 1024 * 64; /* 64MB */
    static char *array = NULL;
    
    if (!array) {
        array = (char*)malloc(size);
        if (!array) return;
        memset(array, 3, size);
    }
    
    /* Random access pattern to defeat prefetching */
    unsigned int seed = 42;
    for (int i = 0; i < 1000000; i++) {
        int idx = rand_r(&seed) % size;
        sum += array[idx];
    }
    
    timing_results[2] = sum;
}

__attribute__((target("arch=znver1")))
static void cache_test_zen(void) {
    volatile int sum = 0;
    const int size = 1024 * 1024 * 8; /* 8MB */
    static char *array = NULL;
    
    if (!array) {
        array = (char*)malloc(size);
        if (!array) return;
        memset(array, 4, size);
    }
    
    /* Sequential access with 32-byte stride */
    for (int i = 0; i < size; i += 32) {
        sum += array[i];
    }
    
    timing_results[3] = sum;
}

/* Use pragmas to trigger different optimization paths */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    /* Vectorized memory access */
    typedef int v8si __attribute__((vector_size(32)));
    static v8si *vec_array = NULL;
    const int vec_size = 1024 * 1024;
    
    if (!vec_array) {
        vec_array = (v8si*)aligned_alloc(32, vec_size * sizeof(v8si));
        if (!vec_array) return;
        memset(vec_array, 0, vec_size * sizeof(v8si));
    }
    
    v8si sum = {0};
    for (int i = 0; i < vec_size; i++) {
        sum += vec_array[i];
    }
    
    /* Prevent optimization */
    asm volatile("" : "+x"(sum));
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    /* Even wider vector accesses */
    typedef int v16si __attribute__((vector_size(64)));
    static v16si *vec_array = NULL;
    const int vec_size = 512 * 1024;
    
    if (!vec_array) {
        vec_array = (v16si*)aligned_alloc(64, vec_size * sizeof(v16si));
        if (!vec_array) return;
        memset(vec_array, 0, vec_size * sizeof(v16si));
    }
    
    v16si sum = {0};
    for (int i = 0; i < vec_size; i++) {
        sum += vec_array[i];
    }
    
    asm volatile("" : "+x"(sum));
}
#pragma GCC pop_options

/* Timing function using RDTSC */
static uint64_t rdtsc(void) {
    unsigned int lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

/* Main test that performs cache-sensitive operations */
int main(void) {
    uint64_t start, end;
    int checksum = 0;
    
    /* Call all cache test functions */
    start = rdtsc();
    cache_test_core2();
    end = rdtsc();
    checksum += (int)(end - start);
    
    start = rdtsc();
    cache_test_sandybridge();
    end = rdtsc();
    checksum += (int)(end - start);
    
    start = rdtsc();
    cache_test_skylake();
    end = rdtsc();
    checksum += (int)(end - start);
    
    start = rdtsc();
    cache_test_zen();
    end = rdtsc();
    checksum += (int)(end - start);
    
    /* Call vectorized tests */
    avx2_cache_test();
    avx512_cache_test();
    
    /* Use CPU feature detection results to create a unique checksum */
    for (int i = 0; i < 256 && cpu_features[i] != 0; i++) {
        checksum ^= cpu_features[i] << (i % 32);
    }
    
    for (int i = 0; i < 256 && cpu_models[i] != 0; i++) {
        checksum ^= cpu_models[i] << ((i + 16) % 32);
    }
    
    /* Add timing results */
    for (int i = 0; i < 4; i++) {
        checksum += timing_results[i];
    }
    
    /* Output checksum to prevent dead code elimination */
    printf("CPU detection checksum: %d\n", checksum);
    
    /* Force compiler to consider all code paths */
    if (checksum == 0x12345678) { /* Unlikely value */
        printf("Impossible branch taken!\n");
    }
    
    return 0;
}
