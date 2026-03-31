#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Condition code counters */
static int cc_counts[8] = {0};
enum CC_TYPES { UNORDERED_IDX, ORDERED_IDX, UNEQ_IDX, UNGE_IDX, 
                UNGT_IDX, UNLE_IDX, UNLT_IDX, LTGT_IDX };

/* Test data arrays */
#define TEST_SIZE 8
static double test_scalars[TEST_SIZE];
static __m128d test_vec128[TEST_SIZE/2];
static __m256d test_vec256[TEST_SIZE/4];

/* Initialize test data with normal values, Inf, -Inf, and NaN */
void init_test_data(void) {
    const double inf = INFINITY;
    const double neg_inf = -INFINITY;
    const double nan_val = __builtin_nan("");
    
    /* Scalar test values */
    test_scalars[0] = 1.0;
    test_scalars[1] = 2.0;
    test_scalars[2] = -1.5;
    test_scalars[3] = 0.0;
    test_scalars[4] = inf;
    test_scalars[5] = neg_inf;
    test_scalars[6] = nan_val;
    test_scalars[7] = 3.14159;
    
    /* 128-bit vector test values */
    for (int i = 0; i < TEST_SIZE/2; i++) {
        test_vec128[i] = _mm_set_pd(test_scalars[2*i+1], test_scalars[2*i]);
    }
    
    /* 256-bit vector test values */
    for (int i = 0; i < TEST_SIZE/4; i++) {
        test_vec256[i] = _mm256_set_pd(test_scalars[4*i+3], test_scalars[4*i+2],
                                       test_scalars[4*i+1], test_scalars[4*i]);
    }
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    printf("Testing scalar conditions with builtins:\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED: a or b is NaN */
            if (__builtin_isunordered(a, b)) {
                cc_counts[UNORDERED_IDX]++;
            }
            
            /* ORDERED: neither a nor b is NaN */
            if (__builtin_isgreater(a, b)) {  /* a > b and !isunordered(a,b) */
                cc_counts[ORDERED_IDX]++;
            }
            
            /* UNEQ: unordered or equal */
            if (!__builtin_islessgreater(a, b)) {  /* !(a < b || a > b) */
                cc_counts[UNEQ_IDX]++;
            }
            
            /* UNGE: unordered or greater-or-equal */
            if (!__builtin_isless(a, b)) {  /* !(a < b) */
                cc_counts[UNGE_IDX]++;
            }
            
            /* UNGT: unordered or greater */
            if (!__builtin_islessequal(a, b)) {  /* !(a <= b) */
                cc_counts[UNGT_IDX]++;
            }
            
            /* UNLE: unordered or less-or-equal */
            if (!__builtin_isgreater(a, b)) {  /* !(a > b) */
                cc_counts[UNLE_IDX]++;
            }
            
            /* UNLT: unordered or less */
            if (!__builtin_isgreaterequal(a, b)) {  /* !(a >= b) */
                cc_counts[UNLT_IDX]++;
            }
            
            /* LTGT: less or greater (ordered and not equal) */
            if (__builtin_islessgreater(a, b)) {  /* a < b || a > b */
                cc_counts[LTGT_IDX]++;
            }
        }
    }
}

