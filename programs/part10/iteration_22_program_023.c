/* test_vector_builtin.c
 * Test program to trigger vectorization of built-in math functions
 * and cover the default_builtin_vectorized_function hook in targhooks.cc
 */

#include <stdio.h>
#include <math.h>

/* Prevent optimization from removing our loops */
volatile float global_sum = 0.0f;
volatile double global_sum_d = 0.0;

/* Aligned arrays to help vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Force noinline and enable vectorization optimizations per function */
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
        out[i] = logf(in[i] + 1.0f); /* +1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrtf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(fabsf(in[i]) + 0.1f); /* Ensure positive input */
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

/* Mixed operations to trigger multiple vectorized builtins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain multiple builtins to increase coverage chance */
        float val = sinf(in[i]);
        val = cosf(val);
        val = expf(val);
        out[i] = sqrtf(fabsf(val) + 0.1f);
    }
}

int main(void) {
    const int n = 256;
    
    /* Initialize input arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in_f[i] = (i * 0.1f) - 12.8f; /* Range: -12.8 to 12.7 */
        in_d[i] = (i * 0.1) - 12.8;   /* Same range for doubles */
    }
    
    /* Test various float builtins */
    test_vector_sinf(in_f, out_f, n);
    test_vector_cosf(in_f, out_f, n);
    test_vector_expf(in_f, out_f, n);
    test_vector_logf(in_f, out_f, n);
    test_vector_sqrtf(in_f, out_f, n);
    test_vector_powf(in_f, out_f, n);
    test_vector_mixed(in_f, out_f, n);
    
    /* Test double precision builtins */
    test_vector_sin(in_d, out_d, n);
    test_vector_cos(in_d, out_d, n);
    test_vector_exp(in_d, out_d, n);
    
    /* Compute checksums to prevent dead code elimination */
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    for (int i = 0; i < n; i++) {
        sum_f += out_f[i];
        sum_d += out_d[i];
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    global_sum = sum_f;
    global_sum_d = sum_d;
    
    printf("Float checksum: %f\n", sum_f);
    printf("Double checksum: %lf\n", sum_d);
    
    return 0;
}
