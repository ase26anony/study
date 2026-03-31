/* test_optabs_10_11.c - Test program to cover 10/11 operand cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile seed to prevent constant propagation */
static volatile int seed = 0;

/* ==================== Vector Shuffle Patterns ==================== */

/* Large vector shuffle that may require many operands */
typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(v16si* out, v16si a, v16si b) {
    /* Complex shuffle with many indices - may expand to many operands */
    *out = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23);
    use(out);
}

/* AVX-512 style mask operation */
#ifdef __AVX512F__
#include <immintrin.h>
__attribute__((noipa, noinline))
void test_avx512_mask(__m512i* out, __m512i a, __m512i b, __mmask16 mask) {
    /* Complex masked operation that may use many operands */
    *out = _mm512_mask_add_epi32(a, mask, a, b);
    use(out);
}
#endif

/* ==================== Atomic Operations ==================== */

__attribute__((noipa, noinline))
void test_atomic_ops(int* ptr, int* expected, int desired) {
    /* Atomic compare-exchange with many parameters */
    int success = __atomic_compare_exchange(ptr, expected, &desired, 
                                            0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    use(&success);
}

/* ==================== Inline Assembly ==================== */

__attribute__((noipa, noinline))
void test_multi_operand_asm(int* results) {
    /* Inline assembly with exactly 10 operands */
    int a = seed + 1, b = seed + 2, c = seed + 3, d = seed + 4;
    int e = seed + 5, f = seed + 6, g = seed + 7, h = seed + 8;
    int i = seed + 9, j = seed + 10;
    int out1, out2, out3, out4;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand test %0 = %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9 */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), 
          "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    asm volatile (
        "/* 11-operand test */\n\t"
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    results[0] = out1;
    results[1] = out2;
    use(results);
}

/* ==================== OpenMP SIMD ==================== */

#define N 1024
__attribute__((noipa, noinline))
void test_omp_simd(float* a, float* b, float* c, float* d) {
    int i;
    
    /* OpenMP SIMD with multiple clauses - may expand to many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:32) simdlen(8) safelen(16)
    for (i = 0; i < N; i++) {
        a[i] = b[i] * c[i] + d[i];
    }
    
    use(a);
    use(b);
    use(c);
    use(d);
}

/* ==================== Complex Built-in ==================== */

/* Simulate a complex built-in with many arguments */
typedef struct {
    int x, y, z, w;
} vec4_t;

__attribute__((noipa, noinline))
vec4_t test_complex_builtin(vec4_t a, vec4_t b, vec4_t c, vec4_t d,
                           int mask, int scale, int offset) {
    /* This might expand to a pattern with many operands */
    vec4_t result;
    result.x = (a.x & mask) * scale + offset;
    result.y = (b.y & mask) * scale + offset;
    result.z = (c.z & mask) * scale + offset;
    result.w = (d.w & mask) * scale + offset;
    
    /* Force complex computation */
    asm volatile ("" : "+r"(result.x), "+r"(result.y), 
                       "+r"(result.z), "+r"(result.w));
    
    use(&result);
    return result;
}

/* ==================== Main Test Driver ==================== */

int main(int argc, char** argv) {
    /* Initialize seed from program name to get varying but deterministic behavior */
    for (char* p = argv[0]; *p; p++) {
        seed = seed * 31 + *p;
    }
    seed &= 0x7FFFFFFF;
    
    /* Allocate test data */
    float* fa = aligned_alloc(32, N * sizeof(float));
    float* fb = aligned_alloc(32, N * sizeof(float));
    float* fc = aligned_alloc(32, N * sizeof(float));
    float* fd = aligned_alloc(32, N * sizeof(float));
    
    int atomic_var = seed;
    int atomic_expected = seed;
    int atomic_desired = seed + 100;
    
    int asm_results[4] = {0};
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        fb[i] = (float)(i + seed);
        fc[i] = (float)(i * 2 + seed);
        fd[i] = (float)(i * 3 + seed);
    }
    
    /* Run different tests based on seed to explore multiple paths */
    int test_case = seed % 6;
    
    switch (test_case) {
        case 0: {
            v16si va = {0}, vb = {0}, vout;
            for (int i = 0; i < 16; i++) {
                va[i] = i + seed;
                vb[i] = i * 2 + seed;
            }
            test_vector_shuffle(&vout, va, vb);
            break;
        }
        
        case 1:
            test_atomic_ops(&atomic_var, &atomic_expected, atomic_desired);
            break;
            
        case 2:
            test_multi_operand_asm(asm_results);
            break;
            
        case 3:
            test_omp_simd(fa, fb, fc, fd);
            break;
            
        case 4: {
            vec4_t v1 = {seed, seed+1, seed+2, seed+3};
            vec4_t v2 = {seed+4, seed+5, seed+6, seed+7};
            vec4_t v3 = {seed+8, seed+9, seed+10, seed+11};
            vec4_t v4 = {seed+12, seed+13, seed+14, seed+15};
            vec4_t r = test_complex_builtin(v1, v2, v3, v4, 0xFF, 2, 10);
            asm_results[0] = r.x + r.y + r.z + r.w;
            break;
        }
            
        #ifdef __AVX512F__
        case 5: {
            __m512i avx_a = _mm512_set1_epi32(seed);
            __m512i avx_b = _mm512_set1_epi32(seed * 2);
            __m512i avx_out;
            test_avx512_mask(&avx_out, avx_a, avx_b, 0xAAAA);
            break;
        }
        #endif
        
        default:
            /* Fallback to atomic test */
            test_atomic_ops(&atomic_var, &atomic_expected, atomic_desired);
            break;
    }
    
    /* Compute checksum to ensure all code has effect */
    int checksum = seed;
    checksum += atomic_var;
    checksum += atomic_expected;
    
    for (int i = 0; i < 4; i++) {
        checksum += asm_results[i];
    }
    
    for (int i = 0; i < N; i += 64) {
        checksum += (int)fa[i];
    }
    
    printf("Checksum: %d (seed: %d, test_case: %d)\n", checksum, seed, test_case);
    
    /* Cleanup */
    free(fa);
    free(fb);
    free(fc);
    free(fd);
    
    return 0;
}
