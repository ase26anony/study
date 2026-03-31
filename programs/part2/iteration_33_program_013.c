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
static volatile int sink = 0;

/* Early CPU initialization - runs before main */
static void __attribute__((constructor)) init_cpu(void) {
    /* Force driver to execute CPUID and cache detection */
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
    cpu_features[9] = __builtin_cpu_supports("fma");
    cpu_features[10] = __builtin_cpu_supports("aes");
    cpu_features[11] = __builtin_cpu_supports("pclmul");
    cpu_features[12] = __builtin_cpu_supports("rdrand");
    cpu_features[13] = __builtin_cpu_supports("rdseed");
    cpu_features[14] = __builtin_cpu_supports("sha");
    cpu_features[15] = __builtin_cpu_supports("xsave");
    cpu_features[16] = __builtin_cpu_supports("xsaveopt");
    cpu_features[17] = __builtin_cpu_supports("xsavec");
    cpu_features[18] = __builtin_cpu_supports("xsaves");
    cpu_features[19] = __builtin_cpu_supports("mmx");
    cpu_features[20] = __builtin_cpu_supports("3dnow");
    cpu_features[21] = __builtin_cpu_supports("popcnt");
    cpu_features[22] = __builtin_cpu_supports("bmi");
    cpu_features[23] = __builtin_cpu_supports("bmi2");
    
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

/* Target-specific functions to force driver to consider different microarchitectures */
static void __attribute__((target("arch=core2"))) 
cache_test_core2(void) {
    volatile char array[32768]; /* 32KB - typical L1 size */
    for (int i = 0; i < sizeof(array); i += 64) { /* 64-byte stride */
        array[i] = i & 0xFF;
        sink += array[i];
    }
}

static void __attribute__((target("arch=sandybridge"))) 
cache_test_sandybridge(void) {
    volatile char array[262144]; /* 256KB - typical L2 size */
    for (int i = 0; i < sizeof(array); i += 32) { /* 32-byte stride */
        array[i] = i & 0xFF;
        sink += array[i];
    }
}

static void __attribute__((target("arch=haswell"))) 
cache_test_haswell(void) {
    volatile char array[8388608]; /* 8MB - larger than typical L3 */
    for (int i = 0; i < sizeof(array); i += 128) { /* 128-byte stride */
        array[i] = i & 0xFF;
        sink += array[i];
    }
}

static void __attribute__((target("arch=znver1"))) 
cache_test_zen(void) {
    volatile char array[16777216]; /* 16MB */
    for (int i = 0; i < sizeof(array); i += 256) { /* 256-byte stride */
        array[i] = i & 0xFF;
        sink += array[i];
    }
}

/* Function with AVX2 target to trigger feature-specific paths */
static void __attribute__((target("avx2"))) 
avx2_cache_test(void) {
    volatile int array[16384];
    for (int i = 0; i < 16384; i++) {
        array[i] = i;
        sink += array[i];
    }
}

/* Function with SSE4.2 target */
static void __attribute__((target("sse4.2"))) 
sse42_cache_test(void) {
    volatile long long array[8192];
    for (int i = 0; i < 8192; i++) {
        array[i] = i;
        sink += array[i];
    }
}

/* Cache size estimation through timing */
static uint64_t estimate_cache_size(void) {
    const size_t max_size = 32 * 1024 * 1024; /* 32MB */
    volatile char *array = malloc(max_size);
    uint64_t start, end, min_time = UINT64_MAX;
    size_t best_size = 0;
    
    if (!array) return 0;
    
    memset((void*)array, 0, max_size);
    
    /* Test different working set sizes */
    for (size_t size = 4096; size <= max_size; size *= 2) {
        const int iterations = 10000000 / (size / 4096);
        
        /* Use rdtsc for timing */
        start = __builtin_ia32_rdtsc();
        
        for (int iter = 0; iter < iterations; iter++) {
            for (size_t i = 0; i < size; i += 64) {
                sink += array[i];
            }
        }
        
        end = __builtin_ia32_rdtsc();
        uint64_t elapsed = end - start;
        
        if (elapsed < min_time) {
            min_time = elapsed;
            best_size = size;
        }
    }
    
    free((void*)array);
    return best_size;
}

/* Main function with cache-sensitive branching */
int main(void) {
    uint64_t cache_estimate = 0;
    int checksum = 0;
    
    /* Call all target-specific functions */
    cache_test_core2();
    cache_test_sandybridge();
    cache_test_haswell();
    cache_test_zen();
    
    /* Call feature-specific functions */
    avx2_cache_test();
    sse42_cache_test();
    
    /* Estimate cache size - this may cause driver to query cache parameters */
    cache_estimate = estimate_cache_size();
    
    /* Create checksum based on CPU features and cache estimate */
    for (int i = 0; i < 24; i++) {
        checksum ^= cpu_features[i] << (i % 16);
    }
    
    for (int i = 0; i < 8; i++) {
        checksum ^= cpu_models[i] << (i % 16);
    }
    
    checksum ^= (cache_estimate >> 32) ^ (cache_estimate & 0xFFFFFFFF);
    
    /* Use checksum to prevent dead code elimination */
    volatile int result = checksum;
    
    printf("CPU feature checksum: 0x%08x\n", checksum);
    printf("Estimated cache size: %llu bytes\n", 
           (unsigned long long)cache_estimate);
    
    /* Additional feature queries in main to ensure driver runs */
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
    }
    
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F supported\n");
    }
    
    /* Query more cache-related features */
    if (__builtin_cpu_supports("clflushopt")) {
        printf("CLFLUSHOPT supported\n");
    }
    
    if (__builtin_cpu_supports("clwb")) {
        printf("CLWB supported\n");
    }
    
    /* Test with pragma for different target */
    #pragma GCC push_options
    #pragma GCC target("arch=skylake")
    {
        volatile int skylake_array[65536];
        for (int i = 0; i < 65536; i += 64) {
            skylake_array[i] = i;
            sink += skylake_array[i];
        }
    }
    #pragma GCC pop_options
    
    /* Test with pragma for AVX */
    #pragma GCC push_options
    #pragma GCC target("avx")
    {
        volatile float avx_array[8192] __attribute__((aligned(32)));
        for (int i = 0; i < 8192; i++) {
            avx_array[i] = i * 0.5f;
            sink += (int)avx_array[i];
        }
    }
    #pragma GCC pop_options
    
    return result != 0;
}

