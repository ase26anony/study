/* test_vector_builtin.c
 * Test program to trigger vectorization of built-in math functions
 * and cover the default_builtin_vectorized_function hook in targhooks.cc
 */

#include <stdio.h>
#include <math.h>

#define SIZE 256
#define ALIGN __attribute__((aligned(32)))

/* Global volatile sink to prevent optimization */
volatile float g_sink = 0.0f;

/* Aligned arrays to enable vectorization */
ALIGN float in_f[SIZE];
ALIGN float out_sin[SIZE];
ALIGN float out_cos[SIZE];
ALIGN float out_exp[SIZE];
ALIGN float out_sqrt[SIZE];
ALIGN float out_log[SIZE];
ALIGN float out_pow[SIZE];

ALIGN double in_d[SIZE];
ALIGN double out_sin_d[SIZE];
ALIGN double out_cos_d[SIZE];
ALIGN double out_exp_d[SIZE];
ALIGN double out_sqrt_d[SIZE];

/* Prevent inlining to ensure separate vectorization decisions */
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
void test_vector_sqrtf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(fabsf(in[i]) + 1.0f);  /* Ensure positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_logf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(fabsf(in[i]) + 1.0f);   /* Ensure positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_powf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(fabsf(in[i]) + 1.0f, 0.5f);  /* sqrt via pow */
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
void test_vector_sqrt_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrt(fabs(in[i]) + 1.0);  /* Ensure positive input */
    }
}

/* Mixed operations to trigger multiple vectorization queries */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain of math operations - each might be vectorized */
        float x = sinf(in[i]);
        x = cosf(x);
        x = expf(x);
        out[i] = sqrtf(fabsf(x));
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
        /* Use values that avoid domain errors for sqrt/log */
        in_f[i] = (i * 0.1f) + 0.5f;
        in_d[i] = (i * 0.1) + 0.5;
    }
}

/* Compute checksum to prevent dead code elimination */
float compute_checksum(void) {
    float sum = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        sum += out_sin[i] + out_cos[i] + out_exp[i] + 
               out_sqrt[i] + out_log[i] + out_pow[i];
    }
    
    double sum_d = 0.0;
    for (int i = 0; i < SIZE; i++) {
        sum_d += out_sin_d[i] + out_cos_d[i] + 
                 out_exp_d[i] + out_sqrt_d[i];
    }
    
    return sum + (float)sum_d;
}

int main(void) {
    init_arrays();
    
    /* Call all vectorized functions */
    test_vector_sinf(in_f, out_sin, SIZE);
    test_vector_cosf(in_f, out_cos, SIZE);
    test_vector_expf(in_f, out_exp, SIZE);
    test_vector_sqrtf(in_f, out_sqrt, SIZE);
    test_vector_logf(in_f, out_log, SIZE);
    test_vector_powf(in_f, out_pow, SIZE);
    
    test_vector_sin(in_d, out_sin_d, SIZE);
    test_vector_cos(in_d, out_cos_d, SIZE);
    test_vector_exp(in_d, out_exp_d, SIZE);
    test_vector_sqrt_d(in_d, out_sqrt_d, SIZE);
    
    /* Mixed operations */
    ALIGN float out_mixed[SIZE];
    test_vector_mixed(in_f, out_mixed, SIZE);
    
    /* Compute and print checksum */
    float checksum = compute_checksum();
    
    /* Add mixed results to checksum */
    for (int i = 0; i < SIZE; i++) {
        checksum += out_mixed[i];
    }
    
    /* Store in volatile global to prevent optimization */
    g_sink = checksum;
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
