#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Condition code counters for verification */
static int cc_counts[8] = {0};
enum CC_TYPE { UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT };

/* Test data with normal values, infinities, and NaNs */
static const double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0, -0.0,
    INFINITY, -INFINITY,
    __builtin_nan(""), -__builtin_nan(""),
    DBL_MAX, DBL_MIN, -DBL_MAX
};

static __m128d test_vecs[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(-1.0, 0.0),
    _mm_set_pd(INFINITY, -INFINITY),
    _mm_set_pd(__builtin_nan(""), __builtin_nan("")),
    _mm_set_pd(DBL_MAX, DBL_MIN)
};

#ifdef __AVX__
static __m256d test_avx_vecs[] = {
    _mm256_set_pd(1.0, 2.0, -1.0, 0.0),
    _mm256_set_pd(INFINITY, -INFINITY, __builtin_nan(""), __builtin_nan(""))
};
#endif

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    int i, j;
    int n = sizeof(test_scalars) / sizeof(test_scalars[0]);
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                cc_counts[UNORDERED]++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                cc_counts[ORDERED]++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || a == b) {
                cc_counts[UNEQ]++;
            }
            
            /* UNGE (not less than) */
            if (!__builtin_isless(a, b)) {
                cc_counts[UNGE]++;
            }
            
            /* UNGT (not less than or equal) */
            if (!__builtin_islessequal(a, b)) {
                cc_counts[UNGT]++;
            }
            
            /* UNLE (unordered or less than or equal) */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                cc_counts[UNLE]++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
                cc_counts[UNLT]++;
            }
            
            /* LTGT (less than or greater than, but not equal) */
            if (__builtin_islessgreater(a, b)) {
                cc_counts[LTGT]++;
            }
        }
    }
}

/* Test vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    int i, j;
    int n = sizeof(test_vecs) / sizeof(test_vecs[0]);
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            __m128d a = test_vecs[i];
            __m128d b = test_vecs[j];
            
            /* Various comparison predicates that map to condition codes */
            __m128d cmp;
            
            /* _CMP_UNORD_Q - unordered */
            cmp = _mm_cmpunord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                cc_counts[UNORDERED]++;
            }
            
            /* _CMP_ORD_Q - ordered */
            cmp = _mm_cmpord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                cc_counts[ORDERED]++;
            }
            
            /* _CMP_EQ_UQ - equal (unordered, non-signaling) */
            cmp = _mm_cmpeq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                cc_counts[UNEQ]++;  /* Maps to UNEQ in some contexts */
            }
            
            /* _CMP_NLT_UQ - not less than (unordered, non-signaling) */
            cmp = _mm_cmpnlt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                cc_counts[UNGE]++;
            }
            
            /* _CMP_NLE_UQ - not less than or equal (unordered, non-signaling) */
            cmp = _mm_cmpnle_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                cc_counts[UNGT]++;
            }
            
            /* _CMP_LE_UQ - less than or equal (unordered, non-signaling) */
            cmp = _mm_cmple_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                cc_counts[UNLE]++;
            }
            
            /* _CMP_LT_UQ - less than (unordered, non-signaling) */
            cmp = _mm_cmplt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                cc_counts[UNLT]++;
            }
            
            /* _CMP_NEQ_UQ - not equal (unordered, non-signaling) */
            cmp = _mm_cmpneq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                cc_counts[LTGT]++;
            }
        }
    }
}

