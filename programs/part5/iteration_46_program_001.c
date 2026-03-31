/* many_args_optab_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for deterministic testing */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Inhibit optimization helpers */
#define NO_INLINE __attribute__((noinline))
#define NO_OPTIMIZE __attribute__((optimize("O0")))
#define VOLATILE_VAR(var) asm volatile("" : "+r"(var))

/* Target-specific vector types */
#ifdef __AVX512F__
typedef __m512i v512i;
typedef __m512 v512f;
typedef __mmask16 m512mask;
#elif defined(__AVX2__)
typedef __m256i v256i;
typedef __m256 v256f;
#endif

typedef __m128i v128i;
typedef __m128 v128f;

/* Initialize arrays with pseudo-random data */
NO_INLINE static void init_arrays(v128i* vec_int, v128f* vec_float, 
                                   int* int_arr, float* float_arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        uint32_t r1 = prng_next();
        uint32_t r2 = prng_next();
        uint32_t r3 = prng_next();
        uint32_t r4 = prng_next();
        
        vec_int[i] = _mm_set_epi32(r4, r3, r2, r1);
        vec_float[i] = _mm_set_ps(*(float*)&r4, *(float*)&r3, 
                                   *(float*)&r2, *(float*)&r1);
        int_arr[i] = (int)r1;
        float_arr[i] = *(float*)&r1;
    }
}

/* Complex expression with many temporaries - forces expander to handle many args */
NO_INLINE NO_OPTIMIZE
static v128i complex_expression_10_args(v128i a, v128i b, v128i c, v128i d,
                                        v128i e, v128i f, v128i g, v128i h,
                                        int imm1, int imm2) {
    /* Create many intermediate values */
    v128i t1 = _mm_add_epi32(a, b);
    v128i t2 = _mm_sub_epi32(c, d);
    v128i t3 = _mm_mullo_epi32(e, f);
    v128i t4 = _mm_and_si128(g, h);
    v128i t5 = _mm_or_si128(t1, t2);
    v128i t6 = _mm_xor_si128(t3, t4);
    
    /* Force dependency chain */
    volatile int v_imm1 = imm1;
    volatile int v_imm2 = imm2;
    VOLATILE_VAR(v_imm1);
    VOLATILE_VAR(v_imm2);
    
    /* Complex shuffle with many arguments simulated through multiple operations */
    v128i shuffled = _mm_shuffle_epi32(t5, v_imm1);
    shuffled = _mm_shuffle_epi32(shuffled, v_imm2);
    
    /* Blend with dependency on all inputs */
    v128i result = _mm_blendv_epi8(shuffled, t6, _mm_cmpeq_epi32(t5, t6));
    
    return result;
}

/* Inline assembly with 11 operands - triggers optab expansion */
NO_INLINE NO_OPTIMIZE
static v128i asm_11_operands(v128i a, v128i b, v128i c, v128i d, v128i e,
                             v128i f, v128i g, v128i h, v128i i, v128i j,
                             int k) {
    v128i result;
    
    /* Extended asm with 11 input operands and memory clobber */
    asm volatile (
        "vpaddd %0, %1, %2\n\t"
        "vpsubd %0, %0, %3\n\t"
        "vpmulld %0, %0, %4\n\t"
        "vpand %0, %0, %5\n\t"
        "vpor %0, %0, %6\n\t"
        "vpxor %0, %0, %7\n\t"
        "vpshufd $0x%11$0x, %0, %0\n\t"
        "vpblendvb %0, %8, %9, %0\n\t"
        "vpaddd %0, %0, %10"
        : "=x"(result)
        : "x"(a), "x"(b), "x"(c), "x"(d), "x"(e), 
          "x"(f), "x"(g), "x"(h), "x"(i), "x"(j),
          "i"(k)
        : "memory"
    );
    
    return result;
}

/* Vector builtin with many arguments - using shufflevector */
NO_INLINE NO_OPTIMIZE
static v128i builtin_10_args(v128i a, v128i b, v128i c, v128i d) {
    /* __builtin_shufflevector can take many arguments for lane selection */
    typedef int v4si __attribute__((vector_size(16)));
    
    v4si va = (v4si)a;
    v4si vb = (v4si)b;
    v4si vc = (v4si)c;
    v4si vd = (v4si)d;
    
    /* Create a complex shuffle with 10 total arguments:
       2 source vectors + 8 lane indices */
    v4si result = __builtin_shufflevector(va, vb, 0, 4, 1, 5);
    
    /* Chain another shuffle to increase argument count in expansion */
    result = __builtin_shufflevector(result, vc, 3, 2, 1, 0);
    
    /* Final shuffle with all vectors involved */
    result = __builtin_shufflevector(result, vd, 
             ((int*)&va)[0] & 3,  /* These complex indices force */
             ((int*)&vb)[1] & 3,  /* the expander to keep them as */
             ((int*)&vc)[2] & 3,  /* separate arguments rather than */
             ((int*)&vd)[3] & 3); /* folding them */
    
    return (v128i)result;
}