/* Test vector comparisons using intrinsics */
void test_vector_conditions(void) {
    printf("Testing vector conditions with intrinsics:\n");
    
    /* Test with __m128d (SSE2) */
    for (int i = 0; i < TEST_SIZE/2; i++) {
        for (int j = 0; j < TEST_SIZE/2; j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            
            /* Compare with various predicates */
            __m128d cmp_unord = _mm_cmpunord_pd(a, b);    /* UNORDERED */
            __m128d cmp_eq = _mm_cmpeq_pd(a, b);          /* EQ */
            __m128d cmp_lt = _mm_cmplt_pd(a, b);          /* LT */
            __m128d cmp_le = _mm_cmple_pd(a, b);          /* LE */
            __m128d cmp_gt = _mm_cmpgt_pd(a, b);          /* GT */
            __m128d cmp_ge = _mm_cmpge_pd(a, b);          /* GE */
            __m128d cmp_neq = _mm_cmpneq_pd(a, b);        /* NEQ */
            __m128d cmp_nlt = _mm_cmpnlt_pd(a, b);        /* NLT (UNGE) */
            __m128d cmp_nle = _mm_cmpnle_pd(a, b);        /* NLE (UNGT) */
            
            /* Extract masks and update counters */
            int mask_unord = _mm_movemask_pd(cmp_unord);
            int mask_eq = _mm_movemask_pd(cmp_eq);
            int mask_lt = _mm_movemask_pd(cmp_lt);
            int mask_nlt = _mm_movemask_pd(cmp_nlt);
            int mask_nle = _mm_movemask_pd(cmp_nle);
            int mask_neq = _mm_movemask_pd(cmp_neq);
            
            if (mask_unord) cc_counts[UNORDERED_IDX]++;
            if (!mask_unord) cc_counts[ORDERED_IDX]++;  /* Ordered when not unordered */
            if (mask_unord || mask_eq) cc_counts[UNEQ_IDX]++;  /* Unordered or equal */
            if (mask_nlt) cc_counts[UNGE_IDX]++;  /* Not less than */
            if (mask_nle) cc_counts[UNGT_IDX]++;  /* Not less or equal */
            if (mask_unord || !mask_gt) cc_counts[UNLE_IDX]++;  /* Unordered or not greater */
            if (mask_unord || mask_lt) cc_counts[UNLT_IDX]++;  /* Unordered or less than */
            if (mask_neq && !mask_unord) cc_counts[LTGT_IDX]++;  /* Not equal and ordered */
        }
    }
    
    /* Test with __m256d (AVX) if available */
#ifdef __AVX__
    printf("Testing AVX vector conditions:\n");
    
    for (int i = 0; i < TEST_SIZE/4; i++) {
        for (int j = 0; j < TEST_SIZE/4; j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* AVX comparison predicates */
            __m256d cmp_unord = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            __m256d cmp_eq_uq = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);    /* Equal (unordered non-signaling) */
            __m256d cmp_nge_us = _mm256_cmp_pd(a, b, _CMP_NGE_US);  /* Not greater-or-equal (unordered signaling) */
            __m256d cmp_ngt_us = _mm256_cmp_pd(a, b, _CMP_NGT_US);  /* Not greater-than (unordered signaling) */
            __m256d cmp_false_oq = _mm256_cmp_pd(a, b, _CMP_FALSE_OQ); /* False (ordered quiet) */
            __m256d cmp_neq_oq = _mm256_cmp_pd(a, b, _CMP_NEQ_OQ);  /* Not equal (ordered quiet) */
            __m256d cmp_ge_os = _mm256_cmp_pd(a, b, _CMP_GE_OS);    /* Greater-or-equal (ordered signaling) */
            __m256d cmp_gt_os = _mm256_cmp_pd(a, b, _CMP_GT_OS);    /* Greater-than (ordered signaling) */
            
            int mask_unord = _mm256_movemask_pd(cmp_unord);
            int mask_eq_uq = _mm256_movemask_pd(cmp_eq_uq);
            int mask_nge_us = _mm256_movemask_pd(cmp_nge_us);
            int mask_neq_oq = _mm256_movemask_pd(cmp_neq_oq);
            
            if (mask_unord) cc_counts[UNORDERED_IDX]++;
            if (!mask_unord) cc_counts[ORDERED_IDX]++;
            if (mask_eq_uq) cc_counts[UNEQ_IDX]++;  /* Equal or unordered */
            if (mask_nge_us) cc_counts[UNGE_IDX]++;  /* Not greater-or-equal (unordered) */
            if (mask_neq_oq && !mask_unord) cc_counts[LTGT_IDX]++;  /* Not equal and ordered */
        }
    }
