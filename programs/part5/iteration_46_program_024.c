#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Inhibit optimization helpers */
static inline void inhibit_cse(volatile int* var) {
    asm volatile("" : "+r"(*var));
}

static inline void fake_dependency(void* ptr) {
    asm volatile("" : : "r"(ptr) : "memory");
}

/* Vector types for different operations */
typedef int32_t v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int64_t v2di __attribute__((vector_size(16)));

/* Complex expression builder that forces many temporaries */
static inline v4si build_complex_expr(v4si a, v4si b, v4si c, v4si d,
                                      v4si e, v4si f, v4si g, v4si h,
                                      int imm1, int imm2) {
    /* Create many intermediate values to force expander to handle many operands */
    v4si t1 = a + b;
    v4si t2 = c - d;
    v4si t3 = e * f;
    v4si t4 = g & h;
    v4si t5 = t1 | t2;
    v4si t6 = t3 ^ t4;
    v4si t7 = __builtin_shuffle(t5, t6, (v4si){imm1, imm2, 1, 3});
    v4si t8 = __builtin_shuffle(t7, t1, (v4si){2, 0, imm2, imm1});
    
    /* Force dependency chain */
    volatile int v_imm1 = imm1;
    volatile int v_imm2 = imm2;
    inhibit_cse(&v_imm1);
    inhibit_cse(&v_imm2);
    
    /* Complex shuffle with many arguments - targeting case 10 */
    v4si result = __builtin_shuffle(t1, t2, t3, t4, t5, t6, t7, t8,
                                   (v4si){v_imm1, v_imm2, 0, 1},
                                   (v4si){2, 3, v_imm1, v_imm2});
    
    return result;
}

#ifdef __AVX512F__
__attribute__((target("avx512f,avx512vl")))
static inline __m512i avx512_complex_op(__m512i a, __m512i b, __m512i c,
                                       __m512i d, __m512i e, __m512i f,
                                       __m512i g, __m512i h, __m512i i,
                                       __m512i j, int imm1, int imm2) {
    /* AVX-512 operation with 11 arguments targeting case 11 */
    __mmask16 mask = _mm512_int2mask(imm1);
    __m512i temp1 = _mm512_add_epi32(a, b);
    __m512i temp2 = _mm512_sub_epi32(c, d);
    __m512i temp3 = _mm512_mullo_epi32(e, f);
    __m512i temp4 = _mm512_and_si512(g, h);
    __m512i temp5 = _mm512_or_si512(i, j);
    
    /* Complex blend with many arguments */
    __m512i result = _mm512_mask_blend_epi32(mask, temp1, temp2);
    result = _mm512_mask_add_epi32(_mm512_int2mask(imm2), result, temp3, temp4);
    result = _mm512_mask_mullo_epi32(mask, result, temp5, a);
    
    return result;
}
#endif

/* Function with inline asm using 10-11 operands */
__attribute__((noinline, target("avx2")))
static void test_many_args(int32_t* output, const int32_t* input1,
                          const int32_t* input2, const int32_t* input3,
                          const int32_t* input4, int n) {
    volatile int counter = 0; /* Prevent loop unrolling */
    
    for (int i = 0; i < n; i += 4) {
        inhibit_cse(&counter);
        counter++;
        
        /* Load vectors */
        v4si v1 = *(v4si*)(input1 + i);
        v4si v2 = *(v4si*)(input2 + i);
        v4si v3 = *(v4si*)(input3 + i);
        v4si v4 = *(v4si*)(input4 + i);
        v4si v5 = v1 + v2;
        v4si v6 = v3 - v4;
        v4si v7 = v1 * v3;
        v4si v8 = v2 & v4;
        
        /* Complex inline asm with 10 operands */
        v4si result;
        asm volatile (
            "vpaddd %[res], %[v1], %[v2]\n\t"
            "vpsubd %[res], %[res], %[v3]\n\t"
            "vpmulld %[res], %[res], %[v4]\n\t"
            "vpand %[res], %[res], %[v5]\n\t"
            "vpor %[res], %[res], %[v6]\n\t"
            "vpxor %[res], %[res], %[v7]\n\t"
            "vpslld $%[imm1], %[res], %[res]\n\t"
            "vpsrad $%[imm2], %[res], %[res]\n\t"
            "vpsllvd %[res], %[res], %[v8]\n\t"
            : [res] "=x" (result)
            : [v1] "x" (v1), [v2] "x" (v2), [v3] "x" (v3),
              [v4] "x" (v4), [v5] "x" (v5), [v6] "x" (v6),
              [v7] "x" (v7), [v8] "x" (v8),
              [imm1] "i" (counter & 3), [imm2] "i" ((counter >> 2) & 3)
            : "memory"
        );
        
        /* Another complex expression with builtin */
        int imm1 = counter & 3;
        int imm2 = (counter >> 2) & 3;
        volatile int vimm1 = imm1;
        volatile int vimm2 = imm2;
        
        v4si complex_result = build_complex_expr(v1, v2, v3, v4, v5, v6, v7, v8,
                                                vimm1, vimm2);
        
        /* Store combined result */
        *(v4si*)(output + i) = result + complex_result;
        
        fake_dependency(output + i);
    }
}

