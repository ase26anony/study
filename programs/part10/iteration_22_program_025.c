/* test_vector_builtin.c
 * This program triggers vectorization of built-in math functions,
 * causing GCC to call default_builtin_vectorized_function and
 * execute the flag-setting code in targhooks.cc.
 */

#include <stdio.h>
#include <math.h>

/* Global volatile variable to prevent optimization */
volatile float global_sum = 0.0f;
volatile double global_sum_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Vectorization test functions with noinline to ensure they're not optimized away */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sinf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cosf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_expf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_logf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrtf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(in[i]);
    }
}

/* Double precision versions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_powf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.0f);
    }
}

/* Function to accumulate results to prevent dead code elimination */
__attribute__((noinline))
void accumulate_results(float *arr_f, double *arr_d, int n) {
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    for (int i = 0; i < n; i++) {
        sum_f += arr_f[i];
        sum_d += arr_d[i];
    }
    
    /* Store to volatile globals */
    global_sum = sum_f;
    global_sum_d = sum_d;
}

int main(void) {
    const int n = 256;
    
    /* Initialize input arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in_f[i] = (i + 1) * 0.1f;  /* Avoid zero for log/sqrt */
        in_d[i] = (i + 1) * 0.1;
    }
    
    /* Test various built-in math functions with vectorization */
    test_vector_sinf(in_f, out_f, n);
    accumulate_results(out_f, out_d, n);
    
    test_vector_cosf(in_f, out_f, n);
    accumulate_results(out_f, out_d, n);
    
    test_vector_expf(in_f, out_f, n);
    accumulate_results(out_f, out_d, n);
    
    test_vector_logf(in_f, out_f, n);
    accumulate_results(out_f, out_d, n);
    
    test_vector_sqrtf(in_f, out_f, n);
    accumulate_results(out_f, out_d, n);
    
    test_vector_powf(in_f, out_f, n);
    accumulate_results(out_f, out_d, n);
    
    /* Double precision tests */
    test_vector_sin(in_d, out_d, n);
    accumulate_results(out_f, out_d, n);
    
    test_vector_cos(in_d, out_d, n);
    accumulate_results(out_f, out_d, n);
    
    test_vector_exp(in_d, out_d, n);
    accumulate_results(out_f, out_d, n);
    
    /* Print results to ensure computation isn't optimized away */
    printf("Float sum: %f\n", (float)global_sum);
    printf("Double sum: %lf\n", (double)global_sum_d);
    
    return 0;
}
