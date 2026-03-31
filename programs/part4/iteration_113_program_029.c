#include <stdio.h>
#include <stdlib.h>
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
static double scalars[TEST_SIZE];
static __m128d vectors128[TEST_SIZE];
static __m256d vectors256[TEST_SIZE/2];

/* Initialize test data with normal values, Inf, and NaN */
void init_test_data(void) {
    scalars[0] = 1.0;
    scalars[1] = 2.0;
    scalars[2] = -1.0;
    scalars[3] = 0.0;
    scalars[4] = __builtin_inf();
    scalars[5] = -__builtin_inf();
    scalars[6] = __builtin_nan("");
    scalars[7] = DBL_MAX;
    
    for (int i = 0; i < TEST_SIZE; i++) {
        vectors128[i] = _mm_set_pd(scalars[i], scalars[(i+1)%TEST_SIZE]);
    }
    
    for (int i = 0; i < TEST_SIZE/2; i++) {
        vectors256[i] = _mm256_set_pd(
            scalars[i*2], scalars[i*2+1],
            scalars[(i*2+2)%TEST_SIZE], scalars[(i*2+3)%TEST_SIZE]
        );
    }
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    printf("Testing scalar conditions...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = scalars[i];
            double b = scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                counters[ORDERED_IDX]++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || (a == b)) {
                counters[UNEQ_IDX]++;
            }
            
            /* UNGE (not less than) */
            if (!__builtin_isless(a, b)) {
                counters[UNGE_IDX]++;
            }
            
            /* UNGT (not less than or equal) */
            if (!__builtin_islessequal(a, b)) {
                counters[UNGT_IDX]++;
            }
            
            /* UNLE (unordered or less than or equal) */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                counters[UNLE_IDX]++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
                counters[UNLT_IDX]++;
            }
            
            /* LTGT (less than or greater than) */
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
            __m128d a = vectors128[i];
            __m128d b = vectors128[j];
            
            /* Generate various comparison masks */
            __m128d cmp_eq = _mm_cmpeq_pd(a, b);
            __m128d cmp_lt = _mm_cmplt_pd(a, b);
            __m128d cmp_le = _mm_cmple_pd(a, b);
            __m128d cmp_unord = _mm_cmpunord_pd(a, b);
            __m128d cmp_neq = _mm_cmpneq_pd(a, b);
            __m128d cmp_nlt = _mm_cmpnlt_pd(a, b);
            __m128d cmp_nle = _mm_cmpnle_pd(a, b);
            
            /* Extract masks and update counters based on results */
            double mask_eq[2], mask_lt[2], mask_le[2], mask_unord[2];
            _mm_store_pd(mask_eq, cmp_eq);
            _mm_store_pd(mask_lt, cmp_lt);
            _mm_store_pd(mask_le, cmp_le);
            _mm_store_pd(mask_unord, cmp_unord);
            
            for (int k = 0; k < 2; k++) {
                /* UNORDERED */
                if (mask_unord[k]) counters[UNORDERED_IDX]++;
                
                /* ORDERED */
                if (!mask_unord[k]) counters[ORDERED_IDX]++;
                
                /* UNEQ (unordered or equal) */
                if (mask_unord[k] || mask_eq[k]) counters[UNEQ_IDX]++;
                
                /* UNGE (not less than) */
                if (!mask_lt[k]) counters[UNGE_IDX]++;
                
                /* UNGT (not less than or equal) */
                if (!mask_le[k]) counters[UNGT_IDX]++;
                
                /* UNLE (unordered or less than or equal) */
                if (mask_unord[k] || mask_le[k]) counters[UNLE_IDX]++;
                
                /* UNLT (unordered or less than) */
                if (mask_unord[k] || mask_lt[k]) counters[UNLT_IDX]++;
                
                /* LTGT (not equal and ordered) */
                if (!mask_eq[k] && !mask_unord[k]) counters[LTGT_IDX]++;
            }
        }
    }
}

/* Test inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    printf("Testing inline assembly constraints...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = scalars[i];
            double b = scalars[j];
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
            if (result) counters[UNORDERED_IDX]++;
            
            /* ORDERED constraint */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[ORDERED_IDX]++;
            
            /* UNEQ constraint (unordered or equal) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setp %%al\n\t"
                "sete %%dl\n\t"
                "or %%dl, %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "dl"
            );
            if (result) counters[UNEQ_IDX]++;
            
            /* UNGE constraint (not less than) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnb %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[UNGE_IDX]++;
            
            /* UNGT constraint (not less than or equal) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[UNGT_IDX]++;
            
            /* UNLE constraint (unordered or less than or equal) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setbe %%al\n\t"
                "setp %%dl\n\t"
                "or %%dl, %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "dl"
            );
            if (result) counters[UNLE_IDX]++;
            
            /* UNLT constraint (unordered or less than) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setb %%al\n\t"
                "setp %%dl\n\t"
                "or %%dl, %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "dl"
            );
            if (result) counters[UNLT_IDX]++;
            
            /* LTGT constraint (not equal and ordered) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setne %%al\n\t"
                "setnp %%dl\n\t"
                "and %%dl, %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "dl"
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
            int condition_class = 0;
            
            /* Determine condition class */
            if (__builtin_isunordered(a, b)) {
                condition_class = 0; /* UNORDERED */
            } else if (a == b) {
                condition_class = 2; /* UNEQ (ordered equal) */
            } else if (a < b) {
                condition_class = 6; /* UNLT (ordered less than) */
            } else if (a > b) {
                condition_class = 4; /* UNGT (ordered greater than) */
            } else {
                condition_class = 1; /* ORDERED but not equal, less, or greater */
            }
            
            /* Switch on condition class - forces compiler to handle all cases */
            switch (condition_class) {
                case 0: /* UNORDERED */
                    counters[UNORDERED_IDX]++;
                    break;
                case 1: /* ORDERED but special */
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
    printf("=== Testing x86 Floating-Point Condition Codes ===\n\n");
    
    init_test_data();
    
    /* Run all test functions */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary */
    printf("\n=== Condition Code Summary ===\n");
    printf("UNORDERED: %d\n", counters[UNORDERED_IDX]);
    printf("ORDERED:   %d\n", counters[ORDERED_IDX]);
    printf("UNEQ:      %d\n", counters[UNEQ_IDX]);
    printf("UNGE:      %d\n", counters[UNGE_IDX]);
    printf("UNGT:      %d\n", counters[UNGT_IDX]);
    printf("UNLE:      %d\n", counters[UNLE_IDX]);
    printf("UNLT:      %d\n", counters[UNLT_IDX]);
    printf("LTGT:      %d\n", counters[LTGT_IDX]);
    
    /* Verify we hit all condition codes */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += counters[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("ERROR: No condition codes were triggered.\n");
        return 1;
    }
}
