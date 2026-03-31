#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for deterministic results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Initialize arrays with pseudo-random data */
#define ARRAY_SIZE 1024
static float array_f32[ARRAY_SIZE];
static double array_f64[ARRAY_SIZE];
static int32_t array_i32[ARRAY_SIZE];
static int64_t array_i64[ARRAY_SIZE];
static float output_f32[ARRAY_SIZE];
static double output_f64[ARRAY_SIZE];

/* Function to inhibit optimization */
static inline void inhibit_opt(volatile int* var) {
    asm volatile("" : "+r"(*var));
}

/* Complex expression with many temporaries - forces expander to handle many operands */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args_avx512(void) {
    volatile int iter = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        inhibit_opt(&iter);
        
        /* Load multiple vectors - creates many operands */
        __m512 v0 = _mm512_loadu_ps(&array_f32[i]);
        __m512 v1 = _mm512_loadu_ps(&array_f32[i + 8]);
        __m512 v2 = _mm512_loadu_ps(&array_f32[i + 16]);
        __m512 v3 = _mm512_loadu_ps(&array_f32[i + 24]);
        __m512 v4 = _mm512_loadu_ps(&array_f32[i + 32]);
        
        /* Complex shuffle with many arguments - potentially 10+ operands */
        /* This creates a pattern that might expand to many operands */
        __mmask16 mask = 0xAAAA; /* 1010101010101010 */
        
        /* AVX-512 blend with mask - multiple vector arguments */
        __m512 blended = _mm512_mask_blend_ps(mask, v0, v1);
        
        /* Another blend with more vectors */
        __m512 blended2 = _mm512_mask_blend_ps(mask ^ 0x5555, v2, v3);
        
        /* Complex permute with immediate - could expand to many arguments */
        __m512 permuted = _mm512_permutexvar_ps(
            _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0),
            blended
        );
        
        /* FMA chain - creates dependency chain with many operands */
        __m512 result = _mm512_fmadd_ps(blended, blended2, v4);
        result = _mm512_fmadd_ps(result, permuted, _mm512_set1_ps(1.0f));
        
        /* Store result */
        _mm512_storeu_ps(&output_f32[i], result);
        
        iter++;
    }
}

/* Function using inline asm with 10-11 operands */
__attribute__((noinline, target("avx2")))
static void test_asm_many_args(void) {
    volatile int iter = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        inhibit_opt(&iter);
        
        /* Declare many vector variables */
        __m256 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        
        /* Load vectors */
        v0 = _mm256_loadu_ps(&array_f32[i]);
        v1 = _mm256_loadu_ps(&array_f32[i + 8]);
        v2 = _mm256_loadu_ps(&array_f32[i + 16]);
        v3 = _mm256_loadu_ps(&array_f32[i + 24]);
        v4 = _mm256_loadu_ps(&array_f32[i + 32]);
        v5 = _mm256_loadu_ps(&array_f32[i + 40]);
        v6 = _mm256_loadu_ps(&array_f32[i + 48]);
        v7 = _mm256_loadu_ps(&array_f32[i + 56]);
        
        /* Inline asm with 11 operands - triggers optab expansion */
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vaddps %2, %0, %0\n\t"
            "vmulps %3, %0, %0\n\t"
            "vsubps %4, %0, %0\n\t"
            "vdivps %5, %0, %0\n\t"
            "vfmadd132ps %6, %7, %0\n\t"
            "vfnmadd132ps %8, %9, %0\n\t"
            "vblendvps %10, %0, %11, %0"
            : "=x"(v8)
            : "x"(v0), "x"(v1), "x"(v2), "x"(v3), 
              "x"(v4), "x"(v5), "x"(v6), "x"(v7),
              "x"(v0), "x"(v1), "x"(v2)
            : "memory"
        );
        
        /* Store result */
        _mm256_storeu_ps(&output_f32[i], v8);
        
        iter++;
    }
}

