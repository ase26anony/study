/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
#define USE_CPU_FEATURES 1
#else
#define USE_CPU_FEATURES 0
#endif

/* Large arrays to encourage cache consideration */
static volatile int large_array[1024 * 1024];
static volatile int medium_array[256 * 256];
static volatile int small_array[64 * 64];

/* Function pointers to prevent optimization */
typedef void (*compute_func_t)(volatile int*, size_t, int);
static compute_func_t compute_func = NULL;

/* Different computation patterns based on CPU features */
static void compute_simple(volatile int* arr, size_t size, int stride) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += stride) {
        sum += arr[i];
        arr[i] = sum & 0xFF;
    }
}

static void compute_sse_optimized(volatile int* arr, size_t size, int stride) {
    volatile int sum = 0;
    /* Simulate SSE-like access pattern */
    for (size_t i = 0; i < size; i += stride * 4) {
        sum += arr[i];
        sum += arr[i + stride];
        sum += arr[i + stride * 2];
        sum += arr[i + stride * 3];
        arr[i] = sum & 0xFF;
    }
}

static void compute_avx_optimized(volatile int* arr, size_t size, int stride) {
    volatile int sum = 0;
    /* Simulate AVX-like access pattern */
    for (size_t i = 0; i < size; i += stride * 8) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i + stride * j];
        }
        arr[i] = sum & 0xFF;
    }
}

/* Conditional compilation that forces driver to evaluate CPUID */
#if USE_CPU_FEATURES
/* These builtins force CPU detection during compilation */
#define CHECK_FEATURE(feat) (__builtin_cpu_supports(feat) ? 1 : 0)

/* Feature checks that map to different microarchitectures */
static const char* feature_checks[] = {
    "sse2",     /* Core 2, Nehalem */
    "sse4.2",   /* Nehalem, Westmere */
    "avx",      /* Sandy Bridge, Ivy Bridge */
    "avx2",     /* Haswell, Broadwell */
    "avx512f",  /* Skylake-X, Cascade Lake */
    "fma",      /* Haswell, Piledriver */
    "aes",      /* Westmere, Bulldozer */
    "popcnt",   /* Nehalem, Barcelona */
    "movbe",    /* Atom, Goldmont */
    "rdrnd",    /* Ivy Bridge, Excavator */
    NULL
};
#endif

int main(void) {
    int checksum = 0;
    
    /* Initialize CPU detection - this triggers driver's CPUID logic */
    __builtin_cpu_init();
    
#if USE_CPU_FEATURES
    /* Force evaluation of multiple CPU features */
    volatile int has_sse2 = CHECK_FEATURE("sse2");
    volatile int has_sse42 = CHECK_FEATURE("sse4.2");
    volatile int has_avx = CHECK_FEATURE("avx");
    volatile int has_avx2 = CHECK_FEATURE("avx2");
    volatile int has_avx512 = CHECK_FEATURE("avx512f");
    volatile int has_fma = CHECK_FEATURE("fma");
    volatile int has_aes = CHECK_FEATURE("aes");
    volatile int has_popcnt = CHECK_FEATURE("popcnt");
    
    /* Choose computation based on detected features */
    if (has_avx512) {
        compute_func = compute_avx_optimized;
        checksum |= 0x1000;
    } else if (has_avx2) {
        compute_func = compute_avx_optimized;
        checksum |= 0x0800;
    } else if (has_avx) {
        compute_func = compute_avx_optimized;
        checksum |= 0x0400;
    } else if (has_sse42) {
        compute_func = compute_sse_optimized;
        checksum |= 0x0200;
    } else if (has_sse2) {
        compute_func = compute_sse_optimized;
        checksum |= 0x0100;
    } else {
        compute_func = compute_simple;
    }
    
    /* Additional feature-based checksum */
    checksum |= (has_fma << 8);
    checksum |= (has_aes << 7);
    checksum |= (has_popcnt << 6);
#else
    compute_func = compute_simple;
#endif

    /* Perform computations with different strides to exercise cache logic */
    int strides[] = {1, 2, 4, 8, 16, 32, 64};
    
    for (int s = 0; s < sizeof(strides)/sizeof(strides[0]); s++) {
        if (compute_func) {
            compute_func(large_array, sizeof(large_array)/sizeof(large_array[0]), 
                        strides[s]);
            compute_func(medium_array, sizeof(medium_array)/sizeof(medium_array[0]), 
                        strides[s]);
            compute_func(small_array, sizeof(small_array)/sizeof(small_array[0]), 
                        strides[s]);
        }
        
        /* Simple checksum from array contents */
        for (int i = 0; i < 100; i += strides[s]) {
            checksum += large_array[i] + medium_array[i] + small_array[i];
        }
    }
    
    printf("CPU Feature Checksum: 0x%08x\n", checksum & 0xFFFFFFFF);
    
    /* Additional architecture-specific conditionals */
#ifdef __SSE2__
    checksum += 0x10000;
#endif
#ifdef __AVX__
    checksum += 0x20000;
#endif
#ifdef __AVX2__
    checksum += 0x40000;
#endif
#ifdef __AVX512F__
    checksum += 0x80000;
#endif
    
    return (checksum & 0xFF);
}
