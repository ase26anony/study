/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile variables to prevent optimization */
volatile int global_checksum = 0;
volatile int volatile_control = 1;

/* Function with specific optimization attributes */
__attribute__((optimize("O0")))
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int result = 0;
    
    /* Test UNORDERED (unord) */
    if (isunordered(a, b)) {
        result |= 1;
    }
    
    /* Test ORDERED (ord) */
    if (!isunordered(a, b)) {
        result |= 2;
    }
    
    /* Test UNEQ (ueq) - unordered or equal */
    if (isunordered(a, b) || (a == b)) {
        result |= 4;
    }
    
    /* Test UNGE (nlt) - unordered or greater than or equal */
    if (isunordered(a, b) || (a >= b)) {
        result |= 8;
    }
    
    /* Test UNGT (nle) - unordered or greater than */
    if (isunordered(a, b) || (a > b)) {
        result |= 16;
    }
    
    /* Test UNLE (ule) - unordered or less than or equal */
    if (isunordered(a, b) || (a <= b)) {
        result |= 32;
    }
    
    /* Test UNLT (ult) - unordered or less than */
    if (isunordered(a, b) || (a < b)) {
        result |= 64;
    }
    
    /* Test LTGT (une) - less than or greater than (but not equal, not unordered) */
    if ((a < b) || (a > b)) {
        result |= 128;
    }
    
    global_checksum += result;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_O2(__m128d va, __m128d vb, __m128 vfa, __m128 vfb) {
    volatile __m128d vresult;
    volatile __m128 vfresult;
    
    /* Test UNORDERED - _CMP_UNORD_Q */
    vresult = _mm_cmp_pd(va, vb, _CMP_UNORD_Q);
    global_checksum += _mm_movemask_pd(vresult);
    
    /* Test ORDERED - _CMP_ORD_Q */
    vresult = _mm_cmp_pd(va, vb, _CMP_ORD_Q);
    global_checksum += _mm_movemask_pd(vresult);
    
    /* Test UNEQ - _CMP_EQ_UQ */
    vresult = _mm_cmp_pd(va, vb, _CMP_EQ_UQ);
    global_checksum += _mm_movemask_pd(vresult);
    
    /* Test UNGE - _CMP_NLT_UQ */
    vresult = _mm_cmp_pd(va, vb, _CMP_NLT_UQ);
    global_checksum += _mm_movemask_pd(vresult);
    
    /* Test UNGT - _CMP_NLE_UQ */
    vresult = _mm_cmp_pd(va, vb, _CMP_NLE_UQ);
    global_checksum += _mm_movemask_pd(vresult);
    
    /* Test UNLE - _CMP_LE_OS */
    vresult = _mm_cmp_pd(va, vb, _CMP_LE_OS);
    global_checksum += _mm_movemask_pd(vresult);
    
    /* Test UNLT - _CMP_LT_OS */
    vresult = _mm_cmp_pd(va, vb, _CMP_LT_OS);
    global_checksum += _mm_movemask_pd(vresult);
    
    /* Test LTGT - _CMP_NEQ_OS */
    vresult = _mm_cmp_pd(va, vb, _CMP_NEQ_OS);
    global_checksum += _mm_movemask_pd(vresult);
    
    /* Float vector tests */
    vfresult = _mm_cmp_ps(vfa, vfb, _CMP_UNORD_Q);
    global_checksum += _mm_movemask_ps(vfresult);
    
    vfresult = _mm_cmp_ps(vfa, vfb, _CMP_ORD_Q);
    global_checksum += _mm_movemask_ps(vfresult);
}

