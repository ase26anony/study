#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Condition code counters */
static int counters[8] = {0};
enum { UNORDERED_IDX, ORDERED_IDX, UNEQ_IDX, UNGE_IDX, 
       UNGT_IDX, UNLE_IDX, UNLT_IDX, LTGT_IDX };

/* Test data arrays */
#define TEST_SIZE 8
static double test_scalars[TEST_SIZE];
static __m128d test_vec128[TEST_SIZE/2];
static __m256d test_vec256[TEST_SIZE/4];

/* Initialize test data with normal values, Inf, -Inf, and NaN */
void init_test_data(void) {
    /* Normal numbers */
    test_scalars[0] = 1.0;
    test_scalars[1] = 2.0;
    test_scalars[2] = -1.0;
    test_scalars[3] = 0.0;
    
    /* Special values */
    test_scalars[4] = INFINITY;
    test_scalars[5] = -INFINITY;
    test_scalars[6] = NAN;
    test_scalars[7] = DBL_MAX;
    
    /* Initialize vector arrays */
    for (int i = 0; i < TEST_SIZE/2; i++) {
        test_vec128[i] = _mm_set_pd(test_scalars[2*i+1], test_scalars[2*i]);
    }
    
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
                counters[UNORDERED_IDX]++;
            }
            
            /* ORDERED: neither is NaN */
            if (__builtin_isordered(a, b)) {
                counters[ORDERED_IDX]++;
            }
            
            /* UNEQ: unordered or equal */
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                counters[UNEQ_IDX]++;
            }
            
            /* UNGE: not less than (greater or equal or unordered) */
            if (!__builtin_isless(a, b)) {
                counters[UNGE_IDX]++;
            }
            
            /* UNGT: not less or equal (greater or unordered) */
            if (!__builtin_islessequal(a, b)) {
                counters[UNGT_IDX]++;
            }
            
            /* UNLE: not greater than (less or equal or unordered) */
            if (!__builtin_isgreater(a, b)) {
                counters[UNLE_IDX]++;
            }
            
            /* UNLT: not greater or equal (less than or unordered) */
            if (!__builtin_isgreaterequal(a, b)) {
                counters[UNLT_IDX]++;
            }
            
            /* LTGT: less or greater (not equal and ordered) */
            if (__builtin_islessgreater(a, b)) {
                counters[LTGT_IDX]++;
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
            
            /* Various comparison predicates */
            __m128d cmp_eq = _mm_cmpeq_pd(a, b);
            __m128d cmp_lt = _mm_cmplt_pd(a, b);
            __m128d cmp_le = _mm_cmple_pd(a, b);
            __m128d cmp_unord = _mm_cmpunord_pd(a, b);
            __m128d cmp_neq = _mm_cmpneq_pd(a, b);
            __m128d cmp_nlt = _mm_cmpnlt_pd(a, b);
            __m128d cmp_nle = _mm_cmpnle_pd(a, b);
            __m128d cmp_ord = _mm_cmpord_pd(a, b);
            
            /* Extract masks and update counters based on results */
            int mask_eq = _mm_movemask_pd(cmp_eq);
            int mask_unord = _mm_movemask_pd(cmp_unord);
            int mask_nlt = _mm_movemask_pd(cmp_nlt);
            int mask_nle = _mm_movemask_pd(cmp_nle);
            int mask_ord = _mm_movemask_pd(cmp_ord);
            int mask_neq = _mm_movemask_pd(cmp_neq);
            
            /* Update counters based on mask bits */
            for (int k = 0; k < 2; k++) {
                if (mask_unord & (1 << k)) counters[UNORDERED_IDX]++;
                if (mask_ord & (1 << k)) counters[ORDERED_IDX]++;
                if ((mask_eq & (1 << k)) || (mask_unord & (1 << k))) counters[UNEQ_IDX]++;
                if (mask_nlt & (1 << k)) counters[UNGE_IDX]++;
                if (mask_nle & (1 << k)) counters[UNGT_IDX]++;
                if (!(mask_nle & (1 << k))) counters[UNLE_IDX]++;  /* Not nle = le or unordered */
                if (!(mask_nlt & (1 << k))) counters[UNLT_IDX]++;  /* Not nlt = lt or unordered */
                if (mask_neq & (1 << k) && !(mask_unord & (1 << k))) counters[LTGT_IDX]++;
            }
        }
    }
    
