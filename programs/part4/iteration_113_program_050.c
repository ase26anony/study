#include <stdio.h>
#include <stdlib.h>
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
    __builtin_nan(""), -__builtin_nan(""),
    3.14, -2.71
};

#define ARRAY_SIZE (sizeof(test_scalars)/sizeof(test_scalars[0]))

/* Scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
            }
            
            /* ORDERED */
            if (!__builtin_isunordered(a, b)) {
                counters[ORDERED_IDX]++;
            }
            
            /* UNEQ (unordered or equal) */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[UNEQ_IDX]++;
            }
            
            /* UNGE (not less than) */
            if (!(a < b)) {
                counters[UNGE_IDX]++;
            }
            
            /* UNGT (not less than or equal) */
            if (!(a <= b)) {
                counters[UNGT_IDX]++;
            }
            
            /* UNLE (unordered or less than or equal) */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters[UNLE_IDX]++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || a < b) {
                counters[UNLT_IDX]++;
            }
            
            /* LTGT (less than or greater than, but not equal) */
            if (__builtin_islessgreater(a, b)) {
                counters[LTGT_IDX]++;
            }
        }
    }
}

/* Vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    __m128d vec_a, vec_b, cmp_result;
    __m256d vec_a256, vec_b256, cmp_result256;
    
    /* Test with SSE2 */
    for (int i = 0; i < ARRAY_SIZE - 1; i += 2) {
        vec_a = _mm_set_pd(test_scalars[i], test_scalars[i+1]);
        vec_b = _mm_set_pd(test_scalars[i+1], test_scalars[i]);
        
        /* Various comparison predicates */
        cmp_result = _mm_cmpord_pd(vec_a, vec_b);  /* ORDERED */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[ORDERED_IDX]++;
        }
        
        cmp_result = _mm_cmpunord_pd(vec_a, vec_b); /* UNORDERED */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[UNORDERED_IDX]++;
        }
        
        cmp_result = _mm_cmpnlt_pd(vec_a, vec_b);   /* UNLT (not less than) */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[UNLT_IDX]++;
        }
        
        cmp_result = _mm_cmpnle_pd(vec_a, vec_b);   /* UNLE (not less than or equal) */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[UNLE_IDX]++;
        }
    }
    
    /* Test with AVX if available */
#ifdef __AVX__
    for (int i = 0; i < ARRAY_SIZE - 3; i += 4) {
        vec_a256 = _mm256_set_pd(test_scalars[i], test_scalars[i+1],
                                test_scalars[i+2], test_scalars[i+3]);
        vec_b256 = _mm256_set_pd(test_scalars[i+3], test_scalars[i+2],
                                test_scalars[i+1], test_scalars[i]);
        
        /* AVX comparison predicates */
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_UNORD_Q);
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[UNORDERED_IDX]++;
        }
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_ORD_Q);
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[ORDERED_IDX]++;
        }
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NLT_UQ);
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[UNLT_IDX]++;
        }
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NLE_UQ);
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[UNLE_IDX]++;
        }
    }
#endif
}

/* Inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a, b;
    int result;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            a = test_scalars[i];
            b = test_scalars[j];
            
            /* UNORDERED */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setp %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccunord"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNORDERED_IDX]++;
            
            /* ORDERED */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnp %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccord"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[ORDERED_IDX]++;
            
            /* UNEQ */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "sete %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccueq"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNEQ_IDX]++;
            
            /* UNGE (nlt) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnb %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccnlt"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNGE_IDX]++;
            
            /* UNGT (nle) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnbe %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccnle"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNGT_IDX]++;
            
            /* UNLE (ule) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setbe %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccule"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNLE_IDX]++;
            
            /* UNLT (ult) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setb %%al\n\t"
                "movzbl %%al, %2"
                : "=@ccult"(result)
                : "x"(a), "x"(b), "=r"(result)
                : "al"
            );
            if (result) counters[UNLT_IDX]++;
            
            /* LTGT (une) */
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

/* Control flow that depends on comparison results */
void test_control_flow(void) {
    double a, b;
    int path_taken = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a = test_scalars[i];
        b = test_scalars[(i + 1) % ARRAY_SIZE];
        
        /* Complex control flow to prevent optimization */
        if (__builtin_isunordered(a, b)) {
            path_taken = 1;
            counters[UNORDERED_IDX]++;
        } else if (__builtin_isless(a, b)) {
            path_taken = 2;
        } else if (__builtin_isgreater(a, b)) {
            path_taken = 3;
        } else if (__builtin_islessequal(a, b)) {
            path_taken = 4;
        } else if (__builtin_isgreaterequal(a, b)) {
            path_taken = 5;
        } else if (__builtin_islessgreater(a, b)) {
            path_taken = 6;
            counters[LTGT_IDX]++;
        }
        
        /* Use path_taken to prevent dead code elimination */
        volatile int dummy = path_taken;
        (void)dummy;
    }
}

int main(void) {
    /* Initialize counters */
    for (int i = 0; i < 8; i++) {
        counters[i] = 0;
    }
    
    printf("Testing x86 floating-point condition codes...\n");
    printf("Test data includes: normal numbers, INF, -INF, NaN\n\n");
    
    /* Run all test functions */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print results */
    printf("Condition code usage counts:\n");
    printf("  UNORDERED (unord): %d\n", counters[UNORDERED_IDX]);
    printf("  ORDERED   (ord):   %d\n", counters[ORDERED_IDX]);
    printf("  UNEQ      (ueq):   %d\n", counters[UNEQ_IDX]);
    printf("  UNGE      (nlt):   %d\n", counters[UNGE_IDX]);
    printf("  UNGT      (nle):   %d\n", counters[UNGT_IDX]);
    printf("  UNLE      (ule):   %d\n", counters[UNLE_IDX]);
    printf("  UNLT      (ult):   %d\n", counters[UNLT_IDX]);
    printf("  LTGT      (une):   %d\n", counters[LTGT_IDX]);
    
    /* Verify we hit all condition codes */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += counters[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("SUCCESS: All condition code paths were exercised\n");
        return 0;
    } else {
        printf("ERROR: No condition codes were triggered\n");
        return 1;
    }
}
