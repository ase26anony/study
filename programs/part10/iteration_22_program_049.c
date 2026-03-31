/* test_vector_builtin.c
 * This program triggers vectorization of built-in math functions,
 * causing GCC to call default_builtin_vectorized_function and
 * execute the flag-setting code for vectorized built-in declarations.
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

/* Test functions with noinline to ensure they're not optimized away */
/* Each function uses a different built-in math function to increase
   chances of triggering the target hook */

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
        out[i] = logf(in[i] + 1.0f);  /* +1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(fabsf(in[i]) + 0.1f);  /* Ensure positive input */
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

/* Function with multiple built-ins in same loop */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Use multiple different built-ins */
        float x = in[i];
        out[i] = sinf(x) * cosf(x) + sqrtf(fabsf(x));
    }
}

int main(void) {
    const int n = 256;
    
    /* Initialize input arrays with pattern-based data */
    for (int i = 0; i < n; i++) {
        in_f[i] = (i - 128) * 0.1f;  /* Range: -12.8 to 12.7 */
        in_d[i] = (i - 128) * 0.1;   /* Same range for double */
    }
    
    /* Call all vectorization test functions */
    test_vector_sin(in_f, out_f, n);
    test_vector_cos(in_f, out_f, n);
    test_vector_exp(in_f, out_f, n);
    test_vector_log(in_f, out_f, n);
    test_vector_sqrt(in_f, out_f, n);
    test_vector_pow(in_f, out_f, n);
    test_vector_mixed(in_f, out_f, n);
    
    test_vector_sin_d(in_d, out_d, n);
    test_vector_cos_d(in_d, out_d, n);
    test_vector_exp_d(in_d, out_d, n);
    
    /* Compute checksums to ensure computations aren't optimized away */
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    for (int i = 0; i < n; i++) {
        sum_f += out_f[i];
        sum_d += out_d[i];
    }
    
    /* Store in volatile globals and print to prevent elimination */
    global_sum = sum_f;
    global_sum_d = sum_d;
    
    printf("Float checksum: %f\n", sum_f);
    printf("Double checksum: %lf\n", sum_d);
    
    return 0;
}
