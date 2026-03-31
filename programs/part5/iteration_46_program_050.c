#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(__m256i* arr_i32, __m256i* arr_i16, __m256i* arr_i8,
                       __m256d* arr_f64, __m256* arr_f32, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr_i32[i] = _mm256_set_epi32(prng_next(), prng_next(), prng_next(), prng_next(),
                                      prng_next(), prng_next(), prng_next(), prng_next());
        arr_i16[i] = _mm256_set_epi16(prng_next(), prng_next(), prng_next(), prng_next(),
                                      prng_next(), prng_next(), prng_next(), prng_next(),
                                      prng_next(), prng_next(), prng_next(), prng_next(),
                                      prng_next(), prng_next(), prng_next(), prng_next());
        arr_i8[i] = _mm256_set_epi8(prng_next(), prng_next(), prng_next(), prng_next(),
                                    prng_next(), prng_next(), prng_next(), prng_next(),
                                    prng_next(), prng_next(), prng_next(), prng_next(),
                                    prng_next(), prng_next(), prng_next(), prng_next(),
                                    prng_next(), prng_next(), prng_next(), prng_next(),
                                    prng_next(), prng_next(), prng_next(), prng_next(),
                                    prng_next(), prng_next(), prng_next(), prng_next(),
                                    prng_next(), prng_next(), prng_next(), prng_next());
        arr_f64[i] = _mm256_set_pd((double)prng_next() / 1000.0, 
                                   (double)prng_next() / 1000.0,
                                   (double)prng_next() / 1000.0,
                                   (double)prng_next() / 1000.0);
        arr_f32[i] = _mm256_set_ps((float)prng_next() / 1000.0f,
                                   (float)prng_next() / 1000.0f,
                                   (float)prng_next() / 1000.0f,
                                   (float)prng_next() / 1000.0f,
                                   (float)prng_next() / 1000.0f,
                                   (float)prng_next() / 1000.0f,
                                   (float)prng_next() / 1000.0f,
                                   (float)prng_next() / 1000.0f);
    }
}

/* Complex expression with many temporaries to force optab expansion */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(__m256i* out, const __m256i* in1, const __m256i* in2,
                          const __m256i* in3, const __m256d* in4, const __m256* in5,
                          size_t size) {
    volatile size_t i = 0; /* Prevent loop unrolling */
    
    for (i = 0; i < size; i++) {
        /* Load multiple vectors */
        __m256i v1 = in1[i];
        __m256i v2 = in2[i];
        __m256i v3 = in3[i];
        __m256d f1 = in4[i];
        __m256 f2 = in5[i];
        
        /* Complex multi-statement expression with many temporaries */
        __m256i temp1, temp2, temp3, temp4, temp5;
        __m256d ftemp1, ftemp2;
        __m256 ftemp3, ftemp4;
        
        /* Create dependencies to inhibit CSE */
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3));
        
        /* Operation 1: Blend with many arguments (8 vector elements + 2 control) */
        temp1 = _mm256_blend_epi32(v1, v2, 0xAA); /* 10 args when expanded */
        
        /* Operation 2: Shuffle with complex pattern */
        temp2 = _mm256_shuffle_epi8(v1, v3);
        
        /* Complex inline asm with 11 operands to trigger case 11 */
        __m256i mask = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);
        int imm1 = 0x1F, imm2 = 0x3F, imm3 = 0x7F, imm4 = 0xFF;
        int imm5 = 0x1, imm6 = 0x2, imm7 = 0x4, imm8 = 0x8;
        
        asm volatile (
            "vpblendvb %[mask], %[v2], %[v1], %[temp3]\n\t"
            "vpaddd %[temp1], %[temp3], %[temp4]\n\t"
            "vpslld $4, %[temp4], %[temp5]"
            : [temp3] "=x"(temp3), [temp4] "=x"(temp4), [temp5] "=x"(temp5)
            : [v1] "x"(v1), [v2] "x"(v2), [mask] "x"(mask),
              [temp1] "x"(temp1), "i"(imm1), "i"(imm2), "i"(imm3), "i"(imm4),
              "m"(*in1), "m"(*in2) /* Memory constraints add more operands */
            : "memory"
        );
        
        /* Another asm with exactly 11 input operands */
        int idx1 = i & 0xF, idx2 = (i >> 4) & 0xF;
        int idx3 = (i >> 8) & 0xF, idx4 = (i >> 12) & 0xF;
        
        asm volatile (
            "/* Complex 11-operand operation */\n\t"
            "vmovdqa %[v1], %%ymm0\n\t"
            "vmovdqa %[v2], %%ymm1\n\t"
            "vmovdqa %[v3], %%ymm2\n\t"
            "vpblendd $%[blend1], %%ymm0, %%ymm1, %%ymm3\n\t"
            "vpblendd $%[blend2], %%ymm3, %%ymm2, %%ymm4\n\t"
            "vpermq $%[perm], %%ymm4, %[result]"
            : [result] "=x"(out[i])
            : [v1] "x"(v1), [v2] "x"(v2), [v3] "x"(v3),
              [blend1] "i"(0xCC), [blend2] "i"(0x33),
              [perm] "i"(0x1B),
              "r"(idx1), "r"(idx2), "r"(idx3), "r"(idx4),
              "m"(in1[i]), "m"(in2[i]) /* Total: 12 operands, triggers case 11 */
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "memory"
        );
        
        /* Use vector builtins with many arguments through wrapper */
        ftemp1 = _mm256_blend_pd(f1, _mm256_set1_pd(1.0), 0x5);
        ftemp2 = _mm256_permute4x64_pd(ftemp1, 0x1B);
        ftemp3 = _mm256_blend_ps(f2, _mm256_set1_ps(2.0f), 0xAA);
        ftemp4 = _mm256_permutevar8x32_ps(ftemp3, mask);
        
        /* Final store with memory barrier */
        asm volatile("" ::: "memory");
    }
}

