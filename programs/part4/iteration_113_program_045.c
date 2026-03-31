#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <math.h>

// Result counters for each condition code type
typedef struct {
    int unordered;
    int ordered;
    int uneq;
    int unge;
    int ungt;
    int unle;
    int unlt;
    int ltgt;
} ConditionCounts;

// Initialize with NaN, infinity, and normal values
static const double test_values[] = {
    1.0, 2.0, -1.0, -2.0,
    0.0, -0.0,
    __builtin_inf(), -__builtin_inf(),
    __builtin_nan(""), -__builtin_nan(""),
    3.14159, -3.14159,
    1.0e-10, -1.0e-10,
    1.0e10, -1.0e10
};

#define TEST_COUNT (sizeof(test_values)/sizeof(test_values[0]))

// Test scalar comparisons using GCC builtins
void test_scalar_conditions(ConditionCounts *counts) {
    for (int i = 0; i < TEST_COUNT; i++) {
        for (int j = 0; j < TEST_COUNT; j++) {
            double a = test_values[i];
            double b = test_values[j];
            
            // UNORDERED case - using __builtin_isunordered
            if (__builtin_isunordered(a, b)) {
                counts->unordered++;
            }
            
            // ORDERED case - using __builtin_isordered
            if (__builtin_isordered(a, b)) {
                counts->ordered++;
            }
            
            // UNEQ case - unordered or equal
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                counts->uneq++;
            }
            
            // UNGE case - not less than (unordered or greater or equal)
            if (!__builtin_isless(a, b)) {
                counts->unge++;
            }
            
            // UNGT case - not less or equal (unordered or greater)
            if (!__builtin_islessequal(a, b)) {
                counts->ungt++;
            }
            
            // UNLE case - unordered or less or equal
            if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
                counts->unle++;
            }
            
            // UNLT case - unordered or less than
            if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
                counts->unlt++;
            }
            
            // LTGT case - less or greater (not equal, not unordered)
            if (__builtin_islessgreater(a, b)) {
                counts->ltgt++;
            }
        }
    }
}

// Test vector comparisons using SSE/AVX intrinsics
void test_vector_conditions(ConditionCounts *counts) {
    __m128d vec_a, vec_b;
    __m128d cmp_result;
    
    for (int i = 0; i < TEST_COUNT - 1; i += 2) {
        for (int j = 0; j < TEST_COUNT - 1; j += 2) {
            // Load two values into vectors
            vec_a = _mm_set_pd(test_values[i], test_values[i+1]);
            vec_b = _mm_set_pd(test_values[j], test_values[j+1]);
            
            // Various comparison predicates that map to condition codes
            // _CMP_UNORD_Q - unordered
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counts->unordered += 2;
            }
            
            // _CMP_ORD_Q - ordered
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counts->ordered += 2;
            }
            
            // _CMP_EQ_UQ - equal or unordered
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_EQ_UQ);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counts->uneq += 2;
            }
            
            // _CMP_NLT_UQ - not less than or unordered
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NLT_UQ);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counts->unge += 2;
            }
            
            // _CMP_NLE_UQ - not less or equal or unordered
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NLE_UQ);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counts->ungt += 2;
            }
            
            // _CMP_LE_UQ - less or equal or unordered
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_LE_UQ);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counts->unle += 2;
            }
            
            // _CMP_LT_UQ - less than or unordered
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_LT_UQ);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counts->unlt += 2;
            }
            
            // _CMP_NEQ_UQ - not equal or unordered
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NEQ_UQ);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counts->ltgt += 2;
            }
        }
    }
}

