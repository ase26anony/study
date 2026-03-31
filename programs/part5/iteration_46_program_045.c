#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t prng_u32(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Volatile counter to prevent loop unrolling */
volatile int volatile_counter = 0;

/* Target-specific function with many arguments */
__attribute__((noinline, target("avx2,avx512f")))
void test_many_args(float* restrict out, const float* restrict in1, 
                    const float* restrict in2, const float* restrict in3,
                    int n) {
    /* Prevent constant propagation */
    volatile int vn = n;
    
    for (int i = 0; i < vn; i += 16) {
        volatile_counter++;
        
        /* Load multiple vectors */
        __m512 v1 = _mm512_loadu_ps(&in1[i]);
        __m512 v2 = _mm512_loadu_ps(&in1[i + 16]);
        __m512 v3 = _mm512_loadu_ps(&in2[i]);
        __m512 v4 = _mm512_loadu_ps(&in2[i + 16]);
        __m512 v5 = _mm512_loadu_ps(&in3[i]);
        __m512 v6 = _mm512_loadu_ps(&in3[i + 16]);
        
        /* Complex expression with many temporaries */
        __m512 t1 = _mm512_add_ps(v1, v2);
        __m512 t2 = _mm512_sub_ps(v3, v4);
        __m512 t3 = _mm512_mul_ps(v5, v6);
        
        /* Create a shuffle mask with many immediate values */
        __m512i mask = _mm512_setr_epi32(
            0, 16, 1, 17, 2, 18, 3, 19,
            4, 20, 5, 21, 6, 22, 7, 23
        );
        
        /* Complex operation that might expand to many arguments */
        __m512 result;
        
        /* Inline assembly with 11 operands to trigger the optab case */
        asm volatile (
            "vmovaps %1, %%zmm0\n\t"
            "vmovaps %2, %%zmm1\n\t"
            "vmovaps %3, %%zmm2\n\t"
            "vmovaps %4, %%zmm3\n\t"
            "vmovaps %5, %%zmm4\n\t"
            "vmovaps %6, %%zmm5\n\t"
            "vaddps %%zmm0, %%zmm1, %%zmm6\n\t"
            "vsubps %%zmm2, %%zmm3, %%zmm7\n\t"
            "vmulps %%zmm4, %%zmm5, %%zmm8\n\t"
            "vfmadd231ps %%zmm6, %%zmm7, %%zmm8\n\t"
            "vpermps %%zmm8, %7, %%zmm9\n\t"
            "vmovaps %%zmm9, %0"
            : "=m" (result)
            : "m" (t1), "m" (t2), "m" (t3), 
              "m" (v1), "m" (v2), "m" (mask),
              "v" (mask), "r" (i), "r" (vn), "m" (*in1)
            : "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5",
              "zmm6", "zmm7", "zmm8", "zmm9", "memory"
        );
        
        _mm512_storeu_ps(&out[i], result);
    }
}

/* Alternative approach using GCC vector builtins */
#ifdef __GNUC__
typedef float v16sf __attribute__((vector_size(64)));

__attribute__((noinline, target("avx512f")))
v16sf complex_shuffle_10_args(v16sf a, v16sf b, v16sf c, v16sf d,
                              v16sf e, v16sf f, v16sf g, v16sf h,
                              int i1, int i2) {
    /* This builtin usage with many arguments might trigger the optab */
    v16sf temp1 = __builtin_shuffle(a, b, 
        (int[16]){0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23});
    
    v16sf temp2 = __builtin_shuffle(c, d,
        (int[16]){8,24,9,25,10,26,11,27,12,28,13,29,14,30,15,31});
    
    /* Complex expression tree */
    v16sf result = temp1 + temp2;
    result = result * e;
    result = result / f;
    result = result - g;
    result = result + h;
    
    /* Additional operations to create complex RTL */
    result = __builtin_shuffle(result, result,
        (int[16]){i1, i2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    
    return result;
}
#endif

/* Function with many integer arguments */
__attribute__((noinline))
long many_int_args(char a, short b, int c, long d,
                   unsigned char e, unsigned short f, 
                   unsigned int g, unsigned long h,
                   int8_t i, int16_t j, int32_t k) {
    /* Complex expression forcing many temporaries */
    long t1 = (long)a * b;
    long t2 = (long)c * d;
    long t3 = (long)e * f;
    long t4 = (long)g * h;
    long t5 = (long)i * j;
    
    /* Array indexing and pointer arithmetic */
    char arr[64];
    for (int idx = 0; idx < 64; idx++) {
        arr[idx] = (char)(idx * k);
    }
    
    /* Complex computation */
    long result = t1 + t2;
    result = result * t3;
    result = result / (t4 + 1);
    result = result ^ t5;
    result = result + (long)arr[c % 64];
    result = result - (long)arr[d % 64];
    
    /* Inline asm with 11 arguments */
    asm volatile (
        "add %1, %0\n\t"
        "sub %2, %0\n\t"
        "imul %3, %0\n\t"
        "xor %4, %0\n\t"
        "or %5, %0\n\t"
        "and %6, %0\n\t"
        "shl $3, %0\n\t"
        "shr $1, %0"
        : "+r" (result)
        : "r" (t1), "r" (t2), "r" (t3), "r" (t4), "r" (t5),
          "r" (k), "m" (arr[0]), "m" (arr[32]), "r" (b), "r" (c)
        : "cc", "memory"
    );
    
    return result;
}

int main() {
    const int N = 1024;
    float* in1 = aligned_alloc(64, N * sizeof(float));
    float* in2 = aligned_alloc(64, N * sizeof(float));
    float* in3 = aligned_alloc(64, N * sizeof(float));
    float* out = aligned_alloc(64, N * sizeof(float));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        in1[i] = (float)prng_u32() / (float)UINT32_MAX;
        in2[i] = (float)prng_u32() / (float)UINT32_MAX;
        in3[i] = (float)prng_u32() / (float)UINT32_MAX;
        out[i] = 0.0f;
    }
    
    /* Test the many-argument vector function */
    test_many_args(out, in1, in2, in3, N - 32);
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += (double)out[i];
    }
    
    printf("Vector checksum: %f\n", checksum);
    
    /* Test integer many-argument function */
    long int_result = many_int_args(
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    printf("Integer result: %ld\n", int_result);
    
    /* Test GCC vector builtin version */
#ifdef __GNUC__
    v16sf va = {0};
    v16sf vb = {0};
    v16sf vc = {0};
    v16sf vd = {0};
    v16sf ve = {0};
    v16sf vf = {0};
    v16sf vg = {0};
    v16sf vh = {0};
    
    for (int i = 0; i < 16; i++) {
        va[i] = (float)i;
        vb[i] = (float)(i + 16);
        vc[i] = (float)(i + 32);
        vd[i] = (float)(i + 48);
        ve[i] = 2.0f;
        vf[i] = 3.0f;
        vg[i] = 4.0f;
        vh[i] = 5.0f;
    }
    
    v16sf vresult = complex_shuffle_10_args(va, vb, vc, vd, ve, vf, vg, vh, 1, 2);
    
    float vchecksum = 0.0f;
    for (int i = 0; i < 16; i++) {
        vchecksum += vresult[i];
    }
    printf("Vector builtin checksum: %f\n", vchecksum);
#endif
    
    free(in1);
    free(in2);
    free(in3);
    free(out);
    
    return 0;
}
