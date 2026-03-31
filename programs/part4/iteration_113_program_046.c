#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Condition code counters for verification */
static int cc_counts[8] = {0};
enum CC_TYPE { CC_UNORDERED, CC_ORDERED, CC_UNEQ, CC_UNGE, CC_UNGT, CC_UNLE, CC_UNLT, CC_LTGT };

/* Test data with normal values, infinities, and NaNs */
static const double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0, 
    INFINITY, -INFINITY, 
    __builtin_nan(""), -__builtin_nan(""),
    DBL_MAX, DBL_MIN
};

static const __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(-1.0, 0.0),
    _mm_set_pd(INFINITY, -INFINITY),
    _mm_set_pd(__builtin_nan(""), -__builtin_nan("")),
    _mm_set_pd(DBL_MAX, DBL_MIN)
};

#ifdef __AVX__
static const __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, -1.0, 0.0),
    _mm256_set_pd(INFINITY, -INFINITY, __builtin_nan(""), -__builtin_nan("")),
    _mm256_set_pd(DBL_MAX, DBL_MIN, 3.14, -2.71)
};
#endif

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    int i, j;
    double a, b;
    
    for (i = 0; i < sizeof(test_scalars)/sizeof(test_scalars[0]); i++) {
        for (j = 0; j < sizeof(test_scalars)/sizeof(test_scalars[0]); j++) {
            a = test_scalars[i];
            b = test_scalars[j];
            
            /* UNORDERED case */
            if (__builtin_isunordered(a, b)) {
                cc_counts[CC_UNORDERED]++;
            }
            
            /* ORDERED case */
            if (__builtin_isordered(a, b)) {
                cc_counts[CC_ORDERED]++;
            }
            
            /* UNEQ case - unordered or equal */
            if (!__builtin_islessgreater(a, b)) {
                /* This includes UNEQ when unordered */
                cc_counts[CC_UNEQ]++;
            }
            
            /* UNGE case - not less than (greater or equal or unordered) */
            if (!__builtin_isless(a, b)) {
                cc_counts[CC_UNGE]++;
            }
            
            /* UNGT case - not less or equal (greater or unordered) */
            if (!__builtin_islessequal(a, b)) {
                cc_counts[CC_UNGT]++;
            }
            
            /* UNLE case - less or equal or unordered */
            if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
                cc_counts[CC_UNLE]++;
            }
            
            /* UNLT case - less than or unordered */
            if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
                cc_counts[CC_UNLT]++;
            }
            
            /* LTGT case - less or greater (ordered and not equal) */
            if (__builtin_islessgreater(a, b)) {
                cc_counts[CC_LTGT]++;
            }
        }
    }
}

/* Test vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    int i, j;
    
    for (i = 0; i < sizeof(test_vec128)/sizeof(test_vec128[0]); i++) {
        for (j = 0; j < sizeof(test_vec128)/sizeof(test_vec128[0]); j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            __m128d cmp_result;
            __m128i mask;
            
            /* Test various comparison predicates */
            
            /* UNORDERED: _CMP_UNORD_Q */
            cmp_result = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNORDERED]++;
            }
            
            /* ORDERED: _CMP_ORD_Q */
            cmp_result = _mm_cmp_pd(a, b, _CMP_ORD_Q);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_movemask_epi8(mask) != 0) {
                cc_counts[CC_ORDERED]++;
            }
            
            /* UNEQ: _CMP_EQ_UQ */
            cmp_result = _mm_cmp_pd(a, b, _CMP_EQ_UQ);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNEQ]++;
            }
            
            /* UNGE: _CMP_NLT_UQ */
            cmp_result = _mm_cmp_pd(a, b, _CMP_NLT_UQ);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNGE]++;
            }
            
            /* UNGT: _CMP_NLE_UQ */
            cmp_result = _mm_cmp_pd(a, b, _CMP_NLE_UQ);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNGT]++;
            }
            
            /* UNLE: _CMP_LE_OS */
            cmp_result = _mm_cmp_pd(a, b, _CMP_LE_OS);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNLE]++;
            }
            
            /* UNLT: _CMP_LT_OS */
            cmp_result = _mm_cmp_pd(a, b, _CMP_LT_OS);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNLT]++;
            }
            
            /* LTGT: _CMP_NEQ_OS */
            cmp_result = _mm_cmp_pd(a, b, _CMP_NEQ_OS);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_movemask_epi8(mask) != 0) {
                cc_counts[CC_LTGT]++;
            }
        }
    }
}

/* Test inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    int result;
    
    /* UNORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) cc_counts[CC_UNORDERED]++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) cc_counts[CC_ORDERED]++;
    
    /* UNEQ constraint - unordered or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cl", "cc"
    );
    if (result) cc_counts[CC_UNEQ]++;
    
    /* UNGE constraint - not less than */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setae %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) cc_counts[CC_UNGE]++;
    
    /* UNGT constraint - not less or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) cc_counts[CC_UNGT]++;
    
    /* UNLE constraint - less or equal or unordered */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) cc_counts[CC_UNLE]++;
    
    /* UNLT constraint - less than or unordered */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) cc_counts[CC_UNLT]++;
    
    /* LTGT constraint - not equal and ordered */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "setnp %%cl\n\t"
        "andb %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cl", "cc"
    );
    if (result) cc_counts[CC_LTGT]++;
}

