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
static volatile int sink;

/* Early CPU initialization - forces driver to run CPUID and cache detection */
__attribute__((constructor)) 
static void init_cpu(void) {
    /* This forces the driver to execute CPUID and cache detection logic */
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
    
    /* Query CPU models to trigger different cache descriptor mappings */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("atom");
    cpu_models[3] = __builtin_cpu_is("core2");
    cpu_models[4] = __builtin_cpu_is("nehalem");
    cpu_models[5] = __builtin_cpu_is("sandybridge");
    cpu_models[6] = __builtin_cpu_is("haswell");
    cpu_models[7] = __builtin_cpu_is("skylake");
}

/* Target-specific functions to force driver to consider different microarchitectures */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *array = malloc(1024 * 1024); /* 1MB array */
    if (array) {
        /* Access with 64-byte stride (typical cache line) */
        for (int i = 0; i < 1024 * 1024; i += 64) {
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *array = malloc(2 * 1024 * 1024); /* 2MB array */
    if (array) {
        /* Access with 32-byte stride */
        for (int i = 0; i < 2 * 1024 * 1024; i += 32) {
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *array = malloc(4 * 1024 * 1024); /* 4MB array */
    if (array) {
        /* Access with 128-byte stride */
        for (int i = 0; i < 4 * 1024 * 1024; i += 128) {
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *array = malloc(8 * 1024 * 1024); /* 8MB array */
    if (array) {
        /* Random access pattern to stress cache */
        for (int i = 0; i < 10000; i++) {
            int idx = (i * 997) % (8 * 1024 * 1024);
            sink = array[idx];
        }
        free((void*)array);
    }
}

/* AVX2-optimized memory test to trigger AVX feature detection */
#pragma GCC push_options
#pragma GCC target("avx2")
__attribute__((noinline))
static void avx2_cache_test(void) {
    volatile int *array = malloc(256 * 1024); /* 256KB array */
    if (array) {
        for (int i = 0; i < 256 * 1024; i += 8) {
            sink = array[i];
        }
        free((void*)array);
    }
}
#pragma GCC pop_options

/* AES-NI optimized test */
#pragma GCC push_options
#pragma GCC target("aes,pclmul")
__attribute__((noinline))
static void aes_cache_test(void) {
    volatile char *array = malloc(128 * 1024); /* 128KB array */
    if (array) {
        for (int i = 0; i < 128 * 1024; i += 16) {
            sink = array[i];
        }
        free((void*)array);
    }
}
#pragma GCC pop_options

/* Main test function that exercises all cache detection paths */
int main(void) {
    uint64_t checksum = 0;
    
    /* Call all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    avx2_cache_test();
    aes_cache_test();
    
    /* Compute checksum from CPU feature flags to prevent dead code elimination */
    for (int i = 0; i < 16; i++) {
        checksum = (checksum << 1) | (cpu_features[i] & 1);
    }
    for (int i = 0; i < 8; i++) {
        checksum = (checksum << 1) | (cpu_models[i] & 1);
    }
    
    /* Memory-intensive test with different access patterns */
    const size_t large_size = 16 * 1024 * 1024; /* 16MB - larger than most L2 caches */
    volatile char *large_array = malloc(large_size);
    
    if (large_array) {
        /* Test 1: Sequential access with 64-byte stride (cache line size) */
        uint64_t start, end;
        start = __builtin_ia32_rdtsc();
        for (size_t i = 0; i < large_size; i += 64) {
            sink = large_array[i];
        }
        end = __builtin_ia32_rdtsc();
        checksum ^= (end - start);
        
        /* Test 2: Sequential access with 32-byte stride */
        start = __builtin_ia32_rdtsc();
        for (size_t i = 0; i < large_size; i += 32) {
            sink = large_array[i];
        }
        end = __builtin_ia32_rdtsc();
        checksum ^= (end - start);
        
        /* Test 3: Random access to defeat prefetching */
        start = __builtin_ia32_rdtsc();
        for (int i = 0; i < 100000; i++) {
            size_t idx = (i * 997) % large_size;
            sink = large_array[idx];
        }
        end = __builtin_ia32_rdtsc();
        checksum ^= (end - start);
        
        free((void*)large_array);
    }
    
    /* Force compiler to consider different optimization levels */
    __attribute__((optimize("O0"))) volatile int o0_var = checksum;
    __attribute__((optimize("O1"))) volatile int o1_var = checksum >> 32;
    __attribute__((optimize("O2"))) volatile int o2_var = checksum >> 16;
    __attribute__((optimize("O3"))) volatile int o3_var = checksum >> 8;
    
    /* Output checksum to prevent complete optimization */
    printf("CPU detection checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    /* Additional CPUID queries in main to ensure driver is fully exercised */
    if (__builtin_cpu_supports("sse4.2")) {
        checksum += 0x1234;
    }
    if (__builtin_cpu_is("intel")) {
        checksum += 0x5678;
    }
    if (__builtin_cpu_is("amd")) {
        checksum += 0x9abc;
    }
    
    return (checksum & 0xFF) == 0 ? 0 : 1;
}

/* Additional test cases for specific cache descriptor values */
__attribute__((constructor))
static void test_cache_descriptors(void) {
    /* This function attempts to trigger specific cache descriptor mappings
       by checking for features associated with different cache configurations */
    
    /* Features that might be associated with specific cache descriptor bytes */
    const char *feature_list[] = {
        "mmx", "sse", "sse2", "sse3", "ssse3",
        "sse4.1", "sse4.2", "avx", "avx2",
        "fma", "aes", "pclmul", "rdrand",
        "rdseed", "sha", "adx", "bmi", "bmi2"
    };
    
    const char *model_list[] = {
        "intel", "amd", "atom", "core2",
        "nehalem", "sandybridge", "ivybridge",
        "haswell", "broadwell", "skylake",
        "kabylake", "cannonlake", "icelake",
        "tigerlake", "rocketlake", "alderlake"
    };
    
    volatile int dummy = 0;
    
    /* Force evaluation of all features */
    for (int i = 0; i < sizeof(feature_list)/sizeof(feature_list[0]); i++) {
        if (__builtin_cpu_supports(feature_list[i])) {
            dummy++;
        }
    }
    
    /* Force evaluation of all models */
    for (int i = 0; i < sizeof(model_list)/sizeof(model_list[0]); i++) {
        if (__builtin_cpu_is(model_list[i])) {
            dummy++;
        }
    }
    
    sink = dummy;
}
