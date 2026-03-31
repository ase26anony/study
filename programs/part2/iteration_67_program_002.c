/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
/* These macros reference builtins that force CPU detection */
#define FORCE_CPUID_INIT() __builtin_cpu_init()
#define CHECK_FEATURE(x) __builtin_cpu_supports(x)
#else
#define FORCE_CPUID_INIT() 
#define CHECK_FEATURE(x) 0
#endif

/* Large array to encourage cache consideration */
static volatile int data_array[1024 * 1024];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int stride, int iterations);
static compute_func_t compute_func = NULL;

/* Different computation patterns based on CPU features */
static int compute_sse2(int stride, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        data_array[i & 0xFFFFF] = i * 3 + 1;
        sum += data_array[i & 0xFFFFF];
    }
    return sum;
}

static int compute_avx(int stride, int iterations) {
    int sum = 0;
    /* Different access pattern */
    for (int i = 0; i < iterations; i += stride) {
        int idx = (i * 7) & 0xFFFFF;
        data_array[idx] = i * 5 - 2;
        sum += data_array[idx];
    }
    return sum;
}

static int compute_avx2(int stride, int iterations) {
    int sum = 0;
    /* Yet another pattern */
    for (int i = 0; i < iterations; i += stride) {
        int idx = (i * 11) & 0xFFFFF;
        data_array[idx] = i * 9 + 3;
        sum ^= data_array[idx]; /* XOR instead of add */
    }
    return sum;
}

static int compute_avx512(int stride, int iterations) {
    int sum = 0;
    /* More complex pattern */
    for (int i = 0; i < iterations; i += stride) {
        int idx = (i * 13) & 0xFFFFF;
        data_array[idx] = (i << 3) | (i >> 5);
        sum = (sum << 1) | (sum >> 31);
        sum += data_array[idx];
    }
    return sum;
}

/* Conditional compilation based on CPU features */
#ifdef __SSE2__
static void setup_sse2_path(void) {
    volatile int has_sse2 = CHECK_FEATURE("sse2");
    if (has_sse2) {
        compute_func = compute_sse2;
    }
}
#endif

#ifdef __AVX__
static void setup_avx_path(void) {
    volatile int has_avx = CHECK_FEATURE("avx");
    if (has_avx) {
        compute_func = compute_avx;
    }
}
#endif

#ifdef __AVX2__
static void setup_avx2_path(void) {
    volatile int has_avx2 = CHECK_FEATURE("avx2");
    if (has_avx2) {
        compute_func = compute_avx2;
    }
}
#endif

#ifdef __AVX512F__
static void setup_avx512_path(void) {
    volatile int has_avx512f = CHECK_FEATURE("avx512f");
    if (has_avx512f) {
        compute_func = compute_avx512;
    }
}
#endif

/* Cache-sensitive memory access patterns */
static int cache_sensitive_operation(int base_stride) {
    volatile int result = 0;
    const int iterations = 1000000;
    
    /* Try different strides to exercise different cache behaviors */
    int strides[] = {1, 2, 4, 8, 16, 32, 64, 128};
    int num_strides = sizeof(strides) / sizeof(strides[0]);
    
    for (int s = 0; s < num_strides; s++) {
        int stride = strides[s] * base_stride;
        if (compute_func) {
            result += compute_func(stride, iterations);
        } else {
            /* Default fallback */
            for (int i = 0; i < iterations; i += stride) {
                data_array[i & 0xFFFFF] = i * 2 + 1;
                result += data_array[i & 0xFFFFF];
            }
        }
    }
    
    return result & 0xFF; /* Return small checksum */
}

int main(void) {
    /* Initialize CPU detection - forces driver to execute CPUID */
    FORCE_CPUID_INIT();
    
    /* Check multiple CPU features to ensure thorough CPUID evaluation */
    volatile int feature_mask = 0;
    
    /* Each of these calls forces CPUID evaluation in the driver */
    feature_mask |= CHECK_FEATURE("sse") ? 1 : 0;
    feature_mask |= CHECK_FEATURE("sse2") ? 2 : 0;
    feature_mask |= CHECK_FEATURE("sse3") ? 4 : 0;
    feature_mask |= CHECK_FEATURE("ssse3") ? 8 : 0;
    feature_mask |= CHECK_FEATURE("sse4.1") ? 16 : 0;
    feature_mask |= CHECK_FEATURE("sse4.2") ? 32 : 0;
    feature_mask |= CHECK_FEATURE("avx") ? 64 : 0;
    feature_mask |= CHECK_FEATURE("avx2") ? 128 : 0;
    feature_mask |= CHECK_FEATURE("avx512f") ? 256 : 0;
    
    /* Setup computation based on available features */
#ifdef __SSE2__
    setup_sse2_path();
#endif
#ifdef __AVX__
    setup_avx_path();
#endif
#ifdef __AVX2__
    setup_avx2_path();
#endif
#ifdef __AVX512F__
    setup_avx512_path();
#endif
    
    /* Perform cache-sensitive operations */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += cache_sensitive_operation(i + 1);
        checksum &= 0xFFFF;
    }
    
    printf("CPU Feature Mask: 0x%03x\n", feature_mask);
    printf("Computed Checksum: 0x%04x\n", checksum);
    
    /* Use checksum to prevent dead code elimination */
    if (checksum == 0x1234) { /* Never true, but compiler doesn't know */
        printf("Impossible branch\n");
    }
    
    return 0;
}
