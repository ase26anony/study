/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#if defined(__OPTIMIZE__)
#define USE_CPUID_FEATURES 1
#else
#define USE_CPUID_FEATURES 0
#endif

/* Large static array to encourage cache consideration */
static volatile int cache_array[1024 * 1024];

/* Function pointers to prevent optimization */
typedef void (*func_ptr_t)(int*, size_t);
volatile func_ptr_t current_func = NULL;

/* Different access patterns based on CPU features */
void stride_1_access(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 1) {
        sum += arr[i];
    }
    printf("Stride 1 sum: %d\n", sum);
}

void stride_2_access(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 2) {
        sum += arr[i];
    }
    printf("Stride 2 sum: %d\n", sum);
}

void stride_4_access(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 4) {
        sum += arr[i];
    }
    printf("Stride 4 sum: %d\n", sum);
}

void stride_8_access(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 8) {
        sum += arr[i];
    }
    printf("Stride 8 sum: %d\n", sum);
}

void stride_16_access(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 16) {
        sum += arr[i];
    }
    printf("Stride 16 sum: %d\n", sum);
}

/* Conditional compilation blocks that force driver to evaluate CPUID */
#if USE_CPUID_FEATURES
/* These conditionals require driver to check CPU features */
#if defined(__SSE2__) || __builtin_cpu_supports("sse2")
#define HAS_SSE2 1
#else
#define HAS_SSE2 0
#endif

#if defined(__SSE3__) || __builtin_cpu_supports("sse3")
#define HAS_SSE3 1
#else
#define HAS_SSE3 0
#endif

#if defined(__SSSE3__) || __builtin_cpu_supports("ssse3")
#define HAS_SSSE3 1
#else
#define HAS_SSSE3 0
#endif

#if defined(__SSE4_1__) || __builtin_cpu_supports("sse4.1")
#define HAS_SSE4_1 1
#else
#define HAS_SSE4_1 0
#endif

#if defined(__SSE4_2__) || __builtin_cpu_supports("sse4.2")
#define HAS_SSE4_2 1
#else
#define HAS_SSE4_2 0
#endif

#if defined(__AVX__) || __builtin_cpu_supports("avx")
#define HAS_AVX 1
#else
#define HAS_AVX 0
#endif

#if defined(__AVX2__) || __builtin_cpu_supports("avx2")
#define HAS_AVX2 1
#else
#define HAS_AVX2 0
#endif

#if defined(__AVX512F__) || __builtin_cpu_supports("avx512f")
#define HAS_AVX512 1
#else
#define HAS_AVX512 0
#endif

/* Check for specific CPU families that have different cache layouts */
#if defined(__pentium4__) || defined(__core2__) || defined(__nehalem__) || \
    defined(__sandybridge__) || defined(__haswell__) || defined(__skylake__) || \
    defined(__znver1__) || defined(__znver2__) || defined(__znver3__)
#define HAS_KNOWN_UARCH 1
#else
#define HAS_KNOWN_UARCH 0
#endif

#endif /* USE_CPUID_FEATURES */

int main(void) {
    /* Initialize CPU detection - forces driver to run CPUID */
    __builtin_cpu_init();
    
    /* Volatile variables to prevent optimization */
    volatile int has_sse2 = 0;
    volatile int has_sse3 = 0;
    volatile int has_ssse3 = 0;
    volatile int has_sse41 = 0;
    volatile int has_sse42 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512 = 0;
    
    /* Check CPU features - each call may trigger cache detection */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_sse3 = __builtin_cpu_supports("sse3");
    has_ssse3 = __builtin_cpu_supports("ssse3");
    has_sse41 = __builtin_cpu_supports("sse4.1");
    has_sse42 = __builtin_cpu_supports("sse4.2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512 = __builtin_cpu_supports("avx512f");
    
    /* Initialize array with pseudo-random data */
    for (size_t i = 0; i < sizeof(cache_array)/sizeof(cache_array[0]); i++) {
        cache_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Choose access pattern based on detected features */
    size_t array_size = sizeof(cache_array)/sizeof(cache_array[0]);
    
    if (has_avx512) {
        current_func = stride_16_access;  /* AVX512 prefers larger strides */
    } else if (has_avx2) {
        current_func = stride_8_access;   /* AVX2 benefits from 256-bit accesses */
    } else if (has_avx) {
        current_func = stride_8_access;
    } else if (has_sse42) {
        current_func = stride_4_access;   /* SSE4.2 with 128-bit registers */
    } else if (has_sse41) {
        current_func = stride_4_access;
    } else if (has_ssse3) {
        current_func = stride_2_access;
    } else if (has_sse3) {
        current_func = stride_2_access;
    } else if (has_sse2) {
        current_func = stride_1_access;
    } else {
        current_func = stride_1_access;   /* Fallback */
    }
    
    /* Execute the chosen access pattern */
    if (current_func) {
        current_func((int*)cache_array, array_size);
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (size_t i = 0; i < array_size; i += 64) {  /* Cache line sized steps */
        checksum += cache_array[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Additional CPUID checks for various cache-related features */
    volatile int has_popcnt = __builtin_cpu_supports("popcnt");
    volatile int has_lzcnt = __builtin_cpu_supports("lzcnt");
    volatile int has_bmi = __builtin_cpu_supports("bmi");
    volatile int has_bmi2 = __builtin_cpu_supports("bmi2");
    volatile int has_fma = __builtin_cpu_supports("fma");
    volatile int has_fma4 = __builtin_cpu_supports("fma4");
    volatile int has_xop = __builtin_cpu_supports("xop");
    
    printf("CPU Features detected: SSE2=%d, SSE3=%d, AVX=%d, AVX2=%d, AVX512=%d\n",
           has_sse2, has_sse3, has_avx, has_avx2, has_avx512);
    
    return 0;
}
