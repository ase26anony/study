/* test_vector_builtin.c
 * Test program to cover default_builtin_vectorized_function in targhooks.cc
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -march=native -c test_vector_builtin.c
 */

#include <math.h>
#include <stdio.h>

/* Prevent dead code elimination */
static volatile float volatile_sink = 0.0f;
static volatile double volatile_sink_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Force vectorization and prevent inlining */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sinf(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = sinf(in_f[i]);
    }
    /* Use result to prevent elimination */
    volatile_sink += out_f[0];
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cosf(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = cosf(in_f[i]);
    }
    volatile_sink += out_f[1];
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_expf(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = expf(in_f[i]);
    }
    volatile_sink += out_f[2];
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_logf(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = logf(in_f[i] + 1.0f); /* +1 to avoid log(0) */
    }
    volatile_sink += out_f[3];
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrtf(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = sqrtf(fabsf(in_f[i]) + 0.1f); /* Ensure positive */
    }
    volatile_sink += out_f[4];
}

/* Double precision versions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = sin(in_d[i]);
    }
    volatile_sink_d += out_d[0];
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = cos(in_d[i]);
    }
    volatile_sink_d += out_d[1];
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = exp(in_d[i]);
    }
    volatile_sink_d += out_d[2];
}

/* Mixed operations to trigger multiple vectorized builtins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(void) {
    for (int i = 0; i < 256; i++) {
        /* Chain multiple builtins */
        float val = in_f[i];
        val = sinf(val);
        val = cosf(val);
        val = expf(val);
        val = sqrtf(fabsf(val));
        out_f[i] = val;
    }
    volatile_sink += out_f[5];
}

/* Restrict pointers for better vectorization analysis */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_restrict(float *__restrict a, float *__restrict b, 
                          float *__restrict c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = sinf(a[i]) * cosf(b[i]);
    }
    volatile_sink += c[0];
}

int main(void) {
    /* Initialize with pattern data */
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
    test_vector_mixed();
    
    /* Test with restrict qualifier */
    float a[256] __attribute__((aligned(32)));
    float b[256] __attribute__((aligned(32)));
    float c[256] __attribute__((aligned(32)));
    for (int i = 0; i < 256; i++) {
        a[i] = i * 0.05f;
        b[i] = i * 0.03f;
    }
    test_vector_restrict(a, b, c, 256);
    
    /* Compute checksum */
    float checksum_f = 0.0f;
    double checksum_d = 0.0;
    for (int i = 0; i < 256; i++) {
        checksum_f += out_f[i];
        checksum_d += out_d[i];
    }
    
    printf("Float checksum: %f\n", checksum_f);
    printf("Double checksum: %lf\n", checksum_d);
    printf("Volatile sinks: %f, %lf\n", (float)volatile_sink, volatile_sink_d);
    
    return 0;
}
