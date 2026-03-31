#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <math.h>

/* Condition code counters */
typedef struct {
    unsigned int unordered;
    unsigned int ordered;
    unsigned int uneq;
    unsigned int unge;
    unsigned int ungt;
    unsigned int unle;
    unsigned int unlt;
    unsigned int ltgt;
} cc_counters;

static cc_counters global_counters = {0};

/* Test data arrays */
#define TEST_SIZE 8
static double scalar_test_a[TEST_SIZE];
static double scalar_test_b[TEST_SIZE];
static __m128d vector_test_a[TEST_SIZE];
static __m128d vector_test_b[TEST_SIZE];

/* Initialize test data with normal values, infinities, and NaNs */
void init_test_data(void) {
    const double test_values[] = {
        1.0, 2.0, -1.0, -2.0,
        0.0, -0.0,
        1.0/0.0,          /* +Inf */
        -1.0/0.0,         /* -Inf */
        __builtin_nan(""), /* NaN */
        3.14159, -3.14159,
        1e308, -1e308,
        1e-308, -1e-308
    };
    
    for (int i = 0; i < TEST_SIZE; i++) {
        scalar_test_a[i] = test_values[i % 14];
        scalar_test_b[i] = test_values[(i + 3) % 14];
        
        vector_test_a[i] = _mm_set_pd(test_values[i % 14], test_values[(i + 1) % 14]);
        vector_test_b[i] = _mm_set_pd(test_values[(i + 3) % 14], test_values[(i + 4) % 14]);
    }
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    printf("Testing scalar conditions...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        double a = scalar_test_a[i];
        double b = scalar_test_b[i];
        
        /* UNORDERED: a or b is NaN */
        if (__builtin_isunordered(a, b)) {
            global_counters.unordered++;
        }
        
        /* ORDERED: neither a nor b is NaN */
        if (__builtin_isordered(a, b)) {
            global_counters.ordered++;
        }
        
        /* UNEQ: unordered or equal */
        if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
            global_counters.uneq++;
        }
        
        /* UNGE: unordered or greater or equal */
        if (__builtin_isgreaterequal(a, b) || __builtin_isunordered(a, b)) {
            global_counters.unge++;
        }
        
        /* UNGT: unordered or greater */
        if (__builtin_isgreater(a, b) || __builtin_isunordered(a, b)) {
            global_counters.ungt++;
        }
        
        /* UNLE: unordered or less or equal */
        if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
            global_counters.unle++;
        }
        
        /* UNLT: unordered or less */
        if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
            global_counters.unlt++;
        }
        
        /* LTGT: less or greater (not equal, not unordered) */
        if (__builtin_islessgreater(a, b) && __builtin_isordered(a, b)) {
            global_counters.ltgt++;
        }
    }
}

