/* test_vector_builtin.c
 * This program triggers vectorization of built-in math functions,
 * causing GCC to call default_builtin_vectorized_function and
 * execute the uncovered flag-setting code in targhooks.cc.
 */

#include <stdio.h>
#include <math.h>

/* Global volatile variable to prevent dead code elimination */
volatile float global_sum = 0.0f;
volatile double global_sum_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Vectorization test functions with noinline to ensure they're processed separately */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = sinf(in_f[i]);  /* Triggers vectorized sinf */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = cosf(in_f[i]);  /* Triggers vectorized cosf */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = expf(in_f[i]);  /* Triggers vectorized expf */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = logf(in_f[i] + 1.0f);  /* Triggers vectorized logf */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = sqrtf(fabsf(in_f[i]) + 0.1f);  /* Triggers vectorized sqrtf */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_double(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = sin(in_d[i]);  /* Triggers vectorized sin (double) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_double(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = cos(in_d[i]);  /* Triggers vectorized cos (double) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_double(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = exp(in_d[i]);  /* Triggers vectorized exp (double) */
    }
}

/* Function with restrict pointers to help vectorization */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict a, float *restrict b, float *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple builtins in one loop */
        a[i] = sinf(b[i]) * cosf(c[i]) + sqrtf(fabsf(b[i]));
    }
}

int main(void) {
    /* Initialize arrays with pattern data */
    for (int i = 0; i < 256; i++) {
        in_f[i] = i * 0.1f;
        in_d[i] = i * 0.1;
    }
    
    /* Call all vectorization test functions */
    test_vector_sin();
    test_vector_cos();
    test_vector_exp();
    test_vector_log();
    test_vector_sqrt();
    test_vector_sin_double();
    test_vector_cos_double();
    test_vector_exp_double();
    
    /* Test with restrict pointers */
    float a[256] __attribute__((aligned(32)));
    float b[256] __attribute__((aligned(32)));
    float c[256] __attribute__((aligned(32)));
    
    for (int i = 0; i < 256; i++) {
        b[i] = i * 0.05f;
        c[i] = i * 0.03f;
    }
    
    test_vector_mixed(a, b, c, 256);
    
    /* Compute checksums to prevent dead code elimination */
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    for (int i = 0; i < 256; i++) {
        sum_f += out_f[i] + a[i];
        sum_d += out_d[i];
    }
    
    global_sum = sum_f;
    global_sum_d = sum_d;
    
    /* Print results to ensure computations aren't optimized away */
    printf("Float checksum: %f\n", sum_f);
    printf("Double checksum: %f\n", sum_d);
    
    return 0;
}
