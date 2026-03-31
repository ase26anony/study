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
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0, 
    INFINITY, -INFINITY,
    __builtin_nan(""), -__builtin_nan(""),
    DBL_MAX, DBL_MIN
};

#define NUM_SCALARS (sizeof(test_scalars)/sizeof(test_scalars[0]))

/* SSE/AVX vector test data */
static __m128d vec_test[4];
static __m256d vec256_test[2];

/* Initialize test vectors */
void init_test_vectors(void) {
    vec_test[0] = _mm_set_pd(1.0, 2.0);
    vec_test[1] = _mm_set_pd(-1.0, __builtin_nan(""));
    vec_test[2] = _mm_set_pd(INFINITY, -INFINITY);
    vec_test[3] = _mm_set_pd(0.0, -__builtin_nan(""));
    
    vec256_test[0] = _mm256_set_pd(1.0, 2.0, __builtin_nan(""), INFINITY);
    vec256_test[1] = _mm256_set_pd(-1.0, -INFINITY, 0.0, -__builtin_nan(""));
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (int i = 0; i < NUM_SCALARS; i++) {
        for (int j = 0; j < NUM_SCALARS; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED: a or b is NaN */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
            }
            
            /* ORDERED: neither is NaN */
            if (__builtin_isordered(a, b)) {
                counters[ORDERED_IDX]++;
                
                /* UNEQ: a == b or both are NaN (but ORDERED excludes NaN case) */
                if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                    counters[UNEQ_IDX]++;
                }
                
                /* UNGE: a >= b or unordered (but we're in ordered case) */
                if (!__builtin_isless(a, b)) {
                    counters[UNGE_IDX]++;
                }
                
                /* UNGT: a > b or unordered */
                if (__builtin_isgreater(a, b)) {
                    counters[UNGT_IDX]++;
                }
                
                /* UNLE: a <= b or unordered */
                if (!__builtin_isgreater(a, b)) {
                    counters[UNLE_IDX]++;
                }
                
                /* UNLT: a < b or unordered */
                if (__builtin_isless(a, b)) {
                    counters[UNLT_IDX]++;
                }
                
                /* LTGT: a != b and ordered (neither NaN) */
                if (__builtin_islessgreater(a, b)) {
                    counters[LTGT_IDX]++;
                }
            }
        }
    }
}

/* Test vector comparisons with SSE/AVX */
void test_vector_conditions(void) {
    __m128d a, b;
    __m128d cmp_result;
    int mask;
    
    /* Test SSE2 comparisons */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            a = vec_test[i];
            b = vec_test[j];
            
            /* Various comparison predicates */
            cmp_result = _mm_cmpord_pd(a, b);  /* ORDERED */
            mask = _mm_movemask_pd(cmp_result);
            if (mask) counters[ORDERED_IDX] += __builtin_popcount(mask);
            
            cmp_result = _mm_cmpunord_pd(a, b); /* UNORDERED */
            mask = _mm_movemask_pd(cmp_result);
            if (mask) counters[UNORDERED_IDX] += __builtin_popcount(mask);
            
            cmp_result = _mm_cmpeq_pd(a, b);   /* EQ (for UNEQ when combined) */
            mask = _mm_movemask_pd(cmp_result);
            
            cmp_result = _mm_cmpnlt_pd(a, b);  /* NLT (UNGE) */
            mask = _mm_movemask_pd(cmp_result);
            if (mask) counters[UNGE_IDX] += __builtin_popcount(mask);
            
            cmp_result = _mm_cmpnle_pd(a, b);  /* NLE (UNGT) */
            mask = _mm_movemask_pd(cmp_result);
            if (mask) counters[UNGT_IDX] += __builtin_popcount(mask);
            
            cmp_result = _mm_cmple_pd(a, b);   /* LE (UNLE when considering unordered) */
            mask = _mm_movemask_pd(cmp_result);
            
            cmp_result = _mm_cmplt_pd(a, b);   /* LT (UNLT when considering unordered) */
            mask = _mm_movemask_pd(cmp_result);
            if (mask) counters[UNLT_IDX] += __builtin_popcount(mask);
            
            /* For UNEQ: (a == b) || (unordered) */
            __m128d eq = _mm_cmpeq_pd(a, b);
            __m128d unord = _mm_cmpunord_pd(a, b);
            cmp_result = _mm_or_pd(eq, unord);
            mask = _mm_movemask_pd(cmp_result);
            if (mask) counters[UNEQ_IDX] += __builtin_popcount(mask);
        }
    }
    
