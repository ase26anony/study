/* driver_cache_test.c - Test program for GCC driver cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function with target attribute for Core2 architecture */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int* data, size_t size) {
    /* Large stride access pattern */
    for (size_t i = 0; i < size; i += 64) {
        data[i] = data[i] * 3 + 7;
    }
    
    /* Inline CPUID for cache descriptors (leaf 2) */
    uint32_t eax, ebx, ecx, edx;
    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(2), "c"(0)
    );
    
    /* Use CPUID results to prevent optimization */
    volatile uint32_t cpuid_sum = eax + ebx + ecx + edx;
    (void)cpuid_sum;
}

/* Function with target attribute for Nehalem architecture */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(float* data, size_t size) {
    /* Sequential access pattern */
    for (size_t i = 0; i < size; i++) {
        data[i] = data[i] * 1.5f - 0.25f;
    }
    
    /* CPUID leaf 4 - deterministic cache parameters */
    uint32_t eax, ebx, ecx, edx;
    uint32_t cache_level = 0;
    
    do {
        asm volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(4), "c"(cache_level)
        );
        
        cache_level++;
        volatile uint32_t cache_info = eax & 0x1F; /* Cache type */
        (void)cache_info;
    } while ((eax & 0x1F) != 0); /* Continue until cache type = 0 */
}

/* Function with target attribute for Sandy Bridge */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(double* data, size_t size) {
    /* Random-ish access pattern */
    for (size_t i = 0; i < size; i++) {
        size_t idx = (i * 97) % size; /* Pseudo-random index */
        data[idx] = data[idx] * 2.71828;
    }
    
    /* Prefetch hints */
    for (size_t i = 0; i < size; i += 16) {
        __builtin_prefetch(&data[i + 64], 0, 3);
    }
}

/* Function with target attribute for Skylake */
__attribute__((target("arch=skylake")))
void skylake_optimized_compute(int64_t* data, size_t size) {
    /* Matrix-style access pattern */
    const size_t dim = 256;
    for (size_t i = 0; i < dim; i++) {
        for (size_t j = 0; j < dim; j++) {
            data[i * dim + j] = data[i * dim + j] + data[j * dim + i];
        }
    }
}

/* Main computation function with architecture-specific paths */
void perform_computations(void) {
    /* Initialize CPU detection */
    __builtin_cpu_init();
    
    /* Large aligned arrays for different data types */
    __attribute__((aligned(64))) int int_data[1024 * 1024];
    __attribute__((aligned(64))) float float_data[512 * 512];
    __attribute__((aligned(64))) double double_data[256 * 256];
    __attribute__((aligned(64))) int64_t int64_data[128 * 128];
    
    /* Fill arrays with pseudo-random data */
    for (size_t i = 0; i < sizeof(int_data)/sizeof(int_data[0]); i++) {
        int_data[i] = lcg_rand() % 1000;
    }
    
    for (size_t i = 0; i < sizeof(float_data)/sizeof(float_data[0]); i++) {
        float_data[i] = (lcg_rand() % 1000) / 100.0f;
    }
    
    for (size_t i = 0; i < sizeof(double_data)/sizeof(double_data[0]); i++) {
        double_data[i] = (lcg_rand() % 1000) / 100.0;
    }
    
    for (size_t i = 0; i < sizeof(int64_data)/sizeof(int64_data[0]); i++) {
        int64_data[i] = lcg_rand() % 1000;
    }
    
    uint64_t checksum = 0;
    
    /* Conditional execution based on CPU features */
    if (__builtin_cpu_supports("sse2")) {
        #ifdef __SSE2__
        core2_optimized_compute(int_data, sizeof(int_data)/sizeof(int_data[0]));
        #endif
        
        /* Access with different strides to hint at cache usage */
        for (int stride = 1; stride <= 64; stride *= 2) {
            for (size_t i = 0; i < sizeof(int_data)/sizeof(int_data[0]); i += stride) {
                checksum += int_data[i];
            }
        }
    }
    
    if (__builtin_cpu_supports("sse4.2")) {
        #ifdef __SSE4_2__
        nehalem_optimized_compute(float_data, sizeof(float_data)/sizeof(float_data[0]));
        #endif
        
        /* Histogram computation */
        int histogram[256] = {0};
        for (size_t i = 0; i < sizeof(float_data)/sizeof(float_data[0]); i++) {
            int bucket = ((int)float_data[i]) & 0xFF;
            histogram[bucket]++;
            checksum += histogram[bucket];
        }
    }
    
    if (__builtin_cpu_supports("avx")) {
        #ifdef __AVX__
        sandybridge_optimized_compute(double_data, sizeof(double_data)/sizeof(double_data[0]));
        #endif
        
        /* Matrix multiplication pattern */
        const size_t n = 64;
        for (size_t i = 0; i < n; i++) {
            for (size_t k = 0; k < n; k++) {
                for (size_t j = 0; j < n; j++) {
                    checksum += (uint64_t)(double_data[i * n + k] * double_data[k * n + j]);
                }
            }
        }
    }
    
    if (__builtin_cpu_supports("avx2")) {
        #ifdef __AVX2__
        skylake_optimized_compute(int64_data, sizeof(int64_data)/sizeof(int64_data[0]));
        #endif
        
        /* Reduction with prefetching */
        for (size_t i = 0; i < sizeof(int64_data)/sizeof(int64_data[0]); i += 8) {
            __builtin_prefetch(&int64_data[i + 32], 0, 1);
            for (size_t j = 0; j < 8 && (i + j) < sizeof(int64_data)/sizeof(int64_data[0]); j++) {
                checksum += int64_data[i + j];
            }
        }
    }
    
    /* Additional CPUID calls for various cache descriptor leaves */
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 0 - get vendor string and max leaf */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    checksum += eax; /* max leaf */
    
    /* CPUID leaf 1 - get feature bits and cache info */
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    checksum += (eax >> 8) & 0xF; /* family */
    checksum += (eax >> 4) & 0xF; /* model */
    
    /* Force use of checksum to prevent dead code elimination */
    volatile uint64_t final_result = checksum;
    (void)final_result;
    
    #ifdef __OPTIMIZE__
    printf("Optimization level: O%d\n", 
    #ifdef __OPTIMIZE__
    #ifdef __OPTIMIZE_SIZE__
    1 /* Os */
    #else
    #ifdef __NO_INLINE__
    1 /* O1 */
    #else
    #ifdef __OPTIMIZE__
    2 /* O2 */
    #endif
    #endif
    #endif
    #else
    0
    #endif
    );
    #endif
}

/* Main function */
int main(void) {
    printf("Starting cache detection test...\n");
    
    /* Call computation function multiple times */
    for (int i = 0; i < 3; i++) {
        perform_computations();
    }
    
    printf("Test completed.\n");
    return 0;
}
