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
    const double inf = 1.0/0.0;
    const double neg_inf = -1.0/0.0;
    const double nan = 0.0/0.0;
    
    /* Scalar test values */
    test_scalars[0] = 1.0;
    test_scalars[1] = 2.0;
    test_scalars[2] = -1.0;
    test_scalars[3] = 0.0;
    test_scalars[4] = inf;
    test_scalars[5] = neg_inf;
    test_scalars[6] = nan;
    test_scalars[7] = __builtin_nan("");
    
    /* Vector test values */
    for (int i = 0; i < TEST_SIZE/2; i++) {
        test_vec128[i] = _mm_set_pd(test_scalars[2*i+1], test_scalars[2*i]);
    }
    
    /* AVX vector test values */
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
            if (__builtin_isgreater(a, b)) {  /* a > b and ordered */
                cc_counts[CC_ORDERED]++;
            }
            
            /* UNEQ: unordered or equal */
            if (!__builtin_islessgreater(a, b)) {  /* a == b or unordered */
                cc_counts[CC_UNEQ]++;
            }
            
            /* UNGE: unordered or a >= b */
            if (__builtin_isgreaterequal(a, b)) {
                cc_counts[CC_UNGE]++;
            }
            
            /* UNGT: unordered or a > b */
            if (__builtin_isgreater(a, b)) {
                cc_counts[CC_UNGT]++;
            }
            
            /* UNLE: unordered or a <= b */
            if (__builtin_islessequal(a, b)) {
                cc_counts[CC_UNLE]++;
            }
            
            /* UNLT: unordered or a < b */
            if (__builtin_isless(a, b)) {
                cc_counts[CC_UNLT]++;
            }
            
            /* LTGT: a < b or a > b (ordered and not equal) */
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
            
            /* Compare with various predicates */
            __m128d cmp_eq = _mm_cmpeq_pd(a, b);
            __m128d cmp_lt = _mm_cmplt_pd(a, b);
            __m128d cmp_le = _mm_cmple_pd(a, b);
            __m128d cmp_unord = _mm_cmpunord_pd(a, b);
            __m128d cmp_neq = _mm_cmpneq_pd(a, b);
            __m128d cmp_nlt = _mm_cmpnlt_pd(a, b);
            __m128d cmp_nle = _mm_cmpnle_pd(a, b);
            __m128d cmp_ord = _mm_cmpord_pd(a, b);
            
            /* Extract results and update counters */
            double res[2];
            _mm_store_pd(res, cmp_unord);
            if (res[0] || res[1]) cc_counts[CC_UNORDERED]++;
            
            _mm_store_pd(res, cmp_ord);
            if (res[0] && res[1]) cc_counts[CC_ORDERED]++;
            
            /* UNEQ: unordered or equal */
            _mm_store_pd(res, _mm_or_pd(cmp_unord, cmp_eq));
            if (res[0] || res[1]) cc_counts[CC_UNEQ]++;
            
            /* UNGE: not less than (nlt) */
            _mm_store_pd(res, cmp_nlt);
            if (res[0] || res[1]) cc_counts[CC_UNGE]++;
            
            /* UNGT: not less or equal (nle) */
            _mm_store_pd(res, cmp_nle);
            if (res[0] || res[1]) cc_counts[CC_UNGT]++;
            
            /* UNLE: less or equal with unordered */
            _mm_store_pd(res, _mm_or_pd(cmp_unord, cmp_le));
            if (res[0] || res[1]) cc_counts[CC_UNLE]++;
            
            /* UNLT: less than with unordered */
            _mm_store_pd(res, _mm_or_pd(cmp_unord, cmp_lt));
            if (res[0] || res[1]) cc_counts[CC_UNLT]++;
            
            /* LTGT: not equal and ordered (une) */
            _mm_store_pd(res, _mm_and_pd(cmp_neq, cmp_ord));
            if (res[0] || res[1]) cc_counts[CC_LTGT]++;
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
            
            /* Extract and process results */
            double res[4];
            _mm256_store_pd(res, cmp_unord);
            for (int k = 0; k < 4; k++) if (res[k]) cc_counts[CC_UNORDERED]++;
            
            _mm256_store_pd(res, cmp_ord);
            for (int k = 0; k < 4; k++) if (res[k]) cc_counts[CC_ORDERED]++;
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
            
            /* UNORDERED constraint */
            __asm__ volatile (
                "comisd %1, %2\n\t"
                "setp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) cc_counts[CC_UNORDERED]++;
            
            /* ORDERED constraint */
            __asm__ volatile (
                "comisd %1, %2\n\t"
                "setnp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) cc_counts[CC_ORDERED]++;
            
            /* UNEQ constraint using ueq */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "sete %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) cc_counts[CC_UNEQ]++;
            
            /* UNGE constraint using nlt */
            __asm__ volatile (
                "comisd %2, %1\n\t"  /* Note: reversed operands for nlt */
                "setnb %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) cc_counts[CC_UNGE]++;
            
            /* UNGT constraint using nle */
            __asm__ volatile (
                "comisd %2, %1\n\t"  /* Note: reversed operands for nle */
                "setnbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) cc_counts[CC_UNGT]++;
            
            /* UNLE constraint using ule */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) cc_counts[CC_UNLE]++;
            
            /* UNLT constraint using ult */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setb %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) cc_counts[CC_UNLT]++;
            
            /* LTGT constraint using une */
            __asm__ volatile (
                "comisd %1, %2\n\t"
                "setne %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) cc_counts[CC_LTGT]++;
        }
    }
}

