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
static void init_arrays(__m256i* vec_int, __m256* vec_float, 
                       __m256d* vec_double, int size) {
    for (int i = 0; i < size; i++) {
        uint32_t data[8];
        for (int j = 0; j < 8; j++) data[j] = prng_next();
        vec_int[i] = _mm256_loadu_si256((const __m256i*)data);
        
        float fdata[8];
        for (int j = 0; j < 8; j++) fdata[j] = (float)(prng_next() % 1000) / 100.0f;
        vec_float[i] = _mm256_loadu_ps(fdata);
        
        double ddata[4];
        for (int j = 0; j < 4; j++) ddata[j] = (double)(prng_next() % 1000) / 100.0;
        vec_double[i] = _mm256_loadu_pd(ddata);
    }
}

/* Complex expression with many temporaries - forces expander to create many operands */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(__m256i* restrict out_int,
                          __m256* restrict out_float,
                          __m256d* restrict out_double,
                          const __m256i* restrict in_int,
                          const __m256* restrict in_float,
                          const __m256d* restrict in_double,
                          int size) {
    
    /* Volatile counter to prevent loop unrolling */
    volatile int vcounter = 0;
    
    for (int i = 0; i < size; i++) {
        vcounter = i;
        
        /* Load multiple vectors - creates many SSA values */
        __m256i v0 = in_int[i];
        __m256i v1 = in_int[(i + 1) % size];
        __m256i v2 = in_int[(i + 2) % size];
        __m256i v3 = in_int[(i + 3) % size];
        
        __m256 f0 = in_float[i];
        __m256 f1 = in_float[(i + 1) % size];
        __m256 f2 = in_float[(i + 2) % size];
        
        __m256d d0 = in_double[i];
        __m256d d1 = in_double[(i + 1) % size];
        
        /* Complex multi-step operation with many intermediate values */
        /* This should create a complex expression tree */
        
        /* Step 1: Integer operations with many arguments */
        __m256i temp1 = _mm256_add_epi32(v0, v1);
        __m256i temp2 = _mm256_sub_epi32(v2, v3);
        __m256i temp3 = _mm256_mullo_epi32(temp1, temp2);
        
        /* Step 2: Floating point operations */
        __m256 ftemp1 = _mm256_add_ps(f0, f1);
        __m256 ftemp2 = _mm256_sub_ps(f2, f0);
        __m256 ftemp3 = _mm256_mul_ps(ftemp1, ftemp2);
        
        /* Step 3: Double precision operations */
        __m256d dtemp1 = _mm256_add_pd(d0, d1);
        __m256d dtemp2 = _mm256_sub_pd(d0, d1);
        __m256d dtemp3 = _mm256_mul_pd(dtemp1, dtemp2);
        
        /* Create complex shuffle/permute with many arguments */
        /* Using inline asm with 10-11 operands to trigger the optab */
        
        /* First, create some volatile variables to inhibit optimization */
        volatile int imm0 = i & 7;
        volatile int imm1 = (i + 1) & 7;
        volatile int imm2 = (i + 2) & 7;
        volatile int imm3 = (i + 3) & 7;
        volatile int imm4 = (i + 4) & 7;
        volatile int imm5 = (i + 5) & 7;
        volatile int imm6 = (i + 6) & 7;
        volatile int imm7 = (i + 7) & 7;
        
        /* Extended inline asm with 11 operands - should trigger case 11 */
        __m256i shuffle_result;
        asm volatile (
            /* Complex operation with many inputs */
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpslld %0, %0, %5\n\t"
            "vpsrld %0, %0, %6\n\t"
            "vpand %0, %0, %7\n\t"
            "vpor %0, %0, %8\n\t"
            "vpxor %0, %0, %9\n\t"
            "vpshufb %0, %0, %10"
            : "=x"(shuffle_result)
            : "x"(v0), "x"(v1), "x"(v2), "x"(v3),
              "r"(imm0), "r"(imm1), "r"(imm2), "r"(imm3),
              "x"(temp3), "x"(temp1)
            : "memory"
        );
        
        /* Another asm with 10 operands - should trigger case 10 */
        __m256 blend_result;
        asm volatile (
            /* Blend operation with many control inputs */
            "vblendvps %0, %1, %2, %3\n\t"
            "vaddps %0, %0, %4\n\t"
            "vmulps %0, %0, %5\n\t"
            "vsubps %0, %0, %6\n\t"
            "vdivps %0, %0, %7\n\t"
            "vmaxps %0, %0, %8\n\t"
            "vminps %0, %0, %9"
            : "=x"(blend_result)
            : "x"(f0), "x"(f1), "x"(ftemp3),
              "x"(f2), "x"(ftemp1), "x"(ftemp2),
              "x"(ftemp3), "x"(f0), "x"(f1)
            : "memory"
        );
        
        /* Complex builtin with many arguments - vector conversion chain */
        /* This creates a dependency chain with many temporaries */
        __m256d final_double;
        {
            /* Multi-step conversion with many intermediate values */
            __m128i lo = _mm256_castsi256_si128(shuffle_result);
            __m128i hi = _mm256_extracti128_si256(shuffle_result, 1);
            
            /* Create complex expression with many operands */
            __m128i converted1 = _mm_cvtepi32_epi64(lo);
            __m128i converted2 = _mm_cvtepi32_epi64(hi);
            
            __m128d dbl1 = _mm_cvtepi32_pd(_mm256_castsi256_si128(v0));
            __m128d dbl2 = _mm_cvtepi32_pd(hi);
            
            /* Combine using inline asm with many operands */
            asm volatile (
                "vmovdqa %0, %1\n\t"
                "vmovdqa %2, %3\n\t"
                "vpunpcklqdq %0, %0, %4\n\t"
                "vpunpckhqdq %2, %2, %5\n\t"
                "vinsertf128 %0, %0, %6, 1\n\t"
                "vaddpd %0, %0, %7\n\t"
                "vmulpd %0, %0, %8\n\t"
                "vsubpd %0, %0, %9"
                : "=&x"(final_double)
                : "x"(dbl1), "x"(dbl2), "x"(dtemp3),
                  "x"(dtemp1), "x"(dtemp2),
                  "x"(_mm256_castpd256_pd128(d0)),
                  "x"(d1), "x"(d0), "x"(d1)
                : "memory"
            );
        }
        
        /* Store results */
        out_int[i] = shuffle_result;
        out_float[i] = blend_result;
        out_double[i] = final_double;
    }
}

