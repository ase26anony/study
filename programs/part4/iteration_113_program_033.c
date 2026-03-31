#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <math.h>

/* Condition code counters */
static int cc_counts[8] = {0};
enum CC_TYPES { CC_UNORDERED, CC_ORDERED, CC_UNEQ, CC_UNGE, CC_UNGT, CC_UNLE, CC_UNLT, CC_LTGT };

/* Test data arrays */
#define TEST_SIZE 8
static double test_scalars[TEST_SIZE];
static __m128d test_vec128[TEST_SIZE/2];
static __m256d test_vec256[TEST_SIZE/4];

/* Initialize test data with normal values, Inf, -Inf, and NaN */
void init_test_data(void) {
    const double inf = 1.0 / 0.0;
    const double neg_inf = -1.0 / 0.0;
    const double nan = 0.0 / 0.0;
    
    /* Scalar test values */
    test_scalars[0] = 1.0;
    test_scalars[1] = 2.0;
    test_scalars[2] = -1.5;
    test_scalars[3] = 0.0;
    test_scalars[4] = inf;
    test_scalars[5] = neg_inf;
    test_scalars[6] = nan;
    test_scalars[7] = 3.14159;
    
    /* 128-bit vector test values */
    for (int i = 0; i < TEST_SIZE/2; i++) {
        test_vec128[i] = _mm_set_pd(test_scalars[2*i+1], test_scalars[2*i]);
    }
    
    /* 256-bit vector test values (if AVX available) */
    for (int i = 0; i < TEST_SIZE/4; i++) {
        test_vec256[i] = _mm256_set_pd(test_scalars[4*i+3], test_scalars[4*i+2],
                                      test_scalars[4*i+1], test_scalars[4*i]);
    }
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    printf("Testing scalar conditions...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED: a or b is NaN */
            if (__builtin_isunordered(a, b)) {
                cc_counts[CC_UNORDERED]++;
            }
            
            /* ORDERED: neither a nor b is NaN */
            if (__builtin_isordered(a, b)) {
                cc_counts[CC_ORDERED]++;
            }
            
            /* UNEQ: unordered or equal */
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                cc_counts[CC_UNEQ]++;
            }
            
            /* UNGE: unordered or greater or equal */
            if (!__builtin_isless(a, b)) {
                cc_counts[CC_UNGE]++;
            }
            
            /* UNGT: unordered or greater */
            if (!__builtin_islessequal(a, b)) {
                cc_counts[CC_UNGT]++;
            }
            
            /* UNLE: unordered or less or equal */
            if (!__builtin_isgreater(a, b)) {
                cc_counts[CC_UNLE]++;
            }
            
            /* UNLT: unordered or less */
            if (!__builtin_isgreaterequal(a, b)) {
                cc_counts[CC_UNLT]++;
            }
            
            /* LTGT: less or greater (ordered and not equal) */
            if (__builtin_islessgreater(a, b)) {
                cc_counts[CC_LTGT]++;
            }
        }
    }
}

/* Test vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    printf("Testing vector conditions...\n");
    
    /* SSE2 vector comparisons */
    for (int i = 0; i < TEST_SIZE/2; i++) {
        for (int j = 0; j < TEST_SIZE/2; j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            
            /* Compare with different predicates */
            __m128d cmp_unord = _mm_cmpunord_pd(a, b);  /* UNORDERED */
            __m128d cmp_ord = _mm_cmpord_pd(a, b);      /* ORDERED */
            __m128d cmp_eq = _mm_cmpeq_pd(a, b);        /* EQ */
            __m128d cmp_lt = _mm_cmplt_pd(a, b);        /* LT */
            __m128d cmp_le = _mm_cmple_pd(a, b);        /* LE */
            __m128d cmp_neq = _mm_cmpneq_pd(a, b);      /* NEQ */
            __m128d cmp_nlt = _mm_cmpnlt_pd(a, b);      /* NLT (UNGE) */
            __m128d cmp_nle = _mm_cmpnle_pd(a, b);      /* NLE (UNGT) */
            
            /* Extract comparison results */
            int mask_unord = _mm_movemask_pd(cmp_unord);
            int mask_ord = _mm_movemask_pd(cmp_ord);
            int mask_nlt = _mm_movemask_pd(cmp_nlt);
            int mask_nle = _mm_movemask_pd(cmp_nle);
            
            /* Update counters based on mask bits */
            for (int k = 0; k < 2; k++) {
                if (mask_unord & (1 << k)) cc_counts[CC_UNORDERED]++;
                if (mask_ord & (1 << k)) cc_counts[CC_ORDERED]++;
                if (mask_nlt & (1 << k)) cc_counts[CC_UNGE]++;
                if (mask_nle & (1 << k)) cc_counts[CC_UNGT]++;
            }
        }
    }
    
    /* AVX vector comparisons if available */