/* Complex integer operations with many arguments */
__attribute__((noinline, target("avx2")))
static void test_many_int_args(void) {
    volatile int iter = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        inhibit_opt(&iter);
        
        /* Load integer vectors */
        __m256i iv0 = _mm256_loadu_si256((__m256i*)&array_i32[i]);
        __m256i iv1 = _mm256_loadu_si256((__m256i*)&array_i32[i + 8]);
        __m256i iv2 = _mm256_loadu_si256((__m256i*)&array_i32[i + 16]);
        __m256i iv3 = _mm256_loadu_si256((__m256i*)&array_i32[i + 24]);
        __m256i iv4 = _mm256_loadu_si256((__m256i*)&array_i32[i + 32]);
        __m256i iv5 = _mm256_loadu_si256((__m256i*)&array_i32[i + 40]);
        
        /* Complex shuffle with many lane indices - could expand to 10+ arguments */
        __m256i shuffled = _mm256_shuffle_epi32(iv0, _MM_SHUFFLE(3,2,1,0));
        
        /* Blend with immediate control - multiple arguments */
        __m256i blended = _mm256_blend_epi32(iv1, iv2, 0xAA);
        
        /* Permute var with index vector - many operands */
        __m256i indices = _mm256_set_epi32(7,6,5,4,3,2,1,0);
        __m256i permuted = _mm256_permutevar8x32_epi32(iv3, indices);
        
        /* Multiple arithmetic operations */
        __m256i added = _mm256_add_epi32(shuffled, blended);
        __m256i multiplied = _mm256_mullo_epi32(added, permuted);
        __m256i result = _mm256_slli_epi32(multiplied, 2);
        
        /* Blend with another vector */
        result = _mm256_blendv_epi8(result, iv4, iv5);
        
        /* Store result */
        _mm256_storeu_si256((__m256i*)&array_i32[i], result);
        
        iter++;
    }
}

/* Function using GCC vector builtins with many arguments */
__attribute__((noinline))
static void test_vector_builtins(void) {
    typedef float v8sf __attribute__((vector_size(32)));
    typedef int v8si __attribute__((vector_size(32)));
    
    volatile int iter = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        inhibit_opt(&iter);
        
        /* Load vectors */
        v8sf v0 = *(v8sf*)&array_f32[i];
        v8sf v1 = *(v8sf*)&array_f32[i + 8];
        v8sf v2 = *(v8sf*)&array_f32[i + 16];
        v8sf v3 = *(v8sf*)&array_f32[i + 24];
        v8sf v4 = *(v8sf*)&array_f32[i + 32];
        v8sf v5 = *(v8sf*)&array_f32[i + 40];
        
        /* Create index vector for shuffle */
        v8si idx = {7,6,5,4,3,2,1,0};
        
        /* Complex expression with many vector operations */
        v8sf temp1 = v0 + v1;
        v8sf temp2 = v2 * v3;
        v8sf temp3 = temp1 - temp2;
        v8sf temp4 = __builtin_shuffle(temp3, idx);
        v8sf temp5 = __builtin_shuffle(v4, v5, idx);
        v8sf result = temp4 + temp5;
        
        /* Additional operations to increase operand count */
        result = result * __builtin_shuffle(v0, v1, idx);
        result = result - __builtin_shuffle(v2, v3, idx);
        
        /* Store result */
        *(v8sf*)&output_f32[i] = result;
        
        iter++;
    }
}

/* Main test function */
int main(void) {
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_f32[i] = (float)prng_next() / (float)UINT32_MAX;
        array_f64[i] = (double)prng_next() / (double)UINT32_MAX;
        array_i32[i] = (int32_t)prng_next();
        array_i64[i] = (int64_t)prng_next() | ((int64_t)prng_next() << 32);
    }
    
    printf("Testing many-argument operations...\n");
    
    /* Test different functions that use many arguments */
#ifdef __AVX512F__
    printf("Testing AVX-512 many args...\n");
    test_many_args_avx512();
#endif
    
#ifdef __AVX2__
    printf("Testing inline asm with many args...\n");
    test_asm_many_args();
    
    printf("Testing integer many args...\n");
    test_many_int_args();
#endif
    
    printf("Testing vector builtins...\n");
    test_vector_builtins();
    
    /* Compute checksum */
    float checksum_f32 = 0.0f;
    double checksum_f64 = 0.0;
    int64_t checksum_i32 = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum_f32 += output_f32[i];
        checksum_f64 += output_f64[i];
        checksum_i32 += array_i32[i];
    }
    
    printf("Checksums:\n");
    printf("  Float32: %f\n", checksum_f32);
    printf("  Float64: %lf\n", checksum_f64);
    printf("  Int32: %ld\n", checksum_i32);
    
    return 0;
}