/* Test vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    printf("Testing vector conditions...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        __m128d a = vector_test_a[i];
        __m128d b = vector_test_b[i];
        
        /* Generate comparison masks for different predicates */
        __m128d cmp_unord = _mm_cmpunord_pd(a, b);   /* UNORDERED */
        __m128d cmp_ord = _mm_cmpord_pd(a, b);       /* ORDERED */
        __m128d cmp_eq = _mm_cmpeq_pd(a, b);         /* Equal */
        __m128d cmp_lt = _mm_cmplt_pd(a, b);         /* Less than */
        __m128d cmp_le = _mm_cmple_pd(a, b);         /* Less or equal */
        __m128d cmp_gt = _mm_cmpgt_pd(a, b);         /* Greater than */
        __m128d cmp_ge = _mm_cmpge_pd(a, b);         /* Greater or equal */
        __m128d cmp_neq = _mm_cmpneq_pd(a, b);       /* Not equal */
        
        /* Extract masks and update counters */
        int mask_unord = _mm_movemask_pd(cmp_unord);
        int mask_ord = _mm_movemask_pd(cmp_ord);
        int mask_eq = _mm_movemask_pd(cmp_eq);
        int mask_lt = _mm_movemask_pd(cmp_lt);
        int mask_gt = _mm_movemask_pd(cmp_gt);
        int mask_neq = _mm_movemask_pd(cmp_neq);
        
        /* UNORDERED */
        if (mask_unord != 0) {
            global_counters.unordered += __builtin_popcount(mask_unord);
        }
        
        /* ORDERED */
        if (mask_ord != 0) {
            global_counters.ordered += __builtin_popcount(mask_ord);
        }
        
        /* UNEQ: unordered or equal */
        if ((mask_unord | mask_eq) != 0) {
            global_counters.uneq += __builtin_popcount(mask_unord | mask_eq);
        }
        
        /* UNGE: unordered or greater or equal (not less than) */
        if ((mask_unord | ~mask_lt & 0x3) != 0) {
            global_counters.unge += __builtin_popcount(mask_unord | ~mask_lt & 0x3);
        }
        
        /* UNGT: unordered or greater (not less or equal) */
        if ((mask_unord | mask_gt) != 0) {
            global_counters.ungt += __builtin_popcount(mask_unord | mask_gt);
        }
        
        /* UNLE: unordered or less or equal */
        if ((mask_unord | ~mask_gt & 0x3) != 0) {
            global_counters.unle += __builtin_popcount(mask_unord | ~mask_gt & 0x3);
        }
        
        /* UNLT: unordered or less */
        if ((mask_unord | mask_lt) != 0) {
            global_counters.unlt += __builtin_popcount(mask_unord | mask_lt);
        }
        
        /* LTGT: less or greater (not equal, not unordered) */
        if ((mask_lt | mask_gt) != 0 && mask_unord == 0) {
            global_counters.ltgt += __builtin_popcount(mask_lt | mask_gt);
        }
    }
}

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    printf("Testing inline assembly constraints...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        double a = scalar_test_a[i];
        double b = scalar_test_b[i];
        int result;
        
        /* UNORDERED constraint */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setp %%al\n\t"
            "movzbl %%al, %0"
            : "=r" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) global_counters.unordered++;
        
        /* ORDERED constraint */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnp %%al\n\t"
            "movzbl %%al, %0"
            : "=r" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) global_counters.ordered++;
        
        /* UNEQ constraint (unordered or equal) */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setbe %%al\n\t"
            "movzbl %%al, %0"
            : "=r" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) global_counters.uneq++;
        
        /* UNGE constraint (unordered or not less than) */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnb %%al\n\t"
            "movzbl %%al, %0"
            : "=r" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) global_counters.unge++;
        
        /* UNGT constraint (unordered or greater than) */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnbe %%al\n\t"
            "movzbl %%al, %0"
            : "=r" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) global_counters.ungt++;
        
        /* UNLE constraint (unordered or less or equal) */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setna %%al\n\t"
            "movzbl %%al, %0"
            : "=r" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) global_counters.unle++;
        
        /* UNLT constraint (unordered or less than) */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setb %%al\n\t"
            "movzbl %%al, %0"
            : "=r" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) global_counters.unlt++;
        
        /* LTGT constraint (not equal and ordered) */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setne %%al\n\t"
            "setnp %%ah\n\t"
            "andb %%ah, %%al\n\t"
            "movzbl %%al, %0"
            : "=r" (result)
            : "x" (a), "x" (b)
            : "al", "ah"
        );
        if (result) global_counters.ltgt++;
    }
}

