/* test_optabs_10_11.c - Cover 10/11 operand expansion cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==================== Pattern 1: Vector Shuffle with Many Elements ==================== */

typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(v16si* result, v16si a, v16si b) {
    /* 16-element shuffle = 18 total operands (2 inputs + 16 indices) */
    *result = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7,
        16, 17, 18, 19, 20, 21, 22, 23);
}

__attribute__((noipa, noinline))
void test_double_shuffle(v8di* result, v8di a, v8di b) {
    /* 8-element shuffle with complex pattern */
    *result = __builtin_shufflevector(a, b,
        0, 2, 4, 6, 8, 10, 12, 14);
}

/* ==================== Pattern 2: x86 AVX-512 Gather Intrinsics ==================== */

#ifdef __AVX512F__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_gather_operations(__m512i* result, 
                           const int* base,
                           __m512i index,
                           __mmask16 mask) {
    /* __mm512_i32gather_epi32 has: index, base, scale, mask */
    /* During expansion, this can become many operands */
    *result = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(),
                                         mask,
                                         index,
                                         base,
                                         4);
}
#endif

/* ==================== Pattern 3: Atomic Operations with Many Parameters ==================== */

__attribute__((noipa, noinline))
int test_atomic_compare_exchange(volatile int* ptr, 
                                 int expected, 
                                 int desired) {
    int exp = expected;
    /* __atomic_compare_exchange has 6 explicit args + implicit ones */
    __atomic_compare_exchange(ptr, &exp, &desired,
                              0, /* weak */
                              __ATOMIC_SEQ_CST,
                              __ATOMIC_ACQUIRE);
    return exp;
}

/* ==================== Pattern 4: OpenMP SIMD with Multiple Clauses ==================== */

#define N 1024
__attribute__((noipa, noinline))
void test_omp_simd(float* a, float* b, float* c, float* d) {
    int i;
    /* Multiple clauses increase operand count during expansion */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:32) \
                simdlen(8) safelen(16) \
                reduction(+:a[0:N])
    for (i = 0; i < N; i++) {
        a[i] = b[i] * c[i] + d[i];
    }
}

/* ==================== Pattern 5: Inline Assembly with Many Operands ==================== */

__attribute__((noipa, noinline))
long test_multi_operand_asm(long a, long b, long c, long d,
                           long e, long f, long g, long h,
                           long i, long j) {
    long result;
    /* 10-operand asm statement (1 output + 9 inputs) */
    asm volatile (
        "/* Multi-operand template %0 = %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9 */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d),
          "r"(e), "r"(f), "r"(g), "r"(h),
          "r"(i)
        : "cc"
    );
    return result;
}

__attribute__((noipa, noinline))
long test_11_operand_asm(long a, long b, long c, long d,
                        long e, long f, long g, long h,
                        long i, long j, long k) {
    long result;
    /* 11-operand asm statement (1 output + 10 inputs) */
    asm volatile (
        "/* 11-operand template */\n\t"
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
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d),
          "r"(e), "r"(f), "r"(g), "r"(h),
          "r"(i), "r"(j)
        : "cc"
    );
    return result;
}

/* ==================== Pattern 6: Complex Built-in with Multiple Arguments ==================== */

__attribute__((noipa, noinline))
void* test_memcpy_many_args(void* dst, const void* src,
                           size_t n1, size_t n2,
                           int align_dst, int align_src) {
    /* __builtin_memcpy with alignment attributes can expand to many operands */
    if (align_dst > 0 && align_src > 0) {
        __builtin_memcpy(__builtin_assume_aligned(dst, align_dst),
                        __builtin_assume_aligned(src, align_src),
                        n1 + n2);
    }
    return dst;
}

/* ==================== Main Test Driver ==================== */

int main(int argc, char* argv[]) {
    /* Use argv[0] to create a pseudo-random seed */
    unsigned seed = 0;
    for (char* p = argv[0]; *p; p++) {
        seed = seed * 31 + *p;
    }
    control = seed;
    
    int result = 0;
    
    /* Test different patterns based on seed */
    switch (control % 7) {
        case 0: {
            v16si a = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
            v16si b = {17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
            v16si res;
            test_vector_shuffle(&res, a, b);
            result = res[0] + res[15];
            use(&res);
            break;
        }
        
        case 1: {
            v8di a = {1,2,3,4,5,6,7,8};
            v8di b = {9,10,11,12,13,14,15,16};
            v8di res;
            test_double_shuffle(&res, a, b);
            result = (int)(res[0] + res[7]);
            use(&res);
            break;
        }
        
#ifdef __AVX512F__
        case 2: {
            int base[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
            __m512i idx = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
            __m512i res;
            test_gather_operations(&res, base, idx, 0xFFFF);
            result = _mm512_extract_epi32(res, 0);
            use(&res);
            break;
        }
#endif
        
        case 3: {
            volatile int atomic_var = 42;
            int expected = 42;
            int desired = 100;
            result = test_atomic_compare_exchange(&atomic_var, expected, desired);
            use((void*)&atomic_var);
            break;
        }
        
        case 4: {
            static float a[N], b[N], c[N], d[N];
            for (int i = 0; i < N; i++) {
                b[i] = i * 0.1f;
                c[i] = i * 0.2f;
                d[i] = i * 0.3f;
            }
            test_omp_simd(a, b, c, d);
            result = (int)a[N-1];
            use(a); use(b); use(c); use(d);
            break;
        }
        
        case 5: {
            long asm_result = test_multi_operand_asm(1,2,3,4,5,6,7,8,9,10);
            result = (int)asm_result;
            use((void*)&asm_result);
            break;
        }
        
        case 6: {
            long asm_result = test_11_operand_asm(1,2,3,4,5,6,7,8,9,10,11);
            result = (int)asm_result;
            use((void*)&asm_result);
            break;
        }
        
        default: {
            char src[64] = "Hello, World!";
            char dst[64];
            test_memcpy_many_args(dst, src, 10, 3, 16, 16);
            result = (int)dst[0];
            use(dst); use(src);
            break;
        }
    }
    
    printf("Result: %d (seed: %u)\n", result, seed);
    return 0;
}
