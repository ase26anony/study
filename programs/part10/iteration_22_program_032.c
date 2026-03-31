/* test_vector_builtin.c
 * Test program to cover default_builtin_vectorized_function hook in GCC
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -march=native -c test_vector_builtin.c
 */

#include <math.h>
#include <stdio.h>

/* Prevent optimization of results */
volatile float global_sum = 0.0f;
volatile double global_sum_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Vectorized sinf function - noinline to ensure separate analysis */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sinf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);
    }
}

/* Vectorized cosf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cosf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);
    }
}

/* Vectorized expf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_expf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);
    }
}

/* Vectorized logf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_logf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i]);
    }
}

/* Vectorized sqrtf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrtf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(in[i] + 1.0f); /* Add 1 to avoid sqrt(negative) */
    }
}

/* Double precision versions to trigger different vectorization paths */
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

/* Complex loop with multiple builtins to increase coverage probability */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain of math operations that should all vectorize */
        float val = in[i];
        val = sinf(val);
        val = cosf(val);
        val = expf(val * 0.5f);
        val = logf(fabsf(val) + 1.0f);
        out[i] = sqrtf(val + 1.0f);
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in_f[i] = (i * 0.1f) + 0.01f;  /* Ensure positive for log/sqrt */
        in_d[i] = (i * 0.1) + 0.01;
    }
}

/* Compute checksum to prevent dead code elimination */
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
    test_vector_sinf(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    test_vector_cosf(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    test_vector_expf(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    test_vector_logf(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    test_vector_sqrtf(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    test_vector_mixed(in_f, out_f, 256);
    global_sum += compute_checksum_float(out_f, 256);
    
    /* Test double precision functions */
    test_vector_sin(in_d, out_d, 256);
    global_sum_d += compute_checksum_double(out_d, 256);
    
    test_vector_cos(in_d, out_d, 256);
    global_sum_d += compute_checksum_double(out_d, 256);
    
    test_vector_exp(in_d, out_d, 256);
    global_sum_d += compute_checksum_double(out_d, 256);
    
    /* Print results to ensure computation isn't optimized away */
    printf("Float checksum: %f\n", (double)global_sum);
    printf("Double checksum: %f\n", global_sum_d);
    
    return 0;
}
