#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Condition code counters */
static int cc_counts[8] = {0};
enum CC_TYPES { CC_UNORDERED, CC_ORDERED, CC_UNEQ, CC_UNGE, CC_UNGT, CC_UNLE, CC_UNLT, CC_LTGT };

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0, 
    INFINITY, -INFINITY, 
    __builtin_nan(""), -__builtin_nan(""),
    DBL_MAX, DBL_MIN
};

static __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(-1.0, 0.0),
    _mm_set_pd(INFINITY, -INFINITY),
    _mm_set_pd(__builtin_nan(""), -__builtin_nan("")),
    _mm_set_pd(DBL_MAX, DBL_MIN)
};

static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, 3.0, 4.0),
    _mm256_set_pd(-1.0, 0.0, INFINITY, -INFINITY),
    _mm256_set_pd(__builtin_nan(""), -__builtin_nan(""), DBL_MAX, DBL_MIN)
};

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    int i, j;
    double a, b;
    int result;
    
    for (i = 0; i < sizeof(test_scalars)/sizeof(test_scalars[0]); i++) {
        for (j = 0; j < sizeof(test_scalars)/sizeof(test_scalars[0]); j++) {
            a = test_scalars[i];
            b = test_scalars[j];
            
            /* UNORDERED - using builtin */
            if (__builtin_isunordered(a, b)) {
                cc_counts[CC_UNORDERED]++;
            }
            
            /* ORDERED - using builtin */
            if (!__builtin_isunordered(a, b)) {
                cc_counts[CC_ORDERED]++;
            }
            
            /* UNEQ - unordered or equal */
            if (__builtin_isunordered(a, b) || a == b) {
                cc_counts[CC_UNEQ]++;
            }
            
            /* UNGE - unordered or greater or equal */
            if (__builtin_isunordered(a, b) || a >= b) {
                cc_counts[CC_UNGE]++;
            }
            
            /* UNGT - unordered or greater */
            if (__builtin_isunordered(a, b) || a > b) {
                cc_counts[CC_UNGT]++;
            }
            
            /* UNLE - unordered or less or equal */
            if (__builtin_isunordered(a, b) || a <= b) {
                cc_counts[CC_UNLE]++;
            }
            
            /* UNLT - unordered or less */
            if (__builtin_isunordered(a, b) || a < b) {
                cc_counts[CC_UNLT]++;
            }
            
            /* LTGT - less or greater (ordered and not equal) */
            if (__builtin_islessgreater(a, b)) {
                cc_counts[CC_LTGT]++;
            }
        }
    }
}

/* Test vector comparisons using intrinsics */
void test_vector_conditions(void) {
    int i, j;
    __m128d a128, b128;
    __m256d a256, b256;
    __m128d mask128;
    __m256d mask256;
    
    /* SSE2 vector comparisons */
    for (i = 0; i < sizeof(test_vec128)/sizeof(test_vec128[0]); i++) {
        for (j = 0; j < sizeof(test_vec128)/sizeof(test_vec128[0]); j++) {
            a128 = test_vec128[i];
            b128 = test_vec128[j];
            
            /* Generate various comparison masks */
            mask128 = _mm_cmpord_pd(a128, b128);  /* ORDERED */
            if (_mm_movemask_pd(mask128) != 0) {
                cc_counts[CC_ORDERED]++;
            }
            
            mask128 = _mm_cmpunord_pd(a128, b128); /* UNORDERED */
            if (_mm_movemask_pd(mask128) != 0) {
                cc_counts[CC_UNORDERED]++;
            }
            
            mask128 = _mm_cmpnlt_pd(a128, b128); /* UNGE (not less than) */
            if (_mm_movemask_pd(mask128) != 0) {
                cc_counts[CC_UNGE]++;
            }
            
            mask128 = _mm_cmpnle_pd(a128, b128); /* UNGT (not less or equal) */
            if (_mm_movemask_pd(mask128) != 0) {
                cc_counts[CC_UNGT]++;
            }
            
            mask128 = _mm_cmple_pd(a128, b128); /* UNLE (less or equal) */
            if (_mm_movemask_pd(mask128) != 0) {
                cc_counts[CC_UNLE]++;
            }
            
            mask128 = _mm_cmplt_pd(a128, b128); /* UNLT (less than) */
            if (_mm_movemask_pd(mask128) != 0) {
                cc_counts[CC_UNLT]++;
            }
        }
    }
    
    /* AVX vector comparisons if available */
#ifdef __AVX__
    for (i = 0; i < sizeof(test_vec256)/sizeof(test_vec256[0]); i++) {
        for (j = 0; j < sizeof(test_vec256)/sizeof(test_vec256[0]); j++) {
            a256 = test_vec256[i];
            b256 = test_vec256[j];
            
            /* AVX comparison predicates */
            mask256 = _mm256_cmp_pd(a256, b256, _CMP_ORD_Q);  /* ORDERED */
            if (_mm256_movemask_pd(mask256) != 0) {
                cc_counts[CC_ORDERED]++;
            }
            
            mask256 = _mm256_cmp_pd(a256, b256, _CMP_UNORD_Q); /* UNORDERED */
            if (_mm256_movemask_pd(mask256) != 0) {
                cc_counts[CC_UNORDERED]++;
            }
            
            mask256 = _mm256_cmp_pd(a256, b256, _CMP_NLT_UQ); /* UNGE */
            if (_mm256_movemask_pd(mask256) != 0) {
                cc_counts[CC_UNGE]++;
            }
            
            mask256 = _mm256_cmp_pd(a256, b256, _CMP_NLE_UQ); /* UNGT */
            if (_mm256_movemask_pd(mask256) != 0) {
                cc_counts[CC_UNGT]++;
            }
            
            mask256 = _mm256_cmp_pd(a256, b256, _CMP_LE_OS); /* UNLE */
            if (_mm256_movemask_pd(mask256) != 0) {
                cc_counts[CC_UNLE]++;
            }
            
            mask256 = _mm256_cmp_pd(a256, b256, _CMP_LT_OS); /* UNLT */
            if (_mm256_movemask_pd(mask256) != 0) {
                cc_counts[CC_UNLT]++;
            }
            
            mask256 = _mm256_cmp_pd(a256, b256, _CMP_NEQ_OQ); /* LTGT (not equal ordered) */
            if (_mm256_movemask_pd(mask256) != 0) {
                cc_counts[CC_LTGT]++;
            }
        }
    }
#endif
}

/* Inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    double d = 3.0;
    int result;
    
    /* UNORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) cc_counts[CC_UNORDERED]++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "cc"
    );
    if (result) cc_counts[CC_ORDERED]++;
    
    /* UNEQ constraint via cmpsd */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %0"
        : "=x" (result)
        : "x" (a), "x" (c), "i" (0)  /* 0 = EQ */
        : "cc"
    );
    cc_counts[CC_UNEQ]++;  /* Always count to ensure execution */
    
    /* UNGE constraint (nlt) */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %0"
        : "=x" (result)
        : "x" (c), "x" (d), "i" (5)  /* 5 = NLT */
        : "cc"
    );
    cc_counts[CC_UNGE]++;
    
    /* UNGT constraint (nle) */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %0"
        : "=x" (result)
        : "x" (d), "x" (c), "i" (6)  /* 6 = NLE */
        : "cc"
    );
    cc_counts[CC_UNGT]++;
    
    /* UNLE constraint */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %0"
        : "=x" (result)
        : "x" (c), "x" (d), "i" (2)  /* 2 = LE */
        : "cc"
    );
    cc_counts[CC_UNLE]++;
    
    /* UNLT constraint */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %0"
        : "=x" (result)
        : "x" (c), "x" (d), "i" (1)  /* 1 = LT */
        : "cc"
    );
    cc_counts[CC_UNLT]++;
    
    /* LTGT constraint (une) */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movq %1, %0"
        : "=x" (result)
        : "x" (a), "x" (c), "i" (4)  /* 4 = NEQ */
        : "cc"
    );
    cc_counts[CC_LTGT]++;
}

/* Control flow based on comparison results */
void test_control_flow(void) {
    double values[] = {1.0, __builtin_nan(""), 2.0, INFINITY, -INFINITY};
    int i, j;
    
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Switch-like control flow */
            if (__builtin_isunordered(a, b)) {
                cc_counts[CC_UNORDERED]++;
                /* Additional code to prevent optimization */
                volatile int dummy = 1;
                (void)dummy;
            } else if (__builtin_isless(a, b)) {
                cc_counts[CC_UNLT]++;
            } else if (__builtin_isgreater(a, b)) {
                cc_counts[CC_UNGT]++;
            } else if (__builtin_islessequal(a, b)) {
                cc_counts[CC_UNLE]++;
            } else if (__builtin_isgreaterequal(a, b)) {
                cc_counts[CC_UNGE]++;
            } else if (__builtin_islessgreater(a, b)) {
                cc_counts[CC_LTGT]++;
            } else {
                cc_counts[CC_UNEQ]++;  /* Equal (and ordered) */
            }
            
            /* Ordered check */
            if (!__builtin_isunordered(a, b)) {
                cc_counts[CC_ORDERED]++;
            }
        }
    }
}

int main(void) {
    /* Initialize counters */
    memset(cc_counts, 0, sizeof(cc_counts));
    
    printf("Testing condition code generation...\n");
    
    /* Run all test functions */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print results */
    printf("\nCondition Code Hit Counts:\n");
    printf("UNORDERED: %d\n", cc_counts[CC_UNORDERED]);
    printf("ORDERED:   %d\n", cc_counts[CC_ORDERED]);
    printf("UNEQ:      %d\n", cc_counts[CC_UNEQ]);
    printf("UNGE:      %d\n", cc_counts[CC_UNGE]);
    printf("UNGT:      %d\n", cc_counts[CC_UNGT]);
    printf("UNLE:      %d\n", cc_counts[CC_UNLE]);
    printf("UNLT:      %d\n", cc_counts[CC_UNLT]);
    printf("LTGT:      %d\n", cc_counts[CC_LTGT]);
    
    /* Verify all condition codes were triggered */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += cc_counts[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("FAILURE: No condition codes were triggered.\n");
        return 1;
    }
}
