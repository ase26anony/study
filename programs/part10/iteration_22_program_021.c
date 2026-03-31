/* test_vector_builtin.c - Test program to trigger vectorization of built-in math functions */

#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent optimization */
volatile float g_sink_float = 0.0f;
volatile double g_sink_double = 0.0;

/* Aligned arrays for vectorization */
float in_float[256] __attribute__((aligned(32)));
float out_float[256] __attribute__((aligned(32)));
double in_double[256] __attribute__((aligned(32)));
double out_double[256] __attribute__((aligned(32)));

/* Test functions with noinline to ensure they're not optimized away */
/* Each function tests a different built-in math function */

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
void test_vector_logf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f); /* Add 1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrtf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(fabsf(in[i]) + 0.1f); /* Ensure positive input */
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
void test_vector_pow(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = pow(fabs(in[i]) + 1.0, 2.0); /* Ensure positive base */
    }
}

/* Mixed operations to trigger multiple vectorized built-ins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Use multiple built-ins in sequence */
        float x = sinf(in[i]);
        x = cosf(x);
        x = expf(x);
        out[i] = sqrtf(fabsf(x));
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in_float[i] = (i - 128) * 0.01f;  /* Range: -1.28 to 1.27 */
        in_double[i] = (i - 128) * 0.01;  /* Same range for double */
    }
}

/* Compute checksum to ensure results are used */
float compute_checksum_float(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    g_sink_float = sum;  /* Store in volatile global */
    return sum;
}

double compute_checksum_double(double *arr, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    g_sink_double = sum;  /* Store in volatile global */
    return sum;
}

int main(void) {
    init_arrays();
    
    /* Test all float vectorized functions */
    test_vector_sinf(in_float, out_float, 256);
    float sum1 = compute_checksum_float(out_float, 256);
    
    test_vector_cosf(in_float, out_float, 256);
    float sum2 = compute_checksum_float(out_float, 256);
    
    test_vector_expf(in_float, out_float, 256);
    float sum3 = compute_checksum_float(out_float, 256);
    
    test_vector_logf(in_float, out_float, 256);
    float sum4 = compute_checksum_float(out_float, 256);
    
    test_vector_sqrtf(in_float, out_float, 256);
    float sum5 = compute_checksum_float(out_float, 256);
    
    test_vector_mixed(in_float, out_float, 256);
    float sum6 = compute_checksum_float(out_float, 256);
    
    /* Test all double vectorized functions */
    test_vector_sin(in_double, out_double, 256);
    double sum7 = compute_checksum_double(out_double, 256);
    
    test_vector_cos(in_double, out_double, 256);
    double sum8 = compute_checksum_double(out_double, 256);
    
    test_vector_exp(in_double, out_double, 256);
    double sum9 = compute_checksum_double(out_double, 256);
    
    test_vector_pow(in_double, out_double, 256);
    double sum10 = compute_checksum_double(out_double, 256);
    
    /* Print results to ensure they're used */
    printf("Float checksums: %f %f %f %f %f %f\n", sum1, sum2, sum3, sum4, sum5, sum6);
    printf("Double checksums: %f %f %f %f\n", sum7, sum8, sum9, sum10);
    printf("Global sinks: %f %f\n", (float)g_sink_float, (float)g_sink_double);
    
    return 0;
}