/* Main test function with target attribute */
__attribute__((target("avx2")))
__attribute__((noinline))
static void test_many_args(v128i* output, v128i* input1, v128i* input2,
                           v128i* input3, v128i* input4, int* int_arr,
                           size_t n) {
    volatile size_t counter = 0;
    
    for (size_t idx = 0; idx < n; idx++) {
        VOLATILE_VAR(counter);
        
        /* Load multiple vectors */
        v128i a = input1[idx];
        v128i b = input2[idx];
        v128i c = input3[idx];
        v128i d = input4[idx];
        v128i e = _mm_set1_epi32(int_arr[(idx + 0) % n]);
        v128i f = _mm_set1_epi32(int_arr[(idx + 1) % n]);
        v128i g = _mm_set1_epi32(int_arr[(idx + 2) % n]);
        v128i h = _mm_set1_epi32(int_arr[(idx + 3) % n]);
        
        /* Create complex mask from array indices */
        int imm1 = int_arr[(idx + 4) % n] & 0xFF;
        int imm2 = int_arr[(idx + 5) % n] & 0xFF;
        
        /* Alternate between different many-argument patterns */
        v128i result;
        if (idx % 3 == 0) {
            /* 10-argument complex expression */
            result = complex_expression_10_args(a, b, c, d, e, f, g, h, 
                                                imm1, imm2);
        } else if (idx % 3 == 1) {
            /* 11-argument inline asm */
            v128i i = _mm_set1_epi32(int_arr[(idx + 6) % n]);
            v128i j = _mm_set1_epi32(int_arr[(idx + 7) % n]);
            result = asm_11_operands(a, b, c, d, e, f, g, h, i, j, imm1);
        } else {
            /* Builtin with many shuffle arguments */
            result = builtin_10_args(a, b, c, d);
        }
        
        output[idx] = result;
        counter++;
    }
}

/* Checksum function */
NO_INLINE static uint64_t compute_checksum(v128i* array, size_t n) {
    uint64_t checksum = 0;
    uint32_t* ptr = (uint32_t*)array;
    
    for (size_t i = 0; i < n * 4; i++) {
        checksum += ptr[i];
        checksum = (checksum << 13) | (checksum >> 51); /* rotate */
    }
    
    return checksum;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    const size_t VEC_ARRAY_SIZE = ARRAY_SIZE / 4;
    
    /* Allocate aligned memory for vector arrays */
    v128i* vec_int1 = (v128i*)_mm_malloc(VEC_ARRAY_SIZE * sizeof(v128i), 32);
    v128i* vec_int2 = (v128i*)_mm_malloc(VEC_ARRAY_SIZE * sizeof(v128i), 32);
    v128i* vec_int3 = (v128i*)_mm_malloc(VEC_ARRAY_SIZE * sizeof(v128i), 32);
    v128i* vec_int4 = (v128i*)_mm_malloc(VEC_ARRAY_SIZE * sizeof(v128i), 32);
    v128i* output = (v128i*)_mm_malloc(VEC_ARRAY_SIZE * sizeof(v128i), 32);
    
    v128f* vec_float = (v128f*)_mm_malloc(VEC_ARRAY_SIZE * sizeof(v128f), 32);
    int* int_arr = (int*)_mm_malloc(ARRAY_SIZE * sizeof(int), 32);
    float* float_arr = (float*)_mm_malloc(ARRAY_SIZE * sizeof(float), 32);
    
    if (!vec_int1 || !vec_int2 || !vec_int3 || !vec_int4 || !output ||
        !vec_float || !int_arr || !float_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(vec_int1, vec_float, int_arr, float_arr, VEC_ARRAY_SIZE);
    
    /* Copy and modify for different arrays */
    memcpy(vec_int2, vec_int1, VEC_ARRAY_SIZE * sizeof(v128i));
    memcpy(vec_int3, vec_int1, VEC_ARRAY_SIZE * sizeof(v128i));
    memcpy(vec_int4, vec_int1, VEC_ARRAY_SIZE * sizeof(v128i));
    
    /* Modify arrays slightly for diversity */
    for (size_t i = 0; i < VEC_ARRAY_SIZE; i++) {
        ((int*)&vec_int2[i])[0] ^= 0x55555555;
        ((int*)&vec_int3[i])[1] ^= 0xAAAAAAAA;
        ((int*)&vec_int4[i])[2] ^= 0x33333333;
    }
    
    /* Run the test */
    test_many_args(output, vec_int1, vec_int2, vec_int3, vec_int4, 
                   int_arr, VEC_ARRAY_SIZE);
    
    /* Compute and print checksum */
    uint64_t checksum = compute_checksum(output, VEC_ARRAY_SIZE);
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    /* Cleanup */
    _mm_free(vec_int1);
    _mm_free(vec_int2);
    _mm_free(vec_int3);
    _mm_free(vec_int4);
    _mm_free(output);
    _mm_free(vec_float);
    _mm_free(int_arr);
    _mm_free(float_arr);
    
    return 0;
}