__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double a, double b, __m128d va, __m128d vb) {
    volatile double result_d;
    volatile __m128d result_v;
    volatile int cc_result;
    
    /* Inline assembly with explicit condition code mnemonics */
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%{b|unord} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    global_checksum += cc_result;
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%{b|ord} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    global_checksum += cc_result;
    
    /* Test UNEQ (ueq) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ueq}\n\t"
        "movq %1, %0"
        : "=x"(result_v)
        : "x"(result_v), "x"(va), "x"(vb)
        : "cc"
    );
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nlt}\n\t"
        "movq %1, %0"
        : "=x"(result_v)
        : "x"(result_v), "x"(va), "x"(vb)
        : "cc"
    );
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nle}\n\t"
        "movq %1, %0"
        : "=x"(result_v)
        : "x"(result_v), "x"(va), "x"(vb)
        : "cc"
    );
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ule}\n\t"
        "movq %1, %0"
        : "=x"(result_v)
        : "x"(result_v), "x"(va), "x"(vb)
        : "cc"
    );
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ult}\n\t"
        "movq %1, %0"
        : "=x"(result_v)
        : "x"(result_v), "x"(va), "x"(vb)
        : "cc"
    );
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|une}\n\t"
        "movq %1, %0"
        : "=x"(result_v)
        : "x"(result_v), "x"(va), "x"(vb)
        : "cc"
    );
}

__attribute__((optimize("O3"), target("sse2")))
void test_mixed_conditions_O3(double a, double b, float fa, float fb) {
    volatile double temp_d;
    volatile float temp_f;
    
    /* Complex nested conditions to force code generation */
    for (int i = 0; i < 3; i++) {
        if (volatile_control & (1 << i)) {
            /* Test UNORDERED with ternary operator */
            temp_d = isunordered(a, b) ? a * 2.0 : b / 2.0;
            global_checksum += (int)temp_d;
            
            /* Test ORDERED with if-else chain */
            if (!isunordered(fa, fb)) {
                if (fa > fb) {
                    temp_f = fa - fb;
                } else if (fa < fb) {
                    temp_f = fb - fa;
                } else {
                    temp_f = fa + fb;
                }
                global_checksum += (int)temp_f;
            }
            
            /* Test UNEQ with switch-like logic */
            int condition = 0;
            if (isunordered(a, b) || (a == b)) condition |= 1;
            if (isunordered(a, b) || (a >= b)) condition |= 2;
            if (isunordered(a, b) || (a > b)) condition |= 4;
            
            switch (condition & 3) {
                case 0: temp_d = a; break;
                case 1: temp_d = b; break;
                case 2: temp_d = a + b; break;
                case 3: temp_d = a - b; break;
            }
            global_checksum += (int)temp_d;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darray[8];
    float farray[8];
    
    for (int i = 0; i < 8; i++) {
        farray[i] = (rand() % 100) / 10.0f;
        darray[i] = (rand() % 100) / 10.0;
        
        /* Introduce some special values */
        if (i == 2) farray[i] = 0.0f / 0.0f; /* NaN */
        if (i == 3) darray[i] = 1.0 / 0.0;   /* Infinity */
        if (i == 4) farray[i] = -0.0f;       /* Negative zero */
        if (i == 5) darray[i] = -1.0 / 0.0;  /* Negative infinity */
    }
    
    /* Initialize vector values */
    __m128d vd1 = _mm_set_pd(darray[0], darray[1]);
    __m128d vd2 = _mm_set_pd(darray[2], darray[3]);
    __m128d vd3 = _mm_set_pd(darray[4], darray[5]);
    __m128d vd4 = _mm_set_pd(darray[6], darray[7]);
    
    __m128 vf1 = _mm_set_ps(farray[0], farray[1], farray[2], farray[3]);
    __m128 vf2 = _mm_set_ps(farray[4], farray[5], farray[6], farray[7]);
    
    /* Run all test functions with different parameter combinations */
    test_scalar_conditions_O0(darray[0], darray[1], farray[0], farray[1]);
    test_scalar_conditions_O0(darray[2], darray[3], farray[2], farray[3]);
    
    test_vector_conditions_O2(vd1, vd2, vf1, vf2);
    test_vector_conditions_O2(vd3, vd4, vf1, vf2);
    
    test_inline_asm_conditions(darray[4], darray[5], vd1, vd2);
    test_inline_asm_conditions(darray[6], darray[7], vd3, vd4);
    
    test_mixed_conditions_O3(darray[0], darray[2], farray[0], farray[2]);
    test_mixed_conditions_O3(darray[1], darray[3], farray[1], farray[3]);
    
    /* Print final checksum */
    printf("Final checksum: %d\n", global_checksum);
    
    return 0;
}
