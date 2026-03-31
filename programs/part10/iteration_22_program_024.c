/* test_vector_builtin.c
 * Test program to cover default_builtin_vectorized_function in targhooks.cc
 * Lines 981-990: Setting flags on vectorized built-in function declarations
 */

#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent optimization */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float in_array[256] __attribute__((aligned(32)));
float out_sin[256] __attribute__((aligned(32)));
float out_cos[256] __attribute__((aligned(32)));
float out_exp[256] __attribute__((aligned(32)));
float out_sqrt[256] __attribute__((aligned(32)));
float out_log[256] __attribute__((aligned(32)));
float out_pow[256] __attribute__((aligned(32)));

/* Double precision arrays for additional coverage */
double in_double[256] __attribute__((aligned(32)));
double out_sin_d[256] __attribute__((aligned(32)));
double out_cos_d[256] __attribute__((aligned(32)));

/* Vectorized built-in test functions with noinline to ensure separate analysis */
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
        out[i] = sqrtf(in[i] + 1.0f);  /* Add 1 to ensure positive values */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f);  /* Add 1 to ensure positive values */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i] + 1.0f, 2.0f);  /* Square operation */
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

/* Combined test with multiple built-ins in one loop */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_mixed_builtins(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Use multiple built-ins to increase coverage chance */
        float x = in[i];
        out[i] = sinf(x) * cosf(x) + expf(x * 0.5f);
    }
}

int main() {
    const int n = 256;
    
    /* Initialize input arrays with pattern */
    for (int i = 0; i < n; i++) {
        in_array[i] = (i + 1) * 0.1f;
        in_double[i] = (i + 1) * 0.1;
    }
    
    /* Call all vectorized test functions */
    test_vector_sin(in_array, out_sin, n);
    test_vector_cos(in_array, out_cos, n);
    test_vector_exp(in_array, out_exp, n);
    test_vector_sqrt(in_array, out_sqrt, n);
    test_vector_log(in_array, out_log, n);
    test_vector_pow(in_array, out_pow, n);
    test_vector_sin_d(in_double, out_sin_d, n);
    test_vector_cos_d(in_double, out_cos_d, n);
    test_mixed_builtins(in_array, out_sin, n);  /* Reuse out_sin array */
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < n; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i] +
                   (float)(out_sin_d[i] + out_cos_d[i]);
    }
    
    /* Use volatile global to ensure computation isn't optimized away */
    global_sink = checksum;
    
    printf("Checksum: %f\n", checksum);
    printf("Global sink: %f\n", (float)global_sink);
    
    return 0;
}