/* Alternative approach using GCC vector extensions */
#ifdef __GNUC__
typedef int32_t v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

__attribute__((noinline, target("avx2")))
static void test_vector_extensions(v8si* out_vec, const v8si* in_vec, int size) {
    volatile int counter = 0;
    
    for (int i = 0; i < size; i++) {
        counter = i;
        
        /* Load multiple vectors */
        v8si v0 = in_vec[i];
        v8si v1 = in_vec[(i + 1) % size];
        v8si v2 = in_vec[(i + 2) % size];
        v8si v3 = in_vec[(i + 3) % size];
        v8si v4 = in_vec[(i + 4) % size];
        v8si v5 = in_vec[(i + 5) % size];
        
        /* Complex expression with many operands */
        /* GCC might expand this into a single optab call with many arguments */
        v8si result = (v0 + v1) * (v2 - v3) | (v4 & v5) ^ 
                     (v0 << 2) | (v1 >> 1) & (v2 << 3) | (v3 >> 2);
        
        /* Add more complexity with conditional operations */
        result = result + (v0 > v1 ? v2 : v3) * (v4 < v5 ? v0 : v1);
        
        /* Use __builtin_shuffle with many arguments */
        int indices[8] = {0, 7, 1, 6, 2, 5, 3, 4};
        v8si shuffled = __builtin_shuffle(v0, v1, 
            indices[0], indices[1], indices[2], indices[3],
            indices[4], indices[5], indices[6], indices[7]);
        
        /* Final complex operation */
        out_vec[i] = result + shuffled * v2 - v3 / (v4 + 1);
    }
}
#endif

/* Compute checksum for validation */
static uint64_t compute_checksum(const void* data, size_t size) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint64_t checksum = 0;
    for (size_t i = 0; i < size; i++) {
        checksum = checksum * 31 + bytes[i];
    }
    return checksum;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    
    /* Allocate aligned memory for vector arrays */
    __m256i* vec_int_in = (__m256i*)_mm_malloc(ARRAY_SIZE * sizeof(__m256i), 32);
    __m256i* vec_int_out = (__m256i*)_mm_malloc(ARRAY_SIZE * sizeof(__m256i), 32);
    
    __m256* vec_float_in = (__m256*)_mm_malloc(ARRAY_SIZE * sizeof(__m256), 32);
    __m256* vec_float_out = (__m256*)_mm_malloc(ARRAY_SIZE * sizeof(__m256), 32);
    
    __m256d* vec_double_in = (__m256d*)_mm_malloc(ARRAY_SIZE * sizeof(__m256d), 32);
    __m256d* vec_double_out = (__m256d*)_mm_malloc(ARRAY_SIZE * sizeof(__m256d), 32);
    
    if (!vec_int_in || !vec_int_out || !vec_float_in || !vec_float_out ||
        !vec_double_in || !vec_double_out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize input arrays */
    init_arrays(vec_int_in, vec_float_in, vec_double_in, ARRAY_SIZE);
    
    /* Run the test with many-argument operations */
    test_many_args(vec_int_out, vec_float_out, vec_double_out,
                   vec_int_in, vec_float_in, vec_double_in,
                   ARRAY_SIZE);
    
#ifdef __GNUC__
    /* Test with GCC vector extensions */
    v8si* vec_ext_in = (v8si*)_mm_malloc(ARRAY_SIZE * sizeof(v8si), 32);
    v8si* vec_ext_out = (v8si*)_mm_malloc(ARRAY_SIZE * sizeof(v8si), 32);
    
    if (vec_ext_in && vec_ext_out) {
        memcpy(vec_ext_in, vec_int_in, ARRAY_SIZE * sizeof(v8si));
        test_vector_extensions(vec_ext_out, vec_ext_in, ARRAY_SIZE);
        _mm_free(vec_ext_in);
        _mm_free(vec_ext_out);
    }
#endif
    
    /* Compute and print checksums */
    uint64_t checksum_int = compute_checksum(vec_int_out, ARRAY_SIZE * sizeof(__m256i));
    uint64_t checksum_float = compute_checksum(vec_float_out, ARRAY_SIZE * sizeof(__m256));
    uint64_t checksum_double = compute_checksum(vec_double_out, ARRAY_SIZE * sizeof(__m256d));
    
    printf("Checksums:\n");
    printf("  Integer:   %016llx\n", (unsigned long long)checksum_int);
    printf("  Float:     %016llx\n", (unsigned long long)checksum_float);
    printf("  Double:    %016llx\n", (unsigned long long)checksum_double);
    
    /* Cleanup */
    _mm_free(vec_int_in);
    _mm_free(vec_int_out);
    _mm_free(vec_float_in);
    _mm_free(vec_float_out);
    _mm_free(vec_double_in);
    _mm_free(vec_double_out);
    
    return 0;
}
