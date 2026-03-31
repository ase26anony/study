/* 
 * Vectorization test program targeting GCC's default_builtin_vectorized_function
 * Specifically designed to trigger flag-setting on compiler-generated built-in declarations
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Architecture-specific intrinsics */
#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#include <cpuid.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* Alignment hints for vectorization */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to influence declaration handling */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Data arrays with explicit alignment */
static float arr_f[1024] ALIGN_32;
static double arr_d[1024] ALIGN_64;
static int arr_i[1024] ALIGN_32;
static char str_buf[2048] ALIGN_32;

/* Hidden visibility helper function with vectorizable math operations */
HIDDEN_VIS USED_FUNC NOTHROW_FUNC
static void process_hidden_math(float *out, const float *in, int n) {
    /* This function's hidden visibility may interact with the flag-setting logic */
    #pragma omp simd simdlen(8)
    for (int i = 0; i < n; i++) {
        /* Multiple built-in calls to increase vectorization opportunities */
        out[i] = sinf(in[i]) * cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Always-inline function with exp/log operations */
ALWAYS_INLINE static inline void process_exp_log(double *out, const double *in, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]) * log(fabs(in[i]) + 1.0);
    }
}

/* Function using __builtin_memcpy in vectorizable context */
static void builtin_memcpy_test(char *dst, const char *src, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i += 64) {
        /* Vectorized memory copy - may trigger builtin vectorization */
        __builtin_memcpy(dst + i, src + i, 64);
    }
}

/* Function with __builtin_strlen in loop */
static int builtin_strlen_test(const char *str, int chunks) {
    int total = 0;
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < chunks; i++) {
        total += __builtin_strlen(str + i * 16);
    }
    return total;
}

/* SIMD function declaration for OpenMP */
#pragma omp declare simd uniform(a, n) linear(i)
double simd_pow(double a, int i, int n);

/* SIMD variant function containing built-in call */
#pragma omp declare simd
double simd_pow(double a, int i, int n) {
    return pow(a, i % 8 + 1);
}

/* Conditional architecture-specific path */
static void conditional_arch_operations(float *data, int n, int use_vector) {
    if (use_vector) {
#ifdef __x86_64__
        /* Check CPU support at compile-time/runtime */
        if (__builtin_cpu_supports("avx2")) {
            /* AVX2 intrinsic path - compiler may still analyze scalar fallback */
            for (int i = 0; i < n; i += 8) {
                __m256 vec = _mm256_load_ps(&data[i]);
                __m256 result = _mm256_sqrt_ps(vec);
                _mm256_store_ps(&data[i], result);
            }
            return;
        }
#endif
        
#ifdef __aarch64__
        /* ARM NEON path */
        for (int i = 0; i < n; i += 4) {
            float32x4_t vec = vld1q_f32(&data[i]);
            /* Note: ARM doesn't have direct sqrt intrinsic, use builtin */
            for (int j = 0; j < 4; j++) {
                data[i + j] = sqrtf(data[i + j]);
            }
        }
        return;
#endif
    }
    
    /* Scalar fallback path - contains built-in calls for vectorization */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        data[i] = sqrtf(data[i]) + sinf(data[i]);
    }
}

/* Dead code path with vectorizable built-ins (still processed by front-end) */
static void dead_code_path(void) {
    if (0) {  /* Dead code, but still parsed */
        double dead_arr[128] ALIGN_32;
        #pragma omp simd
        for (int i = 0; i < 128; i++) {
            dead_arr[i] = __builtin_sqrt(dead_arr[i]) * __builtin_exp(dead_arr[i]);
        }
        
        /* Type-punning union for potential built-in vectorization */
        union {
            __m128i vec;
            int scalars[4];
        } pun ALIGN_32;
        
        __builtin_memcpy(&pun.vec, dead_arr, sizeof(__m128i));
    }
}

/* Multiple small functions with different built-ins */
static ALWAYS_INLINE void func_sin(float *out, const float *in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);
    }
}

static ALWAYS_INLINE void func_cos(float *out, const float *in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);
    }
}

static ALWAYS_INLINE void func_tan(float *out, const float *in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = tanf(in[i]);
    }
}

/* Switch between different vectorizable functions */
static void switch_builtins(int choice, float *out, const float *in, int n) {
    switch (choice) {
        case 0:
            func_sin(out, in, n);
            break;
        case 1:
            func_cos(out, in, n);
            break;
        case 2:
            func_tan(out, in, n);
            break;
        default:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = sinf(in[i]) * cosf(in[i]);
            }
    }
}

/* OpenMP parallel region with SIMD reduction */
static double parallel_simd_reduction(const double *data, int n) {
    double sum = 0.0;
    
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += sin(data[i]) * cos(data[i]);
    }
    
    return sum;
}

/* Initialize arrays with pattern data */
static void init_arrays(void) {
    for (int i = 0; i < 1024; i++) {
        arr_f[i] = (i % 256) * 0.01f;
        arr_d[i] = (i % 128) * 0.02;
        arr_i[i] = i;
    }
    
    /* Fill string buffer with patterns */
    for (int i = 0; i < 2048; i++) {
        str_buf[i] = 'A' + (i % 26);
    }
    str_buf[2047] = '\0';
    
    /* Create null-terminated substrings */
    for (int i = 0; i < 128; i++) {
        str_buf[i * 16 + 15] = '\0';
    }
}

/* Main test driver */
int main(void) {
    float out_f[1024] ALIGN_32;
    double out_d[1024] ALIGN_64;
    char copy_buf[2048] ALIGN_32;
    
    init_arrays();
    
    double total = 0.0;
    
    /* Test 1: Hidden visibility math function */
    process_hidden_math(out_f, arr_f, 1024);
    for (int i = 0; i < 64; i++) total += out_f[i];
    
    /* Test 2: Exp/log operations */
    process_exp_log(out_d, arr_d, 1024);
    for (int i = 0; i < 64; i++) total += out_d[i];
    
    /* Test 3: Built-in memcpy vectorization */
    builtin_memcpy_test(copy_buf, str_buf, 2048);
    total += copy_buf[100];
    
    /* Test 4: Built-in strlen in SIMD loop */
    int len_total = builtin_strlen_test(str_buf, 128);
    total += len_total;
    
    /* Test 5: Conditional architecture paths */
    conditional_arch_operations(out_f, 1024, 1);
    for (int i = 0; i < 64; i++) total += out_f[i];
    
    /* Test 6: Switch between different built-in functions */
    for (int choice = 0; choice < 4; choice++) {
        switch_builtins(choice, out_f, arr_f, 256);
        total += out_f[choice];
    }
    
    /* Test 7: OpenMP parallel SIMD reduction */
    total += parallel_simd_reduction(arr_d, 1024);
    
    /* Test 8: SIMD function variant */
    #pragma omp simd
    for (int i = 0; i < 256; i++) {
        out_d[i] = simd_pow(2.0, i, 256);
        total += out_d[i];
    }
    
    /* Test 9: Integer built-ins */
    #pragma omp simd
    for (int i = 0; i < 1024; i++) {
        arr_i[i] = __builtin_abs(arr_i[i] - 512);
        total += arr_i[i];
    }
    
    /* Ensure dead code path is compiled (though not executed) */
    dead_code_path();
    
    printf("Result: %f\n", total);
    return 0;
}