/* Test with AVX-512 if available */
#ifdef __AVX512F__
void test_avx512_conditions(void) {
    printf("Testing AVX-512 conditions...\n");
    
    __m512d avx_a = _mm512_set_pd(
        scalar_test_a[0], scalar_test_a[1], scalar_test_a[2], scalar_test_a[3],
        scalar_test_a[4], scalar_test_a[5], scalar_test_a[6], scalar_test_a[7]
    );
    __m512d avx_b = _mm512_set_pd(
        scalar_test_b[0], scalar_test_b[1], scalar_test_b[2], scalar_test_b[3],
        scalar_test_b[4], scalar_test_b[5], scalar_test_b[6], scalar_test_b[7]
    );
    
    /* Test various AVX-512 comparison predicates */
    __mmask8 mask;
    
    /* _CMP_UNORD_Q */
    mask = _mm512_cmp_pd_mask(avx_a, avx_b, _CMP_UNORD_Q);
    global_counters.unordered += __builtin_popcount(mask);
    
    /* _CMP_ORD_Q */
    mask = _mm512_cmp_pd_mask(avx_a, avx_b, _CMP_ORD_Q);
    global_counters.ordered += __builtin_popcount(mask);
    
    /* _CMP_EQ_UQ (equal unordered) */
    mask = _mm512_cmp_pd_mask(avx_a, avx_b, _CMP_EQ_UQ);
    global_counters.uneq += __builtin_popcount(mask);
    
    /* _CMP_NLT_UQ (not less than unordered) */
    mask = _mm512_cmp_pd_mask(avx_a, avx_b, _CMP_NLT_UQ);
    global_counters.unge += __builtin_popcount(mask);
    
    /* _CMP_NLE_UQ (not less or equal unordered) */
    mask = _mm512_cmp_pd_mask(avx_a, avx_b, _CMP_NLE_UQ);
    global_counters.ungt += __builtin_popcount(mask);
    
    /* _CMP_LE_UQ (less or equal unordered) */
    mask = _mm512_cmp_pd_mask(avx_a, avx_b, _CMP_LE_UQ);
    global_counters.unle += __builtin_popcount(mask);
    
    /* _CMP_LT_UQ (less than unordered) */
    mask = _mm512_cmp_pd_mask(avx_a, avx_b, _CMP_LT_UQ);
    global_counters.unlt += __builtin_popcount(mask);
    
    /* _CMP_NEQ_OQ (not equal ordered) */
    mask = _mm512_cmp_pd_mask(avx_a, avx_b, _CMP_NEQ_OQ);
    global_counters.ltgt += __builtin_popcount(mask);
}
#endif

/* Print summary of condition code hits */
void print_summary(void) {
    printf("\n=== Condition Code Summary ===\n");
    printf("UNORDERED: %u\n", global_counters.unordered);
    printf("ORDERED:   %u\n", global_counters.ordered);
    printf("UNEQ:      %u\n", global_counters.uneq);
    printf("UNGE:      %u\n", global_counters.unge);
    printf("UNGT:      %u\n", global_counters.ungt);
    printf("UNLE:      %u\n", global_counters.unle);
    printf("UNLT:      %u\n", global_counters.unlt);
    printf("LTGT:      %u\n", global_counters.ltgt);
    
    unsigned int total = global_counters.unordered + global_counters.ordered +
                        global_counters.uneq + global_counters.unge +
                        global_counters.ungt + global_counters.unle +
                        global_counters.unlt + global_counters.ltgt;
    printf("Total:     %u\n", total);
}

int main(void) {
    printf("Starting condition code coverage test...\n");
    
    /* Initialize test data with NaNs, infinities, and normal numbers */
    init_test_data();
    
    /* Run all test suites */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    
    #ifdef __AVX512F__
    test_avx512_conditions();
    #endif
    
    /* Print results */
    print_summary();
    
    /* Verify we actually exercised the code */
    if (global_counters.unordered > 0 &&
        global_counters.ordered > 0 &&
        global_counters.uneq > 0 &&
        global_counters.unge > 0 &&
        global_counters.ungt > 0 &&
        global_counters.unle > 0 &&
        global_counters.unlt > 0 &&
        global_counters.ltgt > 0) {
        printf("\nSUCCESS: All condition code paths were exercised!\n");
        return 0;
    } else {
        printf("\nWARNING: Some condition code paths may not have been fully exercised\n");
        return 1;
    }
}
