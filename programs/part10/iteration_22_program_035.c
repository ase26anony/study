/* test_vector_builtin.c - Test program to cover vectorized built-in function hooks */

#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float in_array[256] __attribute__((aligned(32)));
float out_array[256] __attribute__((aligned(32)));
double in_double[256] __attribute__((aligned(32)));
double out_double[256] __attribute__((aligned(32)));

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
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f); /* Add 1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(fabsf(in[i]) + 0.1f); /* Ensure positive input */
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

/* Mixed operations to trigger multiple vectorization queries */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain of math operations - each may need vectorization */
        float val = in[i];
        val = sinf(val);
        val = cosf(val);
        val = expf(val);
        val = logf(fabsf(val) + 1.0f);
        out[i] = sqrtf(fabsf(val) + 0.1f);
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in_array[i] = (i - 128) * 0.01f; /* Range: -1.28 to 1.27 */
        in_double[i] = (i - 128) * 0.01; /* Same range for doubles */
    }
}

/* Compute checksum to ensure all computations are used */
float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

double compute_checksum_double(double *arr, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(void) {
    init_arrays();
    
    /* Test all vectorized functions */
    test_vector_sin(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    test_vector_cos(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    test_vector_exp(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    test_vector_log(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    test_vector_sqrt(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    test_vector_mixed(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    /* Double precision tests */
    test_vector_sin_double(in_double, out_double, 256);
    global_sink += compute_checksum_double(out_double, 256);
    
    test_vector_cos_double(in_double, out_double, 256);
    global_sink += compute_checksum_double(out_double, 256);
    
    test_vector_exp_double(in_double, out_double, 256);
    global_sink += compute_checksum_double(out_double, 256);
    
    /* Print results to ensure computations aren't optimized away */
    printf("Checksum: %f\n", (double)global_sink);
    
    return 0;
}
