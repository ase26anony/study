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

/* Test data with normal numbers, infinities, and NaNs */
static const double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0, 
    INFINITY, -INFINITY,
    __builtin_nan(""), -__builtin_nan(""),
    DBL_MAX, DBL_MIN
};

static const __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(-1.0, 0.0),
    _mm_set_pd(INFINITY, -INFINITY),
    _mm_set_pd(__builtin_nan(""), __builtin_nan("")),
    _mm_set_pd(DBL_MAX, DBL_MIN)
};

#ifdef __AVX__
static const __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, 3.0, 4.0),
    _mm256_set_pd(-1.0, 0.0, INFINITY, -INFINITY),
    _mm256_set_pd(__builtin_nan(""), __builtin_nan(""), DBL_MAX, DBL_MIN)
};
#endif

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (size_t i = 0; i < sizeof(test_scalars)/sizeof(test_scalars[0]); i++) {
        for (size_t j = 0; j < sizeof(test_scalars)/sizeof(test_scalars[0]); j++) {
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

/* Test vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    for (size_t i = 0; i < sizeof(test_vec128)/sizeof(test_vec128[0]); i++) {
        for (size_t j = 0; j < sizeof(test_vec128)/sizeof(test_vec128[0]); j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            
            /* Various comparison predicates that map to condition codes */
            __m128d cmp;
            
            /* CMP_UNORD_Q - unordered */
            cmp = _mm_cmpunord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNORDERED_IDX]++;
            }
            
            /* CMP_ORD_Q - ordered */
            cmp = _mm_cmpord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[ORDERED_IDX]++;
            }
            
            /* CMP_EQ_UQ - equal (unordered) */
            cmp = _mm_cmpeq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNEQ_IDX]++;
            }
            
            /* CMP_NLT_UQ - not less than (unordered) */
            cmp = _mm_cmpnlt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNGE_IDX]++;
            }
            
            /* CMP_NLE_UQ - not less than or equal (unordered) */
            cmp = _mm_cmpnle_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNGT_IDX]++;
            }
            
            /* CMP_LE_UQ - less than or equal (unordered) */
            cmp = _mm_cmple_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNLE_IDX]++;
            }
            
            /* CMP_LT_UQ - less than (unordered) */
            cmp = _mm_cmplt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNLT_IDX]++;
            }
            
            /* CMP_NEQ_UQ - not equal (unordered) */
            cmp = _mm_cmpneq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[LTGT_IDX]++;
            }
        }
    }
}

#ifdef __AVX__
void test_avx_conditions(void) {
    for (size_t i = 0; i < sizeof(test_vec256)/sizeof(test_vec256[0]); i++) {
        for (size_t j = 0; j < sizeof(test_vec256)/sizeof(test_vec256[0]); j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* AVX comparison predicates */
            __m256d cmp;
            
            /* CMP_UNORD_Q */
            cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNORDERED_IDX]++;
            }
            
            /* CMP_ORD_Q */
            cmp = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[ORDERED_IDX]++;
            }
            
            /* CMP_EQ_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNEQ_IDX]++;
            }
            
            /* CMP_NLT_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNGE_IDX]++;
            }
            
            /* CMP_NLE_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNGT_IDX]++;
            }
            
            /* CMP_LE_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_LE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNLE_IDX]++;
            }
            
            /* CMP_LT_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_LT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNLT_IDX]++;
            }
            
            /* CMP_NEQ_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[LTGT_IDX]++;
            }
        }
    }
}
#endif

/* Inline assembly with condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = 2.0;
    double nan = __builtin_nan("");
    int result;
    
    /* Test various condition codes via inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=r"(result)
        : "x"(nan), "x"(a)
        : "cc"
    );
    if (result) counters[UNORDERED_IDX]++;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) counters[ORDERED_IDX]++;
    
    /* UNEQ (unordered or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %0\n\t"
        "setp %%al\n\t"
        "or %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(a)  /* equal case */
        : "cc", "al"
    );
    if (result) counters[UNEQ_IDX]++;
    
    /* UNGE (not less than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnb %0"
        : "=@nlt"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) counters[UNGE_IDX]++;
    
    /* UNGT (not less than or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnbe %0"
        : "=@nle"(result)
        : "x"(b), "x"(a)  /* b > a */
        : "cc"
    );
    if (result) counters[UNGT_IDX]++;
    
    /* UNLE (unordered or less than or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setna %0"
        : "=@ule"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) counters[UNLE_IDX]++;
    
    /* UNLT (unordered or less than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=@ult"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) counters[UNLT_IDX]++;
    
    /* LTGT (less than or greater than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %0"
        : "=@une"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) counters[LTGT_IDX]++;
}

/* Control flow dependent on comparison results */
void test_control_flow(void) {
    double values[] = {1.0, 2.0, __builtin_nan(""), INFINITY};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
                if (__builtin_isless(a, b)) {
                    /* This should never execute for NaN */
                    counters[UNLT_IDX]--;
                }
            } else if (__builtin_isless(a, b)) {
                counters[UNLT_IDX]++;
                if (__builtin_isgreater(a, b)) {
                    counters[UNGT_IDX]--;
                }
            } else if (a == b) {
                counters[UNEQ_IDX]++;
            } else if (__builtin_isgreater(a, b)) {
                counters[UNGT_IDX]++;
            }
            
            /* Switch based on comparison classification */
            int cmp_class = -1;
            if (__builtin_isunordered(a, b)) {
                cmp_class = 0;  /* UNORDERED */
            } else if (a < b) {
                cmp_class = 1;  /* UNLT */
            } else if (a > b) {
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
    /* Initialize counters */
    for (int i = 0; i < 8; i++) {
        counters[i] = 0;
    }
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX__
    test_avx_conditions();
#endif
    
    /* Print summary */
    const char *names[] = {
        "UNORDERED", "ORDERED", "UNEQ", "UNGE",
        "UNGT", "UNLE", "UNLT", "LTGT"
    };
    
    printf("Condition Code Execution Summary:\n");
    printf("================================\n");
    int total = 0;
    for (int i = 0; i < 8; i++) {
        printf("%-10s: %d\n", names[i], counters[i]);
        total += counters[i];
    }
    printf("================================\n");
    printf("TOTAL: %d condition code evaluations\n", total);
    
    /* Verify we hit all condition codes */
    int all_hit = 1;
    for (int i = 0; i < 8; i++) {
        if (counters[i] == 0) {
            printf("WARNING: %s was not triggered!\n", names[i]);
            all_hit = 0;
        }
    }
    
    if (all_hit) {
        printf("SUCCESS: All condition codes were exercised\n");
    } else {
        printf("PARTIAL: Some condition codes were not triggered\n");
    }
    
    return all_hit ? 0 : 1;
}