/* Alternative function using vector convert operations */
__attribute__((noinline, target("avx2")))
static void test_vector_converts(float* output_f, const int32_t* input_i,
                                const float* input_f, int n) {
    for (int i = 0; i < n; i += 8) {
        /* Load multiple vectors */
        __m256i vi1 = _mm256_loadu_si256((__m256i*)(input_i + i));
        __m256i vi2 = _mm256_loadu_si256((__m256i*)(input_i + i + 8));
        __m256 vf1 = _mm256_loadu_ps(input_f + i);
        __m256 vf2 = _mm256_loadu_ps(input_f + i + 8);
        
        /* Complex chain of conversions and operations */
        __m256 temp1 = _mm256_cvtepi32_ps(vi1);
        __m256 temp2 = _mm256_cvtepi32_ps(vi2);
        
        /* Shuffle with many arguments */
        __m256 shuffled1 = _mm256_shuffle_ps(temp1, temp2, _MM_SHUFFLE(3, 2, 1, 0));
        __m256 shuffled2 = _mm256_shuffle_ps(temp2, temp1, _MM_SHUFFLE(0, 1, 2, 3));
        
        /* Blend with immediate */
        __m256 blended = _mm256_blend_ps(shuffled1, shuffled2, 0xCC);
        
        /* Permute with immediate */
        __m256 permuted = _mm256_permute2f128_ps(blended, blended, 0x01);
        
        /* Store result */
        _mm256_storeu_ps(output_f + i, _mm256_add_ps(permuted, vf1));
    }
}

/* Test with exactly 11 arguments using mixed types */
__attribute__((noinline))
static int64_t test_11_args_mixed(char a, short b, int c, long d,
                                 int8_t e, int16_t f, int32_t g, int64_t h,
                                 float i, double j, int* k) {
    /* Complex expression forcing many temporaries */
    long t1 = (long)a + b;
    long t2 = (long)c * d;
    long t3 = (long)e << f;
    long t4 = (long)g ^ h;
    
    /* Force memory access */
    volatile int* vk = k;
    inhibit_cse(vk);
    
    /* Mixed operations */
    double ft1 = (double)t1 + i;
    double ft2 = (double)t2 * j;
    double ft3 = (double)t3 / (i + 1.0f);
    double ft4 = (double)t4 - j;
    
    /* Complex inline asm with 11 operands */
    double result;
    asm volatile (
        "vmulsd %[res], %[ft1], %[ft2]\n\t"
        "vaddsd %[res], %[res], %[ft3]\n\t"
        "vsubsd %[res], %[res], %[ft4]\n\t"
        "vcvtsi2sd %[res], %[res], %[t1]\n\t"
        "vcvtsi2sd %[res], %[res], %[t2]\n\t"
        "vmulsd %[res], %[res], %[j]\n\t"
        "vdivsd %[res], %[res], qword ptr [%[k]]\n\t"
        : [res] "=x" (result)
        : [ft1] "x" (ft1), [ft2] "x" (ft2), [ft3] "x" (ft3),
          [ft4] "x" (ft4), [t1] "r" (t1), [t2] "r" (t2),
          [j] "x" (j), [k] "r" (vk)
        : "memory"
    );
    
    return (int64_t)result;
}

int main(void) {
    const int N = 1024;
    const int ITERS = 100;
    
    /* Allocate and initialize arrays with pseudo-random data */
    int32_t* input1 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* input2 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* input3 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* input4 = aligned_alloc(64, N * sizeof(int32_t));
    int32_t* output = aligned_alloc(64, N * sizeof(int32_t));
    float* input_f = aligned_alloc(64, N * sizeof(float));
    float* output_f = aligned_alloc(64, N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        input1[i] = (int32_t)prng_next();
        input2[i] = (int32_t)prng_next();
        input3[i] = (int32_t)prng_next();
        input4[i] = (int32_t)prng_next();
        input_f[i] = (float)prng_next() / (float)UINT32_MAX;
    }
    
    /* Run tests multiple times */
    for (int iter = 0; iter < ITERS; iter++) {
        test_many_args(output, input1, input2, input3, input4, N);
        test_vector_converts(output_f, input1, input_f, N);
        
        /* Test 11-argument function */
        int mem_val = prng_next();
        int64_t mixed_result = test_11_args_mixed(
            prng_next() & 0xFF,
            prng_next() & 0xFFFF,
            (int)prng_next(),
            (long)prng_next(),
            (int8_t)(prng_next() & 0xFF),
            (int16_t)(prng_next() & 0xFFFF),
            (int32_t)prng_next(),
            (int64_t)prng_next(),
            (float)prng_next() / (float)UINT32_MAX,
            (double)prng_next() / (double)UINT32_MAX,
            &mem_val
        );
        
        fake_dependency(&mixed_result);
    }
    
    /* Compute checksum */
    int64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += output[i];
        checksum += (int64_t)output_f[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(input1);
    free(input2);
    free(input3);
    free(input4);
    free(output);
    free(input_f);
    free(output_f);
    
    return 0;
}