#ifdef __AVX__
    /* AVX vector comparisons */
    for (int i = 0; i < TEST_SIZE/4; i++) {
        for (int j = 0; j < TEST_SIZE/4; j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* AVX comparison predicates */
            __m256d cmp_eq = _mm256_cmp_pd(a, b, _CMP_EQ_OQ);
            __m256d cmp_lt = _mm256_cmp_pd(a, b, _CMP_LT_OQ);
            __m256d cmp_le = _mm256_cmp_pd(a, b, _CMP_LE_OQ);
            __m256d cmp_unord = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            __m256d cmp_neq = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            __m256d cmp_nlt = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            __m256d cmp_nle = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            __m256d cmp_ord = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            
            int mask = _mm256_movemask_pd(cmp_unord);
            if (mask) counters[UNORDERED_IDX] += __builtin_popcount(mask);
            
            mask = _mm256_movemask_pd(cmp_ord);
            if (mask) counters[ORDERED_IDX] += __builtin_popcount(mask);
            
            mask = _mm256_movemask_pd(cmp_neq);
            if (mask) counters[LTGT_IDX] += __builtin_popcount(mask);
        }
    }
#endif
}

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    printf("Testing inline assembly constraints...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            int result;
            
            /* Test various condition codes via inline assembly */
            
            /* UNORDERED */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[UNORDERED_IDX]++;
            
            /* ORDERED */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setnp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[ORDERED_IDX]++;
            
            /* UNEQ (unordered or equal) */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) counters[UNEQ_IDX]++;
            
            /* UNGE (not less than) */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setnb %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) counters[UNGE_IDX]++;
            
            /* UNGT (not less or equal) */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setnbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) counters[UNGT_IDX]++;
            
            /* UNLE (unordered or less or equal) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"  /* Note: swapped for le */
                "setbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) counters[UNLE_IDX]++;
            
            /* UNLT (unordered or less than) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"  /* Note: swapped for lt */
                "setb %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) counters[UNLT_IDX]++;
            
            /* LTGT (less or greater, ordered) */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setne %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) counters[LTGT_IDX]++;
        }
    }
}

/* Control flow test with switch statement */
void test_control_flow(void) {
    printf("Testing control flow...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            int condition = 0;
            
            /* Determine condition class */
            if (__builtin_isunordered(a, b)) {
                condition = 0;  /* UNORDERED */
            } else if (a == b) {
                condition = 2;  /* UNEQ (ordered equal) */
            } else if (a < b) {
                condition = 6;  /* UNLT (ordered less) */
            } else if (a > b) {
                condition = 4;  /* UNGT (ordered greater) */
            } else {
                condition = 7;  /* LTGT (shouldn't reach here) */
            }
            
            /* Switch on condition - forces compiler to handle all cases */
            switch (condition) {
                case 0:  /* UNORDERED */
                    counters[UNORDERED_IDX]++;
                    break;
                case 1:  /* ORDERED */
                    counters[ORDERED_IDX]++;
                    break;
                case 2:  /* UNEQ */
                    counters[UNEQ_IDX]++;
                    break;
                case 3:  /* UNGE */
                    counters[UNGE_IDX]++;
                    break;
                case 4:  /* UNGT */
                    counters[UNGT_IDX]++;
                    break;
                case 5:  /* UNLE */
                    counters[UNLE_IDX]++;
                    break;
                case 6:  /* UNLT */
                    counters[UNLT_IDX]++;
                    break;
                case 7:  /* LTGT */
                    counters[LTGT_IDX]++;
                    break;
            }
        }
    }
}

int main(void) {
    const char *condition_names[] = {
        "UNORDERED", "ORDERED", "UNEQ", "UNGE",
        "UNGT", "UNLE", "UNLT", "LTGT"
    };
    
    printf("=== Testing x86 Floating-Point Condition Codes ===\n\n");
    
    /* Initialize test data */
    init_test_data();
    
    /* Reset counters */
    memset(counters, 0, sizeof(counters));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print results */
    printf("\n=== Results ===\n");
    int total = 0;
    for (int i = 0; i < 8; i++) {
        printf("%-10s: %d\n", condition_names[i], counters[i]);
        total += counters[i];
    }
    printf("Total comparisons: %d\n", total);
    
    /* Verify we hit all condition codes */
    int all_hit = 1;
    for (int i = 0; i < 8; i++) {
        if (counters[i] == 0) {
            printf("WARNING: %s was never triggered!\n", condition_names[i]);
            all_hit = 0;
        }
    }
    
    if (all_hit) {
        printf("\nSUCCESS: All condition codes were exercised!\n");
    } else {
        printf("\nWARNING: Some condition codes were not exercised.\n");
    }
    
    return all_hit ? 0 : 1;
}