#ifdef __AVX__
    /* Test AVX comparisons if available */
    __m256d a256, b256;
    __m256d cmp_result256;
    int mask256;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            a256 = vec256_test[i];
            b256 = vec256_test[j];
            
            /* AVX comparison predicates */
            cmp_result256 = _mm256_cmp_pd(a256, b256, _CMP_EQ_OQ);    /* Equal (ordered, quiet) */
            mask256 = _mm256_movemask_pd(cmp_result256);
            
            cmp_result256 = _mm256_cmp_pd(a256, b256, _CMP_UNORD_Q);  /* Unordered */
            mask256 = _mm256_movemask_pd(cmp_result256);
            if (mask256) counters[UNORDERED_IDX] += __builtin_popcount(mask256);
            
            cmp_result256 = _mm256_cmp_pd(a256, b256, _CMP_ORD_Q);    /* Ordered */
            mask256 = _mm256_movemask_pd(cmp_result256);
            if (mask256) counters[ORDERED_IDX] += __builtin_popcount(mask256);
            
            cmp_result256 = _mm256_cmp_pd(a256, b256, _CMP_NLT_UQ);   /* Not less than (unordered) */
            mask256 = _mm256_movemask_pd(cmp_result256);
            if (mask256) counters[UNGE_IDX] += __builtin_popcount(mask256);
            
            cmp_result256 = _mm256_cmp_pd(a256, b256, _CMP_NLE_UQ);   /* Not less than or equal (unordered) */
            mask256 = _mm256_movemask_pd(cmp_result256);
            if (mask256) counters[UNGT_IDX] += __builtin_popcount(mask256);
            
            cmp_result256 = _mm256_cmp_pd(a256, b256, _CMP_LE_OQ);    /* Less than or equal (ordered) */
            mask256 = _mm256_movemask_pd(cmp_result256);
            
            cmp_result256 = _mm256_cmp_pd(a256, b256, _CMP_LT_OQ);    /* Less than (ordered) */
            mask256 = _mm256_movemask_pd(cmp_result256);
            if (mask256) counters[UNLT_IDX] += __builtin_popcount(mask256);
            
            /* UNEQ via OR of EQ and UNORD */
            __m256d eq256 = _mm256_cmp_pd(a256, b256, _CMP_EQ_UQ);    /* Equal (unordered, quiet) */
            __m256d unord256 = _mm256_cmp_pd(a256, b256, _CMP_UNORD_Q);
            cmp_result256 = _mm256_or_pd(eq256, unord256);
            mask256 = _mm256_movemask_pd(cmp_result256);
            if (mask256) counters[UNEQ_IDX] += __builtin_popcount(mask256);
        }
    }
#endif
}