/* Test inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    int result;
    
    /* Force generation of condition code strings via inline asm */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) cc_counts[UNORDERED]++;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) cc_counts[ORDERED]++;
    
    /* UNEQ - unordered or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %0\n\t"
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "or %b0, %%al\n\t"
        "movzx %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc", "al"
    );
    if (result) cc_counts[UNEQ]++;
    
    /* UNGE - not less than (nlt) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnb %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) cc_counts[UNGE]++;
    
    /* UNGT - not less than or equal (nle) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) cc_counts[UNGT]++;
    
    /* UNLE - unordered or less than or equal (ule) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) cc_counts[UNLE]++;
    
    /* UNLT - unordered or less than (ult) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) cc_counts[UNLT]++;
    
    /* LTGT - less than or greater than (une) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) cc_counts[LTGT]++;
}

#ifdef __AVX__
/* Test AVX comparisons for wider code generation paths */
void test_avx_conditions(void) {
    int i, j;
    int n = sizeof(test_avx_vecs) / sizeof(test_avx_vecs[0]);
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            __m256d a = test_avx_vecs[i];
            __m256d b = test_avx_vecs[j];
            
            /* AVX comparison predicates */
            __m256d cmp;
            
            /* _CMP_UNORD_Q */
            cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                cc_counts[UNORDERED]++;
            }
            
            /* _CMP_ORD_Q */
            cmp = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                cc_counts[ORDERED]++;
            }
            
            /* _CMP_EQ_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                cc_counts[UNEQ]++;
            }
            
            /* _CMP_NLT_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                cc_counts[UNGE]++;
            }
            
            /* _CMP_NLE_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                cc_counts[UNGT]++;
            }
            
            /* _CMP_LE_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_LE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                cc_counts[UNLE]++;
            }
            
            /* _CMP_LT_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_LT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                cc_counts[UNLT]++;
            }
            
            /* _CMP_NEQ_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                cc_counts[LTGT]++;
            }
        }
    }
}
#endif

/* Control flow dependent on comparison results */
void test_control_flow(void) {
    double values[] = {1.0, __builtin_nan(""), INFINITY, -INFINITY, 0.0};
    int n = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                cc_counts[UNORDERED]++;
                if (__builtin_isless(a, b)) {
                    /* This should never execute for NaN */
                    cc_counts[UNLT]--;
                }
            } else if (__builtin_isless(a, b)) {
                cc_counts[UNLT]++;
                if (__builtin_isgreater(a, b)) {
                    cc_counts[UNGT]--;
                }
            } else if (__builtin_isgreater(a, b)) {
                cc_counts[UNGT]++;
            } else if (a == b) {
                cc_counts[UNEQ]++;
            }
            
            /* Switch on comparison classification */
            int cmp_class = -1;
            if (__builtin_isunordered(a, b)) {
                cmp_class = UNORDERED;
            } else if (__builtin_isless(a, b)) {
                cmp_class = UNLT;
            } else if (__builtin_isgreater(a, b)) {
                cmp_class = UNGT;
            } else {
                cmp_class = UNEQ;
            }
            
            switch (cmp_class) {
                case UNORDERED:
                    cc_counts[UNORDERED]++;
                    break;
                case UNLT:
                    cc_counts[UNLT]++;
                    break;
                case UNGT:
                    cc_counts[UNGT]++;
                    break;
                case UNEQ:
                    cc_counts[UNEQ]++;
                    break;
            }
        }
    }
}

int main(void) {
    const char *cc_names[] = {
        "UNORDERED", "ORDERED", "UNEQ", "UNGE", 
        "UNGT", "UNLE", "UNLT", "LTGT"
    };
    
    printf("Testing x86 floating-point condition code generation...\n");
    
    /* Run all test functions */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX__
    test_avx_conditions();
    printf("AVX extensions enabled\n");
#endif
    
    /* Print summary of condition code hits */
    printf("\nCondition Code Hit Summary:\n");
    printf("===========================\n");
    
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        printf("%-10s: %d\n", cc_names[i], cc_counts[i]);
        total_hits += cc_counts[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    /* Verify we hit all condition code types */
    int all_hit = 1;
    for (int i = 0; i < 8; i++) {
        if (cc_counts[i] == 0) {
            printf("WARNING: No hits for %s\n", cc_names[i]);
            all_hit = 0;
        }
    }
    
    if (all_hit) {
        printf("\nSUCCESS: All condition code types were exercised!\n");
    } else {
        printf("\nWARNING: Some condition code types were not exercised\n");
    }
    
    return (total_hits > 0) ? 0 : 1;
}
