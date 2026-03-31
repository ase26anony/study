/* test_vector_builtin.c
 * This program triggers vectorization of built-in math functions,
 * causing GCC to call default_builtin_vectorized_function and
 * execute the flag-setting code for vectorized built-in declarations.
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
float out_pow[256] __attribute__((aligned(32)));

/* Double precision variants for more coverage */
double in_double[256] __attribute__((aligned(32)));
double out_sin_double[256] __attribute__((aligned(32)));
double out_cos_double[256] __attribute__((aligned(32)));

/* Test functions with noinline to ensure they're not optimized away */
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
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(in[i] + 1.0f);  /* sqrtf requires positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f);   /* logf requires positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i] + 1.0f, 0.5f);
    }
}

/* Double precision versions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_double(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_double(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);
    }
}

/* Mixed operations to trigger multiple vectorized built-ins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain multiple built-in calls */
        float x = sinf(in[i]);
        x = cosf(x);
        x = expf(x);
        out[i] = sqrtf(fabsf(x) + 0.1f);
    }
}

int main() {
    const int N = 256;
    
    /* Initialize input arrays with pattern data */
    for (int i = 0; i < N; i++) {
        in[i] = (i + 1) * 0.1f;
        in_double[i] = (i + 1) * 0.1;
    }
    
    /* Call all vectorized test functions */
    test_vector_sin(in, out_sin, N);
    test_vector_cos(in, out_cos, N);
    test_vector_exp(in, out_exp, N);
    test_vector_sqrt(in, out_sqrt, N);
    test_vector_log(in, out_log, N);
    test_vector_pow(in, out_pow, N);
    test_vector_sin_double(in_double, out_sin_double, N);
    test_vector_cos_double(in_double, out_cos_double, N);
    test_vector_mixed(in, out_sin, N);  /* Reuse out_sin array */
    
    /* Compute checksum to ensure all computations are used */
    float checksum = 0.0f;
    double checksum_double = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i];
        checksum_double += out_sin_double[i] + out_cos_double[i];
    }
    
    /* Store in volatile to prevent optimization */
    global_sink = checksum;
    
    /* Print results to ensure side effects */
    printf("Float checksum: %f\n", checksum);
    printf("Double checksum: %f\n", checksum_double);
    
    return 0;
}
