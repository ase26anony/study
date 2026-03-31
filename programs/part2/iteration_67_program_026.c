/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
#define USE_CPU_BUILTINS 1
#else
#define USE_CPU_BUILTINS 0
#endif

/* Large static array to encourage cache consideration */
static volatile int data_array[1024 * 1024];

/* Function pointers to prevent optimization */
typedef void (*compute_func_t)(int*, size_t, int);
static compute_func_t func_ptr = NULL;

/* Different computation patterns based on CPU features */
void compute_basic(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += stride) {
        arr[i] = (arr[i] * 3 + 7) & 0xFF;
        sum += arr[i];
    }
    (void)sum;
}

void compute_sse_optimized(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    /* Simulate SSE-friendly pattern */
    for (size_t i = 0; i < size; i += stride) {
        arr[i] = (arr[i] << 2) | (arr[i] >> 6);
        sum ^= arr[i];
    }
    (void)sum;
}

void compute_avx_optimized(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    /* Simulate AVX-friendly pattern with wider operations */
    for (size_t i = 0; i < size; i += stride * 2) {
        arr[i] = arr[i] * arr[i] + arr[i + stride];
        sum += arr[i];
    }
    (void)sum;
}

/* Conditional compilation blocks that force driver to evaluate CPUID */
#if USE_CPU_BUILTINS
/* These conditionals are evaluated during compilation by the driver */
#if defined(__i386__) || defined(__x86_64__)
#define CHECK_CPU_FEATURES 1
#else
#define CHECK_CPU_FEATURES 0
#endif
#else
#define CHECK_CPU_FEATURES 0
#endif

int main(void) {
    volatile int use_sse = 0;
    volatile int use_avx = 0;
    volatile int use_avx2 = 0;
    volatile int use_avx512 = 0;
    volatile int use_sse4 = 0;
    volatile int use_popcnt = 0;
    volatile int use_bmi = 0;
    
    /* Initialize CPU detection - triggers driver's __builtin_cpu_init */
    __builtin_cpu_init();
    
    /* Check various CPU features - each call may trigger cache detection */
    /* The driver must evaluate these to resolve conditional compilation */
    
#if CHECK_CPU_FEATURES
    /* These builtins force driver to examine CPUID information */
    use_sse = __builtin_cpu_supports("sse");
    use_sse4 = __builtin_cpu_supports("sse4.2");
    use_avx = __builtin_cpu_supports("avx");
    use_avx2 = __builtin_cpu_supports("avx2");
    use_avx512 = __builtin_cpu_supports("avx512f");
    use_popcnt = __builtin_cpu_supports("popcnt");
    use_bmi = __builtin_cpu_supports("bmi");
    
    /* Additional architecture-specific checks */
    volatile int is_intel = __builtin_cpu_is("intel");
    volatile int is_amd = __builtin_cpu_is("amd");
    volatile int is_core2 = __builtin_cpu_is("core2");
    volatile int is_nehalem = __builtin_cpu_is("nehalem");
    volatile int is_sandybridge = __builtin_cpu_is("sandybridge");
    volatile int is_ivybridge = __builtin_cpu_is("ivybridge");
    volatile int is_haswell = __builtin_cpu_is("haswell");
    volatile int is_skylake = __builtin_cpu_is("skylake");
    volatile int is_zen = __builtin_cpu_is("znver1");
    
    (void)is_intel; (void)is_amd; (void)is_core2;
    (void)is_nehalem; (void)is_sandybridge; (void)is_ivybridge;
    (void)is_haswell; (void)is_skylake; (void)is_zen;
#endif
    
    /* Choose computation based on detected features */
    int stride = 1;
    
    if (use_avx512) {
        func_ptr = compute_avx_optimized;
        stride = 16;  /* Wider stride for AVX-512 */
    } else if (use_avx || use_avx2) {
        func_ptr = compute_avx_optimized;
        stride = 8;   /* Wider stride for AVX */
    } else if (use_sse || use_sse4) {
        func_ptr = compute_sse_optimized;
        stride = 4;   /* SSE-friendly stride */
    } else {
        func_ptr = compute_basic;
        stride = 2;   /* Basic stride */
    }
    
    /* Initialize array with pseudo-random data */
    for (size_t i = 0; i < sizeof(data_array)/sizeof(data_array[0]); i++) {
        data_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Perform computation - pattern depends on CPU features */
    size_t array_size = sizeof(data_array)/sizeof(data_array[0]);
    func_ptr((int*)data_array, array_size, stride);
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (size_t i = 0; i < array_size; i += 64) {  /* Cache line sized steps */
        checksum ^= data_array[i];
    }
    
    printf("CPU Feature Summary:\n");
    printf("  SSE: %d, SSE4.2: %d\n", use_sse, use_sse4);
    printf("  AVX: %d, AVX2: %d, AVX512: %d\n", use_avx, use_avx2, use_avx512);
    printf("  POPCNT: %d, BMI: %d\n", use_popcnt, use_bmi);
    printf("Checksum: 0x%08x\n", checksum);
    
    return 0;
}

/* Additional conditional compilation to force driver evaluation */
#if defined(__SSE__) && CHECK_CPU_FEATURES
/* This section only compiled if SSE is available */
static volatile int sse_detected = __builtin_cpu_supports("sse");
#endif

#if defined(__AVX__) && CHECK_CPU_FEATURES
/* This section only compiled if AVX is available */
static volatile int avx_detected = __builtin_cpu_supports("avx");
#endif

#if defined(__AVX2__) && CHECK_CPU_FEATURES
/* This section only compiled if AVX2 is available */
static volatile int avx2_detected = __builtin_cpu_supports("avx2");
#endif

#if defined(__AVX512F__) && CHECK_CPU_FEATURES
/* This section only compiled if AVX512F is available */
static volatile int avx512_detected = __builtin_cpu_supports("avx512f");
#endif
