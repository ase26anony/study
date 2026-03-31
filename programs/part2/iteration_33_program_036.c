/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[64];
static int cpu_models[16];
static volatile int timing_results[8];

/* Early CPU initialization - runs before main */
__attribute__((constructor(101)))
static void init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger cache detection */
    cpu_features[0] = __builtin_cpu_supports("mmx");
    cpu_features[1] = __builtin_cpu_supports("sse");
    cpu_features[2] = __builtin_cpu_supports("sse2");
    cpu_features[3] = __builtin_cpu_supports("sse3");
    cpu_features[4] = __builtin_cpu_supports("ssse3");
    cpu_features[5] = __builtin_cpu_supports("sse4.1");
    cpu_features[6] = __builtin_cpu_supports("sse4.2");
    cpu_features[7] = __builtin_cpu_supports("avx");
    cpu_features[8] = __builtin_cpu_supports("avx2");
    cpu_features[9] = __builtin_cpu_supports("avx512f");
    cpu_features[10] = __builtin_cpu_supports("fma");
    cpu_features[11] = __builtin_cpu_supports("aes");
    cpu_features[12] = __builtin_cpu_supports("pclmul");
    cpu_features[13] = __builtin_cpu_supports("rdrand");
    cpu_features[14] = __builtin_cpu_supports("rdseed");
    cpu_features[15] = __builtin_cpu_supports("sha");
    cpu_features[16] = __builtin_cpu_supports("xsave");
    cpu_features[17] = __builtin_cpu_supports("xsaveopt");
    cpu_features[18] = __builtin_cpu_supports("xsavec");
    cpu_features[19] = __builtin_cpu_supports("xsaves");
    
    /* Query CPU models - each may have different cache configurations */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("ivybridge");
    cpu_models[6] = __builtin_cpu_is("haswell");
    cpu_models[7] = __builtin_cpu_is("broadwell");
    cpu_models[8] = __builtin_cpu_is("skylake");
    cpu_models[9] = __builtin_cpu_is("cannonlake");
    cpu_models[10] = __builtin_cpu_is("icelake");
    cpu_models[11] = __builtin_cpu_is("tigerlake");
    cpu_models[12] = __builtin_cpu_is("alderlake");
    cpu_models[13] = __builtin_cpu_is("znver1");
    cpu_models[14] = __builtin_cpu_is("znver2");
    cpu_models[15] = __builtin_cpu_is("znver3");
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void test_core2_cache(void) {
    volatile char buffer[32768];  /* 32KB - typical L1 size */
    for (int i = 0; i < sizeof(buffer); i += 64) {  /* 64-byte line */
        buffer[i] = i & 0xFF;
    }
    timing_results[0] = buffer[0];
}

__attribute__((target("arch=sandybridge")))
static void test_sandybridge_cache(void) {
    volatile char buffer[262144];  /* 256KB - typical L2 size */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = i & 0xFF;
    }
    timing_results[1] = buffer[64];
}

__attribute__((target("arch=haswell")))
static void test_haswell_cache(void) {
    volatile char buffer[1048576];  /* 1MB - typical L3 slice */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = i & 0xFF;
    }
    timing_results[2] = buffer[128];
}

__attribute__((target("arch=skylake")))
static void test_skylake_cache(void) {
    volatile char buffer[2097152];  /* 2MB */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        buffer[i] = i & 0xFF;
    }
    timing_results[3] = buffer[256];
}

/* AVX2-optimized memory test */
#pragma GCC push_options
#pragma GCC target("avx2")
static void test_avx2_cache_sensitive(void) {
    /* Larger than typical L2 cache to force evictions */
    #define LARGE_SIZE (4 * 1024 * 1024)
    static volatile char large_buffer[LARGE_SIZE];
    
    /* Access with different strides to be cache-sensitive */
    for (int stride = 32; stride <= 256; stride *= 2) {
        for (int i = 0; i < LARGE_SIZE; i += stride) {
            large_buffer[i] = (i / stride) & 0xFF;
        }
    }
    timing_results[4] = large_buffer[0];
}
#pragma GCC pop_options

/* SSE4.2-optimized test */
#pragma GCC push_options
#pragma GCC target("sse4.2")
static void test_sse42_cache_sensitive(void) {
    volatile int buffer[65536];  /* 256KB when int is 4 bytes */
    
    /* Random-ish access pattern to defeat prefetching */
    for (int i = 0; i < 65536; i++) {
        int idx = (i * 97) & 65535;  /* Pseudo-random pattern */
        buffer[idx] = i;
    }
    timing_results[5] = buffer[0];
}
#pragma GCC pop_options

/* Cache line size detection test */
static void detect_cache_line(void) {
    volatile char probe[8192];
    uint64_t tsc1, tsc2;
    
    /* Time accesses with different offsets */
    for (int offset = 0; offset < 256; offset += 8) {
        /* Use inline assembly for RDTSC to avoid library calls */
        asm volatile ("rdtsc" : "=a" (tsc1) : : "rdx");
        volatile char x = probe[offset];
        asm volatile ("rdtsc" : "=a" (tsc2) : : "rdx");
        (void)x;  /* Use result to prevent optimization */
        
        if ((tsc2 - tsc1) > 100) {  /* Arbitrary threshold */
            timing_results[6] = offset;
            break;
        }
    }
}

/* Main test function */
int main(void) {
    int checksum = 0;
    
    /* Call all target-specific functions */
    test_core2_cache();
    test_sandybridge_cache();
    test_haswell_cache();
    test_skylake_cache();
    test_avx2_cache_sensitive();
    test_sse42_cache_sensitive();
    detect_cache_line();
    
    /* Use CPU feature flags to create checksum */
    for (int i = 0; i < 20; i++) {
        checksum = (checksum << 1) | (cpu_features[i] & 1);
    }
    
    for (int i = 0; i < 16; i++) {
        checksum = (checksum << 1) | (cpu_models[i] & 1);
    }
    
    for (int i = 0; i < 7; i++) {
        checksum ^= timing_results[i];
    }
    
    /* Print result to prevent dead code elimination */
    printf("CPU detection checksum: %d\n", checksum);
    
    /* Additional CPUID-like queries to ensure driver runs */
    if (__builtin_cpu_supports("sse")) {
        printf("SSE supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    if (__builtin_cpu_is("intel")) {
        printf("Intel CPU\n");
    } else if (__builtin_cpu_is("amd")) {
        printf("AMD CPU\n");
    }
    
    return 0;
}

/* Force inclusion of various x86-specific builtins */
__attribute__((used))
static void force_builtins(void) {
    /* These force the compiler to consider various CPU features */
    asm volatile ("# Force CPUID considerations" : : : "memory");
    
    /* Additional feature checks that might trigger cache detection */
    volatile int has_movbe = __builtin_cpu_supports("movbe");
    volatile int has_popcnt = __builtin_cpu_supports("popcnt");
    volatile int has_lzcnt = __builtin_cpu_supports("lzcnt");
    volatile int has_bmi = __builtin_cpu_supports("bmi");
    volatile int has_bmi2 = __builtin_cpu_supports("bmi2");
    volatile int has_rtm = __builtin_cpu_supports("rtm");
    volatile int has_mpx = __builtin_cpu_supports("mpx");
    volatile int has_sgx = __builtin_cpu_supports("sgx");
    
    (void)has_movbe;
    (void)has_popcnt;
    (void)has_lzcnt;
    (void)has_bmi;
    (void)has_bmi2;
    (void)has_rtm;
    (void)has_mpx;
    (void)has_sgx;
}
