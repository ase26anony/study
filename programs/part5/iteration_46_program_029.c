#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Volatile counter to prevent loop unrolling */
static volatile int volatile_counter = 0;

/* Target-specific function with many arguments */
__attribute__((noinline, target("avx2,avx512f")))
void test_many_args(float* restrict out, 
                    const float* restrict in1,
                    const float* restrict in2,
                    const float* restrict in3,
                    const float* restrict in4,
                    int n) {
    /* Prevent constant propagation */
    int limit = n + volatile_counter;
    
    for (int i = 0; i < limit; i += 8) {
        /* Load multiple vectors - create many temporaries */
        __m256 v1 = _mm256_loadu_ps(&in1[i]);
        __m256 v2 = _mm256_loadu_ps(&in2[i]);
        __m256 v3 = _mm256_loadu_ps(&in3[i]);
        __m256 v4 = _mm256_loadu_ps(&in4[i]);
        
        /* Complex expression with many intermediate values */
        __m256 t1 = _mm256_add_ps(v1, v2);
        __m256 t2 = _mm256_sub_ps(v3, v4);
        __m256 t3 = _mm256_mul_ps(t1, t2);
        __m256 t4 = _mm256_fmadd_ps(v1, v2, v3);
        
        /* Create many immediate constants with dependencies */
        int imm1 = (i & 7) + 1;
        int imm2 = (i & 3) + 2;
        int imm3 = (i & 1) + 3;
        int imm4 = 4;
        int imm5 = 5;
        
        /* Use inline asm with many operands (10-11 arguments) */
        __m256 result;
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vaddps %2, %0, %0\n\t"
            "vmulps %3, %0, %0\n\t"
            "vfmadd213ps %4, %5, %0\n\t"
            "vpermilps $0x1B, %0, %0\n\t"
            "vblendps $0xAA, %6, %0, %0\n\t"
            "vshufps $0xE4, %7, %0, %0\n\t"
            "vinsertf128 $0x1, %8, %0, %0\n\t"
            "vroundps $0x8, %0, %0"
            : "=x"(result)
            : "x"(t1), "x"(t2), "x"(t3), "x"(t4), 
              "x"(v1), "x"(v2), "x"(v3), "x"(v4),
              "i"(imm1), "i"(imm2)
            : "memory"
        );
        
        /* Alternative: Use builtin with many arguments through complex expression */
        __m256 final_result;
        {
            /* Create dependency chain to prevent simplification */
            __m256 a1 = _mm256_add_ps(result, v1);
            __m256 a2 = _mm256_sub_ps(a1, v2);
            __m256 a3 = _mm256_mul_ps(a2, v3);
            __m256 a4 = _mm256_div_ps(a3, v4);
            __m256 a5 = _mm256_max_ps(a4, t1);
            __m256 a6 = _mm256_min_ps(a5, t2);
            __m256 a7 = _mm256_sqrt_ps(a6);
            __m256 a8 = _mm256_rcp_ps(a7);
            __m256 a9 = _mm256_rsqrt_ps(a8);
            
            /* Complex blend with many arguments simulated */
            __m256 blend_mask = _mm256_set_ps(
                (i & 1) ? 1.0f : -1.0f,
                (i & 2) ? 2.0f : -2.0f,
                (i & 4) ? 3.0f : -3.0f,
                (i & 8) ? 4.0f : -4.0f,
                5.0f, 6.0f, 7.0f, 8.0f
            );
            
            /* This complex operation may expand to many arguments */
            final_result = _mm256_blendv_ps(a9, blend_mask, 
                _mm256_cmp_ps(a9, blend_mask, _CMP_GT_OS));
        }
        
        _mm256_storeu_ps(&out[i], final_result);
    }
}

