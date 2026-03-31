/* test_vector_builtin.c
 * Test program to cover vectorized built-in function creation in GCC's targhooks.cc
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -fdump-tree-vect-details -c test_vector_builtin.c
 */

#include <math.h>
#include <stdio.h>

/* Prevent dead code elimination */
volatile float global_sum = 0.0f;
volatile double global_sum_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Vectorization test functions with noinline to ensure they're processed separately */
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
        out[i] = logf(in[i] + 1.0f);  /* +1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(fabsf(in[i]) + 0.1f);  /* Ensure positive input */
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

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]);
    }
}

/* Mixed operations to trigger multiple vectorized built-ins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain multiple math operations */
        float x = sinf(in[i]);
        x = cosf(x);
        x = expf(x);
        out[i] = sqrtf(fabsf(x) + 0.1f);
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in_f[i] = (i - 128) * 0.1f;  /* Range: -12.8 to 12.7 */
        in_d[i] = (i - 128) * 0.1;   /* Same range for doubles */
    }
}

/* Compute checksum to prevent optimization */
float compute_checksum_float(float *arr, int n) {
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
    
    /* Test all float vectorized functions */
    test_vector_sin(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    test_vector_cos(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    test_vector_exp(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    test_vector_log(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    test_vector_sqrt(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    test_vector_mixed(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    /* Test double precision functions */
    test_vector_sin_d(in_d, out_d, 256);
    global_sum_d += compute_checksum_double(out_d, 256);
    
    test_vector_cos_d(in_d, out_d, 256);
    global_sum_d += compute_checksum_double(out_d, 256);
    
    test_vector_exp_d(in_d, out_d, 256);
    global_sum_d += compute_checksum_double(out_d, 256);
    
    /* Print results to ensure computations aren't optimized away */
    printf("Float checksum: %f\n", (float)global_sum);
    printf("Double checksum: %lf\n", global_sum_d);
    
    return 0;
}
