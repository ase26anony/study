/* Test program to cover x86 condition code mnemonics in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;
volatile double checksum = 0.0;

/* Function with specific optimization attributes */
__attribute__((optimize("O0"), target("sse2")))
void test_scalar_conditions_O0(volatile double a, volatile double b, 
                               volatile float fa, volatile float fb) {
    /* Test various floating-point conditions */
    
    /* UNORDERED: unord - unordered comparison */
    if (isunordered(a, b)) {
        checksum += 1.0;
    }
    
    /* ORDERED: ord - ordered comparison */
    if (!isunordered(a, b)) {  /* Equivalent to ordered */
        checksum += 2.0;
    }
    
    /* UNEQ: ueq - unordered or equal */
    if (isunordered(a, b) || (a == b)) {
        checksum += 3.0;
    }
    
    /* UNGE: nlt - not less than (unordered or greater or equal) */
    if (!(a < b)) {  /* This includes unordered case */
        checksum += 4.0;
    }
    
    /* UNGT: nle - not less than or equal (unordered or greater) */
    if (!(a <= b)) {  /* This includes unordered case */
        checksum += 5.0;
    }
    
    /* UNLE: ule - unordered or less or equal */
    if (isunordered(a, b) || (a <= b)) {
        checksum += 6.0;
    }
    
    /* UNLT: ult - unordered or less than */
    if (isunordered(a, b) || (a < b)) {
        checksum += 7.0;
    }
    
    /* LTGT: une - less than or greater than (but not equal, not unordered) */
    if ((a < b) || (a > b)) {  /* Excludes equal and unordered */
        checksum += 8.0;
    }
    
    /* Repeat with float types to ensure different modes */
    if (isunordered(fa, fb)) checksum += 9.0;
    if (!isunordered(fa, fb)) checksum += 10.0;
    if (isunordered(fa, fb) || (fa == fb)) checksum += 11.0;
}

__attribute__((optimize("O2"), target("sse2")))
void test_scalar_conditions_O2(volatile double a, volatile double b,
                               volatile float fa, volatile float fb) {
    /* Different optimization level, same tests */
    double temp = 0.0;
    
    /* Use ternary operators to force conditional moves */
    temp += isunordered(a, b) ? 1.0 : 0.0;          /* UNORDERED */
    temp += !isunordered(a, b) ? 2.0 : 0.0;         /* ORDERED */
    temp += (isunordered(a, b) || (a == b)) ? 3.0 : 0.0;  /* UNEQ */
    temp += !(a < b) ? 4.0 : 0.0;                   /* UNGE (nlt) */
    temp += !(a <= b) ? 5.0 : 0.0;                  /* UNGT (nle) */
    temp += (isunordered(a, b) || (a <= b)) ? 6.0 : 0.0;  /* UNLE */
    temp += (isunordered(a, b) || (a < b)) ? 7.0 : 0.0;   /* UNLT */
    temp += ((a < b) || (a > b)) ? 8.0 : 0.0;       /* LTGT (une) */
    
    checksum += temp;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_sse2(void) {
    /* Test with SSE2 vector comparisons */
    __m128d v1 = _mm_set_pd(1.0, 2.0);
    __m128d v2 = _mm_set_pd(2.0, 1.0);
    __m128d v_nan = _mm_set_pd(NAN, 3.0);
    
    /* These intrinsics generate specific comparison predicates */
    __m128d cmp_result;
    
    /* _CMP_UNORD_Q = 3 (unordered) */
    cmp_result = _mm_cmp_pd(v1, v_nan, _CMP_UNORD_Q);
    checksum += ((double*)&cmp_result)[0];
    
    /* _CMP_ORD_Q = 7 (ordered) */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    checksum += ((double*)&cmp_result)[1];
    
    /* _CMP_EQ_UQ = 8 (equal unordered) */
    cmp_result = _mm_cmp_pd(v1, v1, _CMP_EQ_UQ);
    checksum += ((double*)&cmp_result)[0];
    
    /* _CMP_NLT_UQ = 13 (not less than unordered) */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    checksum += ((double*)&cmp_result)[1];
    
    /* _CMP_NLE_UQ = 14 (not less than or equal unordered) */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    checksum += ((double*)&cmp_result)[0];
    
    /* _CMP_LE_OS = 2 (less than or equal ordered signaling) */
    /* Note: We need to mix different comparison types */
    cmp_result = _mm_cmp_pd(v2, v1, _CMP_LE_OS);
    checksum += ((double*)&cmp_result)[1];
    
    /* _CMP_LT_OS = 1 (less than ordered signaling) */
    cmp_result = _mm_cmp_pd(v2, v1, _CMP_LT_OS);
    checksum += ((double*)&cmp_result)[0];
    
    /* _CMP_NEQ_OS = 4 (not equal ordered signaling) */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NEQ_OS);
    checksum += ((double*)&cmp_result)[1];
}