/* Another function using GCC vector builtins directly */
__attribute__((noinline, target("avx2")))
void test_vector_builtins(float* restrict out,
                         const float* restrict in,
                         int n) {
    typedef float v8sf __attribute__((vector_size(32)));
    
    for (int i = 0; i < n; i += 8) {
        /* Load vectors */
        v8sf v1 = *(v8sf*)&in[i];
        v8sf v2 = *(v8sf*)&in[i + 8];
        v8sf v3 = *(v8sf*)&in[i + 16];
        v8sf v4 = *(v8sf*)&in[i + 24];
        
        /* Complex expression that might trigger 10-11 argument optab */
        v8sf r1 = v1 + v2;
        v8sf r2 = v3 - v4;
        v8sf r3 = r1 * r2;
        v8sf r4 = __builtin_ia32_blendps256(r1, r2, 0xAA);
        v8sf r5 = __builtin_ia32_shufps256(r3, r4, 0x1B);
        
        /* Use __builtin_shuffle with many arguments */
        int idx[8] = {0, 2, 4, 6, 1, 3, 5, 7};
        v8sf shuffled = __builtin_shuffle(r5, r1, 
            idx[0], idx[1], idx[2], idx[3],
            idx[4], idx[5], idx[6], idx[7]);
        
        /* Store result */
        *(v8sf*)&out[i] = shuffled;
    }
}

/* Function with mixed integer types creating many temporaries */
__attribute__((noinline))
void test_mixed_types(char* out, const int* in, int n) {
    /* Complex pointer arithmetic and indexing */
    for (int i = 0; i < n; i++) {
        /* Create many intermediate values */
        char c1 = (in[i] >> 0) & 0xFF;
        char c2 = (in[i] >> 8) & 0xFF;
        char c3 = (in[i] >> 16) & 0xFF;
        char c4 = (in[i] >> 24) & 0xFF;
        
        short s1 = c1 * c2;
        short s2 = c3 * c4;
        short s3 = s1 + s2;
        short s4 = s1 - s2;
        
        int i1 = s3 * s4;
        int i2 = i1 ^ in[i];
        int i3 = i1 & in[i];
        int i4 = i1 | in[i];
        
        long l1 = (long)i1 * i2;
        long l2 = (long)i3 * i4;
        long l3 = l1 + l2;
        long l4 = l1 - l2;
        
        /* Complex expression that might expand to many arguments */
        out[i] = (char)(((l3 >> 32) & 0xFF) ^ 
                       ((l4 >> 24) & 0xFF) ^
                       ((i1 >> 16) & 0xFF) ^
                       ((i2 >> 8) & 0xFF) ^
                       (c1 & 0xFF));
    }
}

int main(void) {
    const int N = 1024;
    
    /* Allocate and initialize arrays with pseudo-random data */
    float* in1 = aligned_alloc(32, N * sizeof(float));
    float* in2 = aligned_alloc(32, N * sizeof(float));
    float* in3 = aligned_alloc(32, N * sizeof(float));
    float* in4 = aligned_alloc(32, N * sizeof(float));
    float* out = aligned_alloc(32, N * sizeof(float));
    
    int* int_data = malloc(N * sizeof(int));
    char* char_out = malloc(N * sizeof(char));
    
    for (int i = 0; i < N; i++) {
        in1[i] = (float)prng_next() / (float)UINT32_MAX;
        in2[i] = (float)prng_next() / (float)UINT32_MAX;
        in3[i] = (float)prng_next() / (float)UINT32_MAX;
        in4[i] = (float)prng_next() / (float)UINT32_MAX;
        int_data[i] = prng_next();
    }
    
    /* Call functions that may trigger many-argument optabs */
    test_many_args(out, in1, in2, in3, in4, N);
    test_vector_builtins(out, in1, N);
    test_mixed_types(char_out, int_data, N);
    
    /* Compute checksum */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += out[i];
    }
    
    int char_checksum = 0;
    for (int i = 0; i < N; i++) {
        char_checksum += char_out[i];
    }
    
    printf("Float checksum: %f\n", checksum);
    printf("Char checksum: %d\n", char_checksum);
    
    /* Cleanup */
    free(in1);
    free(in2);
    free(in3);
    free(in4);
    free(out);
    free(int_data);
    free(char_out);
    
    return 0;
}
