/* test_vector_builtin.c
 * Test program to cover default_builtin_vectorized_function in targhooks.cc
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -fdump-tree-vect-details -c test_vector_builtin.c
 */

#include <math.h>
#include <stdio.h>

/* Global volatile sink to prevent optimization */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float in[256] __attribute__((aligned(32)));
float out_sin[256] __attribute__((aligned(32)));
float out_cos[256] __attribute__((aligned(32)));
float out_exp[256] __attribute__((aligned(32)));
float out_sqrt[256] __attribute__((aligned(32)));
float out_log[256] __attribute__((aligned(32)));
float out_pow[256] __attribute__((aligned(32)));

/* Double precision variants */
double in_double[256] __attribute__((aligned(32)));
double out_sin_double[256] __attribute__((aligned(32)));
double out_cos_double[256] __attribute__((aligned(32)));
double out_exp_double[256] __attribute__((aligned(32)));

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
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(in[i] + 1.0f);  /* Add 1 to avoid sqrt(negative) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f);  /* Add 1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i] + 1.0f, 2.0f);  /* Square the values */
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

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_double(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]);
    }
}

/* Mixed operations to trigger multiple vectorization opportunities */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain of math operations that should all vectorize */
        float val = in[i];
        val = sinf(val);
        val = cosf(val);
        val = expf(val);
        val = sqrtf(fabsf(val));  /* Use fabsf to ensure positive */
        out[i] = val;
    }
}

int main(void) {
    const int n = 256;
    float checksum = 0.0f;
    double checksum_double = 0.0;
    
    /* Initialize input arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in[i] = (i + 1) * 0.1f;  /* Range: 0.1 to 25.6 */
        in_double[i] = (i + 1) * 0.1;  /* Same for double */
    }
    
    /* Call all vectorization test functions */
    test_vector_sin(in, out_sin, n);
    test_vector_cos(in, out_cos, n);
    test_vector_exp(in, out_exp, n);
    test_vector_sqrt(in, out_sqrt, n);
    test_vector_log(in, out_log, n);
    test_vector_pow(in, out_pow, n);
    test_vector_mixed(in, out_sin, n);  /* Reuse out_sin array */
    
    test_vector_sin_double(in_double, out_sin_double, n);
    test_vector_cos_double(in_double, out_cos_double, n);
    test_vector_exp_double(in_double, out_exp_double, n);
    
    /* Compute checksums to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i];
        checksum_double += out_sin_double[i] + out_cos_double[i] + 
                          out_exp_double[i];
    }
    
    /* Store to volatile global to ensure computation isn't optimized away */
    global_sink = checksum;
    
    /* Print results to ensure side effects */
    printf("Float checksum: %f\n", checksum);
    printf("Double checksum: %lf\n", checksum_double);
    
    return 0;
}
