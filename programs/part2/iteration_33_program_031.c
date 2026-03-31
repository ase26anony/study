/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */
/* Test program to trigger GCC x86 driver cache detection logic */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Global variables to store CPU features - prevents optimization */
volatile int cpu_features[256];
volatile int cpu_models[32];
volatile int timing_results[8];

/* Early CPU initialization - runs before main */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger cache detection */
    const char* features[] = {
        "cmov", "mmx", "popcnt", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2", "avx512f", "avx512cd",
        "avx512bw", "avx512dq", "avx512vl", "aes", "pclmul",
        "rdrand", "rdseed", "fma", "fma4", "xop", "bmi", "bmi2",
        "adx", "sha", "prefetchwt1", "clflushopt", "xsave",
        "xsaveopt", "xsavec", "xgetbv", "rtm", "hle", "tsx"
    };
    
    for (int i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        cpu_features[i] = __builtin_cpu_supports(features[i]);
    }
    
    /* Query CPU models to trigger different cache configurations */
    const char* models[] = {
        "intel", "amd", "core2", "nehalem", "westmere",
        "sandybridge", "ivybridge", "haswell", "broadwell",
        "skylake", "skylake-avx512", "cannonlake", "icelake",
        "tigerlake", "rocketlake", "alderlake", "atom",
        "silvermont", "goldmont", "tremont", "knl", "knm",
        "znver1", "znver2", "znver3"
    };
    
    for (int i = 0; i < sizeof(models)/sizeof(models[0]); i++) {
        cpu_models[i] = __builtin_cpu_is(models[i]);
    }
}

/* Target-specific functions to force driver to consider different architectures */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile int sum = 0;
    char buffer[32768]; /* 32KB - typical L1 size */
    
    /* Access pattern that depends on cache line size */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
    
    timing_results[0] = sum;
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile int sum = 0;
    char buffer[262144]; /* 256KB - typical L2 size */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
    
    timing_results[1] = sum;
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile int sum = 0;
    char buffer[8388608]; /* 8MB - typical L3 size */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
    
    timing_results[2] = sum;
}

__attribute__((target("arch=znver2")))
static void zen2_cache_test(void) {
    volatile int sum = 0;
    char buffer[4194304]; /* 4MB - Zen2 L3 per CCX */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
    
    timing_results[3] = sum;
}

/* Function with pragmas to trigger different optimization paths */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_sensitive(void) {
    /* Use AVX2 instructions that might trigger cache size queries */
    volatile __m256i vec = _mm256_setzero_si256();
    volatile __m256i result;
    
    char buffer[65536];
    for (int i = 0; i < sizeof(buffer); i += 64) {
        vec = _mm256_loadu_si256((__m256i*)&buffer[i]);
        result = _mm256_add_epi32(vec, vec);
    }
    
    timing_results[4] = _mm256_extract_epi32(result, 0);
}
#pragma GCC pop_options

/* Cache timing test - sensitive to actual cache sizes */
static void perform_cache_timing(void) {
    const size_t large_size = 16 * 1024 * 1024; /* 16MB */
    char* large_buffer = malloc(large_size);
    volatile uint64_t start, end;
    
    if (!large_buffer) return;
    
    /* Fill with non-zero values */
    memset(large_buffer, 1, large_size);
    
    /* Time sequential access (cache friendly) */
    volatile int sum = 0;
    start = __rdtsc();
    for (size_t i = 0; i < large_size; i += 64) {
        sum += large_buffer[i];
    }
    end = __rdtsc();
    timing_results[5] = (int)(end - start);
    
    /* Time random access (cache unfriendly) */
    sum = 0;
    start = __rdtsc();
    for (size_t i = 0; i < large_size; i += 997) { /* Prime stride */
        sum += large_buffer[i % large_size];
    }
    end = __rdtsc();
    timing_results[6] = (int)(end - start);
    
    free(large_buffer);
}

int main(void) {
    int checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    zen2_cache_test();
    avx2_cache_sensitive();
    
    /* Perform actual cache timing */
    perform_cache_timing();
    
    /* Use CPU feature detection results to create checksum */
    for (int i = 0; i < 32; i++) {
        checksum ^= cpu_features[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum ^= cpu_models[i];
    }
    
    for (int i = 0; i < 7; i++) {
        checksum ^= timing_results[i];
    }
    
    /* Prevent dead code elimination */
    volatile int output = checksum;
    
    printf("CPU detection test completed. Checksum: %d\n", output);
    
    /* Additional feature queries in main to ensure driver usage */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX-512 supported\n");
    }
    
    if (__builtin_cpu_is("skylake")) {
        printf("Skylake microarchitecture detected\n");
    }
    
    /* Query cache-related features */
    if (__builtin_cpu_supports("clflushopt")) {
        printf("Cache line flush optimization available\n");
    }
    
    if (__builtin_cpu_supports("prefetchwt1")) {
        printf("PREFETCHWT1 instruction available\n");
    }
    
    return output & 1;
}

/* Additional compilation unit to force driver initialization */
__attribute__((used))
static void force_driver_init(void) {
    /* This forces the driver to process CPUID information */
    asm volatile("" : : : "memory");
    
    /* Check for specific cache sizes through builtins */
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    
    (void)has_sse;
    (void)has_avx;
    (void)is_intel;
    (void)is_amd;
}
