/* test_vector_builtin.c
 * Test program to cover vectorized built-in function creation in GCC's targhooks.cc
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -march=native -c test_vector_builtin.c
 */

#include <math.h>
#include <stdio.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float __attribute__((aligned(32))) in_array[256];
float __attribute__((aligned(32))) out_sin[256];
float __attribute__((aligned(32))) out_cos[256];
float __attribute__((aligned(32))) out_exp[256];
float __attribute__((aligned(32))) out_sqrt[256];
float __attribute__((aligned(32))) out_log[256];
float __attribute__((aligned(32))) out_pow[256];

/* Double precision variants for more coverage */
double __attribute__((aligned(32))) in_array_d[256];
double __attribute__((aligned(32))) out_sin_d[256];
double __attribute__((aligned(32))) out_cos_d[256];

/* Prevent inlining to ensure separate vectorization decisions */
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
        out[i] = sqrtf(in[i] + 1.0f); /* Add 1 to avoid sqrt(negative) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f); /* Add 1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i] + 1.0f, 2.0f); /* Use pow with constant exponent */
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

/* Mixed operations to trigger multiple vectorized builtins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain multiple math operations */
        float x = sinf(in[i]);
        x = cosf(x);
        x = expf(x);
        out[i] = sqrtf(fabsf(x) + 1.0f);
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in_array[i] = (i % 64) * 0.1f + 0.1f; /* Range: 0.1 to 6.4 */
        in_array_d[i] = (i % 64) * 0.1 + 0.1;
    }
}

/* Compute checksum to ensure results are used */
float compute_checksum(void) {
    float sum = 0.0f;
    for (int i = 0; i < 256; i++) {
        sum += out_sin[i] + out_cos[i] + out_exp[i] + out_sqrt[i] + 
               out_log[i] + out_pow[i];
    }
    
    double sum_d = 0.0;
    for (int i = 0; i < 256; i++) {
        sum_d += out_sin_d[i] + out_cos_d[i];
    }
    
    return sum + (float)sum_d;
}

int main(void) {
    init_arrays();
    
    /* Call all vectorized test functions */
    test_vector_sin(in_array, out_sin, 256);
    test_vector_cos(in_array, out_cos, 256);
    test_vector_exp(in_array, out_exp, 256);
    test_vector_sqrt(in_array, out_sqrt, 256);
    test_vector_log(in_array, out_log, 256);
    test_vector_pow(in_array, out_pow, 256);
    
    test_vector_sin_d(in_array_d, out_sin_d, 256);
    test_vector_cos_d(in_array_d, out_cos_d, 256);
    
    /* Mixed operation test */
    float __attribute__((aligned(32))) out_mixed[256];
    test_vector_mixed(in_array, out_mixed, 256);
    
    /* Compute and print checksum to prevent optimization */
    float checksum = compute_checksum();
    
    /* Add mixed results to checksum */
    for (int i = 0; i < 256; i++) {
        checksum += out_mixed[i];
    }
    
    /* Store in volatile global to prevent dead code elimination */
    global_sink = checksum;
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