/* AVX-512 specific function with mask registers (more arguments) */
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
static void test_avx512_many_args(__m512i* out, const __m512i* in1, 
                                 const __m512i* in2, const __m512i* in3,
                                 const __m512d* in4, size_t size) {
    volatile size_t i = 0;
    
    for (i = 0; i < size; i++) {
        __m512i v1 = in1[i];
        __m512i v2 = in2[i];
        __m512i v3 = in3[i];
        __m512d f1 = in4[i];
        
        __mmask8 k1 = 0xAA;
        __mmask16 k2 = 0xAAAA;
        __mmask8 k3 = 0x55;
        
        /* AVX-512 operations with mask registers add more arguments */
        __m512i temp1 = _mm512_mask_blend_epi32(k1, v1, v2);
        __m512i temp2 = _mm512_maskz_permutexvar_epi32(k2, v3, v1);
        
        /* Complex inline asm with mask registers */
        asm volatile (
            "vpmovm2d %k[mask1], %[temp3]\n\t"
            "vpmovm2d %k[mask2], %[temp4]\n\t"
            "vpord %[temp3], %[temp4], %[temp5]\n\t"
            "vpmadd52luq %[v1], %[v2], %[v3], %[result]"
            : [temp3] "=v"(temp1), [temp4] "=v"(temp2), 
              [temp5] "=v"(temp1), [result] "=v"(out[i])
            : [v1] "v"(v1), [v2] "v"(v2), [v3] "v"(v3),
              [mask1] "k"(k1), [mask2] "k"(k2),
              "i"(0x1), "i"(0x2), "i"(0x4), "i"(0x8),
              "m"(in1[i]), "m"(in2[i]), "m"(in3[i])
            : "memory"
        );
    }
}
#endif

/* ARM NEON version for portability */
#if defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>

__attribute__((noinline))
static void test_neon_many_args(int32x4_t* out, const int32x4_t* in1,
                               const int32x4_t* in2, const int32x4_t* in3,
                               const float32x4_t* in4, size_t size) {
    volatile size_t i = 0;
    
    for (i = 0; i < size; i++) {
        int32x4_t v1 = in1[i];
        int32x4_t v2 = in2[i];
        int32x4_t v3 = in3[i];
        float32x4_t f1 = in4[i];
        
        /* NEON operations with lane selection */
        int32x4_t temp1 = vaddq_s32(v1, v2);
        int32x4_t temp2 = vmulq_s32(v1, v3);
        
        /* Complex inline asm with many lane indices */
        int lane0 = 0, lane1 = 1, lane2 = 2, lane3 = 3;
        int imm0 = 0xFF, imm1 = 0x0F, imm2 = 0xF0, imm3 = 0xAA;
        
        asm volatile (
            "vtrn.32 %q[v1], %q[v2]\n\t"
            "vzip.32 %q[v3], %q[temp1]\n\t"
            "vtbl.8 %q[temp2], {%q[v1], %q[v2]}, %q[v3]"
            : [temp1] "+w"(temp1), [temp2] "=w"(temp2)
            : [v1] "w"(v1), [v2] "w"(v2), [v3] "w"(v3),
              "r"(lane0), "r"(lane1), "r"(lane2), "r"(lane3),
              "i"(imm0), "i"(imm1), "i"(imm2), "i"(imm3),
              "m"(in1[i]), "m"(in2[i])
            : "memory"
        );
        
        out[i] = vaddq_s32(temp1, temp2);
    }
}
#endif

