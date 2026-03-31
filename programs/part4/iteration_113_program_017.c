#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Condition code counters */
static int counters[8] = {0};
enum {
    UNORDERED_IDX = 0,
    ORDERED_IDX = 1,
    UNEQ_IDX = 2,
    UNGE_IDX = 3,
    UNGT_IDX = 4,
    UNLE_IDX = 5,
    UNLT_IDX = 6,
    LTGT_IDX = 7
};

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    INFINITY, -INFINITY,
    NAN, -NAN,
    3.14, -2.71
};
#define NUM_SCALARS (sizeof(test_scalars)/sizeof(test_scalars[0]))

/* GCC builtins targeting specific condition codes */
void test_scalar_conditions(void) {
    double nan = __builtin_nan("");
    double inf = INFINITY;
    
    for (int i = 0; i < NUM_SCALARS; i++) {
        double a = test_scalars[i];
        
        /* UNORDERED: a or b is NaN */
        if (__builtin_isunordered(a, nan)) {
            counters[UNORDERED_IDX]++;
        }
        
        /* ORDERED: neither is NaN */
        if (__builtin_isordered(a, 1.0)) {
            counters[ORDERED_IDX]++;
        }
        
        /* UNEQ: unordered or equal */
        if (!__builtin_isgreater(a, a) && !__builtin_isless(a, a)) {
            counters[UNEQ_IDX]++;
        }
        
        /* UNGE: not less than (unordered or greater or equal) */
        if (!__builtin_isless(a, 2.0)) {
            counters[UNGE_IDX]++;
        }
        
        /* UNGT: not less or equal (unordered or greater) */
        if (!__builtin_islessequal(a, 1.0)) {
            counters[UNGT_IDX]++;
        }
        
        /* UNLE: unordered or less or equal */
        if (__builtin_islessequal(a, inf) || __builtin_isunordered(a, inf)) {
            counters[UNLE_IDX]++;
        }
        
        /* UNLT: unordered or less than */
        if (__builtin_isless(a, inf) || __builtin_isunordered(a, inf)) {
            counters[UNLT_IDX]++;
        }
        
        /* LTGT: less or greater (ordered and not equal) */
        if (__builtin_islessgreater(a, 2.0)) {
            counters[LTGT_IDX]++;
        }
    }
}

/* Vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    __m128d v1, v2, v_nan, v_inf;
    __m128i mask;
    
    /* Create vectors with mixed values */
    v1 = _mm_set_pd(1.0, 2.0);
    v2 = _mm_set_pd(NAN, INFINITY);
    v_nan = _mm_set1_pd(__builtin_nan(""));
    v_inf = _mm_set1_pd(INFINITY);
    
    /* Various comparison predicates */
    mask = _mm_castpd_si128(_mm_cmpord_pd(v1, v2));    /* ORDERED */
    if (_mm_movemask_pd(_mm_castsi128_pd(mask)) != 0) {
        counters[ORDERED_IDX]++;
    }
    
    mask = _mm_castpd_si128(_mm_cmpunord_pd(v1, v_nan)); /* UNORDERED */
    if (_mm_movemask_pd(_mm_castsi128_pd(mask)) != 0) {
        counters[UNORDERED_IDX]++;
    }
    
    mask = _mm_castpd_si128(_mm_cmpneq_pd(v1, v1));    /* UNEQ for NaN */
    if (_mm_movemask_pd(_mm_castsi128_pd(mask)) != 0) {
        counters[UNEQ_IDX]++;
    }
    
    mask = _mm_castpd_si128(_mm_cmpnlt_pd(v1, v2));    /* UNGE (not less than) */
    if (_mm_movemask_pd(_mm_castsi128_pd(mask)) != 0) {
        counters[UNGE_IDX]++;
    }
    
    mask = _mm_castpd_si128(_mm_cmpnle_pd(v1, v2));    /* UNGT (not less or equal) */
    if (_mm_movemask_pd(_mm_castsi128_pd(mask)) != 0) {
        counters[UNGT_IDX]++;
    }
    
    /* Test with AVX if available */