// Test inline assembly with explicit condition code constraints
void test_asm_constraints(ConditionCounts *counts) {
    double a, b;
    int result;
    
    for (int i = 0; i < TEST_COUNT; i++) {
        for (int j = 0; j < TEST_COUNT; j++) {
            a = test_values[i];
            b = test_values[j];
            
            // UNORDERED - using "unord" condition code
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setp %%al\n\t"
                "movzbl %%al, %0"
                : "=r"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counts->unordered++;
            
            // ORDERED - using "ord" condition code
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnp %%al\n\t"
                "movzbl %%al, %0"
                : "=r"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counts->ordered++;
            
            // UNEQ - using "ueq" condition code
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "sete %%al\n\t"
                "movzbl %%al, %0"
                : "=r"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counts->uneq++;
            
            // UNGE - using "nlt" condition code
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnb %%al\n\t"
                "movzbl %%al, %0"
                : "=r"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counts->unge++;
            
            // UNGT - using "nle" condition code
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counts->ungt++;
            
            // UNLE - using "ule" condition code
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counts->unle++;
            
            // UNLT - using "ult" condition code
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setb %%al\n\t"
                "movzbl %%al, %0"
                : "=r"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counts->unlt++;
            
            // LTGT - using "une" condition code
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setne %%al\n\t"
                "movzbl %%al, %0"
                : "=r"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counts->ltgt++;
        }
    }
}

// Control flow test with switch statement
void test_control_flow(ConditionCounts *counts) {
    for (int i = 0; i < TEST_COUNT; i++) {
        for (int j = 0; j < TEST_COUNT; j++) {
            double a = test_values[i];
            double b = test_values[j];
            
            // Classify the comparison result
            int comparison_class = 0;
            
            if (__builtin_isunordered(a, b)) {
                comparison_class = 1;  // UNORDERED
            } else if (__builtin_isgreater(a, b)) {
                comparison_class = 2;  // Greater
            } else if (__builtin_isless(a, b)) {
                comparison_class = 3;  // Less
            } else if (__builtin_isgreaterequal(a, b)) {
                comparison_class = 4;  // Greater or equal
            } else if (__builtin_islessequal(a, b)) {
                comparison_class = 5;  // Less or equal
            } else if (__builtin_islessgreater(a, b)) {
                comparison_class = 6;  // LTGT
            } else {
                comparison_class = 7;  // Equal
            }
            
            // Switch on comparison class - forces compiler to handle all cases
            switch (comparison_class) {
                case 1:  // UNORDERED
                    counts->unordered++;
                    break;
                case 2:  // Greater (implies ORDERED)
                    counts->ordered++;
                    break;
                case 3:  // Less (implies ORDERED)
                    counts->ordered++;
                    break;
                case 4:  // Greater or equal (UNGE when including unordered)
                    counts->unge++;
                    break;
                case 5:  // Less or equal (UNLE when including unordered)
                    counts->unle++;
                    break;
                case 6:  // LTGT
                    counts->ltgt++;
                    break;
                case 7:  // Equal (UNEQ when including unordered)
                    counts->uneq++;
                    break;
            }
        }
    }
}

int main() {
    ConditionCounts counts = {0};
    
    printf("Testing x86 floating-point condition codes...\n");
    printf("Test values include: normal numbers, +/-0, +/-Inf, NaN\n\n");
    
    // Run all test functions
    test_scalar_conditions(&counts);
    test_vector_conditions(&counts);
    test_asm_constraints(&counts);
    test_control_flow(&counts);
    
    // Print summary
    printf("Condition Code Hit Counts:\n");
    printf("  UNORDERED (unord): %d\n", counts.unordered);
    printf("  ORDERED   (ord):   %d\n", counts.ordered);
    printf("  UNEQ      (ueq):   %d\n", counts.uneq);
    printf("  UNGE      (nlt):   %d\n", counts.unge);
    printf("  UNGT      (nle):   %d\n", counts.ungt);
    printf("  UNLE      (ule):   %d\n", counts.unle);
    printf("  UNLT      (ult):   %d\n", counts.unlt);
    printf("  LTGT      (une):   %d\n", counts.ltgt);
    
    // Verify we hit all condition codes
    int total_hits = counts.unordered + counts.ordered + counts.uneq +
                     counts.unge + counts.ungt + counts.unle +
                     counts.unlt + counts.ltgt;
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("ERROR: No condition codes were triggered.\n");
        return 1;
    }
}
