/* test_optabs_10_11_operands.c
 * This program aims to trigger the 10 and 11 operand cases in optabs.cc
 * by using various GCC features that expand to multi-operand instructions.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile variables to prevent dead code elimination */
static volatile int vol_seed = 0;
static volatile int vol_result = 0;

/* ==================== Approach 1: Vector Shuffle with Many Elements ==================== */

typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(v16si* out, v16si* a, v16si* b) {
    /* This shuffle uses 18 operands: 2 input vectors + 16 indices */
    v16si va = *a;
    v16si vb = *b;
    
    /* Complex shuffle pattern that might expand to many operands */
    *out = __builtin_shufflevector(va, vb, 
        0, 1, 2, 3, 4, 5, 6, 7, 
        16, 17, 18, 19, 20, 21, 22, 23);
    
    /* Use volatile to ensure computation */
    vol_result += (*out)[0];
}

/* ==================== Approach 2: x86 AVX-512 Gather Intrinsic ==================== */

#ifdef __x86_64__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_avx512_gather(double* out, double* base, int* indices, __mmask8 mask) {
    /* __m512d _mm512_mask_i32gather_pd(__m512d src, __mmask8 k, __m256i vindex, void const *base, int scale);
     * This expands to multiple operands including base, scale, mask, etc.
     */
    __m512d src = _mm512_set1_pd(0.0);
    __m256i vindex = _mm256_loadu_si256((__m256i*)indices);
    
    __m512d result = _mm512_mask_i32gather_pd(src, mask, vindex, base, 8);
    _mm512_storeu_pd(out, result);
    
    vol_result += (int)out[0];
}
#endif

/* ==================== Approach 3: Atomic Compare Exchange with Many Parameters ==================== */

__attribute__((noipa, noinline))
void test_atomic_compare_exchange(int* ptr, int* expected, int desired) {
    /* __atomic_compare_exchange(ptr, expected, desired, weak, success_memorder, failure_memorder)
     * This has many parameters that might expand to multiple operands
     */
    int exp_val = *expected;
    int weak = 0;
    
    __atomic_compare_exchange(ptr, &exp_val, &desired, weak, 
                             __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    *expected = exp_val;
    vol_result += *ptr;
}

/* ==================== Approach 4: OpenMP SIMD with Many Clauses ==================== */

__attribute__((noipa, noinline))
void test_openmp_simd(float* a, float* b, float* c, int n) {
    int i;
    
    /* Complex OpenMP SIMD pragma with multiple clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:32) simdlen(8) safelen(16) \
                    reduction(+:vol_result)
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i] + 1.0f;
        vol_result += (int)a[i];
    }
}

/* ==================== Approach 5: Inline Assembly with Many Operands ==================== */

__attribute__((noipa, noinline))
void test_multi_operand_asm(int* out, int a, int b, int c, int d, int e, 
                           int f, int g, int h, int i, int j) {
    /* 10-operand inline assembly (9 inputs + 1 output) */
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
        : "=r" (*out)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), 
          "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    /* Use j to prevent optimization */
    *out += j;
    vol_result += *out;
}

/* ==================== Approach 6: Complex Vector Operation ==================== */

__attribute__((noipa, noinline))
void test_complex_vector_op(v16sf* out, v16sf* a, v16sf* b, v16sf* c) {
    /* Complex vector expression that might expand to many operands */
    v16sf va = *a;
    v16sf vb = *b;
    v16sf vc = *c;
    
    /* Fused multiply-add style operation */
    *out = va * vb + vc;
    
    /* Additional operations to increase complexity */
    v16sf temp = __builtin_shufflevector(*out, *out, 
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
    *out = *out + temp;
    
    vol_result += (int)(*out)[0];
}

/* ==================== Main Test Driver ==================== */

int main(int argc, char *argv[]) {
    /* Use argv[0] to create a simple hash for volatile control flow */
    unsigned int seed = 0;
    for (int i = 0; argv[0][i]; i++) {
        seed = seed * 31 + argv[0][i];
    }
    vol_seed = seed;
    
    /* Initialize test data */
    v16si vec_a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si vec_b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    v16si vec_out;
    
    float arr_a[64], arr_b[64], arr_c[64];
    for (int i = 0; i < 64; i++) {
        arr_b[i] = (float)i;
        arr_c[i] = (float)(i * 2);
    }
    
    int atomic_var = 42;
    int atomic_expected = 42;
    int atomic_desired = 100;
    
    int asm_out;
    
    /* Execute different test cases based on seed */
    int test_case = seed % 6;
    
    switch (test_case) {
        case 0:
            test_vector_shuffle(&vec_out, &vec_a, &vec_b);
            use(&vec_out);
            break;
            
        case 1:
            #ifdef __x86_64__
            {
                double gather_out[8];
                int gather_idx[8] = {0,2,4,6,8,10,12,14};
                test_avx512_gather(gather_out, (double*)arr_b, gather_idx, 0xFF);
                use(gather_out);
            }
            #endif
            break;
            
        case 2:
            test_atomic_compare_exchange(&atomic_var, &atomic_expected, atomic_desired);
            use(&atomic_var);
            break;
            
        case 3:
            test_openmp_simd(arr_a, arr_b, arr_c, 64);
            use(arr_a);
            break;
            
        case 4:
            test_multi_operand_asm(&asm_out, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            use(&asm_out);
            break;
            
        case 5:
            {
                v16sf vf_a, vf_b, vf_c, vf_out;
                for (int i = 0; i < 16; i++) {
                    vf_a[i] = (float)i;
                    vf_b[i] = (float)(i * 0.5);
                    vf_c[i] = (float)(i * 0.25);
                }
                test_complex_vector_op(&vf_out, &vf_a, &vf_b, &vf_c);
                use(&vf_out);
            }
            break;
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d (seed: %u, test_case: %d)\n", vol_result, seed, test_case);
    
    return 0;
}

/* Dummy use function to prevent dead code elimination */
void use(void* p) {
    /* Empty but referenced */
    (void)p;
}
