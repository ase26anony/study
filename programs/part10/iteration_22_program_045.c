/* test_vector_builtin.c - Test program to cover vectorized built-in function handling */

#include <math.h>
#include <stdio.h>

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

/* Double precision variants */
double in_double[256] __attribute__((aligned(32)));
double out_sin_double[256] __attribute__((aligned(32)));
double out_cos_double[256] __attribute__((aligned(32)));

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
        out[i] = sqrtf(in[i] + 1.0f);  /* Ensure positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f);  /* Ensure positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i] + 1.0f, 2.0f);  /* Simple power function */
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

/* Combined test with multiple built-ins in one loop */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Use multiple built-ins to increase coverage chance */
        float x = in[i];
        out[i] = sinf(x) * cosf(x) + expf(x * 0.5f) / sqrtf(x + 2.0f);
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in[i] = (i * 0.1f) + 0.01f;  /* Avoid exact zeros for log/sqrt */
        in_double[i] = (i * 0.1) + 0.01;
    }
}

/* Compute checksum to ensure all computations are used */
float compute_checksum(void) {
    float sum = 0.0f;
    for (int i = 0; i < 256; i++) {
        sum += out_sin[i] + out_cos[i] + out_exp[i] + 
               out_sqrt[i] + out_log[i] + out_pow[i];
    }
    
    /* Add double precision results */
    double sum_double = 0.0;
    for (int i = 0; i < 256; i++) {
        sum_double += out_sin_double[i] + out_cos_double[i];
    }
    
    return sum + (float)sum_double;
}

int main(void) {
    /* Initialize input arrays */
    init_arrays();
    
    /* Call all vectorization test functions */
    test_vector_sin(in, out_sin, 256);
    test_vector_cos(in, out_cos, 256);
    test_vector_exp(in, out_exp, 256);
    test_vector_sqrt(in, out_sqrt, 256);
    test_vector_log(in, out_log, 256);
    test_vector_pow(in, out_pow, 256);
    
    /* Double precision tests */
    test_vector_sin_double(in_double, out_sin_double, 256);
    test_vector_cos_double(in_double, out_cos_double, 256);
    
    /* Mixed built-in test */
    float out_mixed[256] __attribute__((aligned(32)));
    test_vector_mixed(in, out_mixed, 256);
    
    /* Compute and print checksum to prevent optimization */
    float checksum = compute_checksum();
    
    /* Add mixed results to checksum */
    for (int i = 0; i < 256; i++) {
        checksum += out_mixed[i];
    }
    
    /* Store in volatile global to ensure computation isn't eliminated */
    global_sink = checksum;
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
