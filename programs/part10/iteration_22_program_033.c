/* test_vector_builtin.c
 * Test program to cover default_builtin_vectorized_function in targhooks.cc
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -fdump-tree-vect-details test_vector_builtin.c -o test_vector_builtin
 */

#include <stdio.h>
#include <math.h>

/* Global volatile variable to prevent optimization */
volatile float g_checksum = 0.0f;
volatile double g_dchecksum = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Vectorization test functions with noinline to ensure they're not optimized away */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sinf(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = sinf(in_f[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cosf(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = cosf(in_f[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_expf(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = expf(in_f[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_logf(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = logf(in_f[i] + 1.0f);  /* +1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrtf(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = sqrtf(fabsf(in_f[i]) + 0.1f);  /* Ensure positive argument */
    }
}

/* Double precision versions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = sin(in_d[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = cos(in_d[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = exp(in_d[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = pow(fabs(in_d[i]) + 1.0, 2.0);  /* Ensure positive base */
    }
}

/* Function with multiple builtins in same loop */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(void) {
    for (int i = 0; i < 256; i++) {
        /* Use multiple builtins to increase chance of vectorization */
        float x = in_f[i];
        out_f[i] = sinf(x) * cosf(x) + sqrtf(fabsf(x) + 0.1f);
    }
}

/* Function with restrict pointers to help vectorization */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_restrict(float * __restrict a, float * __restrict b, float * __restrict c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = sinf(a[i]) * expf(b[i]);
    }
}

int main(void) {
    /* Initialize arrays with pattern data */
    for (int i = 0; i < 256; i++) {
        in_f[i] = (i - 128) * 0.1f;  /* Range: -12.8 to 12.7 */
        in_d[i] = (i - 128) * 0.1;
    }
    
    /* Call all vectorization test functions */
    test_vector_sinf();
    test_vector_cosf();
    test_vector_expf();
    test_vector_logf();
    test_vector_sqrtf();
    test_vector_sin();
    test_vector_cos();
    test_vector_exp();
    test_vector_pow();
    test_vector_mixed();
    
    /* Test with restrict pointers */
    float a[128] __attribute__((aligned(32)));
    float b[128] __attribute__((aligned(32)));
    float c[128] __attribute__((aligned(32)));
    
    for (int i = 0; i < 128; i++) {
        a[i] = i * 0.05f;
        b[i] = i * 0.03f;
    }
    test_vector_restrict(a, b, c, 128);
    
    /* Compute checksums to prevent dead code elimination */
    for (int i = 0; i < 256; i++) {
        g_checksum += out_f[i];
        g_dchecksum += out_d[i];
    }
    for (int i = 0; i < 128; i++) {
        g_checksum += c[i];
    }
    
    /* Print results to ensure computations aren't optimized away */
    printf("Float checksum: %f\n", (float)g_checksum);
    printf("Double checksum: %f\n", (double)g_dchecksum);
    
    return 0;
}
