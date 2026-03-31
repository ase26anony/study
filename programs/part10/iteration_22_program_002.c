/* test_vector_builtin.c
 * Test program to cover default_builtin_vectorized_function hook in targhooks.cc
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -march=native -c test_vector_builtin.c
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
float out_log[256] __attribute__((aligned(32)));
float out_sqrt[256] __attribute__((aligned(32)));
float out_pow[256] __attribute__((aligned(32)));

/* Vectorized sin function - noinline to ensure separate analysis */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);
    }
}

/* Vectorized cos function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);
    }
}

/* Vectorized exp function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);
    }
}

/* Vectorized log function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(fabsf(in[i]) + 1.0f); /* Ensure positive input */
    }
}

/* Vectorized sqrt function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(fabsf(in[i]) + 0.1f); /* Ensure positive input */
    }
}

/* Vectorized pow function - uses two builtins (powf) */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(fabsf(in[i]) + 1.0f, 0.5f); /* sqrt via pow */
    }
}

/* Double precision versions to trigger different vectorization */
double din[256] __attribute__((aligned(32)));
double dout_sin[256] __attribute__((aligned(32)));
double dout_cos[256] __attribute__((aligned(32)));

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

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in[i] = (i - 128) * 0.01f;  /* Range: -1.28 to 1.27 */
        din[i] = (i - 128) * 0.01;
    }
}

/* Compute checksum to ensure results are used */
float compute_checksum(void) {
    float sum = 0.0f;
    for (int i = 0; i < 256; i++) {
        sum += out_sin[i] + out_cos[i] + out_exp[i] + 
               out_log[i] + out_sqrt[i] + out_pow[i];
    }
    return sum;
}

int main(void) {
    init_arrays();
    
    /* Call all vectorized functions */
    test_vector_sin(in, out_sin, 256);
    test_vector_cos(in, out_cos, 256);
    test_vector_exp(in, out_exp, 256);
    test_vector_log(in, out_log, 256);
    test_vector_sqrt(in, out_sqrt, 256);
    test_vector_pow(in, out_pow, 256);
    
    /* Double precision versions */
    test_vector_sin_double(din, dout_sin, 256);
    test_vector_cos_double(din, dout_cos, 256);
    
    /* Use results to prevent optimization */
    float checksum = compute_checksum();
    global_sink = checksum;
    
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
