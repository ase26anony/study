/* Test program for x86 condition code mnemonics */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long checksum = 0;

/* Test functions with different optimization attributes */
__attribute__((optimize("O0"))) 
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int result;
    
    /* Test UNORDERED (unord) */
    result = isunordered(a, b);
    checksum += result;
    
    /* Test ORDERED (ord) */
    result = !isunordered(a, b);
    checksum += result;
    
    /* Test UNEQ (ueq) - unordered or equal */
    result = (isunordered(fa, fb) || (fa == fb));
    checksum += result;
    
    /* Test UNGE (nlt) - unordered or not less than */
    result = (isunordered(a, b) || !(a < b));
    checksum += result;
    
    /* Test UNGT (nle) - unordered or not less than or equal */
    result = (isunordered(a, b) || !(a <= b));
    checksum += result;
    
    /* Test UNLE (ule) - unordered or less than or equal */
    result = (isunordered(a, b) || (a <= b));
    checksum += result;
    
    /* Test UNLT (ult) - unordered or less than */
    result = (isunordered(a, b) || (a < b));
    checksum += result;
    
    /* Test LTGT (une) - less than or greater than (ordered and not equal) */
    result = (!isunordered(a, b) && (a != b));
    checksum += result;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_O2(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    volatile __m128d mask_d;
    volatile __m128 mask_f;
    volatile int results[4];
    
    /* Test UNORDERED (unord) - _CMP_UNORD_Q */
    mask_d = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    results[0] = _mm_movemask_pd(mask_d);
    checksum += results[0];
    
    /* Test ORDERED (ord) - _CMP_ORD_Q */
    mask_d = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    results[1] = _mm_movemask_pd(mask_d);
    checksum += results[1];
    
    /* Test UNEQ (ueq) - _CMP_EQ_UQ */
    mask_d = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    results[2] = _mm_movemask_pd(mask_d);
    checksum += results[2];
    
    /* Test UNGE (nlt) - _CMP_NLT_UQ */
    mask_d = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    results[3] = _mm_movemask_pd(mask_d);
    checksum += results[3];
    
    /* Test UNGT (nle) - _CMP_NLE_UQ */
    mask_f = _mm_cmp_ps(f1, f2, _CMP_NLE_UQ);
    results[0] = _mm_movemask_ps(mask_f);
    checksum += results[0];
    
    /* Test UNLE (ule) - _CMP_LE_UQ */
    mask_f = _mm_cmp_ps(f1, f2, _CMP_LE_UQ);
    results[1] = _mm_movemask_ps(mask_f);
    checksum += results[1];
    
    /* Test UNLT (ult) - _CMP_LT_UQ */
    mask_f = _mm_cmp_ps(f1, f2, _CMP_LT_UQ);
    results[2] = _mm_movemask_ps(mask_f);
    checksum += results[2];
    
    /* Test LTGT (une) - _CMP_NEQ_OQ */
    mask_d = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    results[3] = _mm_movemask_pd(mask_d);
    checksum += results[3];
}

__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double a, double b, __m128d v1, __m128d v2) {
    volatile double result_d;
    volatile __m128d mask;
    volatile int cc_result;
    
    /* Inline assembly tests with explicit condition code mnemonics */
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%{b|unord} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += cc_result;
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%{ae|ord} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += cc_result;
    
    /* Test UNEQ (ueq) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%{be|ueq} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += cc_result;
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%{ae|nlt} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += cc_result;
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%{a|nle} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += cc_result;
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%{be|ule} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += cc_result;
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%{b|ult} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += cc_result;
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%{ne|une} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += cc_result;
    
    /* Vector comparisons with inline assembly */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        "movmskpd %0, %k0"
        : "=x"(mask)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    cc_result = _mm_movemask_pd(mask);
    checksum += cc_result;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        "movmskpd %0, %k0"
        : "=x"(mask)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    cc_result = _mm_movemask_pd(mask);
    checksum += cc_result;
}

__attribute__((optimize("O3"), target("sse2")))
void test_complex_branches_O3(double *arr, int n) {
    volatile double a, b;
    volatile int count_unord = 0, count_ord = 0;
    volatile int count_ueq = 0, count_nlt = 0;
    volatile int count_nle = 0, count_ule = 0;
    volatile int count_ult = 0, count_une = 0;
    
    for (int i = 0; i < n - 1; i++) {
        a = arr[i];
        b = arr[i + 1];
        
        /* Complex branching to force condition code generation */
        if (isunordered(a, b)) {
            count_unord++;
        } else if (!(a < b)) {  /* UNGE (nlt) */
            count_nlt++;
        } else if (!(a <= b)) { /* UNGT (nle) */
            count_nle++;
        }
        
        /* Nested conditions */
        if (isunordered(a, b) || (a == b)) {  /* UNEQ (ueq) */
            count_ueq++;
            if (isunordered(a, b) || (a <= b)) {  /* UNLE (ule) */
                count_ule++;
            }
        }
        
        if (isunordered(a, b) || (a < b)) {  /* UNLT (ult) */
            count_ult++;
        }
        
        if (!isunordered(a, b) && (a != b)) {  /* LTGT (une) */
            count_une++;
        }
        
        if (!isunordered(a, b)) {  /* ORDERED (ord) */
            count_ord++;
        }
    }
    
    checksum += count_unord + count_ord + count_ueq + count_nlt;
    checksum += count_nle + count_ule + count_ult + count_une;
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[20];
    float farr[20];
    
    for (int i = 0; i < 20; i++) {
        switch (i % 7) {
            case 0: darr[i] = 1.0 / (i + 1); break;
            case 1: darr[i] = -2.0 * i; break;
            case 2: darr[i] = 0.0; break;
            case 3: darr[i] = -0.0; break;
            case 4: darr[i] = __builtin_nan(""); break;  /* NaN */
            case 5: darr[i] = __builtin_inf(); break;    /* Infinity */
            case 6: darr[i] = -__builtin_inf(); break;   /* -Infinity */
        }
        farr[i] = (float)darr[i];
    }
    
    /* Initialize vector values */
    __m128d v1 = _mm_set_pd(darr[0], darr[1]);
    __m128d v2 = _mm_set_pd(darr[2], darr[3]);
    __m128 f1 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128 f2 = _mm_set_ps(farr[4], farr[5], farr[6], farr[7]);
    
    /* Call test functions with different optimization levels */
    test_scalar_conditions_O0(darr[0], darr[1], farr[0], farr[1]);
    test_vector_conditions_O2(v1, v2, f1, f2);
    test_inline_asm_conditions(darr[2], darr[3], v1, v2);
    test_complex_branches_O3(darr, 20);
    
    /* Print final checksum */
    printf("Final checksum: %lu\n", checksum);
    
    return 0;
}
