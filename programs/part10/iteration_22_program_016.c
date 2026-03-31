/* test_vector_builtin.c
 * Test program to trigger vectorization of built-in math functions
 * and cover the default_builtin_vectorized_function hook in targhooks.cc
 */

#include <stdio.h>
#include <math.h>

/* Global volatile variable to prevent dead code elimination */
volatile float global_sum = 0.0f;
volatile double global_sum_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Test functions with noinline to ensure they're not optimized away */
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
        out_f[i] = logf(in_f[i] + 1.0f);  /* +1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = sqrtf(fabsf(in_f[i]) + 0.1f);  /* Ensure positive */
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

/* Function with multiple built-ins in same loop */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(void) {
    for (int i = 0; i < 256; i++) {
        /* Use multiple built-ins to increase chance of vectorization */
        float x = in_f[i];
        out_f[i] = sinf(x) * cosf(x) + sqrtf(fabsf(x));
    }
}

/* Function with restrict pointers for better vectorization */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_restrict(float * __restrict a, 
                          const float * __restrict b, 
                          int n) {
    for (int i = 0; i < n; i++) {
        a[i] = sinf(b[i]) + cosf(b[i]);
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
    test_vector_restrict(out_f, in_f, 256);
    
    /* Compute checksums to ensure loops execute */
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    for (int i = 0; i < 256; i++) {
        sum_f += out_f[i];
        sum_d += out_d[i];
    }
    
    /* Store in volatile to prevent optimization */
    global_sum = sum_f;
    global_sum_d = sum_d;
    
    /* Print results to ensure execution */
    printf("Float checksum: %f\n", sum_f);
    printf("Double checksum: %lf\n", sum_d);
    
    return 0;
}
