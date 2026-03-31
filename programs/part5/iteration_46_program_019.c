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
static void init_arrays(__m256i* arr_int, __m256* arr_float, 
                       __m256d* arr_double, size_t size) {
    for (size_t i = 0; i < size; i++) {
        uint32_t vals[8];
        for (int j = 0; j < 8; j++) vals[j] = prng_next();
        arr_int[i] = _mm256_set_epi32(vals[7], vals[6], vals[5], vals[4],
                                     vals[3], vals[2], vals[1], vals[0]);
        
        float fvals[8];
        for (int j = 0; j < 8; j++) fvals[j] = (float)prng_next() / 4294967296.0f;
        arr_float[i] = _mm256_set_ps(fvals[7], fvals[6], fvals[5], fvals[4],
                                    fvals[3], fvals[2], fvals[1], fvals[0]);
        
        double dvals[4];
        for (int j = 0; j < 4; j++) dvals[j] = (double)prng_next() / 4294967296.0;
        arr_double[i] = _mm256_set_pd(dvals[3], dvals[2], dvals[1], dvals[0]);
    }
}

/* Complex expression with many temporaries - forces expander to handle many operands */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(__m256i* out, const __m256i* in1, const __m256i* in2,
                          const __m256* fin, const __m256d* din, size_t n) {
    volatile size_t counter = 0; /* Prevent loop unrolling */
    
    for (size_t i = 0; i < n; i++) {
        /* Load multiple vectors - creates many SSA values */
        __m256i v1 = in1[i];
        __m256i v2 = in2[i];
        __m256 fv = fin[i];
        __m256d dv = din[i % (n/2 + 1)];
        
        /* Create complex dependency chain with many intermediate values */
        __m256i t1 = _mm256_add_epi32(v1, v2);
        __m256i t2 = _mm256_sub_epi32(v1, v2);
        __m256i t3 = _mm256_mullo_epi32(t1, t2);
        
        /* Convert float to int with rounding - uses multiple arguments internally */
        __m256i t4 = _mm256_cvtps_epi32(fv);
        
        /* Complex blend operation with many arguments simulated through inline asm */
        __m256i mask = _mm256_set1_epi32(0xFFFFFFFF);
        __m256i result;
        
        /* Inline asm with 11 operands - should trigger the 11-argument case */
        asm volatile (
            "vpblendvb %[mask], %[src1], %[src2], %[dst]\n\t"
            : [dst] "=x" (result)
            : [src1] "x" (t3), 
              [src2] "x" (t4),
              [mask] "x" (mask),
              "m" (in1[i]),  /* Memory operand 1 */
              "m" (in2[i]),  /* Memory operand 2 */
              "m" (fin[i]),  /* Memory operand 3 */
              "r" (i),       /* Integer operand 1 */
              "r" (counter), /* Integer operand 2 */
              "r" (n),       /* Integer operand 3 */
              "i" (255),     /* Immediate operand 1 */
              "i" (0)        /* Immediate operand 2 */
            : "memory"
        );
        
        /* Another complex operation using builtin shuffle with many arguments */
        int indices[8] = {7, 6, 5, 4, 3, 2, 1, 0};
        __m256i shuffled;
        
        /* Simulate a 10-argument operation through multiple steps */
        asm volatile (
            "vpermq %[src], %[dst], %[imm]\n\t"
            : [dst] "=x" (shuffled)
            : [src] "x" (result),
              [imm] "i" (0x1B),  /* 0b011011 = 27 = reverse order */
              "m" (indices[0]),  /* Memory operands for indices */
              "m" (indices[1]),
              "m" (indices[2]),
              "m" (indices[3]),
              "m" (indices[4]),
              "m" (indices[5]),
              "m" (indices[6]),
              "m" (indices[7])
            : "memory"
        );
        
        /* Final store with dependency on all previous operations */
        out[i] = _mm256_add_epi32(result, shuffled);
        
        /* Volatile update to prevent optimizations */
        counter = counter + 1;
    }
}

/* Alternative approach using GCC vector extensions */
#ifdef __GNUC__
typedef int32_t v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

__attribute__((noinline, target("avx2")))
static void test_vector_builtins(v8si* out, const v8si* in1, const v8si* in2,
                                const v8sf* fin, size_t n) {
    /* Use __builtin_shuffle with many arguments */
    for (size_t i = 0; i < n; i++) {
        v8si a = in1[i];
        v8si b = in2[i];
        
        /* Create a complex shuffle pattern - GCC might expand this as multi-arg optab */
        v8si shuffled = __builtin_shuffle(a, b, 
            (v8si){7, 6, 5, 4, 3, 2, 1, 0});  /* 8 arguments + 2 vectors = 10 total */
        
        /* Convert vector types - another potential multi-argument operation */
        v8sf f = fin[i];
        v8si converted = __builtin_convertvector(f, v8si);  /* 2 arguments */
        
        /* Complex expression tree */
        v8si temp1 = a + b;
        v8si temp2 = a - b;
        v8si temp3 = temp1 * temp2;
        v8si temp4 = shuffled & converted;
        v8si temp5 = temp3 | temp4;
        v8si temp6 = temp5 ^ shuffled;
        
        /* Final result using all temporaries */
        out[i] = temp6 + converted;
    }
}
#endif

/* Compute checksum for validation */
static uint64_t compute_checksum(const __m256i* data, size_t n) {
    uint64_t sum = 0;
    const uint32_t* ptr = (const uint32_t*)data;
    for (size_t i = 0; i < n * 8; i++) {
        sum += ptr[i];
    }
    return sum;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    const size_t VEC_COUNT = ARRAY_SIZE / sizeof(__m256i);
    
    /* Allocate aligned memory for vector arrays */
    __m256i* vec_int1 = (__m256i*)_mm_malloc(ARRAY_SIZE, 32);
    __m256i* vec_int2 = (__m256i*)_mm_malloc(ARRAY_SIZE, 32);
    __m256i* vec_out = (__m256i*)_mm_malloc(ARRAY_SIZE, 32);
    __m256* vec_float = (__m256*)_mm_malloc(ARRAY_SIZE, 32);
    __m256d* vec_double = (__m256d*)_mm_malloc(ARRAY_SIZE, 32);
    
    if (!vec_int1 || !vec_int2 || !vec_out || !vec_float || !vec_double) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(vec_int1, vec_float, vec_double, VEC_COUNT);
    init_arrays(vec_int2, vec_float, vec_double, VEC_COUNT); /* Reuse for second array */
    
    /* Run the test with many-argument operations */
    test_many_args(vec_out, vec_int1, vec_int2, vec_float, vec_double, VEC_COUNT);
    
    /* Compute and print checksum */
    uint64_t checksum = compute_checksum(vec_out, VEC_COUNT);
    printf("Checksum: 0x%016lx\n", checksum);
    
    /* Cleanup */
    _mm_free(vec_int1);
    _mm_free(vec_int2);
    _mm_free(vec_out);
    _mm_free(vec_float);
    _mm_free(vec_double);
    
    return 0;
}
