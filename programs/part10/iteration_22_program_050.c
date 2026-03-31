/* test_vector_builtin.c
 * This program triggers GCC's vectorization of built-in math functions,
 * causing the default_builtin_vectorized_function hook to be called
 * and execute the flag-setting code in targhooks.cc lines 981-990.
 */

#include <stdio.h>
#include <math.h>

/* Prevent dead code elimination */
volatile float global_sum = 0.0f;
volatile double global_sum_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Vectorization test functions with noinline to ensure they're not optimized away */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(in[i]);
    }
}

/* Double precision versions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.0f);
    }
}

/* Mixed operations to trigger multiple vectorized builtins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain multiple builtins to increase chance of vectorization */
        float val = in[i];
        val = sinf(val);
        val = cosf(val);
        val = expf(val);
        out[i] = sqrtf(fabsf(val));
    }
}

int main() {
    const int n = 256;
    
    /* Initialize input arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in_f[i] = (i + 1) * 0.1f;  /* Avoid zero for log */
        in_d[i] = (i + 1) * 0.1;
    }
    
    /* Test various vectorized built-in functions */
    test_vector_sin(in_f, out_f, n);
    test_vector_cos(in_f, out_f, n);
    test_vector_exp(in_f, out_f, n);
    test_vector_log(in_f, out_f, n);
    test_vector_sqrt(in_f, out_f, n);
    test_vector_pow(in_f, out_f, n);
    test_vector_mixed(in_f, out_f, n);
    
    /* Double precision tests */
    test_vector_sin_d(in_d, out_d, n);
    test_vector_cos_d(in_d, out_d, n);
    test_vector_exp_d(in_d, out_d, n);
    
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
    
    printf("Float checksum: %f\n", sum_f);
    printf("Double checksum: %lf\n", sum_d);
    
    return 0;
}
