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

/* Initialize arrays with pseudo-random data */
static void init_arrays(__m256i* arr_i32, __m256i* arr_i16, __m256i* arr_i8,
                       __m256* arr_f32, __m256d* arr_f64, size_t size) {
    for (size_t i = 0; i < size; i++) {
        uint32_t vals[8];
        for (int j = 0; j < 8; j++) vals[j] = prng_next();
        arr_i32[i] = _mm256_set_epi32(vals[0], vals[1], vals[2], vals[3],
                                      vals[4], vals[5], vals[6], vals[7]);
        
        for (int j = 0; j < 16; j++) vals[j/2] = prng_next();
        arr_i16[i] = _mm256_set_epi16(
            vals[0] & 0xFFFF, vals[1] & 0xFFFF, vals[2] & 0xFFFF, vals[3] & 0xFFFF,
            vals[4] & 0xFFFF, vals[5] & 0xFFFF, vals[6] & 0xFFFF, vals[7] & 0xFFFF,
            vals[0] >> 16, vals[1] >> 16, vals[2] >> 16, vals[3] >> 16,
            vals[4] >> 16, vals[5] >> 16, vals[6] >> 16, vals[7] >> 16);
        
        for (int j = 0; j < 32; j++) vals[j/4] = prng_next();
        arr_i8[i] = _mm256_set_epi8(
            vals[0] & 0xFF, vals[1] & 0xFF, vals[2] & 0xFF, vals[3] & 0xFF,
            vals[4] & 0xFF, vals[5] & 0xFF, vals[6] & 0xFF, vals[7] & 0xFF,
            (vals[0] >> 8) & 0xFF, (vals[1] >> 8) & 0xFF, (vals[2] >> 8) & 0xFF, (vals[3] >> 8) & 0xFF,
            (vals[4] >> 8) & 0xFF, (vals[5] >> 8) & 0xFF, (vals[6] >> 8) & 0xFF, (vals[7] >> 8) & 0xFF,
            (vals[0] >> 16) & 0xFF, (vals[1] >> 16) & 0xFF, (vals[2] >> 16) & 0xFF, (vals[3] >> 16) & 0xFF,
            (vals[4] >> 16) & 0xFF, (vals[5] >> 16) & 0xFF, (vals[6] >> 16) & 0xFF, (vals[7] >> 16) & 0xFF,
            vals[0] >> 24, vals[1] >> 24, vals[2] >> 24, vals[3] >> 24,
            vals[4] >> 24, vals[5] >> 24, vals[6] >> 24, vals[7] >> 24);
        
        float fvals[8];
        for (int j = 0; j < 8; j++) fvals[j] = (float)(prng_next() % 1000) / 100.0f;
        arr_f32[i] = _mm256_set_ps(fvals[0], fvals[1], fvals[2], fvals[3],
                                   fvals[4], fvals[5], fvals[6], fvals[7]);
        
        double dvals[4];
        for (int j = 0; j < 4; j++) dvals[j] = (double)(prng_next() % 1000) / 100.0;
        arr_f64[i] = _mm256_set_pd(dvals[0], dvals[1], dvals[2], dvals[3]);
    }
}

/* Complex expression with many temporaries to force optab expansion */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(__m256i* out, const __m256i* in1, const __m256i* in2,
                          const __m256i* in3, const __m256* in_f32, 
                          const __m256d* in_f64, size_t size) {
    volatile size_t i = 0; /* Prevent loop unrolling */
    
    for (i = 0; i < size; i = i + 1) { /* volatile prevents optimization */
        /* Load multiple vectors */
        __m256i v1 = in1[i];
        __m256i v2 = in2[i];
        __m256i v3 = in3[i];
        __m256 fv1 = in_f32[i];
        __m256d dv1 = in_f64[i];
        
        /* Complex multi-statement expression with many temporaries */
        __m256i t1 = _mm256_add_epi32(v1, v2);
        __m256i t2 = _mm256_sub_epi32(v1, v3);
        __m256i t3 = _mm256_mullo_epi16(v2, v3);
        
        /* Create dependencies to inhibit CSE */
        asm volatile("" : "+x"(t1), "+x"(t2), "+x"(t3) : : "memory");
        
        /* Convert float to int with rounding - potentially many args */
        __m256i t4 = _mm256_cvtps_epi32(fv1);
        
        /* Complex shuffle with many arguments - targeting 10-11 args */
        /* Using inline asm with many operands */
        __m256i shuffle_result;
        asm volatile (
            "vpshufd $0x1B, %1, %0\n\t"           /* shuffle v1 */
            "vpaddd %2, %0, %0\n\t"               /* add t1 */
            "vpsubd %3, %0, %0\n\t"               /* subtract t2 */
            "vpmulld %4, %0, %0\n\t"              /* multiply by t3 */
            "vpaddd %5, %0, %0\n\t"               /* add t4 */
            "vpslld $2, %0, %0\n\t"               /* shift left */
            "vpsrld $1, %0, %0\n\t"               /* shift right */
            : "=x"(shuffle_result)
            : "x"(v1), "x"(t1), "x"(t2), "x"(t3), "x"(t4),
              "i"(0x1B), "i"(2), "i"(1), "m"(in1[i]), "m"(in2[i])
            : "memory"
        );
        
        /* Another complex operation using builtins with many args */
        /* This should trigger the 10-argument case in optabs */
        __m256i blend_result;
        asm volatile (
            "vpblendd $0xF0, %1, %2, %0\n\t"      /* blend v1 and v2 */
            "vpaddd %3, %0, %0\n\t"               /* add v3 */
            "vpsllvd %4, %0, %0\n\t"              /* variable shift by t1 */
            "vpsravd %5, %0, %0\n\t"              /* arithmetic shift by t2 */
            : "=x"(blend_result)
            : "x"(v1), "x"(v2), "x"(v3), "x"(t1), "x"(t2),
              "i"(0xF0), "m"(in3[i]), "m"(in_f32[i]), "m"(in_f64[i]), "i"(4)
            : "memory"
        );
        
        /* Combine results */
        __m256i final = _mm256_xor_si256(shuffle_result, blend_result);
        
        /* Store with memory barrier */
        asm volatile("" : : "m"(*out) : "memory");
        out[i] = final;
    }
}

