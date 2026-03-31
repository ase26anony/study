/* { dg-do run } */
/* { dg-require-effective-target i386 } */
/* { dg-options "-O2 -march=native -mtune=generic -fdump-tree-all" } */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink; /* Prevent optimization */

/* Early CPU initialization - runs before main */
__attribute__((constructor(101)))
static void init_cpu(void) {
    /* Force driver to execute CPUID and cache detection */
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
    cpu_features[15] = __builtin_cpu_supports("xsave");
    cpu_features[16] = __builtin_cpu_supports("xsaveopt");
    cpu_features[17] = __builtin_cpu_supports("xsavec");
    cpu_features[18] = __builtin_cpu_supports("xgetbv");
    cpu_features[19] = __builtin_cpu_supports("rtm");
    cpu_features[20] = __builtin_cpu_supports("hle");
    cpu_features[21] = __builtin_cpu_supports("bmi");
    cpu_features[22] = __builtin_cpu_supports("bmi2");
    cpu_features[23] = __builtin_cpu_supports("adx");
    
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

/* Target-specific functions to force driver to consider different architectures */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char array[32768]; /* 32KB - typical L1 size */
    for (int i = 0; i < sizeof(array); i += 64) /* 64-byte stride */
        sink = array[i];
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char array[262144]; /* 256KB - typical L2 size */
    for (int i = 0; i < sizeof(array); i += 32) /* 32-byte stride */
        sink = array[i];
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char array[8388608]; /* 8MB - larger than typical L3 */
    for (int i = 0; i < sizeof(array); i += 128) /* 128-byte stride */
        sink = array[i];
}

__attribute__((target("arch=znver1")))
static void zen_cache_test(void) {
    volatile char array[16777216]; /* 16MB */
    /* Random access pattern to defeat prefetching */
    for (int i = 0; i < 100000; i++) {
        int idx = (i * 97) % sizeof(array);
        sink = array[idx];
    }
}

/* Cache-sensitive timing test */
static uint64_t time_cache_access(size_t size, int stride) {
    char *array = malloc(size);
    if (!array) return 0;
    
    memset(array, 0, size);
    
    uint64_t start, end;
    
    /* Use rdtsc for timing */
    start = __builtin_ia32_rdtsc();
    
    volatile char sum = 0;
    for (size_t i = 0; i < size; i += stride) {
        sum += array[i];
    }
    
    end = __builtin_ia32_rdtsc();
    
    sink = sum;
    free(array);
    
    return end - start;
}

/* Function with pragmas to trigger different optimization paths */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_test(void) {
    volatile int array[16384];
    for (int i = 0; i < 16384; i++) {
        array[i] = i;
    }
    /* Force vectorization */
    for (int i = 0; i < 16384; i += 8) {
        sink = array[i];
    }
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_test(void) {
    volatile long long array[8192];
    for (int i = 0; i < 8192; i++) {
        array[i] = i;
    }
    /* 64-byte cache lines */
    for (int i = 0; i < 8192; i += 8) {
        sink = array[i];
    }
}
#pragma GCC pop_options

int main(void) {
    uint64_t checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    zen_cache_test();
    avx2_cache_test();
    avx512_cache_test();
    
    /* Perform cache timing tests with different sizes and strides */
    uint64_t time1 = time_cache_access(8192, 32);   /* 8KB, 32-byte stride */
    uint64_t time2 = time_cache_access(65536, 64);  /* 64KB, 64-byte stride */
    uint64_t time3 = time_cache_access(524288, 128); /* 512KB, 128-byte stride */
    uint64_t time4 = time_cache_access(4194304, 256); /* 4MB, 256-byte stride */
    
    /* Use timing ratios to branch - this may trigger cache queries */
    if (time2 > time1 * 3) {
        /* Likely L1 cache miss pattern */
        checksum += 1;
    }
    if (time3 > time2 * 2) {
        /* Likely L2 cache miss pattern */
        checksum += 2;
    }
    if (time4 > time3 * 1.5) {
        /* Likely L3 cache miss pattern */
        checksum += 4;
    }
    
    /* Incorporate CPU feature flags into checksum */
    for (int i = 0; i < 24; i++) {
        if (cpu_features[i]) {
            checksum += (1ULL << (i % 32));
        }
    }
    
    for (int i = 0; i < 8; i++) {
        if (cpu_models[i]) {
            checksum += (1ULL << (24 + i));
        }
    }
    
    /* Prevent dead code elimination */
    printf("CPU checksum: %llu\n", (unsigned long long)checksum);
    printf("Timing results: %llu %llu %llu %llu\n", 
           (unsigned long long)time1,
           (unsigned long long)time2,
           (unsigned long long)time3,
           (unsigned long long)time4);
    
    return 0;
}

/* { dg-final { scan-assembler "cpuid" } } */
/* { dg-final { scan-tree-dump "cache" "optimized" } } */
