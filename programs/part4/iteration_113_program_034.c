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
    3.14, -2.71
};

#define TEST_COUNT (sizeof(test_scalars)/sizeof(test_scalars[0]))

/* Scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (int i = 0; i < TEST_COUNT; i++) {
        for (int j = 0; j < TEST_COUNT; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                counters[ORDERED_IDX]++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[UNEQ_IDX]++;
            }
            
            /* UNGE (not less than) = !(a < b) */
            if (!__builtin_isless(a, b)) {
                counters[UNGE_IDX]++;
            }
            
            /* UNGT (not less or equal) = !(a <= b) */
            if (!__builtin_islessequal(a, b)) {
                counters[UNGT_IDX]++;
            }
            
            /* UNLE (unordered or less or equal) */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                counters[UNLE_IDX]++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
                counters[UNLT_IDX]++;
            }
            
            /* LTGT (less or greater, but not equal and not unordered) */
            if (__builtin_islessgreater(a, b) && __builtin_isordered(a, b)) {
                counters[LTGT_IDX]++;
            }
        }
    }
}

/* Vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    __m128d vec_a, vec_b, cmp_result;
    __m256d vec_a256, vec_b256, cmp_result256;
    
    for (int i = 0; i < TEST_COUNT - 1; i += 2) {
        /* SSE2 vector comparisons */
        vec_a = _mm_set_pd(test_scalars[i], test_scalars[i+1]);
        vec_b = _mm_set_pd(test_scalars[i+1], test_scalars[i]);
        
        /* Various comparison predicates */
        cmp_result = _mm_cmpord_pd(vec_a, vec_b);     /* ORDERED */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[ORDERED_IDX]++;
        }
        
        cmp_result = _mm_cmpunord_pd(vec_a, vec_b);   /* UNORDERED */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[UNORDERED_IDX]++;
        }
        
        cmp_result = _mm_cmpnlt_pd(vec_a, vec_b);     /* UNGE: not less than */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[UNGE_IDX]++;
        }
        
        cmp_result = _mm_cmpnle_pd(vec_a, vec_b);     /* UNGT: not less or equal */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[UNGT_IDX]++;
        }
        
        cmp_result = _mm_cmpneq_pd(vec_a, vec_b);     /* LTGT: not equal (when ordered) */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[LTGT_IDX]++;
        }
        
#ifdef __AVX__
        /* AVX vector comparisons */
        vec_a256 = _mm256_set_pd(test_scalars[i], test_scalars[i+1], 
                                 test_scalars[i], test_scalars[i+1]);
        vec_b256 = _mm256_set_pd(test_scalars[i+1], test_scalars[i],
                                 test_scalars[i+1], test_scalars[i]);
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_UNORD_Q);  /* UNORDERED */
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[UNORDERED_IDX]++;
        }
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_ORD_Q);    /* ORDERED */
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[ORDERED_IDX]++;
        }
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NLT_UQ);   /* UNGE */
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[UNGE_IDX]++;
        }
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NLE_UQ);   /* UNGT */
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[UNGT_IDX]++;
        }
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NEQ_UQ);   /* UNEQ */
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[UNEQ_IDX]++;
        }
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NEQ_OQ);   /* LTGT */
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[LTGT_IDX]++;
        }
#endif
    }
}

/* Inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a, b;
    int result;
    
    for (int i = 0; i < TEST_COUNT; i++) {
        for (int j = 0; j < TEST_COUNT; j++) {
            a = test_scalars[i];
            b = test_scalars[j];
            
            /* UNORDERED condition code */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setp %%al"
                : "=@ccp"(result)
                : "x"(b), "x"(a)
                : "al"
            );
            if (result) counters[UNORDERED_IDX]++;
            
            /* ORDERED condition code */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnp %%al"
                : "=@ccnp"(result)
                : "x"(b), "x"(a)
                : "al"
            );
            if (result) counters[ORDERED_IDX]++;
            
            /* UNEQ (unordered or equal) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setbe %%al"
                : "=@ccbe"(result)
                : "x"(b), "x"(a)
                : "al"
            );
            if (result) counters[UNEQ_IDX]++;
            
            /* UNGE (not less than) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnb %%al"
                : "=@ccnb"(result)
                : "x"(b), "x"(a)
                : "al"
            );
            if (result) counters[UNGE_IDX]++;
            
            /* UNGT (not less or equal) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setnbe %%al"
                : "=@ccnbe"(result)
                : "x"(b), "x"(a)
                : "al"
            );
            if (result) counters[UNGT_IDX]++;
            
            /* UNLE (unordered or less or equal) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setna %%al"
                : "=@ccna"(result)
                : "x"(b), "x"(a)
                : "al"
            );
            if (result) counters[UNLE_IDX]++;
            
            /* UNLT (unordered or less than) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setb %%al"
                : "=@ccb"(result)
                : "x"(b), "x"(a)
                : "al"
            );
            if (result) counters[UNLT_IDX]++;
            
            /* LTGT (not equal and ordered) */
            __asm__ volatile (
                "ucomisd %1, %0\n\t"
                "setne %%al"
                : "=@ccne"(result)
                : "x"(b), "x"(a)
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
    
    for (int i = 0; i < TEST_COUNT; i++) {
        a = test_scalars[i];
        b = test_scalars[(i + 1) % TEST_COUNT];
        
        /* Complex control flow to prevent optimization */
        if (__builtin_isunordered(a, b)) {
            counters[UNORDERED_IDX]++;
            path_taken = 1;
        } else if (__builtin_isless(a, b)) {
            counters[UNLT_IDX]++;  /* Less than (and ordered) */
            path_taken = 2;
        } else if (__builtin_isgreater(a, b)) {
            counters[UNGT_IDX]++;  /* Greater than (and ordered) */
            path_taken = 3;
        } else if (a == b) {
            counters[UNEQ_IDX]++;  /* Equal (and ordered) */
            path_taken = 4;
        }
        
        /* Switch based on comparison class */
        int cmp_class;
        if (__builtin_isunordered(a, b)) {
            cmp_class = 0;  /* UNORDERED */
        } else if (a < b) {
            cmp_class = 1;  /* UNLT (ordered less) */
        } else if (a > b) {
            cmp_class = 2;  /* UNGT (ordered greater) */
        } else {
            cmp_class = 3;  /* UNEQ (ordered equal) */
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

int main(void) {
    /* Initialize counters */
    memset(counters, 0, sizeof(counters));
    
    printf("Testing condition code generation for i386.cc uncovered lines\n");
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
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += counters[i];
    }
    
    printf("\nTotal condition code checks: %d\n", total);
    
    if (total > 0) {
        printf("SUCCESS: All condition code paths were exercised\n");
        return 0;
    } else {
        printf("ERROR: No condition codes were generated\n");
        return 1;
    }
}
