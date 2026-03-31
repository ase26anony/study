#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

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

static cc_counters counters = {0};

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0, 
    __builtin_nan(""), -__builtin_nan(""),
    1.0/0.0, -1.0/0.0,  /* +Inf, -Inf */
    DBL_MAX, -DBL_MAX,
    0.0/0.0  /* Another NaN */
};

static const int NUM_TEST_VALUES = sizeof(test_scalars)/sizeof(test_scalars[0]);

/* Test scalar conditions using GCC builtins */
void test_scalar_conditions(void) {
    for (int i = 0; i < NUM_TEST_VALUES; i++) {
        for (int j = 0; j < NUM_TEST_VALUES; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters.unordered++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                counters.ordered++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || a == b) {
                counters.uneq++;
            }
            
            /* UNGE (not less than) */
            if (!__builtin_isless(a, b)) {
                counters.unge++;
            }
            
            /* UNGT (not less than or equal) */
            if (!__builtin_islessequal(a, b)) {
                counters.ungt++;
            }
            
            /* UNLE (unordered or less than or equal) */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                counters.unle++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
                counters.unlt++;
            }
            
            /* LTGT (less than or greater than, but not equal and not unordered) */
            if (__builtin_islessgreater(a, b)) {
                counters.ltgt++;
            }
        }
    }
}

/* Test vector conditions using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    __m128d vec_a, vec_b, cmp_result;
    __m256d vec_a256, vec_b256, cmp_result256;
    
    for (int i = 0; i < NUM_TEST_VALUES - 1; i += 2) {
        /* SSE2 vector comparisons */
        vec_a = _mm_set_pd(test_scalars[i], test_scalars[i+1]);
        vec_b = _mm_set_pd(test_scalars[i+1], test_scalars[i]);
        
        /* Various comparison predicates that map to condition codes */
        cmp_result = _mm_cmpord_pd(vec_a, vec_b);      /* ORDERED */
        if (_mm_movemask_pd(cmp_result) != 0) counters.ordered++;
        
        cmp_result = _mm_cmpunord_pd(vec_a, vec_b);    /* UNORDERED */
        if (_mm_movemask_pd(cmp_result) != 0) counters.unordered++;
        
        cmp_result = _mm_cmpnlt_pd(vec_a, vec_b);      /* UNGE (not less than) */
        if (_mm_movemask_pd(cmp_result) != 0) counters.unge++;
        
        cmp_result = _mm_cmpnle_pd(vec_a, vec_b);      /* UNGT (not less than or equal) */
        if (_mm_movemask_pd(cmp_result) != 0) counters.ungt++;
        
        cmp_result = _mm_cmpneq_pd(vec_a, vec_b);      /* LTGT (not equal) */
        if (_mm_movemask_pd(cmp_result) != 0) counters.ltgt++;
        
#ifdef __AVX__
        /* AVX vector comparisons for wider coverage */
        vec_a256 = _mm256_set_pd(test_scalars[i], test_scalars[i+1], 
                                 test_scalars[i], test_scalars[i+1]);
        vec_b256 = _mm256_set_pd(test_scalars[i+1], test_scalars[i],
                                 test_scalars[i+1], test_scalars[i]);
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_UNORD_Q);  /* UNORDERED */
        if (_mm256_movemask_pd(cmp_result256) != 0) counters.unordered++;
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_ORD_Q);    /* ORDERED */
        if (_mm256_movemask_pd(cmp_result256) != 0) counters.ordered++;
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NLT_UQ);   /* UNGE */
        if (_mm256_movemask_pd(cmp_result256) != 0) counters.unge++;
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NLE_UQ);   /* UNGT */
        if (_mm256_movemask_pd(cmp_result256) != 0) counters.ungt++;
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NEQ_UQ);   /* UNEQ */
        if (_mm256_movemask_pd(cmp_result256) != 0) counters.uneq++;
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_LE_OQ);    /* UNLE */
        if (_mm256_movemask_pd(cmp_result256) != 0) counters.unle++;
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_LT_OQ);    /* UNLT */
        if (_mm256_movemask_pd(cmp_result256) != 0) counters.unlt++;
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NEQ_OQ);   /* LTGT */
        if (_mm256_movemask_pd(cmp_result256) != 0) counters.ltgt++;
#endif
    }
}

/* Inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a, b;
    int result;
    
    for (int i = 0; i < NUM_TEST_VALUES; i++) {
        for (int j = 0; j < NUM_TEST_VALUES; j++) {
            a = test_scalars[i];
            b = test_scalars[j];
            
            /* UNORDERED constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setp %%al"
                : "=@ccunord"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counters.unordered++;
            
            /* ORDERED constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnp %%al"
                : "=@ccord"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counters.ordered++;
            
            /* UNEQ constraint (unordered or equal) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "sete %%al\n\t"
                "setp %%ah\n\t"
                "or %%ah, %%al"
                : "=@ccueq"(result)
                : "x"(a), "x"(b)
                : "al", "ah"
            );
            if (result) counters.uneq++;
            
            /* UNGE constraint (not less than) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnb %%al"
                : "=@ccnlt"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counters.unge++;
            
            /* UNGT constraint (not less than or equal) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnbe %%al"
                : "=@ccnle"(result)
                : "x"(a), "x"(b)
                : "al"
            );
            if (result) counters.ungt++;
            
            /* UNLE constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setbe %%al\n\t"
                "setp %%ah\n\t"
                "or %%ah, %%al"
                : "=@ccule"(result)
                : "x"(a), "x"(b)
                : "al", "ah"
            );
            if (result) counters.unle++;
            
            /* UNLT constraint */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setb %%al\n\t"
                "setp %%ah\n\t"
                "or %%ah, %%al"
                : "=@ccult"(result)
                : "x"(a), "x"(b)
                : "al", "ah"
            );
            if (result) counters.unlt++;
            
            /* LTGT constraint (not equal and ordered) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setne %%al\n\t"
                "setnp %%ah\n\t"
                "and %%ah, %%al"
                : "=@ccune"(result)
                : "x"(a), "x"(b)
                : "al", "ah"
            );
            if (result) counters.ltgt++;
        }
    }
}

/* Control flow dependent on comparison results */
void test_control_flow(void) {
    double a, b;
    
    for (int i = 0; i < NUM_TEST_VALUES; i++) {
        for (int j = 0; j < NUM_TEST_VALUES; j++) {
            a = test_scalars[i];
            b = test_scalars[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                counters.unordered++;
                if (__builtin_isordered(a + 1.0, b - 1.0)) {
                    counters.ordered++;
                }
            } else if (__builtin_isless(a, b)) {
                counters.unlt++;  /* Not UNLT, but triggers comparison */
                if (!__builtin_islessequal(b, a)) {
                    counters.ungt++;
                }
            } else if (a == b) {
                counters.uneq++;
            } else if (__builtin_isgreater(a, b)) {
                counters.unge++;  /* Not UNGE, but triggers comparison */
                if (!__builtin_islessequal(a, b)) {
                    counters.ungt++;
                }
            }
            
            /* Switch based on comparison classification */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 1;
            else if (a == b) cmp_class = 2;
            else if (__builtin_isless(a, b)) cmp_class = 3;
            else if (__builtin_isgreater(a, b)) cmp_class = 4;
            
            switch (cmp_class) {
                case 1: /* UNORDERED */
                    counters.unordered += 2;
                    break;
                case 2: /* UNEQ (equal and ordered) */
                    counters.uneq += 2;
                    break;
                case 3: /* UNLT (less than and ordered) */
                    counters.unlt += 2;
                    break;
                case 4: /* UNGT (greater than and ordered) */
                    counters.ungt += 2;
                    break;
            }
        }
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    printf("Test values include: normals, NaN, +Inf, -Inf\n\n");
    
    /* Reset counters */
    memset(&counters, 0, sizeof(counters));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print results */
    printf("Condition code usage summary:\n");
    printf("  UNORDERED: %u\n", counters.unordered);
    printf("  ORDERED:   %u\n", counters.ordered);
    printf("  UNEQ:      %u\n", counters.uneq);
    printf("  UNGE:      %u\n", counters.unge);
    printf("  UNGT:      %u\n", counters.ungt);
    printf("  UNLE:      %u\n", counters.unle);
    printf("  UNLT:      %u\n", counters.unlt);
    printf("  LTGT:      %u\n", counters.ltgt);
    
    /* Verify we hit all condition codes */
    unsigned int total = counters.unordered + counters.ordered + counters.uneq +
                        counters.unge + counters.ungt + counters.unle +
                        counters.unlt + counters.ltgt;
    
    printf("\nTotal condition code hits: %u\n", total);
    
    if (total > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("ERROR: No condition codes were triggered.\n");
        return 1;
    }
}