#ifdef __AVX__
/* Test AVX comparisons for wider code generation paths */
void test_avx_conditions(void) {
    int i, j;
    
    for (i = 0; i < sizeof(test_vec256)/sizeof(test_vec256[0]); i++) {
        for (j = 0; j < sizeof(test_vec256)/sizeof(test_vec256[0]); j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            __m256d cmp_result;
            __m256i mask;
            
            /* Use various AVX comparison predicates */
            
            /* UNORDERED */
            cmp_result = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            mask = _mm256_castpd_si256(cmp_result);
            if (_mm256_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNORDERED]++;
            }
            
            /* ORDERED */
            cmp_result = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            mask = _mm256_castpd_si256(cmp_result);
            if (_mm256_movemask_epi8(mask) != 0) {
                cc_counts[CC_ORDERED]++;
            }
            
            /* UNEQ */
            cmp_result = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            mask = _mm256_castpd_si256(cmp_result);
            if (_mm256_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNEQ]++;
            }
            
            /* UNGE */
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            mask = _mm256_castpd_si256(cmp_result);
            if (_mm256_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNGE]++;
            }
            
            /* UNGT */
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            mask = _mm256_castpd_si256(cmp_result);
            if (_mm256_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNGT]++;
            }
            
            /* UNLE */
            cmp_result = _mm256_cmp_pd(a, b, _CMP_LE_OS);
            mask = _mm256_castpd_si256(cmp_result);
            if (_mm256_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNLE]++;
            }
            
            /* UNLT */
            cmp_result = _mm256_cmp_pd(a, b, _CMP_LT_OS);
            mask = _mm256_castpd_si256(cmp_result);
            if (_mm256_movemask_epi8(mask) != 0) {
                cc_counts[CC_UNLT]++;
            }
            
            /* LTGT */
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NEQ_OS);
            mask = _mm256_castpd_si256(cmp_result);
            if (_mm256_movemask_epi8(mask) != 0) {
                cc_counts[CC_LTGT]++;
            }
        }
    }
}
#endif

/* Control flow that depends on comparison results */
void test_control_flow(void) {
    double a, b;
    int i, j;
    
    for (i = 0; i < 5; i++) {
        a = test_scalars[i];
        b = test_scalars[i + 5];
        
        /* Complex control flow based on comparisons */
        if (__builtin_isunordered(a, b)) {
            cc_counts[CC_UNORDERED]++;
            for (j = 0; j < 3; j++) {
                if (__builtin_isordered(a + j, b - j)) {
                    cc_counts[CC_ORDERED]++;
                }
            }
        } else if (__builtin_isless(a, b)) {
            cc_counts[CC_UNLT]++;
            if (!__builtin_islessequal(a, b)) {
                /* Should never happen, but creates control flow */
                cc_counts[CC_UNGT]++;
            }
        } else if (__builtin_islessequal(a, b)) {
            cc_counts[CC_UNLE]++;
        } else if (__builtin_islessgreater(a, b)) {
            cc_counts[CC_LTGT]++;
        }
        
        /* Switch based on comparison classification */
        int cmp_class = -1;
        if (__builtin_isunordered(a, b)) cmp_class = 0;
        else if (a == b) cmp_class = 1;
        else if (a < b) cmp_class = 2;
        else cmp_class = 3;
        
        switch (cmp_class) {
            case 0: /* UNORDERED */
                cc_counts[CC_UNORDERED]++;
                break;
            case 1: /* EQUAL (includes UNEQ when ordered) */
                cc_counts[CC_UNEQ]++;
                break;
            case 2: /* LESS */
                cc_counts[CC_UNLT]++;
                break;
            case 3: /* GREATER */
                if (!__builtin_isless(a, b) && !__builtin_isunordered(a, b)) {
                    cc_counts[CC_UNGE]++;
                }
                break;
        }
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Reset counters */
    memset(cc_counts, 0, sizeof(cc_counts));
    
    /* Run all test functions */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX__
    test_avx_conditions();
    printf("AVX support detected and tested.\n");
#endif
    
    /* Print summary of condition code hits */
    printf("\nCondition Code Summary:\n");
    printf("UNORDERED: %d\n", cc_counts[CC_UNORDERED]);
    printf("ORDERED: %d\n", cc_counts[CC_ORDERED]);
    printf("UNEQ: %d\n", cc_counts[CC_UNEQ]);
    printf("UNGE: %d\n", cc_counts[CC_UNGE]);
    printf("UNGT: %d\n", cc_counts[CC_UNGT]);
    printf("UNLE: %d\n", cc_counts[CC_UNLE]);
    printf("UNLT: %d\n", cc_counts[CC_UNLT]);
    printf("LTGT: %d\n", cc_counts[CC_LTGT]);
    
    /* Verify all condition codes were triggered */
    int total = 0;
    int all_nonzero = 1;
    for (int i = 0; i < 8; i++) {
        total += cc_counts[i];
        if (cc_counts[i] == 0) {
            printf("Warning: Condition code %d was not triggered\n", i);
            all_nonzero = 0;
        }
    }
    
    printf("\nTotal condition code hits: %d\n", total);
    
    if (all_nonzero) {
        printf("SUCCESS: All condition code paths were exercised!\n");
    } else {
        printf("NOTE: Some condition codes may need different test data\n");
    }
    
    return 0;
}