#ifdef __AVX__
    __m256d avx1 = _mm256_set_pd(1.0, 2.0, NAN, INFINITY);
    __m256d avx2 = _mm256_set_pd(2.0, 1.0, INFINITY, NAN);
    
    __m256d cmp_result = _mm256_cmp_pd(avx1, avx2, _CMP_UNORD_Q); /* UNORDERED */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[UNORDERED_IDX]++;
    }
    
    cmp_result = _mm256_cmp_pd(avx1, avx2, _CMP_ORD_Q); /* ORDERED */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[ORDERED_IDX]++;
    }
    
    cmp_result = _mm256_cmp_pd(avx1, avx2, _CMP_EQ_UQ); /* UNEQ */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[UNEQ_IDX]++;
    }
    
    cmp_result = _mm256_cmp_pd(avx1, avx2, _CMP_NLT_UQ); /* UNGE */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[UNGE_IDX]++;
    }
    
    cmp_result = _mm256_cmp_pd(avx1, avx2, _CMP_NLE_UQ); /* UNGT */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[UNGT_IDX]++;
    }
    
    cmp_result = _mm256_cmp_pd(avx1, avx2, _CMP_LE_OS); /* UNLE (ordered signaling) */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[UNLE_IDX]++;
    }
    
    cmp_result = _mm256_cmp_pd(avx1, avx2, _CMP_LT_OS); /* UNLT (ordered signaling) */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[UNLT_IDX]++;
    }
    
    cmp_result = _mm256_cmp_pd(avx1, avx2, _CMP_NEQ_OS); /* LTGT (ordered signaling) */
    if (_mm256_movemask_pd(cmp_result) != 0) {
        counters[LTGT_IDX]++;
    }
#endif
}

/* Inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = NAN;
    double c = 2.0;
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
    if (result) counters[UNORDERED_IDX]++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "cc"
    );
    if (result) counters[ORDERED_IDX]++;
    
    /* UNEQ constraint via flag combination */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (b), "x" (b)  /* NaN vs NaN */
        : "al", "bl", "cc"
    );
    if (result) counters[UNEQ_IDX]++;
    
    /* UNGE (not less than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (a)  /* 2.0 vs 1.0 */
        : "al", "cc"
    );
    if (result) counters[UNGE_IDX]++;
    
    /* UNGT (not less or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (a)
        : "al", "cc"
    );
    if (result) counters[UNGT_IDX]++;
    
    /* Using explicit condition code strings in constraints */
    int unord_result, ord_result, ueq_result;
    
    /* The following asm blocks force the compiler to generate
       condition code strings during RTL expansion */
    __asm__ (
        "/* %=unord */\n\t"
        : "=@unord" (unord_result)
        : "0" (0)
    );
    
    __asm__ (
        "/* %=ord */\n\t"
        : "=@ord" (ord_result)
        : "0" (0)
    );
    
    __asm__ (
        "/* %=ueq */\n\t"
        : "=@ueq" (ueq_result)
        : "0" (0)
    );
    
    /* These may not produce valid code but force condition code
       string generation in the compiler's RTL */
    (void)unord_result;
    (void)ord_result;
    (void)ueq_result;
}

/* Control flow that depends on comparison results */
void test_control_flow(void) {
    double values[] = {1.0, NAN, INFINITY, -INFINITY, 0.0};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow preventing optimization */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
                if (__builtin_isordered(a + 1.0, b - 1.0)) {
                    counters[ORDERED_IDX]++;
                }
            } else {
                counters[ORDERED_IDX]++;
                if (__builtin_islessgreater(a, b)) {
                    counters[LTGT_IDX]++;
                } else {
                    counters[UNEQ_IDX]++;
                }
            }
            
            /* Switch based on comparison classification */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 0;
            else if (__builtin_isless(a, b)) cmp_class = 1;
            else if (__builtin_isgreater(a, b)) cmp_class = 2;
            else cmp_class = 3;
            
            switch (cmp_class) {
                case 0: /* UNORDERED */
                    counters[UNORDERED_IDX]++;
                    break;
                case 1: /* UNLT or similar */
                    if (!__builtin_islessequal(a, b)) {
                        counters[UNGT_IDX]++;
                    } else {
                        counters[UNLE_IDX]++;
                    }
                    break;
                case 2: /* UNGT or similar */
                    if (!__builtin_isless(a, b)) {
                        counters[UNGE_IDX]++;
                    }
                    break;
                case 3: /* UNEQ */
                    counters[UNEQ_IDX]++;
                    break;
            }
        }
    }
}

int main(void) {
    /* Initialize counters */
    memset(counters, 0, sizeof(counters));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary */
    printf("Condition code coverage summary:\n");
    printf("UNORDERED: %d\n", counters[UNORDERED_IDX]);
    printf("ORDERED:   %d\n", counters[ORDERED_IDX]);
    printf("UNEQ:      %d\n", counters[UNEQ_IDX]);
    printf("UNGE:      %d\n", counters[UNGE_IDX]);
    printf("UNGT:      %d\n", counters[UNGT_IDX]);
    printf("UNLE:      %d\n", counters[UNLE_IDX]);
    printf("UNLT:      %d\n", counters[UNLT_IDX]);
    printf("LTGT:      %d\n", counters[LTGT_IDX]);
    
    /* Verify we hit all condition codes */
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += counters[i] > 0 ? 1 : 0;
    }
    
    printf("\nCovered %d/8 condition code types\n", total);
    
    return total == 8 ? 0 : 1;
}
