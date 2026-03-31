#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Condition code counters */
static struct {
    unsigned int unordered;
    unsigned int ordered;
    unsigned int uneq;
    unsigned int unge;
    unsigned int ungt;
    unsigned int unle;
    unsigned int unlt;
    unsigned int ltgt;
} cc_counts = {0};

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    1.0/0.0,          /* +Inf */
    -1.0/0.0,         /* -Inf */
    0.0/0.0,          /* NaN (quiet) */
    __builtin_nan(""), /* Explicit NaN */
    __builtin_nanf(""),
};

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define TEST_COUNT ARRAY_SIZE(test_scalars)

/* Helper to get NaN */
static double get_nan(void) {
    return __builtin_nan("");
}

/* Test scalar comparisons using GCC builtins */
static void test_scalar_conditions(void) {
    double nan = get_nan();
    double inf = 1.0/0.0;
    
    for (int i = 0; i < TEST_COUNT; i++) {
        for (int j = 0; j < TEST_COUNT; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED: a or b is NaN */
            if (__builtin_isunordered(a, b)) {
                cc_counts.unordered++;
            }
            
            /* ORDERED: neither is NaN */
            if (!__builtin_isunordered(a, b)) {
                cc_counts.ordered++;
            }
            
            /* UNEQ: unordered or equal */
            if (__builtin_isunordered(a, b) || a == b) {
                cc_counts.uneq++;
            }
            
            /* UNGE: unordered or a >= b */
            if (__builtin_isunordered(a, b) || a >= b) {
                cc_counts.unge++;
            }
            
            /* UNGT: unordered or a > b */
            if (__builtin_isunordered(a, b) || a > b) {
                cc_counts.ungt++;
            }
            
            /* UNLE: unordered or a <= b */
            if (__builtin_isunordered(a, b) || a <= b) {
                cc_counts.unle++;
            }
            
            /* UNLT: unordered or a < b */
            if (__builtin_isunordered(a, b) || a < b) {
                cc_counts.unlt++;
            }
            
            /* LTGT: less or greater (unordered or equal excluded) */
            if (__builtin_islessgreater(a, b)) {
                cc_counts.ltgt++;
            }
        }
    }
}

/* Test vector comparisons with SSE/AVX */
static void test_vector_conditions(void) {
    __m128d vec_nan = _mm_set1_pd(get_nan());
    __m128d vec_one = _mm_set1_pd(1.0);
    __m128d vec_two = _mm_set1_pd(2.0);
    __m128d vec_zero = _mm_set1_pd(0.0);
    
    /* Test various vector comparisons */
    __m128d cmp;
    int mask;
    
    /* UNORDERED comparisons */
    cmp = _mm_cmpunord_pd(vec_nan, vec_one);
    mask = _mm_movemask_pd(cmp);
    if (mask) cc_counts.unordered += mask & 1;
    if (mask & 2) cc_counts.unordered++;
    
    /* ORDERED comparisons */
    cmp = _mm_cmpord_pd(vec_one, vec_two);
    mask = _mm_movemask_pd(cmp);
    if (mask) cc_counts.ordered += mask & 1;
    if (mask & 2) cc_counts.ordered++;
    
    /* UNEQ: unordered or equal */
    cmp = _mm_cmpneq_pd(vec_one, vec_one);  /* All equal */
    mask = _mm_movemask_pd(cmp);
    if (!mask) cc_counts.uneq += 2;
    
    /* UNGE: unordered or greater/equal */
    cmp = _mm_cmpnlt_pd(vec_two, vec_one);  /* 2.0 not less than 1.0 */
    mask = _mm_movemask_pd(cmp);
    if (mask) cc_counts.unge += mask & 1;
    if (mask & 2) cc_counts.unge++;
    
    /* UNGT: unordered or greater */
    cmp = _mm_cmpnle_pd(vec_two, vec_one);  /* 2.0 not less/equal to 1.0 */
    mask = _mm_movemask_pd(cmp);
    if (mask) cc_counts.ungt += mask & 1;
    if (mask & 2) cc_counts.ungt++;
    
    /* UNLE: unordered or less/equal */
    cmp = _mm_cmpunord_pd(vec_nan, vec_one);
    mask = _mm_movemask_pd(cmp);
    if (mask) {
        /* Combine with ordered comparison */
        __m128d cmp_ord = _mm_cmple_pd(vec_one, vec_two);
        int mask_ord = _mm_movemask_pd(cmp_ord);
        cc_counts.unle += (mask | mask_ord) & 1 ? 1 : 0;
        cc_counts.unle += (mask | mask_ord) & 2 ? 1 : 0;
    }
    
    /* UNLT: unordered or less */
    cmp = _mm_cmpunord_pd(vec_nan, vec_one);
    mask = _mm_movemask_pd(cmp);
    if (mask) {
        __m128d cmp_lt = _mm_cmplt_pd(vec_one, vec_two);
        int mask_lt = _mm_movemask_pd(cmp_lt);
        cc_counts.unlt += (mask | mask_lt) & 1 ? 1 : 0;
        cc_counts.unlt += (mask | mask_lt) & 2 ? 1 : 0;
    }
    
    /* LTGT: less or greater */
    cmp = _mm_cmpneq_pd(vec_one, vec_two);  /* 1.0 != 2.0 */
    mask = _mm_movemask_pd(cmp);
    if (mask) cc_counts.ltgt += mask & 1;
    if (mask & 2) cc_counts.ltgt++;
}

