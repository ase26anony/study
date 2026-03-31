/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */
/* Test program to exercise GCC x86 driver cache detection logic */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink;

/* Early CPU initialization to force driver cache detection */
__attribute__((constructor))
static void init_cpu(void) {
    /* This forces the driver to execute CPUID and cache detection */
    __builtin_cpu_init();
    
    /* Query a wide variety of CPU features to trigger different paths */
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
    cpu_features[10] = __builtin_cpu_supports("avx512bw");
    cpu_features[11] = __builtin_cpu_supports("avx512dq");
    cpu_features[12] = __builtin_cpu_supports("aes");
    cpu_features[13] = __builtin_cpu_supports("pclmul");
    cpu_features[14] = __builtin_cpu_supports("rdrand");
    cpu_features[15] = __builtin_cpu_supports("rdseed");
    cpu_features[16] = __builtin_cpu_supports("sha");
    cpu_features[17] = __builtin_cpu_supports("fma");
    cpu_features[18] = __builtin_cpu_supports("f16c");
    cpu_features[19] = __builtin_cpu_supports("bmi");
    cpu_features[20] = __builtin_cpu_supports("bmi2");
    cpu_features[21] = __builtin_cpu_supports("adx");
    cpu_features[22] = __builtin_cpu_supports("clflushopt");
    cpu_features[23] = __builtin_cpu_supports("xsave");
    
    /* Query CPU models to trigger different cache descriptor tables */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1");
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *buffer = malloc(256 * 1024); /* 256KB - typical L2 for Core2 */
    if (buffer) {
        for (int i = 0; i < 256 * 1024; i += 64) { /* 64-byte stride */
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *buffer = malloc(1024 * 1024); /* 1MB - typical L3 for Sandy Bridge */
    if (buffer) {
        for (int i = 0; i < 1024 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *buffer = malloc(2048 * 1024); /* 2MB - typical L3 for Haswell */
    if (buffer) {
        for (int i = 0; i < 2048 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *buffer = malloc(8192 * 1024); /* 8MB - typical L3 for Skylake */
    if (buffer) {
        for (int i = 0; i < 8192 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

__attribute__((target("arch=znver1")))
static void zen_cache_test(void) {
    volatile char *buffer = malloc(16384 * 1024); /* 16MB - typical L3 for Zen */
    if (buffer) {
        for (int i = 0; i < 16384 * 1024; i += 64) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}

/* Cache-sensitive memory access pattern */
static uint64_t cache_sensitive_access(int size_kb, int stride) {
    volatile char *buffer = malloc(size_kb * 1024);
    uint64_t start, end;
    
    if (!buffer) return 0;
    
    /* Initialize */
    memset((void*)buffer, 0, size_kb * 1024);
    
    /* Time the access */
    start = __builtin_ia32_rdtsc();
    for (int i = 0; i < size_kb * 1024; i += stride) {
        sink = buffer[i];
    }
    end = __builtin_ia32_rdtsc();
    
    free((void*)buffer);
    return end - start;
}

/* Main test function with pragmas to trigger driver paths */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    /* This function with AVX2 target may trigger different cache detection */
    volatile int *buffer = malloc(512 * 1024);
    if (buffer) {
        for (int i = 0; i < 128 * 1024; i++) { /* Process as 4-byte ints */
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    /* AVX-512 may have different cache requirements */
    volatile int *buffer = malloc(1024 * 1024);
    if (buffer) {
        for (int i = 0; i < 256 * 1024; i++) {
            sink = buffer[i];
        }
        free((void*)buffer);
    }
}
#pragma GCC pop_options

int main(void) {
    uint64_t timing_results[8];
    int checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    zen_cache_test();
    avx2_cache_test();
    avx512_cache_test();
    
    /* Perform cache-sensitive timing tests with different strides */
    timing_results[0] = cache_sensitive_access(32, 32);   /* 32B stride */
    timing_results[1] = cache_sensitive_access(32, 64);   /* 64B stride */
    timing_results[2] = cache_sensitive_access(256, 64);  /* L2 size test */
    timing_results[3] = cache_sensitive_access(1024, 64); /* L3 size test */
    timing_results[4] = cache_sensitive_access(8192, 64); /* Large size */
    
    /* Create checksum from CPU features and timing to prevent optimization */
    for (int i = 0; i < 24; i++) {
        checksum ^= cpu_features[i] << (i % 8);
    }
    for (int i = 0; i < 8; i++) {
        checksum ^= cpu_models[i] << (i % 8);
    }
    for (int i = 0; i < 5; i++) {
        checksum ^= (timing_results[i] >> 32) ^ timing_results[i];
    }
    
    /* Use checksum to prevent dead code elimination */
    sink = checksum;
    
    printf("CPU detection test completed. Checksum: %d\n", checksum);
    
    /* Additional feature queries in main to ensure driver runs */
    if (__builtin_cpu_supports("sse4.2")) {
        printf("SSE4.2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
    }
    
    return 0;
}