__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(void) {
    /* Test inline assembly with condition code mnemonics */
    double a = 1.5;
    double b = 2.5;
    double result;
    __m128d va, vb, vresult;
    
    /* Test various condition codes in inline assembly templates */
    
    /* UNORDERED: unord */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzb %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    checksum += result;
    
    /* ORDERED: ord */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzb %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    checksum += result;
    
    /* Test with vector registers and cmppd */
    va = _mm_set1_pd(a);
    vb = _mm_set1_pd(b);
    
    /* Using extended asm with condition code substitution */
    /* The {unord} should be substituted by the compiler */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        "movq %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    checksum += ((double*)&vresult)[0];
    
    /* Test ord condition */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        "movq %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    checksum += ((double*)&vresult)[1];
    
    /* Test ueq condition */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ueq}\n\t"
        "movq %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    checksum += ((double*)&vresult)[0];
    
    /* Test nlt condition */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nlt}\n\t"
        "movq %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    checksum += ((double*)&vresult)[1];
    
    /* Test nle condition */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nle}\n\t"
        "movq %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    checksum += ((double*)&vresult)[0];
    
    /* Test ule condition */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ule}\n\t"
        "movq %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    checksum += ((double*)&vresult)[1];
    
    /* Test ult condition */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ult}\n\t"
        "movq %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    checksum += ((double*)&vresult)[0];
    
    /* Test une condition */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|une}\n\t"
        "movq %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    checksum += ((double*)&vresult)[1];
}

__attribute__((optimize("O3"), target("sse2")))
void test_complex_branches(volatile double* arr, int n) {
    /* Complex branching to force condition code generation */
    volatile double sum = 0.0;
    
    for (int i = 0; i < n - 1; i++) {
        volatile double a = arr[i];
        volatile double b = arr[i + 1];
        
        /* Nested conditions using different comparisons */
        if (isunordered(a, b)) {
            sum += 1.0;
        } else if (!(a < b)) {  /* UNGE: nlt */
            sum += 2.0;
            if (!(a <= b)) {    /* UNGT: nle */
                sum += 3.0;
            }
        } else if (isunordered(a, b) || (a <= b)) {  /* UNLE: ule */
            sum += 4.0;
        }
        
        /* LTGT: une */
        if ((a < b) || (a > b)) {
            sum += 5.0;
        }
        
        /* UNEQ: ueq */
        if (isunordered(a, b) || (a == b)) {
            sum += 6.0;
        }
        
        /* UNLT: ult */
        if (isunordered(a, b) || (a < b)) {
            sum += 7.0;
        }
    }
    
    checksum += sum;
}

int main(int argc, char* argv[]) {
    /* Initialize with non-uniform values */
    unsigned int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity */
    volatile double darr[20];
    volatile float farr[20];
    
    for (int i = 0; i < 20; i++) {
        darr[i] = (rand() % 100) / 10.0;
        farr[i] = (rand() % 100) / 10.0f;
        
        /* Insert some special values */
        if (i % 7 == 0) darr[i] = NAN;
        if (i % 5 == 0) darr[i] = INFINITY;
        if (i % 3 == 0) farr[i] = NAN;
    }
    
    /* Run all test functions */
    for (int i = 0; i < 5; i++) {
        test_scalar_conditions_O0(darr[i], darr[i+1], farr[i], farr[i+1]);
        test_scalar_conditions_O2(darr[i+2], darr[i+3], farr[i+2], farr[i+3]);
    }
    
    test_vector_conditions_sse2();
    test_inline_asm_conditions();
    test_complex_branches(darr, 20);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
