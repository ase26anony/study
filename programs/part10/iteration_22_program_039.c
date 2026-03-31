/* test_vector_builtin.c
 * This program triggers GCC's vectorization of built-in math functions,
 * causing the compiler to call default_builtin_vectorized_function
 * and execute the flag-setting code in targhooks.cc lines 981-990.
 */

#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float in_array[256] __attribute__((aligned(32)));
float out_array[256] __attribute__((aligned(32)));
double in_double[256] __attribute__((aligned(32)));
double out_double[256] __attribute__((aligned(32)));

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in_array[i] = i * 0.1f;
        in_double[i] = i * 0.1;
    }
}

/* Vectorized sinf function - noinline to ensure separate analysis */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);
    }
}

/* Vectorized cosf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);
    }
}

/* Vectorized expf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);
    }
}

/* Vectorized sqrtf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(fabsf(in[i]) + 0.1f); /* Ensure positive input */
    }
}

/* Vectorized logf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(fabsf(in[i]) + 1.0f); /* Ensure positive input */
    }
}

/* Double precision versions to trigger different vectorization */
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

/* Mixed operations to trigger multiple vectorized builtins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Use multiple builtins in sequence */
        float x = sinf(in[i]);
        float y = cosf(in[i]);
        out[i] = x * x + y * y; /* Should be ~1.0 */
    }
}

/* Compute checksum to ensure results are used */
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
    
    /* Test all vectorized float functions */
    test_vector_sin(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    test_vector_cos(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    test_vector_exp(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    test_vector_sqrt(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    test_vector_log(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    test_vector_mixed(in_array, out_array, 256);
    global_sink += compute_checksum(out_array, 256);
    
    /* Test double precision functions */
    test_vector_sin_double(in_double, out_double, 256);
    global_sink += compute_checksum_double(out_double, 256);
    
    test_vector_cos_double(in_double, out_double, 256);
    global_sink += compute_checksum_double(out_double, 256);
    
    /* Print results to prevent optimization */
    printf("Checksum: %f\n", (double)global_sink);
    
    return 0;
}
