/* test_optabs_10_11.c - Test program to cover 10/11 operand cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==================== Approach 1: Vector Shuffle with Many Elements ==================== */

typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices - during expansion this may require many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,    /* First 8 from a */
        16,17,18,19,20,21,22,23  /* Next 8 from b */
    );
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,
        31,30,29,28,27,26,25,24
    );
    
    /* Mix them */
    v16si e = c + d;
    
    /* Store result */
    for (int i = 0; i < 16; i++) {
        result[i] = e[i];
    }
    
    use(&e);
}

/* ==================== Approach 2: x86 AVX-512 Gather Intrinsics ==================== */

#ifdef __x86_64__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_avx512_gather(volatile int* result) {
    /* AVX-512 gather instructions can have many operands */
    __m512i index = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask = 0xFFFF;
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        base[i] = i * 2;
    }
    
    __m512i gathered = _mm512_i32gather_epi32(index, base, 4);
    
    /* Store to volatile memory */
    _mm512_store_epi32((void*)result, gathered);
    
    use(&gathered);
}
#endif

/* ==================== Approach 3: Atomic Operations ==================== */

__attribute__((noipa, noinline))
void test_atomic_ops(volatile int* result) {
    _Atomic int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange_n(&atomic_var, &expected, desired,
                                              0, /* weak */
                                              __ATOMIC_SEQ_CST,
                                              __ATOMIC_RELAXED);
    
    /* Another atomic with multiple memory orders */
    int old = __atomic_exchange_n(&atomic_var, 200, __ATOMIC_ACQ_REL);
    
    /* Atomic fetch-add with memory order */
    int fetched = __atomic_fetch_add(&atomic_var, 10, __ATOMIC_SEQ_CST);
    
    result[0] = success;
    result[1] = old;
    result[2] = fetched;
    result[3] = atomic_var;
    
    use(&atomic_var);
}

/* ==================== Approach 4: OpenMP SIMD with Many Clauses ==================== */

__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result, int n) {
    int a[n], b[n], c[n];
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = 0;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(16) \
                    reduction(+:control) if(control > 0)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 3;
    }
    
    /* Store results */
    for (int i = 0; i < n && i < 16; i++) {
        result[i] = c[i];
    }
    
    use(c);
}

/* ==================== Approach 5: Inline Assembly with Many Operands ==================== */

__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2, out3, out4, out5;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand test %0 = (%1 + %2) * (%3 + %4) - (%5 * %6) + (%7 - %8) / %9 */\n\t"
        "addl %1, %2\n\t"
        "addl %3, %4\n\t"
        "imull %2, %4\n\t"
        "movl %4, %0\n\t"
        "imull %5, %6\n\t"
        "subl %6, %0\n\t"
        "subl %8, %7\n\t"
        "cltd\n\t"
        "idivl %9\n\t"
        "addl %7, %0"
        : "=&r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "rax", "rdx", "cc"
    );
    
    /* Another asm with 11 operands using memory inputs */
    int inputs[10] = {1,2,3,4,5,6,7,8,9,10};
    asm volatile (
        "movl 0(%1), %%eax\n\t"
        "addl 4(%1), %%eax\n\t"
        "addl 8(%1), %%eax\n\t"
        "addl 12(%1), %%eax\n\t"
        "addl 16(%1), %%eax\n\t"
        "addl 20(%1), %%eax\n\t"
        "addl 24(%1), %%eax\n\t"
        "addl 28(%1), %%eax\n\t"
        "addl 32(%1), %%eax\n\t"
        "addl 36(%1), %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(out2)
        : "r"(inputs), "m"(*inputs)
        : "rax", "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    
    use(&out1);
}

/* ==================== Approach 6: Complex Vector Operations ==================== */

typedef float v16sf __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_complex_vector_ops(volatile float* result) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f,
                 8.5f, 9.5f, 10.5f, 11.5f, 12.5f, 13.5f, 14.5f, 15.5f};
    
    /* Complex sequence of vector operations */
    v16sf v3 = v1 * v2;
    v16sf v4 = v1 + v2;
    v16sf v5 = __builtin_shufflevector(v3, v4,
        0,17,2,19,4,21,6,23,8,25,10,27,12,29,14,31);
    
    /* Conditional select (ternary operation expands to many operands) */
    v16sf mask = v1 > v2;
    v16sf v6 = mask ? v3 : v4;
    
    /* Horizontal add pattern */
    v16sf v7 = __builtin_shufflevector(v6, v6,
        1,0,3,2,5,4,7,6,9,8,11,10,13,12,15,14);
    v16sf v8 = v6 + v7;
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result[i] = v8[i];
    }
    
    use(&v8);
}

/* ==================== Main Function ==================== */

int main(int argc, char *argv[]) {
    /* Use argv[0] to create a simple hash for control flow */
    unsigned int seed = 0;
    for (char *p = argv[0]; *p; p++) {
        seed = seed * 31 + *p;
    }
    
    /* Allocate result arrays */
    volatile int int_results[64] = {0};
    volatile float float_results[64] = {0};
    
    /* Execute different test cases based on seed */
    int test_case = seed % 6;
    
    switch (test_case) {
        case 0:
            printf("Testing vector shuffle...\n");
            test_vector_shuffle(int_results);
            break;
            
        case 1:
#ifdef __x86_64__
            printf("Testing AVX-512 gather...\n");
            test_avx512_gather(int_results);
#else
            printf("AVX-512 not available, using atomic ops instead...\n");
            test_atomic_ops(int_results);
#endif
            break;
            
        case 2:
            printf("Testing atomic operations...\n");
            test_atomic_ops(int_results);
            break;
            
        case 3:
            printf("Testing OpenMP SIMD...\n");
            test_openmp_simd(int_results, 64);
            break;
            
        case 4:
            printf("Testing multi-operand assembly...\n");
            test_multi_operand_asm(int_results);
            break;
            
        case 5:
            printf("Testing complex vector operations...\n");
            test_complex_vector_ops(float_results);
            break;
    }
    
    /* Compute checksum to ensure code executed */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += int_results[i];
        checksum += (int)float_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Control: %d\n", control);
    
    return 0;
}
