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
    *(volatile int*)&arr[0] = sum;
}

void stride_2_access(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 2) {
        sum += arr[i];
    }
    *(volatile int*)&arr[0] = sum;
}

void stride_4_access(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 4) {
        sum += arr[i];
    }
    *(volatile int*)&arr[0] = sum;
}

void stride_8_access(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 8) {
        sum += arr[i];
    }
    *(volatile int*)&arr[0] = sum;
}

void stride_16_access(int* arr, size_t size) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += 16) {
        sum += arr[i];
    }
    *(volatile int*)&arr[0] = sum;
}

/* Conditional compilation blocks that force driver to evaluate CPUID */
#if USE_CPUID_FEATURES && (defined(__i386__) || defined(__x86_64__))
/* This section forces the driver to initialize CPU detection */
static volatile int cpu_features_detected = 0;

/* Check multiple CPU features to ensure thorough CPUID execution */
static void detect_cpu_features(void) {
    /* These builtins force CPUID execution in the driver */
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Also check cache-related features */
    volatile int has_popcnt = __builtin_cpu_supports("popcnt");
    volatile int has_lzcnt = __builtin_cpu_supports("lzcnt");
    volatile int has_bmi = __builtin_cpu_supports("bmi");
    volatile int has_bmi2 = __builtin_cpu_supports("bmi2");
    
    cpu_features_detected = has_sse | has_sse2 | has_sse3 | has_ssse3 |
                           has_sse4_1 | has_sse4_2 | has_avx | has_avx2 |
                           has_avx512f | has_popcnt | has_lzcnt | has_bmi | has_bmi2;
}
#endif

int main(void) {
    size_t array_size = sizeof(cache_array) / sizeof(cache_array[0]);
    volatile int checksum = 0;
    
    /* Initialize CPU detection - this triggers driver's CPUID logic */
#if defined(__i386__) || defined(__x86_64__)
    __builtin_cpu_init();
    
#if USE_CPUID_FEATURES
    detect_cpu_features();
    
    /* Select access pattern based on CPU features */
    if (__builtin_cpu_supports("avx512f")) {
        current_func = stride_16_access;  /* Wider stride for AVX-512 */
    } else if (__builtin_cpu_supports("avx2")) {
        current_func = stride_8_access;   /* Medium stride for AVX2 */
    } else if (__builtin_cpu_supports("sse4.2")) {
        current_func = stride_4_access;   /* SSE4.2 gets stride 4 */
    } else if (__builtin_cpu_supports("sse2")) {
        current_func = stride_2_access;   /* SSE2 gets stride 2 */
    } else {
        current_func = stride_1_access;   /* Baseline gets stride 1 */
    }
#else
    current_func = stride_1_access;
#endif
    
    /* Execute the selected access pattern */
    if (current_func) {
        current_func((int*)cache_array, array_size);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (size_t i = 0; i < array_size && i < 1024; i += 64) {
        checksum += cache_array[i];
    }
    
    /* Use checksum in a way that can't be optimized away */
    printf("CPU Cache Test - Checksum: %d\n", checksum);
    
    /* Additional CPU feature reporting to ensure builtins are evaluated */
#if USE_CPUID_FEATURES
    printf("CPU Features detected: %d\n", cpu_features_detected);
#endif
    
#else
    /* Non-x86 fallback */
    printf("This test is for x86/x86-64 architectures only.\n");
#endif
    
    return checksum != 0 ? 0 : 1;
}

/* Force inclusion of math functions that might trigger cache considerations */
double compute_pi_approximation(void) {
    volatile double pi = 0.0;
    for (int k = 0; k < 1000; k++) {
        pi += (k % 2 == 0 ? 1.0 : -1.0) / (2 * k + 1);
    }
    return pi * 4;
}

/* Another function with different access pattern */
void matrix_multiply_pattern(void) {
    volatile int matrix[64][64];
    volatile int result = 0;
    
    /* Cache-sensitive access pattern */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            for (int k = 0; k < 64; k++) {
                matrix[i][j] += matrix[i][k] * matrix[k][j];
            }
            result += matrix[i][j];
        }
    }
    
    *(volatile int*)&matrix[0][0] = result;
}
