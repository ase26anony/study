/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise GCC x86 driver cache detection logic */
/* This forces execution of CPUID interpretation for various cache descriptors */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[64];
static int cpu_models[16];
static volatile int sink; /* Prevent optimization */

/* Early CPU initialization - mimics driver startup */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* Force driver to initialize CPUID data */
    __builtin_cpu_init();
    
    /* Query a wide range of CPU features to trigger cache detection */
    cpu_features[0] = __builtin_cpu_supports("cmov");
    cpu_features[1] = __builtin_cpu_supports("mmx");
    cpu_features[2] = __builtin_cpu_supports("sse");
    cpu_features[3] = __builtin_cpu_supports("sse2");
    cpu_features[4] = __builtin_cpu_supports("sse3");
    cpu_features[5] = __builtin_cpu_supports("ssse3");
    cpu_features[6] = __builtin_cpu_supports("sse4.1");
    cpu_features[7] = __builtin_cpu_supports("sse4.2");
    cpu_features[8] = __builtin_cpu_supports("avx");
    cpu_features[9] = __builtin_cpu_supports("avx2");
    cpu_features[10] = __builtin_cpu_supports("avx512f");
    cpu_features[11] = __builtin_cpu_supports("fma");
    cpu_features[12] = __builtin_cpu_supports("aes");
    cpu_features[13] = __builtin_cpu_supports("pclmul");
    cpu_features[14] = __builtin_cpu_supports("rdrand");
    cpu_features[15] = __builtin_cpu_supports("rdseed");
    cpu_features[16] = __builtin_cpu_supports("sha");
    cpu_features[17] = __builtin_cpu_supports("xsave");
    cpu_features[18] = __builtin_cpu_supports("xsaveopt");
    
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
    volatile char *buffer = malloc(1024 * 1024); /* 1MB */
    if (buffer) {
        /* Access with 64-byte stride (typical cache line) */
        for (int i = 0; i < 1024 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=sandybridge")))
static void test_sandybridge_cache(void) {
    volatile char *buffer = malloc(2 * 1024 * 1024); /* 2MB */
    if (buffer) {
        /* Access with 32-byte and 64-byte strides */
        for (int i = 0; i < 2 * 1024 * 1024; i += 32) {
            sink = buffer[i];
        }
        for (int i = 0; i < 2 * 1024 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=haswell")))
static void test_haswell_cache(void) {
    volatile char *buffer = malloc(8 * 1024 * 1024); /* 8MB */
    if (buffer) {
        /* Random access pattern to stress cache */
        for (int i = 0; i < 100000; i++) {
            int idx = (i * 97) % (8 * 1024 * 1024);
            sink = buffer[idx];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=znver2")))
static void test_zen2_cache(void) {
    volatile char *buffer = malloc(16 * 1024 * 1024); /* 16MB */
    if (buffer) {
        /* Large stride to miss caches */
        for (int i = 0; i < 16 * 1024 * 1024; i += 4096) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

/* Function with pragma to trigger AVX-specific paths */
#pragma GCC push_options
#pragma GCC target("avx2")
static void test_avx2_cache_sensitive(void) {
    /* Use AVX2 instructions that might query cache parameters */
    volatile int array[1024];
    for (int i = 0; i < 1024; i++) {
        array[i] = i;
    }
    
    /* Sum with different strides to be cache-sensitive */
    int sum1 = 0, sum2 = 0;
    for (int i = 0; i < 1024; i += 1) {
        sum1 += array[i];
    }
    for (int i = 0; i < 1024; i += 16) {
        sum2 += array[i];
    }
    sink = sum1 + sum2;
}
#pragma GCC pop_options

/* Timing function using RDTSC */
static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

/* Cache size estimation through timing */
static void estimate_cache_sizes(void) {
    const size_t max_size = 32 * 1024 * 1024; /* 32MB */
    volatile char *buffer = malloc(max_size);
    uint64_t times[6] = {0};
    
    if (!buffer) return;
    
    /* Warm up */
    for (size_t i = 0; i < max_size; i += 64) {
        sink = buffer[i];
    }
    
    /* Time different access patterns */
    uint64_t start, end;
    
    /* Linear access, should hit L1/L2 */
    start = rdtsc();
    for (size_t i = 0; i < 64 * 1024; i += 64) {
        sink = buffer[i];
    }
    end = rdtsc();
    times[0] = end - start;
    
    /* Larger working set, may exceed L1 */
    start = rdtsc();
    for (size_t i = 0; i < 512 * 1024; i += 64) {
        sink = buffer[i];
    }
    end = rdtsc();
    times[1] = end - start;
    
    /* Even larger, may exceed L2 */
    start = rdtsc();
    for (size_t i = 0; i < 8 * 1024 * 1024; i += 64) {
        sink = buffer[i];
    }
    end = rdtsc();
    times[2] = end - start;
    
    /* Store timing results to prevent optimization */
    for (int i = 0; i < 3; i++) {
        cpu_features[30 + i] = (int)(times[i] & 0xFFFFFFFF);
    }
    
    free((void*)buffer);
}

int main(void) {
    /* Execute all target-specific functions */
    test_core2_cache();
    test_sandybridge_cache();
    test_haswell_cache();
    test_zen2_cache();
    test_avx2_cache_sensitive();
    
    /* Force cache timing measurements */
    estimate_cache_sizes();
    
    /* Compute checksum from all detected features to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum ^= cpu_features[i] * (i + 1);
    }
    for (int i = 0; i < 16; i++) {
        checksum ^= cpu_models[i] * (i + 100);
    }
    
    /* Output minimal result to satisfy compiler */
    printf("CPU checksum: %d\n", checksum);
    
    return checksum & 1;
}

/* Additional test cases for specific cache descriptor values */
/* These comments help guide the driver toward specific cache configurations */

/* Case 0x0a: L1 8KB, 2-way, 32B line */
/* Expected on: Some early Intel CPUs */
__attribute__((target("arch=pentium3")))
static void test_case_0x0a(void) {
    volatile int array[2048]; /* 8KB */
    for (int i = 0; i < 2048; i += 8) { /* 32B stride */
        sink = array[i];
    }
}

/* Case 0x2c: L1 32KB, 8-way, 64B line */
/* Expected on: Core 2, Nehalem */
__attribute__((target("arch=core2")))
static void test_case_0x2c(void) {
    volatile int array[8192]; /* 32KB */
    for (int i = 0; i < 8192; i += 16) { /* 64B stride */
        sink = array[i];
    }
}

/* Case 0x78: L2 1024KB, 4-way, 64B line */
/* Expected on: Various Intel CPUs */
__attribute__((target("arch=nehalem")))
static void test_case_0x78(void) {
    volatile char *buffer = malloc(1024 * 1024);
    if (buffer) {
        for (int i = 0; i < 1024 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}
