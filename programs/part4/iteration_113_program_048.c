#include <stdio.h>
#include <stdlib.h>
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
    __builtin_nan(""), __builtin_nan("0xdead"),
    DBL_MAX, -DBL_MAX
};

static __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(-1.0, 0.0),
    _mm_set_pd(INFINITY, -INFINITY),
    _mm_set_pd(__builtin_nan(""), 3.14),
    _mm_set_pd(DBL_MAX, -DBL_MAX)
};

#ifdef __AVX__
static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, -1.0, 0.0),
    _mm256_set_pd(INFINITY, -INFINITY, __builtin_nan(""), 3.14),
    _mm256_set_pd(DBL_MAX, -DBL_MAX, 0.0, -0.0)
};
#endif

/* Test function using GCC builtins to generate condition codes */
void test_scalar_conditions(void) {
    int i, j;
    double nan = __builtin_nan("");
    
    for (i = 0; i < sizeof(test_scalars)/sizeof(test_scalars[0]); i++) {
        for (j = 0; j < sizeof(test_scalars)/sizeof(test_scalars[0]); j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
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
            if (a < b || a > b) {
                cc_counts[CC_LTGT]++;
            }
        }
    }
}

/* Test function using vector comparisons */
void test_vector_conditions(void) {
    int i, j;
    
    for (i = 0; i < sizeof(test_vec128)/sizeof(test_vec128[0]); i++) {
        for (j = 0; j < sizeof(test_vec128)/sizeof(test_vec128[0]); j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            
            /* Generate various condition codes through vector comparisons */
            __m128d cmp;
            int mask;
            
            /* UNORDERED: _CMP_UNORD_Q */
            cmp = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNORDERED]++;
            
            /* ORDERED: _CMP_ORD_Q */
            cmp = _mm_cmp_pd(a, b, _CMP_ORD_Q);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_ORDERED]++;
            
            /* UNEQ: _CMP_EQ_UQ */
            cmp = _mm_cmp_pd(a, b, _CMP_EQ_UQ);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNEQ]++;
            
            /* UNGE: _CMP_NLT_UQ */
            cmp = _mm_cmp_pd(a, b, _CMP_NLT_UQ);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNGE]++;
            
            /* UNGT: _CMP_NLE_UQ */
            cmp = _mm_cmp_pd(a, b, _CMP_NLE_UQ);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNGT]++;
            
            /* UNLE: _CMP_LE_UQ */
            cmp = _mm_cmp_pd(a, b, _CMP_LE_UQ);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNLE]++;
            
            /* UNLT: _CMP_LT_UQ */
            cmp = _mm_cmp_pd(a, b, _CMP_LT_UQ);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNLT]++;
            
            /* LTGT: _CMP_NEQ_OQ */
            cmp = _mm_cmp_pd(a, b, _CMP_NEQ_OQ);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_LTGT]++;
        }
    }
}

#ifdef __AVX__
void test_avx_conditions(void) {
    int i, j;
    
    for (i = 0; i < sizeof(test_vec256)/sizeof(test_vec256[0]); i++) {
        for (j = 0; j < sizeof(test_vec256)/sizeof(test_vec256[0]); j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* AVX comparisons with various predicates */
            __m256d cmp;
            int mask;
            
            /* UNORDERED */
            cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNORDERED]++;
            
            /* ORDERED */
            cmp = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_ORDERED]++;
            
            /* UNEQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNEQ]++;
            
            /* UNGE */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNGE]++;
            
            /* UNGT */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNGT]++;
            
            /* UNLE */
            cmp = _mm256_cmp_pd(a, b, _CMP_LE_UQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNLE]++;
            
            /* UNLT */
            cmp = _mm256_cmp_pd(a, b, _CMP_LT_UQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_UNLT]++;
            
            /* LTGT */
            cmp = _mm256_cmp_pd(a, b, _CMP_NEQ_OQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) cc_counts[CC_LTGT]++;
        }
    }
}
#endif

/* Inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    double d = 2.0;
    int result;
    
    /* Force generation of condition code strings in assembly */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al"
    );
    if (result) cc_counts[CC_UNORDERED]++;
    
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(c), "x"(d)
        : "al"
    );
    if (result) cc_counts[CC_ORDERED]++;
    
    /* Using condition code constraints directly */
    __asm__ volatile (
        "ucomisd %1, %2"
        : "=@ueq"(result)
        : "x"(a), "x"(a)
    );
    if (result) cc_counts[CC_UNEQ]++;
    
    __asm__ volatile (
        "ucomisd %1, %2"
        : "=@unord"(result)
        : "x"(a), "x"(b)
    );
    if (result) cc_counts[CC_UNORDERED]++;
    
    __asm__ volatile (
        "ucomisd %1, %2"
        : "=@nlt"(result)
        : "x"(c), "x"(a)
    );
    if (result) cc_counts[CC_UNGE]++;
    
    __asm__ volatile (
        "ucomisd %1, %2"
        : "=@nle"(result)
        : "x"(c), "x"(a)
    );
    if (result) cc_counts[CC_UNGT]++;
    
    __asm__ volatile (
        "ucomisd %1, %2"
        : "=@ule"(result)
        : "x"(a), "x"(c)
    );
    if (result) cc_counts[CC_UNLE]++;
    
    __asm__ volatile (
        "ucomisd %1, %2"
        : "=@ult"(result)
        : "x"(a), "x"(c)
    );
    if (result) cc_counts[CC_UNLT]++;
    
    __asm__ volatile (
        "ucomisd %1, %2"
        : "=@une"(result)
        : "x"(a), "x"(c)
    );
    if (result) cc_counts[CC_LTGT]++;
}

/* Control flow dependent on comparison results */
void test_control_flow(void) {
    double values[] = {1.0, __builtin_nan(""), 2.0, INFINITY, -INFINITY};
    int i, j;
    
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                cc_counts[CC_UNORDERED]++;
                if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                    cc_counts[CC_UNEQ]++;
                }
            } else {
                cc_counts[CC_ORDERED]++;
                if (__builtin_islessgreater(a, b)) {
                    cc_counts[CC_LTGT]++;
                }
            }
            
            /* Switch based on comparison classification */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 0;
            else if (a == b) cmp_class = 1;
            else if (a > b) cmp_class = 2;
            else cmp_class = 3;
            
            switch (cmp_class) {
                case 0: /* UNORDERED */
                    cc_counts[CC_UNORDERED]++;
                    break;
                case 1: /* EQUAL (ordered) */
                    if (!__builtin_isunordered(a, b)) {
                        cc_counts[CC_ORDERED]++;
                    }
                    break;
                case 2: /* GREATER */
                    if (!__builtin_isunordered(a, b)) {
                        cc_counts[CC_UNGT]++;
                        cc_counts[CC_UNGE]++;
                    }
                    break;
                case 3: /* LESS */
                    if (!__builtin_isunordered(a, b)) {
                        cc_counts[CC_UNLT]++;
                        cc_counts[CC_UNLE]++;
                    }
                    break;
            }
        }
    }
}

int main(void) {
    /* Initialize counters */
    for (int i = 0; i < 8; i++) {
        cc_counts[i] = 0;
    }
    
    /* Run all test functions */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX__
    test_avx_conditions();
#endif
    
    /* Print summary of condition code hits */
    const char *cc_names[] = {
        "UNORDERED", "ORDERED", "UNEQ", "UNGE",
        "UNGT", "UNLE", "UNLT", "LTGT"
    };
    
    printf("Condition Code Execution Summary:\n");
    printf("================================\n");
    int total = 0;
    for (int i = 0; i < 8; i++) {
        printf("%-10s: %d\n", cc_names[i], cc_counts[i]);
        total += cc_counts[i];
    }
    printf("================================\n");
    printf("Total comparisons: %d\n", total);
    
    /* Verify all condition codes were exercised */
    int all_exercised = 1;
    for (int i = 0; i < 8; i++) {
        if (cc_counts[i] == 0) {
            printf("WARNING: %s was not exercised!\n", cc_names[i]);
            all_exercised = 0;
        }
    }
    
    if (all_exercised) {
        printf("SUCCESS: All condition codes were exercised.\n");
    } else {
        printf("WARNING: Some condition codes were not exercised.\n");
    }
    
    return all_exercised ? 0 : 1;
}
