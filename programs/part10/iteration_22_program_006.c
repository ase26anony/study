/* test_vector_builtin.c
 * Test program to cover default_builtin_vectorized_function hook in targhooks.cc
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -fdump-tree-vect-details -c test_vector_builtin.c
 */

#include <math.h>
#include <stdio.h>

/* Prevent dead code elimination */
static volatile float global_sum = 0.0f;
static volatile double global_sum_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Vectorization test functions with noinline to prevent inlining */
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

/* Mixed operations to trigger multiple vectorized builtins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain multiple math operations */
        float val = sinf(in[i]);
        val = cosf(val);
        val = expf(val);
        out[i] = sqrtf(val);
    }
}

int main() {
    const int n = 256;
    
    /* Initialize input arrays with pattern */
    for (int i = 0; i < n; i++) {
        in_f[i] = (i + 1) * 0.1f;
        in_d[i] = (i + 1) * 0.1;
    }
    
    /* Test various vectorized built-in functions */
    test_vector_sinf(in_f, out_f, n);
    test_vector_cosf(in_f, out_f, n);
    test_vector_expf(in_f, out_f, n);
    test_vector_logf(in_f, out_f, n);
    test_vector_sqrtf(in_f, out_f, n);
    test_vector_powf(in_f, out_f, n);
    test_vector_mixed(in_f, out_f, n);
    
    test_vector_sin(in_d, out_d, n);
    test_vector_cos(in_d, out_d, n);
    test_vector_exp(in_d, out_d, n);
    
    /* Compute checksums to prevent optimization */
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    for (int i = 0; i < n; i++) {
        sum_f += out_f[i];
        sum_d += out_d[i];
    }
    
    /* Store in volatile to ensure computation isn't optimized away */
    global_sum = sum_f;
    global_sum_d = sum_d;
    
    /* Print results to ensure side effects */
    printf("Float checksum: %f\n", sum_f);
    printf("Double checksum: %lf\n", sum_d);
    
    return 0;
}