#ifdef __AVX__
    printf("Testing AVX vector conditions...\n");
    
    for (int i = 0; i < TEST_SIZE/4; i++) {
        for (int j = 0; j < TEST_SIZE/4; j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* AVX comparison predicates */
            __m256d cmp_unord = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            __m256d cmp_ord = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            __m256d cmp_eq_uq = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);   /* EQ (unordered quiet) */
            __m256d cmp_nge_us = _mm256_cmp_pd(a, b, _CMP_NGE_US); /* NGE (unordered signaling) */
            __m256d cmp_ngt_us = _mm256_cmp_pd(a, b, _CMP_NGT_US); /* NGT (unordered signaling) */
            __m256d cmp_neq_oq = _mm256_cmp_pd(a, b, _CMP_NEQ_OQ); /* NEQ (ordered quiet) */
            
            int mask_unord = _mm256_movemask_pd(cmp_unord);
            int mask_ord = _mm256_movemask_pd(cmp_ord);
            
            for (int k = 0; k < 4; k++) {
                if (mask_unord & (1 << k)) cc_counts[CC_UNORDERED]++;
                if (mask_ord & (1 << k)) cc_counts[CC_ORDERED]++;
            }
        }
    }
#endif
}

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    printf("Testing inline assembly with condition codes...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            int result;
            
            /* Test various condition code constraints */
            
            /* UNORDERED */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) cc_counts[CC_UNORDERED]++;
            
            /* ORDERED */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) cc_counts[CC_ORDERED]++;
            
            /* UNEQ (unordered or equal) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) cc_counts[CC_UNEQ]++;
            
            /* UNGE (not less than) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnb %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) cc_counts[CC_UNGE]++;
            
            /* UNGT (not less or equal) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) cc_counts[CC_UNGT]++;
            
            /* UNLE (unordered or less or equal) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setna %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) cc_counts[CC_UNLE]++;
            
            /* UNLT (unordered or less) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setb %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) cc_counts[CC_UNLT]++;
            
            /* LTGT (not equal and ordered) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setne %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) cc_counts[CC_LTGT]++;
        }
    }
}

/* Control flow test with switch statement */
void test_control_flow(void) {
    printf("Testing control flow dependent on comparisons...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            int condition = 0;
            
            /* Determine condition class */
            if (__builtin_isunordered(a, b)) {
                condition = CC_UNORDERED;
            } else if (__builtin_isgreater(a, b)) {
                condition = CC_UNGT;
            } else if (__builtin_isless(a, b)) {
                condition = CC_UNLT;
            } else if (__builtin_isgreaterequal(a, b)) {
                condition = CC_UNGE;
            } else if (__builtin_islessequal(a, b)) {
                condition = CC_UNLE;
            } else if (__builtin_islessgreater(a, b)) {
                condition = CC_LTGT;
            } else {
                condition = CC_UNEQ;
            }
            
            /* Switch on condition code class */
            switch (condition) {
                case CC_UNORDERED:
                    cc_counts[CC_UNORDERED]++;
                    break;
                case CC_ORDERED:
                    cc_counts[CC_ORDERED]++;
                    break;
                case CC_UNEQ:
                    cc_counts[CC_UNEQ]++;
                    break;
                case CC_UNGE:
                    cc_counts[CC_UNGE]++;
                    break;
                case CC_UNGT:
                    cc_counts[CC_UNGT]++;
                    break;
                case CC_UNLE:
                    cc_counts[CC_UNLE]++;
                    break;
                case CC_UNLT:
                    cc_counts[CC_UNLT]++;
                    break;
                case CC_LTGT:
                    cc_counts[CC_LTGT]++;
                    break;
            }
        }
    }
}

int main(void) {
    printf("Starting condition code coverage test...\n\n");
    
    /* Initialize test data */
    init_test_data();
    
    /* Reset counters */
    memset(cc_counts, 0, sizeof(cc_counts));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary */
    printf("\n=== Condition Code Summary ===\n");
    printf("UNORDERED: %d\n", cc_counts[CC_UNORDERED]);
    printf("ORDERED:   %d\n", cc_counts[CC_ORDERED]);
    printf("UNEQ:      %d\n", cc_counts[CC_UNEQ]);
    printf("UNGE:      %d\n", cc_counts[CC_UNGE]);
    printf("UNGT:      %d\n", cc_counts[CC_UNGT]);
    printf("UNLE:      %d\n", cc_counts[CC_UNLE]);
    printf("UNLT:      %d\n", cc_counts[CC_UNLT]);
    printf("LTGT:      %d\n", cc_counts[CC_LTGT]);
    
    /* Verify all condition codes were triggered */
    int total = 0;
    int all_nonzero = 1;
    for (int i = 0; i < 8; i++) {
        total += cc_counts[i];
        if (cc_counts[i] == 0) {
            printf("Warning: Condition code %d was not triggered!\n", i);
            all_nonzero = 0;
        }
    }
    
    printf("\nTotal comparisons: %d\n", total);
    printf("All condition codes triggered: %s\n", all_nonzero ? "YES" : "NO");
    
    return all_nonzero ? 0 : 1;
}
