/* { dg-require-effective-target i386 } */
/* { dg-options "-march=native -mtune=generic -O2" } */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to store CPU feature detection results */
static int cpu_features[32];
static int cpu_models[8];
static volatile int sink; /* Prevent dead code elimination */

/* Early CPU initialization - forces driver cache detection */
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
    cpu_features[12] = __builtin_cpu_supports("fma");
    cpu_features[13] = __builtin_cpu_supports("aes");
    cpu_features[14] = __builtin_cpu_supports("pclmul");
    cpu_features[15] = __builtin_cpu_supports("rdrand");
    cpu_features[16] = __builtin_cpu_supports("rdseed");
    cpu_features[17] = __builtin_cpu_supports("sha");
    cpu_features[18] = __builtin_cpu_supports("xsave");
    cpu_features[19] = __builtin_cpu_supports("xsaveopt");
    cpu_features[20] = __builtin_cpu_supports("xsavec");
    cpu_features[21] = __builtin_cpu_supports("xsaves");
    
    /* Query CPU models - each may have different cache descriptors */
    cpu_models[0] = __builtin_cpu_is("intel");
    cpu_models[1] = __builtin_cpu_is("amd");
    cpu_models[2] = __builtin_cpu_is("core2");
    cpu_models[3] = __builtin_cpu_is("nehalem");
    cpu_models[4] = __builtin_cpu_is("sandybridge");
    cpu_models[5] = __builtin_cpu_is("haswell");
    cpu_models[6] = __builtin_cpu_is("skylake");
    cpu_models[7] = __builtin_cpu_is("znver1"); /* AMD Zen */
}

/* Target-specific functions to force driver to consider different cache configs */
__attribute__((target("arch=core2")))
static void core2_cache_test(void) {
    volatile char *array = malloc(256 * 1024); /* 256KB - typical L2 size */
    if (array) {
        /* Access with 64-byte stride (typical cache line) */
        for (int i = 0; i < 256 * 1024; i += 64) {
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=sandybridge")))
static void sandybridge_cache_test(void) {
    volatile char *array = malloc(1024 * 1024); /* 1MB */
    if (array) {
        /* 32-byte stride for AVX */
        for (int i = 0; i < 1024 * 1024; i += 32) {
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=haswell")))
static void haswell_cache_test(void) {
    volatile char *array = malloc(2048 * 1024); /* 2MB */
    if (array) {
        /* 64-byte stride */
        for (int i = 0; i < 2048 * 1024; i += 64) {
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=skylake")))
static void skylake_cache_test(void) {
    volatile char *array = malloc(4096 * 1024); /* 4MB */
    if (array) {
        /* Mixed strides to trigger different cache logic */
        for (int i = 0; i < 4096 * 1024; i += 128) {
            sink = array[i];
        }
        free((void*)array);
    }
}

__attribute__((target("arch=znver1")))
static void zen_cache_test(void) {
    volatile char *array = malloc(8192 * 1024); /* 8MB */
    if (array) {
        /* Large stride for L3 cache */
        for (int i = 0; i < 8192 * 1024; i += 256) {
            sink = array[i];
        }
        free((void*)array);
    }
}

/* Force AVX code generation which may affect cache detection */
#pragma GCC push_options
#pragma GCC target("avx2")
static void avx2_cache_sensitive_loop(void) {
    /* Aligned memory for AVX */
    __attribute__((aligned(32))) volatile float data[1024];
    
    /* Vectorized loop - compiler may consider cache for optimization */
    for (int i = 0; i < 1024; i += 8) {
        /* Simulate vector load */
        volatile float *ptr = &data[i];
        sink = *(int*)ptr;
    }
}
#pragma GCC pop_options

/* Force AVX-512 code generation */
#pragma GCC push_options
#pragma GCC target("avx512f")
static void avx512_cache_sensitive_loop(void) {
    __attribute__((aligned(64))) volatile double data[2048];
    
    for (int i = 0; i < 2048; i += 8) {
        volatile double *ptr = &data[i];
        sink = *(int*)ptr;
    }
}
#pragma GCC pop_options

/* Cache size estimation via timing */
static uint64_t estimate_cache_size(void) {
    const size_t max_size = 32 * 1024 * 1024; /* 32MB */
    volatile char *buffer = malloc(max_size);
    uint64_t start, end;
    uint64_t min_time = UINT64_MAX;
    size_t estimated_size = 0;
    
    if (!buffer) return 0;
    
    /* Warm up */
    for (size_t i = 0; i < max_size; i += 64) {
        sink = buffer[i];
    }
    
    /* Test different sizes */
    for (size_t size = 4 * 1024; size <= max_size; size *= 2) {
        start = __builtin_ia32_rdtsc();
        
        /* Access entire block */
        for (size_t i = 0; i < size; i += 64) {
            sink = buffer[i];
        }
        
        end = __builtin_ia32_rdtsc();
        uint64_t duration = end - start;
        
        if (duration < min_time) {
            min_time = duration;
            estimated_size = size;
        }
    }
    
    free((void*)buffer);
    return estimated_size;
}

int main(void) {
    uint64_t checksum = 0;
    
    /* Execute all target-specific functions */
    core2_cache_test();
    sandybridge_cache_test();
    haswell_cache_test();
    skylake_cache_test();
    zen_cache_test();
    
    /* Execute vectorized loops */
    avx2_cache_sensitive_loop();
    avx512_cache_sensitive_loop();
    
    /* Aggregate feature flags into checksum */
    for (int i = 0; i < 22; i++) {
        checksum = (checksum << 1) | (cpu_features[i] & 1);
    }
    
    for (int i = 0; i < 8; i++) {
        checksum = (checksum << 1) | (cpu_models[i] & 1);
    }
    
    /* Estimate cache size - this may trigger driver cache queries */
    uint64_t cache_size = estimate_cache_size();
    checksum ^= cache_size;
    
    /* Use checksum to prevent dead code elimination */
    sink = (int)checksum;
    
    printf("CPU feature checksum: 0x%016llx\n", (unsigned long long)checksum);
    printf("Estimated cache size: %llu KB\n", (unsigned long long)cache_size / 1024);
    
    return 0;
}
