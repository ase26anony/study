/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int timing_results[4];

/* Early CPU initialization - forces driver to run CPUID and cache detection */
static void __attribute__((constructor)) init_cpu(void) {
    /* This forces GCC's driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger different cache paths */
    cpu_features[0] = __builtin_cpu_supports("sse");
    cpu_features[1] = __builtin_cpu_supports("sse2");
    cpu_features[2] = __builtin_cpu_supports("sse3");
    cpu_features[3] = __builtin_cpu_supports("ssse3");
    cpu_features[4] = __builtin_cpu_supports("sse4.1");
    cpu_features[5] = __builtin_cpu_supports("sse4.2");
    cpu_features[6] = __builtin_cpu_supports("avx");
    cpu_features[7] = __builtin_cpu_supports("avx2");
    cpu_features[8] = __builtin_cpu_supports("avx512f");
    cpu_features[9] = __builtin_cpu_supports("fma");
    cpu_features[10] = __builtin_cpu_supports("aes");
    cpu_features[11] = __builtin_cpu_supports("pclmul");
    cpu_features[12] = __builtin_cpu_supports("rdrand");
    cpu_features[13] = __builtin_cpu_supports("rdseed");
    cpu_features[14] = __builtin_cpu_supports("sha");
    cpu_features[15] = __builtin_cpu_supports("mmx");
    cpu_features[16] = __builtin_cpu_supports("3dnow");
    cpu_features[17] = __builtin_cpu_supports("popcnt");
    cpu_features[18] = __builtin_cpu_supports("abm");
    cpu_features[19] = __builtin_cpu_supports("bmi");
    cpu_features[20] = __builtin_cpu_supports("bmi2");
    cpu_features[21] = __builtin_cpu_supports("adx");
    cpu_features[22] = __builtin_cpu_supports("prefetchwt1");
    cpu_features[23] = __builtin_cpu_supports("clflushopt");
    
    /* Query CPU models - each may have different cache configurations */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1");
}

/* Target-specific functions to force driver to consider different microarchitectures */
static void __attribute__((target("arch=core2"))) 
cache_test_core2(void) {
    volatile int sum = 0;
    const int size = 1024 * 1024; /* 1MB array */
    static int array[1024 * 1024];
    
    /* Access pattern sensitive to cache line size (32/64 bytes) */
    for (int i = 0; i < size; i += 16) { /* Stride of 64 bytes if int=4 */
        sum += array[i];
    }
    timing_results[0] = sum;
}

static void __attribute__((target("arch=sandybridge"))) 
cache_test_sandybridge(void) {
    volatile int sum = 0;
    const int size = 2 * 1024 * 1024; /* 2MB array */
    static int array[2 * 1024 * 1024];
    
    /* Different stride to test associativity */
    for (int i = 0; i < size; i += 32) { /* Stride of 128 bytes */
        sum += array[i];
    }
    timing_results[1] = sum;
}

static void __attribute__((target("arch=haswell"))) 
cache_test_haswell(void) {
    volatile int sum = 0;
    const int size = 4 * 1024 * 1024; /* 4MB array */
    static int array[4 * 1024 * 1024];
    
    /* Random access pattern to stress cache */
    for (int i = 0; i < 100000; i++) {
        int idx = (i * 97) % size; /* Simple pseudo-random */
        sum += array[idx];
    }
    timing_results[2] = sum;
}

static void __attribute__((target("arch=znver1"))) 
cache_test_zen(void) {
    volatile int sum = 0;
    const int size = 8 * 1024 * 1024; /* 8MB array */
    static int array[8 * 1024 * 1024];
    
    /* Large stride to potentially miss L2 */
    for (int i = 0; i < size; i += 256) { /* Stride of 1KB */
        sum += array[i];
    }
    timing_results[3] = sum;
}

/* AVX2-optimized memory test to trigger vector path */
#pragma GCC push_options
#pragma GCC target("avx2")
static void __attribute__((noinline))
avx2_cache_test(void) {
    volatile int sum = 0;
    const int size = 1024 * 1024;
    static int array[1024 * 1024] __attribute__((aligned(32)));
    
    /* Vectorized access pattern */
    for (int i = 0; i < size; i += 8) { /* Process 8 ints at a time */
        sum += array[i];
    }
    timing_results[0] += sum;
}
#pragma GCC pop_options

/* AES-specific test to trigger that feature path */
#pragma GCC push_options
#pragma GCC target("aes")
static void __attribute__((noinline))
aes_cache_test(void) {
    volatile int sum = 0;
    const int size = 512 * 1024;
    static int array[512 * 1024];
    
    /* Different access pattern */
    for (int i = 0; i < size; i += 4) {
        sum ^= array[i]; /* Use XOR for variation */
    }
    timing_results[1] += sum;
}
#pragma GCC pop_options

/* Main test function with timing measurements */
int main(void) {
    uint64_t start, end;
    volatile int checksum = 0;
    
    /* Call all target-specific functions */
    cache_test_core2();
    cache_test_sandybridge();
    cache_test_haswell();
    cache_test_zen();
    
    /* Call feature-specific tests */
    avx2_cache_test();
    aes_cache_test();
    
    /* Create a checksum from all feature flags to prevent dead code elimination */
    for (int i = 0; i < 24; i++) {
        checksum = (checksum << 1) | (cpu_features[i] & 1);
    }
    for (int i = 0; i < 8; i++) {
        checksum = (checksum << 1) | (cpu_models[i] & 1);
    }
    for (int i = 0; i < 4; i++) {
        checksum ^= timing_results[i];
    }
    
    /* Use the checksum to affect control flow */
    if (checksum & 1) {
        printf("CPU features detected: ");
        for (int i = 0; i < 24; i++) {
            if (cpu_features[i]) {
                switch(i) {
                    case 0: printf("sse "); break;
                    case 1: printf("sse2 "); break;
                    case 2: printf("sse3 "); break;
                    case 3: printf("ssse3 "); break;
                    case 4: printf("sse4.1 "); break;
                    case 5: printf("sse4.2 "); break;
                    case 6: printf("avx "); break;
                    case 7: printf("avx2 "); break;
                    case 8: printf("avx512f "); break;
                    case 9: printf("fma "); break;
                    case 10: printf("aes "); break;
                    case 11: printf("pclmul "); break;
                }
            }
        }
        printf("\n");
    }
    
    /* Prevent compiler from optimizing everything away */
    volatile int result = checksum;
    return result & 0xFF;
}

/* Additional test to specifically trigger cache descriptor processing */
#ifdef __SELF_TEST__
/* This would be compiled with special driver flags to test cache detection */
void __attribute__((used)) 
force_cache_detection(void) {
    /* Array of cache descriptor bytes we want to test */
    static const unsigned char test_descriptors[] = {
        0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39,
        0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43,
        0x44, 0x45, 0x48, 0x49, 0x4e, 0x60, 0x66, 0x67,
        0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7f,
        0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
    };
    
    volatile unsigned int dummy = 0;
    for (int i = 0; i < sizeof(test_descriptors); i++) {
        /* Force compiler to consider each descriptor value */
        dummy += test_descriptors[i];
    }
    
    /* Use the result to affect program behavior */
    if (dummy > 0) {
        __builtin_cpu_init(); /* Force re-initialization */
    }
}
#endif