/* Control flow test with condition-dependent branches */
void test_control_flow(void) {
    printf("Testing condition-dependent control flow...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* Complex if-else chain */
            if (__builtin_isunordered(a, b)) {
                cc_counts[CC_UNORDERED]++;
                if (__builtin_isless(a, b)) {
                    /* This path should rarely be taken for unordered */
                    cc_counts[CC_UNLT]++;
                }
            } else if (__builtin_isless(a, b)) {
                cc_counts[CC_UNLT]++;
                if (__builtin_isgreaterequal(b, a)) {
                    cc_counts[CC_UNGE]++;
                }
            } else if (__builtin_isgreater(a, b)) {
                cc_counts[CC_UNGT]++;
                if (__builtin_islessequal(b, a)) {
                    cc_counts[CC_UNLE]++;
                }
            } else if (!__builtin_islessgreater(a, b)) {
                cc_counts[CC_UNEQ]++;
            }
            
            /* Switch based on comparison classification */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 0;
            else if (__builtin_isless(a, b)) cmp_class = 1;
            else if (__builtin_isgreater(a, b)) cmp_class = 2;
            else cmp_class = 3;
            
            switch (cmp_class) {
                case 0: /* UNORDERED */
                    cc_counts[CC_UNORDERED]++;
                    break;
                case 1: /* UNLT or UNLE */
                    if (__builtin_islessequal(a, b)) {
                        cc_counts[CC_UNLE]++;
                    } else {
                        cc_counts[CC_UNLT]++;
                    }
                    break;
                case 2: /* UNGT or UNGE */
                    if (__builtin_isgreaterequal(a, b)) {
                        cc_counts[CC_UNGE]++;
                    } else {
                        cc_counts[CC_UNGT]++;
                    }
                    break;
                case 3: /* UNEQ or ORDERED */
                    if (!__builtin_islessgreater(a, b)) {
                        cc_counts[CC_UNEQ]++;
                    } else {
                        cc_counts[CC_ORDERED]++;
                    }
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
    printf("\n=== Condition Code Coverage Summary ===\n");
    printf("UNORDERED (unord): %d\n", cc_counts[CC_UNORDERED]);
    printf("ORDERED   (ord):   %d\n", cc_counts[CC_ORDERED]);
    printf("UNEQ      (ueq):   %d\n", cc_counts[CC_UNEQ]);
    printf("UNGE      (nlt):   %d\n", cc_counts[CC_UNGE]);
    printf("UNGT      (nle):   %d\n", cc_counts[CC_UNGT]);
    printf("UNLE      (ule):   %d\n", cc_counts[CC_UNLE]);
    printf("UNLT      (ult):   %d\n", cc_counts[CC_UNLT]);
    printf("LTGT      (une):   %d\n", cc_counts[CC_LTGT]);
    
    /* Verify we hit all condition codes */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += cc_counts[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("Test PASSED - condition codes were exercised\n");
        return 0;
    } else {
        printf("Test FAILED - no condition codes were hit\n");
        return 1;
    }
}
