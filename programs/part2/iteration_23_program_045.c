/* Test program for x86 condition code mnemonics coverage */
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
    
    /* Test UNGE (nlt) - unordered or greater-or-equal */
    result = (isunordered(a, b) || (a >= b));
    checksum += result;
    
    /* Test UNGT (nle) - unordered or greater */
    result = (isunordered(a, b) || (a > b));
    checksum += result;
    
    /* Test UNLE (ule) - unordered or less-or-equal */
    result = (isunordered(fa, fb) || (fa <= fb));
    checksum += result;
    
    /* Test UNLT (ult) - unordered or less */
    result = (isunordered(fa, fb) || (fa < fb));
    checksum += result;
    
    /* Test LTGT (une) - less or greater (ordered and not equal) */
    result = (!isunordered(a, b) && (a != b));
    checksum += result;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_O2(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    volatile __m128d mask_d;
    volatile __m128 mask_f;
    
    /* Test UNORDERED (unord) - compare unordered */
    mask_d = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    checksum += _mm_movemask_pd(mask_d);
    
    /* Test ORDERED (ord) - compare ordered */
    mask_d = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    checksum += _mm_movemask_pd(mask_d);
    
    /* Test UNEQ (ueq) - compare equal unordered */
    mask_d = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    checksum += _mm_movemask_pd(mask_d);
    
    /* Test UNGE (nlt) - compare not-less-than unordered */
    mask_d = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    checksum += _mm_movemask_pd(mask_d);
    
    /* Test UNGT (nle) - compare not-less-or-equal unordered */
    mask_d = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    checksum += _mm_movemask_pd(mask_d);
    
    /* Test UNLE (ule) - compare less-or-equal unordered */
    mask_f = _mm_cmp_ps(f1, f2, _CMP_LE_UQ);
    checksum += _mm_movemask_ps(mask_f);
    
    /* Test UNLT (ult) - compare less-than unordered */
    mask_f = _mm_cmp_ps(f1, f2, _CMP_LT_UQ);
    checksum += _mm_movemask_ps(mask_f);
    
    /* Test LTGT (une) - compare not-equal ordered */
    mask_d = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    checksum += _mm_movemask_pd(mask_d);
}

__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile int result;
    volatile double da = a, db = b;
    
    /* Inline assembly tests with various condition codes */
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result) : "x"(db), "0"(0) : "eax", "cc"
    );
    checksum += result;
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result) : "x"(db), "0"(0) : "eax", "cc"
    );
    checksum += result;
    
    /* Test UNEQ (ueq) - Using cmppd with SSE2 */
    __asm__ volatile (
        "cmppd $3, %1, %0\n\t"  /* _CMP_EQ_UQ = 3 */
        "movmskpd %0, %k0"
        : "+x"(da) : "x"(db) : "cc"
    );
    checksum += (int)da;
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmppd $5, %1, %0\n\t"  /* _CMP_NLT_UQ = 5 */
        "movmskpd %0, %k0"
        : "+x"(da) : "x"(db) : "cc"
    );
    checksum += (int)da;
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmppd $6, %1, %0\n\t"  /* _CMP_NLE_UQ = 6 */
        "movmskpd %0, %k0"
        : "+x"(da) : "x"(db) : "cc"
    );
    checksum += (int)da;
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmppd $2, %1, %0\n\t"  /* _CMP_LE_UQ = 2 */
        "movmskpd %0, %k0"
        : "+x"(da) : "x"(db) : "cc"
    );
    checksum += (int)da;
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmppd $1, %1, %0\n\t"  /* _CMP_LT_UQ = 1 */
        "movmskpd %0, %k0"
        : "+x"(da) : "x"(db) : "cc"
    );
    checksum += (int)da;
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmppd $12, %1, %0\n\t"  /* _CMP_NEQ_OQ = 12 */
        "movmskpd %0, %k0"
        : "+x"(da) : "x"(db) : "cc"
    );
    checksum += (int)da;
}

__attribute__((optimize("O3"), target("avx")))
void test_complex_branches(double *arr, int n) {
    volatile int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        double a = arr[i];
        double b = arr[i + 1];
        
        /* Complex branching with multiple conditions */
        if (isunordered(a, b)) {
            count++;  /* UNORDERED */
        } else if (!isunordered(a, b) && a != b) {
            count += 2;  /* LTGT */
        } else if (isunordered(a, b) || a >= b) {
            count += 3;  /* UNGE */
        } else if (isunordered(a, b) || a > b) {
            count += 4;  /* UNGT */
        } else if (isunordered(a, b) || a <= b) {
            count += 5;  /* UNLE */
        } else if (isunordered(a, b) || a < b) {
            count += 6;  /* UNLT */
        } else if (isunordered(a, b) || a == b) {
            count += 7;  /* UNEQ */
        }
    }
    
    checksum += count;
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[16];
    float farr[16];
    
    for (int i = 0; i < 16; i++) {
        switch (i % 5) {
            case 0: darr[i] = 1.0 / (i + 1); break;
            case 1: darr[i] = -2.0 * i; break;
            case 2: darr[i] = 0.0 / 0.0; break;  /* NaN */
            case 3: darr[i] = 1.0 / 0.0; break;  /* Infinity */
            case 4: darr[i] = -1.0 / 0.0; break; /* -Infinity */
        }
        farr[i] = (float)darr[i];
    }
    
    /* Test with different value pairs to trigger various conditions */
    for (int i = 0; i < 8; i++) {
        double a = darr[i];
        double b = darr[i + 8];
        float fa = farr[i];
        float fb = farr[i + 8];
        
        __m128d v1 = _mm_set_pd(a, b);
        __m128d v2 = _mm_set_pd(b, a);
        __m128 fv1 = _mm_set_ps(fa, fb, fa, fb);
        __m128 fv2 = _mm_set_ps(fb, fa, fb, fa);
        
        /* Call test functions with different optimization levels */
        test_scalar_conditions_O0(a, b, fa, fb);
        test_vector_conditions_O2(v1, v2, fv1, fv2);
        test_inline_asm_conditions(a, b);
    }
    
    /* Test complex branching */
    test_complex_branches(darr, 16);
    
    /* Print final checksum */
    printf("Final checksum: %lu\n", checksum);
    
    return 0;
}
