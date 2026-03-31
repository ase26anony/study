/* test_vector_builtin.c
 * This program triggers GCC's vectorization of built-in math functions,
 * causing the target hook default_builtin_vectorized_function to be called.
 * The uncovered lines set flags on the vectorized function declaration.
 */

#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float in[256] __attribute__((aligned(32)));
float out_sin[256] __attribute__((aligned(32)));
float out_cos[256] __attribute__((aligned(32)));
float out_exp[256] __attribute__((aligned(32)));
float out_sqrt[256] __attribute__((aligned(32)));
float out_log[256] __attribute__((aligned(32)));

/* Prevent inlining and enable specific optimizations */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(float *restrict in_array, float *restrict out_array, int n) {
    for (int i = 0; i < n; i++) {
        out_array[i] = sinf(in_array[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(float *restrict in_array, float *restrict out_array, int n) {
    for (int i = 0; i < n; i++) {
        out_array[i] = cosf(in_array[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(float *restrict in_array, float *restrict out_array, int n) {
    for (int i = 0; i < n; i++) {
        out_array[i] = expf(in_array[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in_array, float *restrict out_array, int n) {
    for (int i = 0; i < n; i++) {
        out_array[i] = sqrtf(fabsf(in_array[i]) + 0.1f); /* Ensure positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in_array, float *restrict out_array, int n) {
    for (int i = 0; i < n; i++) {
        out_array[i] = logf(fabsf(in_array[i]) + 1.0f); /* Ensure positive input */
    }
}

/* Double precision versions to trigger different vectorization */
double in_double[256] __attribute__((aligned(32)));
double out_sin_double[256] __attribute__((aligned(32)));

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_double(double *restrict in_array, double *restrict out_array, int n) {
    for (int i = 0; i < n; i++) {
        out_array[i] = sin(in_array[i]);
    }
}

int main(void) {
    const int n = 256;
    float checksum = 0.0f;
    
    /* Initialize input arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in[i] = (i * 0.1f) - 12.8f; /* Range that exercises sin/cos well */
        in_double[i] = (i * 0.1) - 12.8;
    }
    
    /* Call vectorized functions */
    test_vector_sin(in, out_sin, n);
    test_vector_cos(in, out_cos, n);
    test_vector_exp(in, out_exp, n);
    test_vector_sqrt(in, out_sqrt, n);
    test_vector_log(in, out_log, n);
    test_vector_sin_double(in_double, out_sin_double, n);
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < n; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + (float)out_sin_double[i];
    }
    
    /* Use volatile global to ensure computations aren't eliminated */
    global_sink = checksum;
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