/* Inline assembly with condition code constraints */
void test_asm_constraints(void) {
    double a, b;
    int result;
    
    /* Test various condition codes via inline assembly */
    for (int i = 0; i < NUM_SCALARS; i += 2) {
        a = test_scalars[i];
        b = test_scalars[i + 1];
        
        /* UNORDERED */
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setp %%al\n\t"
            "movzbl %%al, %0"
            : "=r"(result)
            : "x"(a), "x"(b)
            : "al"
        );
        if (result) counters[UNORDERED_IDX]++;
        
        /* ORDERED */
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setnp %%al\n\t"
            "movzbl %%al, %0"
            : "=r"(result)
            : "x"(a), "x"(b)
            : "al"
        );
        if (result) counters[ORDERED_IDX]++;
        
        /* UNEQ (equal or unordered) */
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "sete %%al\n\t"
            "setp %%cl\n\t"
            "orb %%cl, %%al\n\t"
            "movzbl %%al, %0"
            : "=r"(result)
            : "x"(a), "x"(b)
            : "al", "cl"
        );
        if (result) counters[UNEQ_IDX]++;
        
        /* UNGE (not less than) */
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setnb %%al\n\t"
            "movzbl %%al, %0"
            : "=r"(result)
            : "x"(a), "x"(b)
            : "al"
        );
        if (result) counters[UNGE_IDX]++;
        
        /* UNGT (not less than or equal) */
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setnbe %%al\n\t"
            "movzbl %%al, %0"
            : "=r"(result)
            : "x"(a), "x"(b)
            : "al"
        );
        if (result) counters[UNGT_IDX]++;
        
        /* UNLE (less than or equal or unordered) */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"  /* swapped for LE */
            "setnb %%al\n\t"
            "movzbl %%al, %0"
            : "=r"(result)
            : "x"(a), "x"(b)
            : "al"
        );
        if (result) counters[UNLE_IDX]++;
        
        /* UNLT (less than or unordered) */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"  /* swapped for LT */
            "setnbe %%al\n\t"
            "movzbl %%al, %0"
            : "=r"(result)
            : "x"(a), "x"(b)
            : "al"
        );
        if (result) counters[UNLT_IDX]++;
        
        /* LTGT (not equal and ordered) */
        __asm__ volatile (
            "ucomisd %1, %2\n\t"
            "setne %%al\n\t"
            "setnp %%cl\n\t"
            "andb %%cl, %%al\n\t"
            "movzbl %%al, %0"
            : "=r"(result)
            : "x"(a), "x"(b)
            : "al", "cl"
        );
        if (result) counters[LTGT_IDX]++;
    }
}

/* Control flow dependent on comparison results */
void test_control_flow(void) {
    double a, b;
    
    for (int i = 0; i < NUM_SCALARS; i++) {
        for (int j = 0; j < NUM_SCALARS; j++) {
            a = test_scalars[i];
            b = test_scalars[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
                if (__builtin_isless(a, b)) {
                    /* This shouldn't happen for unordered, but creates control flow */
                }
            } else if (__builtin_isless(a, b)) {
                counters[UNLT_IDX]++;
                if (__builtin_isgreater(a, b)) {
                    /* Impossible, but creates control flow */
                }
            } else if (__builtin_isgreater(a, b)) {
                counters[UNGT_IDX]++;
            } else {
                /* a == b (and ordered) */
                counters[UNEQ_IDX]++;
            }
            
            /* Switch-like behavior */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) {
                cmp_class = 0;  /* UNORDERED */
            } else if (__builtin_isless(a, b)) {
                cmp_class = 1;  /* UNLT */
            } else if (__builtin_isgreater(a, b)) {
                cmp_class = 2;  /* UNGT */
            } else {
                cmp_class = 3;  /* UNEQ */
            }
            
            switch (cmp_class) {
                case 0:
                    counters[UNORDERED_IDX]++;
                    break;
                case 1:
                    counters[UNLT_IDX]++;
                    break;
                case 2:
                    counters[UNGT_IDX]++;
                    break;
                case 3:
                    counters[UNEQ_IDX]++;
                    break;
            }
        }
    }
}

int main(void) {
    /* Initialize test data */
    init_test_vectors();
    memset(counters, 0, sizeof(counters));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print results */
    printf("Condition Code Test Results:\n");
    printf("UNORDERED: %d\n", counters[UNORDERED_IDX]);
    printf("ORDERED:   %d\n", counters[ORDERED_IDX]);
    printf("UNEQ:      %d\n", counters[UNEQ_IDX]);
    printf("UNGE:      %d\n", counters[UNGE_IDX]);
    printf("UNGT:      %d\n", counters[UNGT_IDX]);
    printf("UNLE:      %d\n", counters[UNLE_IDX]);
    printf("UNLT:      %d\n", counters[UNLT_IDX]);
    printf("LTGT:      %d\n", counters[LTGT_IDX]);
    
    /* Verify we hit all condition codes */
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += counters[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total);
    
    if (total > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("FAILURE: No condition codes were triggered.\n");
        return 1;
    }
}
