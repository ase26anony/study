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
#define TEST_SIZE 8
static double scalars[TEST_SIZE];
static __m128d vectors128[TEST_SIZE];
static __m256d vectors256[TEST_SIZE/2];

/* Initialize test data with normal values, infinities, and NaNs */
void init_test_data(void) {
    const double test_values[] = {
        1.0, -1.0, 0.0, -0.0,
        INFINITY, -INFINITY,
        NAN, -NAN
    };
    
    for (int i = 0; i < TEST_SIZE; i++) {
        scalars[i] = test_values[i];
        vectors128[i] = _mm_set_pd(test_values[i], test_values[(i+1)%TEST_SIZE]);
        if (i % 2 == 0) {
            vectors256[i/2] = _mm256_set_pd(
                test_values[i],
                test_values[(i+1)%TEST_SIZE],
                test_values[(i+2)%TEST_SIZE],
                test_values[(i+3)%TEST_SIZE]
            );
        }
    }
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    printf("Testing scalar conditions...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = scalars[i];
            double b = scalars[j];
            
            /* UNORDERED: __builtin_isunordered */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
            }
            
            /* ORDERED: !__builtin_isunordered */
            if (!__builtin_isunordered(a, b)) {
                counters[ORDERED_IDX]++;
            }
            
            /* UNEQ: unordered or equal */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[UNEQ_IDX]++;
            }
            
            /* UNGE: not less than (unordered or greater/equal) */
            if (!__builtin_isless(a, b)) {
                counters[UNGE_IDX]++;
            }
            
            /* UNGT: not less or equal (unordered or greater) */
            if (!__builtin_islessequal(a, b)) {
                counters[UNGT_IDX]++;
            }
            
            /* UNLE: unordered or less/equal */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                counters[UNLE_IDX]++;
            }
            
            /* UNLT: unordered or less */
            if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
                counters[UNLT_IDX]++;
            }
            
            /* LTGT: less or greater (not equal and ordered) */
            if (__builtin_islessgreater(a, b)) {
                counters[LTGT_IDX]++;
            }
        }
    }
}

