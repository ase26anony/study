/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int prevent_optimize = 0;

/* Early CPU initialization - runs before main */
static void __attribute__((constructor)) init_cpu(void) {
    /* Force driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger cache detection */
    cpu_features[0] = __builtin_cpu_supports("sse");
    cpu_features[1] = __builtin_cpu_supports("sse2");
    cpu_features[2] = __builtin_cpu_supports("sse3");
    cpu_features[3] = __builtin_cpu_supports("ssse3");
    cpu_features[4] = __builtin_cpu_supports("sse4.1");
    cpu_features[5] = __builtin_cpu_supports("sse4.2");
    cpu_features[6] = __builtin_cpu_supports("avx");
    cpu_features[7] = __builtin_cpu_supports("avx2");
    cpu_features[8] = __builtin_cpu_supports("avx512f");
    cpu_features[9] = __builtin_cpu_supports("avx512vl");
    cpu_features[10] = __builtin_cpu_supports("aes");
    cpu_features[11] = __builtin_cpu_supports("pclmul");
    cpu_features[12] = __builtin_cpu_supports("rdrand");
    cpu_features[13] = __builtin_cpu_supports("rdseed");
    cpu_features[14] = __builtin_cpu_supports("sha");
    cpu_features[15] = __builtin_cpu_supports("fma");
    cpu_features[16] = __builtin_cpu_supports("f16c");
    cpu_features[17] = __builtin_cpu_supports("bmi");
    cpu_features[18] = __builtin_cpu_supports("bmi2");
    cpu_features[19] = __builtin_cpu_supports("adx");
    
    /* Query CPU models - each may have different cache configurations */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1");  /* AMD Zen */
}

/* Target-specific functions to force driver to consider different cache configs */
static void __attribute__((target("arch=core2"))) 
cache_test_core2(void) {
    volatile int sum = 0;
    char buffer[32768];  /* 32KB - typical L1 cache size */
    
    /* Access pattern that depends on cache line size */
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
    prevent_optimize = sum;
}

static void __attribute__((target("arch=sandybridge"))) 
cache_test_sandybridge(void) {
    volatile int sum = 0;
    char buffer[262144];  /* 256KB - typical L2 cache size */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
    prevent_optimize = sum;
}

static void __attribute__((target("arch=haswell"))) 
cache_test_haswell(void) {
    volatile int sum = 0;
    char buffer[8388608];  /* 8MB - typical L3 cache size */
    
    for (int i = 0; i < sizeof(buffer); i += 64) {
        sum += buffer[i];
    }
    prevent_optimize = sum;
}

static void __attribute__((target("arch=skylake"))) 
cache_test_skylake(void) {
    volatile int sum = 0;
    char buffer[1048576];  /* 1MB - L2 cache for some Skylake variants */
    
    for (int i = 0; i < sizeof(buffer); i += 32) {
        sum += buffer[i];
    }
    prevent_optimize = sum;
}

/* Function to perform cache-sensitive timing */
static uint64_t time_cache_sensitive_access(size_t size, int stride) {
    char *buffer = malloc(size);
    if (!buffer) return 0;
    
    /* Initialize buffer */
    for (size_t i = 0; i < size; i++) {
        buffer[i] = (char)(i & 0xFF);
    }
    
    volatile char sink;
    uint64_t start, end;
    
    /* Use rdtsc for timing if available */
    #ifdef __i386__
    __asm__ volatile ("rdtsc" : "=A" (start));
    #else
    __asm__ volatile ("rdtsc" : "=a" (start), "=d" (end));
    start = (end << 32) | start;
    #endif
    
    /* Access with given stride */
    for (size_t i = 0; i < size; i += stride) {
        sink = buffer[i];
    }
    
    #ifdef __i386__
    __asm__ volatile ("rdtsc" : "=A" (end));
    #else
    __asm__ volatile ("rdtsc" : "=a" (start), "=d" (end));
    end = (end << 32) | start;
    #endif
    
    free(buffer);
    return end - start;
}

/* Test different cache configurations through pragmas */
#pragma GCC push_options
#pragma GCC target("avx2")
static void test_avx2_cache(void) {
    volatile int sum = 0;
    int array[1024];
    
    for (int i = 0; i < 1024; i++) {
        array[i] = i;
        sum += array[i];
    }
    prevent_optimize = sum;
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void test_avx512_cache(void) {
    volatile int sum = 0;
    int array[2048];
    
    for (int i = 0; i < 2048; i++) {
        array[i] = i;
        sum += array[i];
    }
    prevent_optimize = sum;
}
#pragma GCC pop_options

int main(void) {
    uint64_t checksum = 0;
    
    /* Call all target-specific functions */
    cache_test_core2();
    cache_test_sandybridge();
    cache_test_haswell();
    cache_test_skylake();
    
    /* Test with different pragmas */
    test_avx2_cache();
    test_avx512_cache();
    
    /* Perform cache-sensitive timing tests with different strides */
    uint64_t time32 = time_cache_sensitive_access(1024 * 1024, 32);
    uint64_t time64 = time_cache_sensitive_access(1024 * 1024, 64);
    uint64_t time128 = time_cache_sensitive_access(1024 * 1024, 128);
    
    /* Use timing results to create branches that might trigger cache queries */
    if (time64 < time128) {
        for (int i = 0; i < 20; i++) {
            checksum += cpu_features[i];
        }
    }
    
    if (time32 * 2 < time64) {
        for (int i = 0; i < 8; i++) {
            checksum += cpu_models[i];
        }
    }
    
    /* Additional CPU feature queries in main */
    if (__builtin_cpu_supports("sse4.2")) {
        checksum += 1000;
    }
    
    if (__builtin_cpu_supports("avx")) {
        checksum += 2000;
    }
    
    if (__builtin_cpu_supports("avx2")) {
        checksum += 3000;
    }
    
    /* Query more specific cache-related features */
    if (__builtin_cpu_supports("clflushopt")) {
        checksum += 4000;
    }
    
    if (__builtin_cpu_supports("clwb")) {
        checksum += 5000;
    }
    
    if (__builtin_cpu_supports("pcommit")) {
        checksum += 6000;
    }
    
    /* Test different CPU models */
    const char *models[] = {
        "intel", "amd", "core2", "nehalem", "sandybridge",
        "ivybridge", "haswell", "broadwell", "skylake",
        "kabylake", "cannonlake", "icelake", "tigerlake",
        "alderlake", "znver1", "znver2", "znver3"
    };
    
    for (size_t i = 0; i < sizeof(models)/sizeof(models[0]); i++) {
        if (__builtin_cpu_is(models[i])) {
            checksum += i * 100;
        }
    }
    
    /* Prevent dead code elimination */
    printf("CPU checksum: %llu\n", (unsigned long long)checksum);
    printf("Prevent optimize: %d\n", prevent_optimize);
    
    return 0;
}