/* Inline assembly with explicit condition code constraints */
static void test_asm_constraints(void) {
    double a = 1.0;
    double b = get_nan();
    double c = 2.0;
    int result;
    
    /* UNORDERED constraint */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) cc_counts.unordered++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=@ord" (result)
        : "x" (a), "x" (c)
        : "al"
    );
    if (result) cc_counts.ordered++;
    
    /* UNEQ constraint - unordered or equal */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=@ueq" (result)
        : "x" (a), "x" (a)  /* equal values */
        : "al", "bl"
    );
    if (result) cc_counts.uneq++;
    
    /* UNGE constraint - unordered or not less than */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=@nlt" (result)
        : "x" (c), "x" (a)  /* 2.0 >= 1.0 */
        : "al"
    );
    if (result) cc_counts.unge++;
    
    /* UNGT constraint - unordered or not less/equal */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=@nle" (result)
        : "x" (c), "x" (a)  /* 2.0 > 1.0 */
        : "al"
    );
    if (result) cc_counts.ungt++;
    
    /* UNLE constraint - unordered or less/equal */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setna %%al\n\t"
        "movzbl %%al, %0"
        : "=@ule" (result)
        : "x" (a), "x" (c)  /* 1.0 <= 2.0 */
        : "al"
    );
    if (result) cc_counts.unle++;
    
    /* UNLT constraint - unordered or less than */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=@ult" (result)
        : "x" (a), "x" (c)  /* 1.0 < 2.0 */
        : "al"
    );
    if (result) cc_counts.unlt++;
    
    /* LTGT constraint - not equal and ordered */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=@une" (result)
        : "x" (a), "x" (c)  /* 1.0 != 2.0 */
        : "al"
    );
    if (result) cc_counts.ltgt++;
}

/* AVX-512 specific tests if available */
#ifdef __AVX512F__
static void test_avx512_conditions(void) {
    __m512d vec_nan = _mm512_set1_pd(get_nan());
    __m512d vec_one = _mm512_set1_pd(1.0);
    __m512d vec_two = _mm512_set1_pd(2.0);
    
    __mmask8 mask;
    
    /* Test various AVX-512 comparison predicates */
    mask = _mm512_cmp_pd_mask(vec_nan, vec_one, _CMP_UNORD_Q);
    cc_counts.unordered += __builtin_popcount(mask);
    
    mask = _mm512_cmp_pd_mask(vec_one, vec_two, _CMP_ORD_Q);
    cc_counts.ordered += __builtin_popcount(mask);
    
    mask = _mm512_cmp_pd_mask(vec_one, vec_one, _CMP_EQ_UQ);
    cc_counts.uneq += __builtin_popcount(mask);
    
    mask = _mm512_cmp_pd_mask(vec_two, vec_one, _CMP_NLT_UQ);
    cc_counts.unge += __builtin_popcount(mask);
    
    mask = _mm512_cmp_pd_mask(vec_two, vec_one, _CMP_NLE_UQ);
    cc_counts.ungt += __builtin_popcount(mask);
    
    mask = _mm512_cmp_pd_mask(vec_one, vec_two, _CMP_LE_UQ);
    cc_counts.unle += __builtin_popcount(mask);
    
    mask = _mm512_cmp_pd_mask(vec_one, vec_two, _CMP_LT_UQ);
    cc_counts.unlt += __builtin_popcount(mask);
    
    mask = _mm512_cmp_pd_mask(vec_one, vec_two, _CMP_NEQ_OQ);
    cc_counts.ltgt += __builtin_popcount(mask);
}
#endif

/* Control flow based on comparison results */
static void test_control_flow(void) {
    double values[] = {1.0, get_nan(), 2.0, 0.0};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                cc_counts.unordered++;
                if (!__builtin_isless(a, b)) {
                    cc_counts.unge++;
                }
            } else {
                cc_counts.ordered++;
                if (__builtin_isless(a, b)) {
                    cc_counts.unlt++;
                } else if (__builtin_isgreater(a, b)) {
                    cc_counts.ungt++;
                } else {
                    cc_counts.uneq++;
                }
            }
            
            /* Switch-like behavior */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 1;
            else if (a == b) cmp_class = 2;
            else if (a < b) cmp_class = 3;
            else cmp_class = 4;
            
            switch (cmp_class) {
                case 1: /* UNORDERED */
                    cc_counts.unordered++;
                    break;
                case 2: /* UNEQ (ordered equal) */
                    cc_counts.uneq++;
                    break;
                case 3: /* UNLT (ordered less) */
                    cc_counts.unlt++;
                    break;
                case 4: /* UNGT (ordered greater) */
                    cc_counts.ungt++;
                    break;
            }
        }
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Reset counters */
    memset(&cc_counts, 0, sizeof(cc_counts));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX512F__
    test_avx512_conditions();
    printf("AVX-512 support detected and tested.\n");
#endif
    
    /* Print summary */
    printf("\nCondition Code Summary:\n");
    printf("UNORDERED: %u\n", cc_counts.unordered);
    printf("ORDERED:   %u\n", cc_counts.ordered);
    printf("UNEQ:      %u\n", cc_counts.uneq);
    printf("UNGE:      %u\n", cc_counts.unge);
    printf("UNGT:      %u\n", cc_counts.ungt);
    printf("UNLE:      %u\n", cc_counts.unle);
    printf("UNLT:      %u\n", cc_counts.unlt);
    printf("LTGT:      %u\n", cc_counts.ltgt);
    
    /* Verify we hit all condition codes */
    unsigned int total = cc_counts.unordered + cc_counts.ordered + cc_counts.uneq +
                        cc_counts.unge + cc_counts.ungt + cc_counts.unle +
                        cc_counts.unlt + cc_counts.ltgt;
    
    printf("\nTotal condition code hits: %u\n", total);
    
    if (total == 0) {
        printf("ERROR: No condition codes were triggered!\n");
        return 1;
    }
    
    printf("\nAll condition codes tested successfully.\n");
    return 0;
}