/* Alternative approach using GCC vector extensions */
#ifdef __GNUC__
typedef int32_t v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

__attribute__((noinline, target("avx2")))
static void test_vector_builtins(v8si* out, const v8si* in1, const v8si* in2,
                                const v8si* in3, const v8sf* in_f32,
                                size_t size) {
    volatile size_t i = 0;
    
    for (i = 0; i < size; i = i + 1) {
        v8si v1 = in1[i];
        v8si v2 = in2[i];
        v8si v3 = in3[i];
        v8sf fv = in_f32[i];
        
        /* Complex expression tree with many operations */
        v8si t1 = v1 + v2;
        v8si t2 = v1 - v3;
        v8si t3 = v2 * v3;
        
        /* Inhibit constant propagation */
        asm volatile("" : "+x"(t1), "+x"(t2), "+x"(t3));
        
        /* Type conversion that might use optabs */
        v8si t4 = __builtin_convertvector(fv, v8si);
        
        /* Shuffle with computed indices - potentially many args */
        v8si indices = {7, 6, 5, 4, 3, 2, 1, 0};
        v8si shuffled = __builtin_shuffle(v1, v2, indices);
        
        /* Complex blend operation */
        v8si mask = (v1 > v2);
        v8si blended = __builtin_shufflevector(v1, v2, 
            mask[0] ? 0 : 8, mask[1] ? 1 : 9, mask[2] ? 2 : 10,
            mask[3] ? 3 : 11, mask[4] ? 4 : 12, mask[5] ? 5 : 13,
            mask[6] ? 6 : 14, mask[7] ? 7 : 15);
        
        /* Final complex expression */
        out[i] = (shuffled * t1 + blended * t2 - t3 * t4) >> 2;
    }
}
#endif

/* Compute checksum for validation */
static uint64_t compute_checksum(const __m256i* data, size_t size) {
    uint64_t checksum = 0;
    const uint32_t* ptr = (const uint32_t*)data;
    for (size_t i = 0; i < size * 8; i++) {
        checksum = checksum * 31 + ptr[i];
    }
    return checksum;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    
    /* Allocate aligned memory for vector arrays */
    __m256i* arr_i32_1 = aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    __m256i* arr_i32_2 = aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    __m256i* arr_i32_3 = aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    __m256i* arr_i16 = aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    __m256i* arr_i8 = aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    __m256* arr_f32 = aligned_alloc(32, ARRAY_SIZE * sizeof(__m256));
    __m256d* arr_f64 = aligned_alloc(32, ARRAY_SIZE * sizeof(__m256d));
    __m256i* output = aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    
    if (!arr_i32_1 || !arr_i32_2 || !arr_i32_3 || !arr_i16 || !arr_i8 ||
        !arr_f32 || !arr_f64 || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(arr_i32_1, arr_i16, arr_i8, arr_f32, arr_f64, ARRAY_SIZE);
    
    /* Copy and modify for other arrays */
    memcpy(arr_i32_2, arr_i32_1, ARRAY_SIZE * sizeof(__m256i));
    memcpy(arr_i32_3, arr_i32_1, ARRAY_SIZE * sizeof(__m256i));
    
    /* Add some variation */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        arr_i32_2[i] = _mm256_add_epi32(arr_i32_2[i], _mm256_set1_epi32(1));
        arr_i32_3[i] = _mm256_sub_epi32(arr_i32_3[i], _mm256_set1_epi32(1));
    }
    
    /* Test the many-argument function */
    test_many_args(output, arr_i32_1, arr_i32_2, arr_i32_3, arr_f32, arr_f64, ARRAY_SIZE);
    
#ifdef __GNUC__
    /* Also test with GCC vector builtins */
    test_vector_builtins((v8si*)output, (const v8si*)arr_i32_1,
                        (const v8si*)arr_i32_2, (const v8si*)arr_i32_3,
                        (const v8sf*)arr_f32, ARRAY_SIZE);
#endif
    
    /* Compute and print checksum */
    uint64_t checksum = compute_checksum(output, ARRAY_SIZE);
    printf("Checksum: %016llx\n", (unsigned long long)checksum);
    
    /* Cleanup */
    free(arr_i32_1);
    free(arr_i32_2);
    free(arr_i32_3);
    free(arr_i16);
    free(arr_i8);
    free(arr_f32);
    free(arr_f64);
    free(output);
    
    return 0;
}
