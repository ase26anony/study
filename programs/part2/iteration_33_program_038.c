/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise x86 CPU cache detection logic in GCC driver */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[50];
static int cpu_models[20];
static volatile int sink; /* Prevent optimization */

/* Early CPU initialization - forces driver to run CPUID and cache detection */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* This forces GCC's driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a comprehensive set of CPU features to trigger various cache detection paths */
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
    cpu_features[11] = __builtin_cpu_supports("avx512vl");
    cpu_features[12] = __builtin_cpu_supports("avx512bw");
    cpu_features[13] = __builtin_cpu_supports("avx512dq");
    cpu_features[14] = __builtin_cpu_supports("fma");
    cpu_features[15] = __builtin_cpu_supports("aes");
    cpu_features[16] = __builtin_cpu_supports("pclmul");
    cpu_features[17] = __builtin_cpu_supports("rdrand");
    cpu_features[18] = __builtin_cpu_supports("rdseed");
    cpu_features[19] = __builtin_cpu_supports("sha");
    cpu_features[20] = __builtin_cpu_supports("xsave");
    cpu_features[21] = __builtin_cpu_supports("xsavec");
    cpu_features[22] = __builtin_cpu_supports("xsaves");
    
    /* Query CPU models - each may have different cache configurations */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("atom");
    cpu_models[3] = __builtin_cpu_is("core2");
    cpu_models[4] = __builtin_cpu_is("nehalem");
    cpu_models[5] = __builtin_cpu_is("sandybridge");
    cpu_models[6] = __builtin_cpu_is("ivybridge");
    cpu_models[7] = __builtin_cpu_is("haswell");
    cpu_models[8] = __builtin_cpu_is("broadwell");
    cpu_models[9] = __builtin_cpu_is("skylake");
    cpu_models[10] = __builtin_cpu_is("cannonlake");
    cpu_models[11] = __builtin_cpu_is("icelake");
    cpu_models[12] = __builtin_cpu_is("tigerlake");
    cpu_models[13] = __builtin_cpu_is("alderlake");
    cpu_models[14] = __builtin_cpu_is("znver1");
    cpu_models[15] = __builtin_cpu_is("znver2");
    cpu_models[16] = __builtin_cpu_is("znver3");
    cpu_models[17] = __builtin_cpu_is("znver4");
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void test_core2_cache(void) {
    volatile char array[32768]; /* 32KB - typical L1 cache size */
    for (int i = 0; i < sizeof(array); i += 64) { /* 64-byte line */
        array[i] = i & 0xFF;
        sink = array[i];
    }
}

__attribute__((target("arch=sandybridge")))
static void test_sandybridge_cache(void) {
    volatile char array[262144]; /* 256KB - typical L2 cache size */
    for (int i = 0; i < sizeof(array); i += 64) {
        array[i] = i & 0xFF;
        sink = array[i];
    }
}

__attribute__((target("arch=skylake")))
static void test_skylake_cache(void) {
    volatile char array[1048576]; /* 1MB - typical L3 cache size */
    for (int i = 0; i < sizeof(array); i += 64) {
        array[i] = i & 0xFF;
        sink = array[i];
    }
}

__attribute__((target("arch=znver2")))
static void test_zen2_cache(void) {
    volatile char array[524288]; /* 512KB - typical L2 for Zen */
    for (int i = 0; i < sizeof(array); i += 64) {
        array[i] = i & 0xFF;
        sink = array[i];
    }
}

/* Cache size detection through timing */
static uint64_t detect_cache_size_hint(void) {
    const size_t max_size = 8 * 1024 * 1024; /* 8MB */
    volatile char *array = malloc(max_size);
    uint64_t start, end;
    uint64_t min_time = UINT64_MAX;
    size_t suspected_cache_size = 0;
    
    /* Initialize array */
    for (size_t i = 0; i < max_size; i++) {
        array[i] = (char)(i & 0xFF);
    }
    
    /* Test different strides and sizes */
    for (size_t size = 4096; size <= max_size; size *= 2) {
        start = __builtin_ia32_rdtsc();
        
        /* Access every cache line in the region */
        for (size_t i = 0; i < size; i += 64) {
            sink = array[i];
        }
        
        end = __builtin_ia32_rdtsc();
        uint64_t duration = end - start;
        
        if (duration < min_time) {
            min_time = duration;
            suspected_cache_size = size;
        }
    }
    
    free((void*)array);
    return suspected_cache_size;
}

/* Vectorized operations with different ISA extensions */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    volatile int array[16384];
    for (int i = 0; i < 16384; i += 8) {
        /* Simulate AVX2 operations */
        array[i] = i;
        array[i+1] = i+1;
        array[i+2] = i+2;
        array[i+3] = i+3;
        array[i+4] = i+4;
        array[i+5] = i+5;
        array[i+6] = i+6;
        array[i+7] = i+7;
        sink = array[i] + array[i+7];
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    volatile int array[32768];
    for (int i = 0; i < 32768; i += 16) {
        /* Simulate AVX512 operations */
        for (int j = 0; j < 16; j++) {
            array[i+j] = i + j;
        }
        sink = array[i] + array[i+15];
    }
}
#pragma GCC pop_options

int main(void) {
    uint64_t checksum = 0;
    
    /* Call all target-specific functions */
    test_core2_cache();
    test_sandybridge_cache();
    test_skylake_cache();
    test_zen2_cache();
    
    /* Call vectorized functions */
    avx2_cache_test();
    if (cpu_features[10]) { /* AVX512F supported */
        avx512_cache_test();
    }
    
    /* Perform cache timing analysis */
    uint64_t cache_hint = detect_cache_size_hint();
    
    /* Compute checksum from all CPU feature and model queries */
    for (int i = 0; i < 23; i++) {
        checksum = (checksum << 1) | (cpu_features[i] & 1);
    }
    for (int i = 0; i < 18; i++) {
        checksum = (checksum << 1) | (cpu_models[i] & 1);
    }
    checksum ^= cache_hint;
    
    /* Use checksum to prevent dead code elimination */
    volatile uint64_t final_result = checksum;
    
    printf("CPU detection test completed. Checksum: %llu\n", 
           (unsigned long long)final_result);
    printf("Suspected cache size from timing: %llu bytes\n",
           (unsigned long long)cache_hint);
    
    return 0;
}