#endif
}

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    printf("Testing inline assembly with condition code constraints:\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            int result;
            
            /* Test each condition code via inline assembly */
            
            /* UNORDERED */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setp %%al\n\t"
                "movzbl %%al, %2"
                : "=r"(result) : "x"(a), "x"(b), "=@ccunord"(result)
                : "al"
            );
            if (result) cc_counts[UNORDERED_IDX]++;
            
            /* ORDERED */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnp %%al\n\t"
                "movzbl %%al, %2"
                : "=r"(result) : "x"(a), "x"(b), "=@ccord"(result)
                : "al"
            );
            if (result) cc_counts[ORDERED_IDX]++;
            
            /* UNEQ (unordered or equal) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "sete %%al\n\t"
                "setp %%ah\n\t"
                "or %%ah, %%al\n\t"
                "movzbl %%al, %2"
                : "=r"(result) : "x"(a), "x"(b), "=@ccueq"(result)
                : "al", "ah"
            );
            if (result) cc_counts[UNEQ_IDX]++;
            
            /* UNGE (not less than) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnb %%al\n\t"
                "movzbl %%al, %2"
                : "=r"(result) : "x"(a), "x"(b), "=@ccnlt"(result)
                : "al"
            );
            if (result) cc_counts[UNGE_IDX]++;
            
            /* UNGT (not less or equal) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnbe %%al\n\t"
                "movzbl %%al, %2"
                : "=r"(result) : "x"(a), "x"(b), "=@ccnle"(result)
                : "al"
            );
            if (result) cc_counts[UNGT_IDX]++;
            
            /* UNLE (unordered or less-or-equal) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setbe %%al\n\t"
                "setp %%ah\n\t"
                "or %%ah, %%al\n\t"
                "movzbl %%al, %2"
                : "=r"(result) : "x"(a), "x"(b), "=@ccule"(result)
                : "al", "ah"
            );
            if (result) cc_counts[UNLE_IDX]++;
            
            /* UNLT (unordered or less than) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setb %%al\n\t"
                "setp %%ah\n\t"
                "or %%ah, %%al\n\t"
                "movzbl %%al, %2"
                : "=r"(result) : "x"(a), "x"(b), "=@ccult"(result)
                : "al", "ah"
            );
            if (result) cc_counts[UNLT_IDX]++;
            
            /* LTGT (less or greater, ordered) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setne %%al\n\t"
                "setnp %%ah\n\t"
                "and %%ah, %%al\n\t"
                "movzbl %%al, %2"
                : "=r"(result) : "x"(a), "x"(b), "=@ccune"(result)
                : "al", "ah"
            );
            if (result) cc_counts[LTGT_IDX]++;
        }
    }
}

/* Control flow test with condition-dependent branching */
void test_control_flow(void) {
    printf("Testing control flow dependent on comparisons:\n");
    
    volatile double a = 1.0;
    volatile double b = __builtin_nan("");
    volatile double c = 2.0;
    volatile double d = -INFINITY;
    
    /* Complex control flow to prevent optimization */
    for (int i = 0; i < 100; i++) {
        /* UNORDERED path */
        if (__builtin_isunordered(a + i, b)) {
            cc_counts[UNORDERED_IDX]++;
            a += 0.1;
        } else {
            b = __builtin_nan("");
        }
        
        /* ORDERED path */
        if (__builtin_isgreater(c, d)) {
            cc_counts[ORDERED_IDX]++;
            c *= 1.01;
        }
        
        /* UNEQ path */
        if (!__builtin_islessgreater(a, c)) {
            cc_counts[UNEQ_IDX]++;
            d = -d;
        }
        
        /* Switch statement based on comparison results */
        int cmp_result = 0;
        if (__builtin_isless(a, c)) cmp_result |= 1;
        if (__builtin_isgreater(a, d)) cmp_result |= 2;
        if (__builtin_isunordered(a, b)) cmp_result |= 4;
        
        switch (cmp_result) {
            case 0: /* No conditions true */
                cc_counts[UNGE_IDX]++;  /* Not less than */
                break;
            case 1: /* a < c */
                cc_counts[UNLT_IDX]++;
                break;
            case 2: /* a > d */
                cc_counts[UNGT_IDX]++;
                break;
            case 3: /* a < c && a > d */
                cc_counts[LTGT_IDX]++;
                break;
            case 4: /* Unordered */
                cc_counts[UNLE_IDX]++;  /* Unordered or less-or-equal */
                break;
            default:
                cc_counts[UNEQ_IDX]++;
                break;
        }
    }
}

int main(void) {
    printf("=== Testing x86 Floating-Point Condition Codes ===\n\n");
    
    /* Initialize test data */
    init_test_data();
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary */
    printf("\n=== Condition Code Summary ===\n");
    printf("UNORDERED:  %d\n", cc_counts[UNORDERED_IDX]);
    printf("ORDERED:    %d\n", cc_counts[ORDERED_IDX]);
    printf("UNEQ:       %d\n", cc_counts[UNEQ_IDX]);
    printf("UNGE:       %d\n", cc_counts[UNGE_IDX]);
    printf("UNGT:       %d\n", cc_counts[UNGT_IDX]);
    printf("UNLE:       %d\n", cc_counts[UNLE_IDX]);
    printf("UNLT:       %d\n", cc_counts[UNLT_IDX]);
    printf("LTGT:       %d\n", cc_counts[LTGT_IDX]);
    
    /* Verify all condition codes were triggered */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += cc_counts[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("FAILURE: No condition codes were triggered.\n");
        return 1;
    }
}
