/* test_vector_builtin.c
 * Test program to cover default_builtin_vectorized_function hook in GCC
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -march=native -c test_vector_builtin.c
 */

#include <math.h>
#include <stdio.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;

/* Aligned arrays to enable vectorization */
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

/* Noinline to ensure separate function calls and prevent inlining */
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
        out[i] = sqrtf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f); /* +1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.0f);
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

/* Mixed operations to potentially trigger more cases */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain of math operations */
        float x = sinf(in[i]);
        x = cosf(x);
        x = expf(x);
        out[i] = sqrtf(fabsf(x));
    }
}

int main(void) {
    const int n = 256;
    float checksum = 0.0f;
    double checksum_double = 0.0;
    
    /* Initialize input arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in[i] = (i + 1) * 0.1f; /* Avoid zero for sqrt/log */
        in_double[i] = (i + 1) * 0.1;
    }
    
    /* Call all vectorized test functions */
    test_vector_sin(in, out_sin, n);
    test_vector_cos(in, out_cos, n);
    test_vector_exp(in, out_exp, n);
    test_vector_sqrt(in, out_sqrt, n);
    test_vector_log(in, out_log, n);
    test_vector_pow(in, out_pow, n);
    test_vector_sin_double(in_double, out_sin_double, n);
    test_vector_cos_double(in_double, out_cos_double, n);
    test_vector_mixed(in, out_sin, n); /* Reuse out_sin as temp */
    
    /* Compute checksums to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i];
        checksum_double += out_sin_double[i] + out_cos_double[i];
    }
    
    /* Use volatile global to ensure computations aren't optimized away */
    global_sink = checksum;
    
    /* Print results to ensure side effects */
    printf("Float checksum: %f\n", checksum);
    printf("Double checksum: %f\n", checksum_double);
    
    return 0;
}
