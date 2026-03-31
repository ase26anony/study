/* 
 * Test program designed to trigger GCC's default_builtin_vectorized_function
 * and specifically execute the flag-setting block in targhooks.cc (lines 981-990)
 * Compile with: g++ -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -fno-builtin -o test_vector test_vector.cc
 * Additional flags for coverage: -fopt-info-vec-missed -fdump-tree-vect-details
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>

#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* Alignment hints to assist vectorizer */
#define ALIGN_32 __attribute__((aligned(32)))

/* Function attributes to interact with declaration handling */
static inline float test_sinf(float x) __attribute__((always_inline));
static void process_math(float* ALIGN_32 arr, int n) __attribute__((visibility("hidden"), used, nothrow));
static void vector_memcpy_test(char* ALIGN_32 dst, const char* ALIGN_32 src, int n) __attribute__((used));

/* Simple random generator to prevent compile-time computation */
static float simple_rand(int seed) {
    static unsigned int state = 0;
    if (seed) state = seed;
    state = state * 1103515245 + 12345;
    return (float)(state % 1000) / 1000.0f;
}

/* 
 * Function 1: Math-intensive loop with OpenMP SIMD directive
 * Triggers vectorization of sinf/cosf builtins
 */
#pragma omp declare simd
static inline float test_sinf(float x) {
    return sinf(x) + cosf(x);  /* Both should be considered for vectorization */
}

static void process_math(float* ALIGN_32 arr, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = test_sinf(arr[i]) * sqrtf(arr[i] + 1.0f);
        /* Multiple builtins: sinf, cosf, sqrtf */
    }
}

/* 
 * Function 2: Memory operations with __builtin_memcpy in vectorizable loop
 */
static void vector_memcpy_test(char* ALIGN_32 dst, const char* ALIGN_32 src, int n) {
    const int block = 32;
    #pragma GCC ivdep
    for (int i = 0; i < n; i += block) {
        int size = (n - i) > block ? block : (n - i);
        __builtin_memcpy(dst + i, src + i, size);
    }
}

/* 
 * Function 3: Conditional path with architecture-specific intrinsics
 * Forces compiler to analyze both vector and scalar paths
 */
static double conditional_vector_path(double* ALIGN_32 arr, int n) {
    double sum = 0.0;
    
    if (__builtin_cpu_supports("avx2")) {
        /* Vector path - compiler may create vectorized builtin declarations */
        #ifdef __AVX2__
        for (int i = 0; i < n; i += 4) {
            __m256d vec = _mm256_load_pd(&arr[i]);
            __m256d result = _mm256_sqrt_pd(vec);
            _mm256_store_pd(&arr[i], result);
            sum += arr[i] + arr[i+1] + arr[i+2] + arr[i+3];
        }
        #endif
    } else {
        /* Scalar fallback - still vectorizable builtins */
        #pragma omp simd reduction(+:sum)
        for (int i = 0; i < n; i++) {
            arr[i] = sqrt(arr[i]);  /* Builtin sqrt */
            sum += arr[i];
        }
    }
    return sum;
}

/* 
 * Function 4: Hidden visibility helper with mixed math builtins
 */
__attribute__((visibility("hidden"), used))
static double hidden_visibility_helper(double* ALIGN_32 arr, int n) {
    double result = 0.0;
    
    #pragma omp parallel for simd reduction(+:result)
    for (int i = 0; i < n; i++) {
        arr[i] = exp(arr[i]) + log(fabs(arr[i]) + 1.0);
        /* Builtins: exp, log, fabs */
        result += arr[i];
    }
    
    return result;
}

/* 
 * Function 5: Dead code path containing vectorizable builtins
 * Frontend may still process declarations even if code is unreachable
 */
static void dead_code_path(float* ALIGN_32 arr, int n) {
    if (0) {  /* Dead code, but still parsed */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            arr[i] = powf(arr[i], 2.0f) + expf(arr[i]);
            /* Builtins: powf, expf */
        }
    }
}

/* 
 * Function 6: Type-punning with unions and memcpy between types
 */