/* Compute checksum for validation */
static uint64_t compute_checksum(const __m256i* data, size_t size) {
    uint64_t checksum = 0;
    const uint8_t* ptr = (const uint8_t*)data;
    size_t total_bytes = size * sizeof(__m256i);
    
    for (size_t i = 0; i < total_bytes; i++) {
        checksum = (checksum << 5) - checksum + ptr[i];
    }
    
    return checksum;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    const size_t VEC_SIZE = ARRAY_SIZE / 8; /* 8 int32 per __m256i */
    
    /* Allocate aligned memory */
    __m256i* arr_i32 = (__m256i*)_mm_malloc(ARRAY_SIZE * sizeof(int32_t), 32);
    __m256i* arr_i16 = (__m256i*)_mm_malloc(ARRAY_SIZE * sizeof(int16_t), 32);
    __m256i* arr_i8 = (__m256i*)_mm_malloc(ARRAY_SIZE * sizeof(int8_t), 32);
    __m256d* arr_f64 = (__m256d*)_mm_malloc(VEC_SIZE * sizeof(__m256d), 32);
    __m256* arr_f32 = (__m256*)_mm_malloc(VEC_SIZE * sizeof(__m256), 32);
    __m256i* out = (__m256i*)_mm_malloc(VEC_SIZE * sizeof(__m256i), 32);
    
    if (!arr_i32 || !arr_i16 || !arr_i8 || !arr_f64 || !arr_f32 || !out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(arr_i32, arr_i16, arr_i8, arr_f64, arr_f32, VEC_SIZE);
    
    /* Run the test with many-argument operations */
    test_many_args(out, arr_i32, arr_i16, arr_i8, arr_f64, arr_f32, VEC_SIZE);
    
#ifdef __AVX512F__
    /* Test AVX-512 if available */
    __m512i* avx512_out = (__m512i*)_mm_malloc(VEC_SIZE * sizeof(__m512i), 64);
    __m512i* avx512_in1 = (__m512i*)_mm_malloc(VEC_SIZE * sizeof(__m512i), 64);
    __m512i* avx512_in2 = (__m512i*)_mm_malloc(VEC_SIZE * sizeof(__m512i), 64);
    __m512i* avx512_in3 = (__m512i*)_mm_malloc(VEC_SIZE * sizeof(__m512i), 64);
    __m512d* avx512_f64 = (__m512d*)_mm_malloc(VEC_SIZE * sizeof(__m512d), 64);
    
    if (avx512_out && avx512_in1 && avx512_in2 && avx512_in3 && avx512_f64) {
        /* Initialize AVX-512 arrays */
        for (size_t i = 0; i < VEC_SIZE; i++) {
            avx512_in1[i] = _mm512_set_epi32(
                prng_next(), prng_next(), prng_next(), prng_next(),
                prng_next(), prng_next(), prng_next(), prng_next(),
                prng_next(), prng_next(), prng_next(), prng_next(),
                prng_next(), prng_next(), prng_next(), prng_next());
            avx512_in2[i] = _mm512_set_epi32(
                prng_next(), prng_next(), prng_next(), prng_next(),
                prng_next(), prng_next(), prng_next(), prng_next(),
                prng_next(), prng_next(), prng_next(), prng_next(),
                prng_next(), prng_next(), prng_next(), prng_next());
            avx512_f64[i] = _mm512_set_pd(
                (double)prng_next() / 1000.0, (double)prng_next() / 1000.0,
                (double)prng_next() / 1000.0, (double)prng_next() / 1000.0,
                (double)prng_next() / 1000.0, (double)prng_next() / 1000.0,
                (double)prng_next() / 1000.0, (double)prng_next() / 1000.0);
        }
        
        test_avx512_many_args(avx512_out, avx512_in1, avx512_in2, 
                             avx512_in3, avx512_f64, VEC_SIZE);
        
        _mm_free(avx512_out);
        _mm_free(avx512_in1);
        _mm_free(avx512_in2);
        _mm_free(avx512_in3);
        _mm_free(avx512_f64);
    }
#endif
    
    /* Compute and print checksum */
    uint64_t checksum = compute_checksum(out, VEC_SIZE);
    printf("Checksum: 0x%016llX\n", (unsigned long long)checksum);
    
    /* Cleanup */
    _mm_free(arr_i32);
    _mm_free(arr_i16);
    _mm_free(arr_i8);
    _mm_free(arr_f64);
    _mm_free(arr_f32);
    _mm_free(out);
    
    return 0;
}
