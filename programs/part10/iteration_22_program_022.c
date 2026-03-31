/* test_vector_builtin.c
 * Test program to cover default_builtin_vectorized_function in targhooks.cc
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -march=native -c test_vector_builtin.c
 */

#include <math.h>
#include <stdio.h>

/* Global volatile variable to prevent dead code elimination */
volatile float global_sum = 0.0f;
volatile double global_sum_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Prevent inlining to ensure separate vectorization decisions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = sinf(in_f[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = cosf(in_f[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = expf(in_f[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = logf(in_f[i] + 1.0f); /* Add 1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = sqrtf(fabsf(in_f[i]) + 0.1f); /* Ensure positive input */
    }
}

/* Double precision versions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_double(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = sin(in_d[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_double(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = cos(in_d[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_double(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = exp(in_d[i]);
    }
}

/* Function with multiple builtins in same loop */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(void) {
    for (int i = 0; i < 256; i++) {
        /* Use multiple builtins to increase chance of vectorization */
        float x = in_f[i];
        out_f[i] = sinf(x) * cosf(x) + sqrtf(fabsf(x));
    }
}

/* Function with restrict pointers for better aliasing info */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_restrict(float *__restrict a, float *__restrict b, 
                          float *__restrict c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = sinf(a[i]) * expf(b[i]);
    }
}

int main(void) {
    /* Initialize arrays with pattern data */
    for (int i = 0; i < 256; i++) {
        in_f[i] = i * 0.1f;
        in_d[i] = i * 0.1;
    }
    
    /* Call all vectorization test functions */
    test_vector_sin();
    test_vector_cos();
    test_vector_exp();
    test_vector_log();
    test_vector_sqrt();
    test_vector_sin_double();
    test_vector_cos_double();
    test_vector_exp_double();
    test_vector_mixed();
    
    /* Test with restrict pointers */
    float a[256] __attribute__((aligned(32)));
    float b[256] __attribute__((aligned(32)));
    float c[256] __attribute__((aligned(32)));
    
    for (int i = 0; i < 256; i++) {
        a[i] = i * 0.05f;
        b[i] = i * 0.02f;
    }
    test_vector_restrict(a, b, c, 256);
    
    /* Compute checksums to prevent elimination */
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    for (int i = 0; i < 256; i++) {
        sum_f += out_f[i] + c[i];
        sum_d += out_d[i];
    }
    
    global_sum = sum_f;
    global_sum_d = sum_d;
    
    /* Print results to ensure execution */
    printf("Float checksum: %f\n", sum_f);
    printf("Double checksum: %f\n", sum_d);
    
    return 0;
}
