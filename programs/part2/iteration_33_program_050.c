/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise GCC x86 driver cache detection logic */
/* This forces execution of CPUID interpretation for cache descriptors */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU features - prevent optimization */
volatile int cpu_features[64];
volatile int cpu_models[16];
volatile int timing_results[8];

/* Cache-sensitive memory access patterns */
#define ARRAY_SIZE (4 * 1024 * 1024) /* 4MB > typical L2 cache */
static char data_array[ARRAY_SIZE] __attribute__((aligned(64)));

/* Early CPU initialization - runs before main */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a comprehensive set of CPU features to trigger 
       various CPUID leaves including cache descriptors */
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
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1");  /* AMD Zen */
    cpu_models[8] = __builtin_cpu_is("znver2");  /* AMD Zen 2 */
    cpu_models[9] = __builtin_cpu_is("znver3");  /* AMD Zen 3 */
}

/* Target-specific functions to force driver to consider different microarchitectures */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    /* Access pattern sensitive to 32-byte cache lines */
    volatile char *ptr = data_array;
    for (int i = 0; i < ARRAY_SIZE; i += 32) {
        ptr[i] = (char)(i & 0xFF);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    /* Access pattern sensitive to 64-byte cache lines */
    volatile char *ptr = data_array;
    for (int i = 0; i < ARRAY_SIZE; i += 64) {
        ptr[i] = (char)(i & 0xFF);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    /* Different stride to potentially trigger different cache logic */
    volatile char *ptr = data_array;
    for (int i = 0; i < ARRAY_SIZE; i += 128) {
        ptr[i] = (char)(i & 0xFF);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    /* Random access pattern to defeat prefetching */
    volatile char *ptr = data_array;
    for (int i = 0; i < 10000; i++) {
        int idx = (i * 97) % ARRAY_SIZE;  /* Pseudo-random */
        ptr[idx] = (char)(i & 0xFF);
    }
}

__attribute__((target("arch=znver1")))
static void zen_cache_test(void) {
    /* AMD Zen specific pattern */
    volatile char *ptr = data_array;
    for (int i = 0; i < ARRAY_SIZE; i += 256) {
        ptr[i] = (char)(i & 0xFF);
    }
}

/* Timing function using RDTSC */
static uint64_t read_tsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

/* Cache size estimation through timing */
static void measure_cache_effects(void) {
    uint64_t start, end;
    
    /* Warm up cache */
    for (int i = 0; i < ARRAY_SIZE; i += 64) {
        data_array[i] = 1;
    }
    
    /* Measure linear access (should be cache friendly) */
    start = read_tsc();
    volatile char sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i += 64) {
        sum += data_array[i];
    }
    end = read_tsc();
    timing_results[0] = (int)(end - start);
    
    /* Measure random access (cache unfriendly) */
    start = read_tsc();
    sum = 0;
    for (int i = 0; i < 100000; i++) {
        int idx = (i * 97) % ARRAY_SIZE;
        sum += data_array[idx];
    }
    end = read_tsc();
    timing_results[1] = (int)(end - start);
}

/* Force compiler to consider various optimization levels */
#pragma GCC push_options
#pragma GCC optimize ("O0")
__attribute__((noinline))
static void force_cache_query(void) {
    /* This function's compilation may trigger cache queries */
    asm volatile ("# Cache query trigger point" : : : "memory");
}
#pragma GCC pop_options

int main(void) {
    int checksum = 0;
    
    /* Execute all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    zen_cache_test();
    
    /* Force cache detection through timing measurements */
    measure_cache_effects();
    
    /* Use feature detection results to create checksum */
    for (int i = 0; i < 19; i++) {
        checksum = (checksum * 31 + cpu_features[i]) & 0xFFFF;
    }
    
    for (int i = 0; i < 10; i++) {
        checksum = (checksum * 17 + cpu_models[i]) & 0xFFFF;
    }
    
    for (int i = 0; i < 2; i++) {
        checksum = (checksum * 13 + timing_results[i]) & 0xFFFF;
    }
    
    /* Output checksum to prevent dead code elimination */
    printf("CPU detection checksum: 0x%04x\n", checksum);
    
    /* Additional feature queries in main to ensure they're not optimized away */
    if (__builtin_cpu_supports("sse4.2")) {
        checksum ^= 0x1234;
    }
    
    if (__builtin_cpu_is("intel")) {
        checksum ^= 0x5678;
    }
    
    /* Force one more round of cache-sensitive computation */
    volatile char final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i += 32) {
        final_sum += data_array[i];
    }
    
    printf("Final checksum: 0x%04x\n", checksum ^ (final_sum & 0xFF));
    
    return 0;
}

/* Additional test cases using pragmas for different targets */
#pragma GCC target("arch=goldmont")
static void goldmont_test(void) {
    /* Intel Atom architecture */
    asm volatile ("# Goldmont test" : : : "memory");
}

#pragma GCC target("arch=bonnell")
static void bonnell_test(void) {
    /* Older Atom architecture */
    asm volatile ("# Bonnell test" : : : "memory");
}

/* Force inclusion of these functions */
__attribute__((used))
static void ensure_functions_used(void) {
    goldmont_test();
    bonnell_test();
    force_cache_query();
}
