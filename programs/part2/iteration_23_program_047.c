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
    
    /* Test UNGE (nlt) - not less than (greater or equal or unordered) */
    result = !(fa < fb);
    checksum += result;
    
    /* Test UNGT (nle) - not less or equal (greater or unordered) */
    result = !(fa <= fb);
    checksum += result;
    
    /* Test UNLE (ule) - unordered or less or equal */
    result = (isunordered(a, b) || (a <= b));
    checksum += result;
    
    /* Test UNLT (ult) - unordered or less than */
    result = (isunordered(a, b) || (a < b));
    checksum += result;
    
    /* Test LTGT (une) - less than or greater than (not equal and ordered) */
    result = (!isunordered(a, b) && (a != b));
    checksum += result;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_O2(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    volatile __m128d vresult;
    volatile __m128 fresult;
    volatile int mask[4];
    
    /* Test UNORDERED - _CMP_UNORD_Q */
    vresult = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)mask, vresult);
    checksum += mask[0] + mask[1];
    
    /* Test ORDERED - _CMP_ORD_Q */
    vresult = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)mask, vresult);
    checksum += mask[0] + mask[1];
    
    /* Test UNEQ - _CMP_EQ_UQ */
    vresult = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)mask, vresult);
    checksum += mask[0] + mask[1];
    
    /* Test UNGE - _CMP_NLT_UQ (not less than unordered quiet) */
    fresult = _mm_cmp_ps(f1, f2, _CMP_NLT_UQ);
    _mm_storeu_ps((float*)mask, fresult);
    checksum += mask[0] + mask[1] + mask[2] + mask[3];
    
    /* Test UNGT - _CMP_NLE_UQ (not less or equal unordered quiet) */
    fresult = _mm_cmp_ps(f1, f2, _CMP_NLE_UQ);
    _mm_storeu_ps((float*)mask, fresult);
    checksum += mask[0] + mask[1] + mask[2] + mask[3];
    
    /* Test UNLE - _CMP_LE_OQ (less or equal ordered quiet) */
    vresult = _mm_cmp_pd(v1, v2, _CMP_LE_OQ);
    _mm_storeu_pd((double*)mask, vresult);
    checksum += mask[0] + mask[1];
    
    /* Test UNLT - _CMP_LT_OQ (less than ordered quiet) */
    vresult = _mm_cmp_pd(v1, v2, _CMP_LT_OQ);
    _mm_storeu_pd((double*)mask, vresult);
    checksum += mask[0] + mask[1];
    
    /* Test LTGT - _CMP_NEQ_OQ (not equal ordered quiet) */
    fresult = _mm_cmp_ps(f1, f2, _CMP_NEQ_OQ);
    _mm_storeu_ps((float*)mask, fresult);
    checksum += mask[0] + mask[1] + mask[2] + mask[3];
}

__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double a, double b, __m128d v1, __m128d v2) {
    volatile double result_d;
    volatile __m128d result_v;
    volatile int flags;
    
    /* Inline assembly tests with condition code mnemonics */
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %2"
        : "=r"(flags) : "x"(b), "x"(a), "0"(flags) : "eax", "cc"
    );
    checksum += flags;
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %2"
        : "=r"(flags) : "x"(b), "x"(a), "0"(flags) : "eax", "cc"
    );
    checksum += flags;
    
    /* Test UNEQ (ueq) with cmppd */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ueq}\n\t"
        "movapd %1, %0"
        : "=x"(result_v) : "x"(v1), "x"(v2), "x"(result_v) : "cc"
    );
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nlt}\n\t"
        "movapd %1, %0"
        : "=x"(result_v) : "x"(v1), "x"(v2), "x"(result_v) : "cc"
    );
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nle}\n\t"
        "movapd %1, %0"
        : "=x"(result_v) : "x"(v1), "x"(v2), "x"(result_v) : "cc"
    );
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ule}\n\t"
        "movapd %1, %0"
        : "=x"(result_v) : "x"(v1), "x"(v2), "x"(result_v) : "cc"
    );
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ult}\n\t"
        "movapd %1, %0"
        : "=x"(result_v) : "x"(v1), "x"(v2), "x"(result_v) : "cc"
    );
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|une}\n\t"
        "movapd %1, %0"
        : "=x"(result_v) : "x"(v1), "x"(v2), "x"(result_v) : "cc"
    );
}

__attribute__((optimize("O3"), target("sse2")))
void test_complex_branches(double* arr, int n) {
    volatile int count_unord = 0;
    volatile int count_ord = 0;
    volatile int count_ueq = 0;
    volatile int count_nlt = 0;
    volatile int count_nle = 0;
    volatile int count_ule = 0;
    volatile int count_ult = 0;
    volatile int count_une = 0;
    
    for (int i = 0; i < n - 1; i++) {
        double a = arr[i];
        double b = arr[i + 1];
        
        /* Complex branching to force condition code generation */
        if (isunordered(a, b)) {
            count_unord++;
        } else if (!isunordered(a, b)) {
            count_ord++;
        }
        
        if (isunordered(a, b) || (a == b)) {
            count_ueq++;
        }
        
        if (!(a < b)) {
            count_nlt++;
        }
        
        if (!(a <= b)) {
            count_nle++;
        }
        
        if (isunordered(a, b) || (a <= b)) {
            count_ule++;
        }
        
        if (isunordered(a, b) || (a < b)) {
            count_ult++;
        }
        
        if (!isunordered(a, b) && (a != b)) {
            count_une++;
        }
    }
    
    checksum += count_unord + count_ord + count_ueq + count_nlt + 
                count_nle + count_ule + count_ult + count_une;
}

int main(int argc, char* argv[]) {
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
            case 3: darr[i] = __builtin_nan(""); break;  /* NaN */
            case 4: darr[i] = __builtin_inf(); break;    /* Infinity */
            case 5: darr[i] = -__builtin_inf(); break;   /* -Infinity */
            case 6: darr[i] = 3.14159 * i; break;
        }
        farr[i] = (float)darr[i];
    }
    
    /* Initialize vector values */
    __m128d v1 = _mm_set_pd(darr[0], darr[1]);
    __m128d v2 = _mm_set_pd(darr[2], darr[3]);
    __m128 f1 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128 f2 = _mm_set_ps(farr[4], farr[5], farr[6], farr[7]);
    
    /* Run all test functions */
    for (int i = 0; i < 10; i++) {
        test_scalar_conditions_O0(darr[i], darr[i+1], farr[i], farr[i+1]);
        test_vector_conditions_O2(v1, v2, f1, f2);
        test_inline_asm_conditions(darr[i], darr[i+1], v1, v2);
    }
    
    test_complex_branches(darr, 20);
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