/* Test vector comparisons with SSE/AVX intrinsics */
void test_vector_conditions(void) {
    printf("Testing vector conditions...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            __m128d a128 = vectors128[i];
            __m128d b128 = vectors128[j];
            __m256d a256 = vectors256[i/2];
            __m256d b256 = vectors256[j/2];
            
            /* SSE2 comparisons with different predicates */
            __m128d cmp;
            
            /* _CMP_UNORD_Q triggers UNORDERED */
            cmp = _mm_cmpunord_pd(a128, b128);
            if (_mm_movemask_pd(cmp) != 0) counters[UNORDERED_IDX]++;
            
            /* _CMP_ORD_Q triggers ORDERED */
            cmp = _mm_cmpord_pd(a128, b128);
            if (_mm_movemask_pd(cmp) != 0) counters[ORDERED_IDX]++;
            
            /* _CMP_EQ_UQ triggers UNEQ */
            cmp = _mm_cmpeq_pd(a128, b128);
            if (_mm_movemask_pd(cmp) != 0) counters[UNEQ_IDX]++;
            
            /* _CMP_NLT_US triggers UNGE */
            cmp = _mm_cmpnlt_pd(a128, b128);
            if (_mm_movemask_pd(cmp) != 0) counters[UNGE_IDX]++;
            
            /* _CMP_NLE_US triggers UNGT */
            cmp = _mm_cmpnle_pd(a128, b128);
            if (_mm_movemask_pd(cmp) != 0) counters[UNGT_IDX]++;
            
            /* _CMP_LE_UQ triggers UNLE */
            cmp = _mm_cmple_pd(a128, b128);
            if (_mm_movemask_pd(cmp) != 0) counters[UNLE_IDX]++;
            
            /* _CMP_LT_UQ triggers UNLT */
            cmp = _mm_cmplt_pd(a128, b128);
            if (_mm_movemask_pd(cmp) != 0) counters[UNLT_IDX]++;
            
            /* _CMP_NEQ_OQ triggers LTGT */
            cmp = _mm_cmpneq_pd(a128, b128);
            if (_mm_movemask_pd(cmp) != 0) counters[LTGT_IDX]++;
            
            /* AVX comparisons for wider code generation */
            #ifdef __AVX__
            __m256d cmp256;
            
            cmp256 = _mm256_cmp_pd(a256, b256, _CMP_UNORD_Q);
            if (_mm256_movemask_pd(cmp256) != 0) counters[UNORDERED_IDX]++;
            
            cmp256 = _mm256_cmp_pd(a256, b256, _CMP_ORD_Q);
            if (_mm256_movemask_pd(cmp256) != 0) counters[ORDERED_IDX]++;
            
            cmp256 = _mm256_cmp_pd(a256, b256, _CMP_EQ_UQ);
            if (_mm256_movemask_pd(cmp256) != 0) counters[UNEQ_IDX]++;
            
            cmp256 = _mm256_cmp_pd(a256, b256, _CMP_NLT_US);
            if (_mm256_movemask_pd(cmp256) != 0) counters[UNGE_IDX]++;
            
            cmp256 = _mm256_cmp_pd(a256, b256, _CMP_NLE_US);
            if (_mm256_movemask_pd(cmp256) != 0) counters[UNGT_IDX]++;
            
            cmp256 = _mm256_cmp_pd(a256, b256, _CMP_LE_UQ);
            if (_mm256_movemask_pd(cmp256) != 0) counters[UNLE_IDX]++;
            
            cmp256 = _mm256_cmp_pd(a256, b256, _CMP_LT_UQ);
            if (_mm256_movemask_pd(cmp256) != 0) counters[UNLT_IDX]++;
            
            cmp256 = _mm256_cmp_pd(a256, b256, _CMP_NEQ_OQ);
            if (_mm256_movemask_pd(cmp256) != 0) counters[LTGT_IDX]++;
            #endif
        }
    }
}

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    printf("Testing inline assembly constraints...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = scalars[i];
            double b = scalars[j];
            int result;
            
            /* UNORDERED constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setp %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccunord"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNORDERED_IDX]++;
            
            /* ORDERED constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnp %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccord"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[ORDERED_IDX]++;
            
            /* UNEQ constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "sete %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccueq"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNEQ_IDX]++;
            
            /* UNGE constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnb %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccnlt"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNGE_IDX]++;
            
            /* UNGT constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnbe %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccnle"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNGT_IDX]++;
            
            /* UNLE constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setbe %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccule"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNLE_IDX]++;
            
            /* UNLT constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setb %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccult"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNLT_IDX]++;
            
            /* LTGT constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setne %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccune"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
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
            double a = scalars[i];
            double b = scalars[j];
            int condition = 0;
            
            /* Determine condition class */
            if (__builtin_isunordered(a, b)) {
                condition = 0; /* UNORDERED */
            } else if (a == b) {
                condition = 2; /* UNEQ */
            } else if (a > b) {
                condition = 4; /* UNGT */
            } else {
                condition = 6; /* UNLT */
            }
            
            /* Switch on condition - prevents optimization */
            switch (condition) {
                case 0: /* UNORDERED */
                    counters[UNORDERED_IDX]++;
                    break;
                case 1: /* ORDERED */
                    counters[ORDERED_IDX]++;
                    break;
                case 2: /* UNEQ */
                    counters[UNEQ_IDX]++;
                    break;
                case 3: /* UNGE */
                    counters[UNGE_IDX]++;
                    break;
                case 4: /* UNGT */
                    counters[UNGT_IDX]++;
                    break;
                case 5: /* UNLE */
                    counters[UNLE_IDX]++;
                    break;
                case 6: /* UNLT */
                    counters[UNLT_IDX]++;
                    break;
                case 7: /* LTGT */
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
    
    printf("=== Testing x86 Condition Code Generation ===\n\n");
    
    /* Initialize test data */
    init_test_data();
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary */
    printf("\n=== Condition Code Summary ===\n");
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
        printf("\nWARNING: Some condition codes were not exercised\n");
    }
    
    return all_hit ? 0 : 1;
}