/* Additional test cases for specific cache descriptor values */
#ifdef TEST_SPECIFIC_CACHE
/* This would require mocking CPUID responses - not possible in user code,
   but included to show what would be needed for complete coverage */
static void test_specific_cache_cases(void) {
    /* The following cases correspond to specific cache descriptor bytes:
       0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b,
       0x3c, 0x3d, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49,
       0x4e, 0x60, 0x66, 0x67, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
       0x7d, 0x7f, 0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
       
       These would need to be triggered by specific CPU models/steppings
       that report these exact cache descriptor bytes through CPUID leaf 2/4.
    */
    
    /* Simulate different CPU models that might have these cache descriptors */
    const char *test_models[] = {
        "intel", "amd", "core2", "nehalem", "sandybridge",
        "ivybridge", "haswell", "broadwell", "skylake",
        "cascadelake", "cooperlake", "icelake", "tigerlake",
        "rocketlake", "alderlake", "raptorlake",
        "znver1", "znver2", "znver3", "znver4"
    };
    
    for (int i = 0; i < sizeof(test_models)/sizeof(test_models[0]); i++) {
        if (__builtin_cpu_is(test_models[i])) {
            /* This might trigger driver to look up cache descriptors
               for this specific model */
            volatile int dummy = 0;
            for (int j = 0; j < 1000; j++) {
                dummy += __builtin_cpu_supports("sse") ? 1 : 0;
            }
            sink += dummy;
        }
    }
}
#endif