static void type_punning_test(float* ALIGN_32 farr, int* ALIGN_32 iarr, int n) {
    union {
        float f;
        int i;
    } converter;
    
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        /* Use __builtin_memcpy for type punning */
        converter.f = farr[i];
        __builtin_memcpy(&iarr[i], &converter.i, sizeof(int));
        
        /* Use builtin for bit operations */
        iarr[i] = __builtin_ilogb(farr[i]) + __builtin_popcount(iarr[i]);
    }
}

/* 
 * Function 7: Switch statement with multiple vectorization candidates
 */
static double switch_vectorization(int mode, double* ALIGN_32 arr, int n) {
    double sum = 0.0;
    
    switch (mode) {
        case 0:
            #pragma omp simd reduction(+:sum)
            for (int i = 0; i < n; i++) {
                arr[i] = sin(arr[i]) * cos(arr[i]);
                sum += arr[i];
            }
            break;
            
        case 1:
            #pragma omp simd reduction(+:sum)
            for (int i = 0; i < n; i++) {
                arr[i] = tan(arr[i]) + atan(arr[i]);
                sum += arr[i];
            }
            break;
            
        case 2:
            #pragma omp simd reduction(+:sum)
            for (int i = 0; i < n; i++) {
                arr[i] = __builtin_sqrt(arr[i]) * __builtin_exp(arr[i]);
                sum += arr[i];
            }
            break;
            
        default:
            #pragma omp simd reduction(+:sum)
            for (int i = 0; i < n; i++) {
                arr[i] = __builtin_log(arr[i] + 1.0);
                sum += arr[i];
            }
    }
    
    return sum;
}

/* Main function orchestrates all tests */
int main() {
    const int N = 1024;
    const int M = 512;
    
    /* Aligned arrays for vectorization */
    float* farr = (float*)aligned_alloc(32, N * sizeof(float));
    double* darr = (double*)aligned_alloc(32, N * sizeof(double));
    char* src = (char*)aligned_alloc(32, M * sizeof(char));
    char* dst = (char*)aligned_alloc(32, M * sizeof(char));
    int* iarr = (int*)aligned_alloc(32, N * sizeof(int));
    
    /* Initialize with pattern data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        farr[i] = simple_rand(i) * 3.14159f;
        darr[i] = simple_rand(i + N) * 2.0;
        iarr[i] = rand() % 1000;
    }
    
    for (int i = 0; i < M; i++) {
        src[i] = 'A' + (i % 26);
    }
    
    double total_sum = 0.0;
    
    /* Test 1: Math function vectorization */
    printf("Test 1: Processing math functions...\n");
    process_math(farr, N);
    total_sum += farr[0];
    
    /* Test 2: Memory builtin vectorization */
    printf("Test 2: Vector memcpy test...\n");
    vector_memcpy_test(dst, src, M);
    total_sum += dst[0];
    
    /* Test 3: Conditional vector path */
    printf("Test 3: Conditional vector path...\n");
    total_sum += conditional_vector_path(darr, N);
    
    /* Test 4: Hidden visibility helper */
    printf("Test 4: Hidden visibility helper...\n");
    total_sum += hidden_visibility_helper(darr, N);
    
    /* Test 5: Dead code path (shouldn't execute but still parsed) */
    printf("Test 5: Dead code path...\n");
    dead_code_path(farr, N);
    total_sum += farr[0];
    
    /* Test 6: Type punning with builtins */
    printf("Test 6: Type punning test...\n");
    type_punning_test(farr, iarr, N);
    total_sum += iarr[0];
    
    /* Test 7: Switch with multiple vectorization candidates */
    printf("Test 7: Switch vectorization...\n");
    for (int mode = 0; mode < 4; mode++) {
        total_sum += switch_vectorization(mode, darr, N);
    }
    
    /* Additional test: strlen in vectorizable context */
    printf("Additional test: strlen vectorization...\n");
    int len_sum = 0;
    const char* strings[] = {"test1", "vectorization", "builtin", "function"};
    #pragma omp simd reduction(+:len_sum)
    for (int i = 0; i < 4; i++) {
        len_sum += __builtin_strlen(strings[i]);
    }
    total_sum += len_sum;
    
    /* Prevent dead code elimination */
    printf("Total sum (prevents optimization): %f\n", total_sum);
    
    /* Cleanup */
    free(farr);
    free(darr);
    free(src);
    free(dst);
    free(iarr);
    
    return 0;
}
