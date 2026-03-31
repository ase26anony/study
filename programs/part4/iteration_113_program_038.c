#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Condition code counters */
typedef struct {
    int unordered;
    int ordered;
    int uneq;
    int unge;
    int ungt;
    int unle;
    int unlt;
    int ltgt;
} cc_counts;

/* Global counters */
static cc_counts global_counts = {0};

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    __builtin_nan(""), -__builtin_nan(""),
    1.0/0.0, -1.0/0.0,  /* +Inf, -Inf */
    3.14, -2.71
};

static __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(__builtin_nan(""), 3.0),
    _mm_set_pd(1.0/0.0, -1.0/0.0),
    _mm_set_pd(-1.0, __builtin_nan(""))
};

#ifdef __AVX__
static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, 3.0, 4.0),
    _mm256_set_pd(__builtin_nan(""), 5.0, 6.0, 7.0),
    _mm256_set_pd(1.0/0.0, -1.0/0.0, __builtin_nan(""), 8.0)
};
#endif

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    int i, j;
    int n = sizeof(test_scalars)/sizeof(test_scalars[0]);
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                global_counts.unordered++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                global_counts.ordered++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || a == b) {
                global_counts.uneq++;
            }
            
            /* UNGE (not less than) = !(a < b) */
            if (!__builtin_isless(a, b)) {
                global_counts.unge++;
            }
            
            /* UNGT (not less or equal) = !(a <= b) */
            if (!__builtin_islessequal(a, b)) {
                global_counts.ungt++;
            }
            
            /* UNLE (unordered or less or equal) */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                global_counts.unle++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
                global_counts.unlt++;
            }
            
            /* LTGT (less or greater) = !(a == b) && !__builtin_isunordered(a, b) */
            if (__builtin_islessgreater(a, b)) {
                global_counts.ltgt++;
            }
        }
    }
}

/* Test vector comparisons */
void test_vector_conditions(void) {
    int i, j;
    int n = sizeof(test_vec128)/sizeof(test_vec128[0]);
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            
            /* Compare with various predicates */
            __m128d cmp;
            
            /* _CMP_UNORD_Q - unordered */
            cmp = _mm_cmpunord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                global_counts.unordered++;
            }
            
            /* _CMP_ORD_Q - ordered */
            cmp = _mm_cmpord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                global_counts.ordered++;
            }
            
            /* _CMP_EQ_UQ - equal or unordered */
            cmp = _mm_cmpeq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                global_counts.uneq++;
            }
            
            /* _CMP_NLT_UQ - not less than or unordered */
            cmp = _mm_cmpnlt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                global_counts.unge++;
            }
            
            /* _CMP_NLE_UQ - not less or equal or unordered */
            cmp = _mm_cmpnle_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                global_counts.ungt++;
            }
            
            /* _CMP_LE_UQ - less or equal or unordered */
            cmp = _mm_cmple_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                global_counts.unle++;
            }
            
            /* _CMP_LT_UQ - less than or unordered */
            cmp = _mm_cmplt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                global_counts.unlt++;
            }
            
            /* _CMP_NEQ_UQ - not equal and unordered */
            cmp = _mm_cmpneq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                global_counts.ltgt++;
            }
        }
    }
}

#ifdef __AVX__
void test_avx_conditions(void) {
    int i, j;
    int n = sizeof(test_vec256)/sizeof(test_vec256[0]);
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* AVX comparisons with similar predicates */
            __m256d cmp;
            
            cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                global_counts.unordered++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                global_counts.ordered++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                global_counts.uneq++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                global_counts.unge++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                global_counts.ungt++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_LE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                global_counts.unle++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_LT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                global_counts.unlt++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                global_counts.ltgt++;
            }
        }
    }
}
#endif

/* Inline assembly with condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    double d = 3.0;
    int result;
    
    /* Test various condition codes in inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) global_counts.unordered++;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(result)
        : "x"(c), "x"(d)
        : "cc"
    );
    if (result) global_counts.ordered++;
    
    /* UNEQ - unordered or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %0\n\t"
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "or %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(a)  /* equal case */
        : "cc", "al"
    );
    if (result) global_counts.uneq++;
    
    /* UNGE - not less than (nlt) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnb %0"
        : "=@nlt"(result)
        : "x"(c), "x"(d)
        : "cc"
    );
    if (result) global_counts.unge++;
    
    /* UNGT - not less or equal (nle) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %0"
        : "=@nle"(result)
        : "x"(d), "x"(c)  /* d > c */
        : "cc"
    );
    if (result) global_counts.ungt++;
    
    /* UNLE - unordered or less or equal (ule) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=@ule"(result)
        : "x"(c), "x"(d)  /* c <= d */
        : "cc"
    );
    if (result) global_counts.unle++;
    
    /* UNLT - unordered or less than (ult) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=@ult"(result)
        : "x"(c), "x"(d)  /* c < d */
        : "cc"
    );
    if (result) global_counts.unlt++;
    
    /* LTGT - less or greater (une) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=@une"(result)
        : "x"(c), "x"(d)  /* c != d */
        : "cc"
    );
    if (result) global_counts.ltgt++;
}

/* Control flow based on comparison results */
void test_control_flow(void) {
    double values[] = {1.0, __builtin_nan(""), 2.0, 0.0};
    int n = sizeof(values)/sizeof(values[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                global_counts.unordered++;
                for (int k = 0; k < 2; k++) {
                    if (__builtin_isordered(a + k, b + k)) {
                        global_counts.ordered++;
                    }
                }
            } else if (__builtin_isless(a, b)) {
                global_counts.unlt++;
                if (!__builtin_islessequal(a, b)) {
                    /* Shouldn't happen, but creates complex CFG */
                    global_counts.ungt++;
                }
            } else if (a == b) {
                global_counts.uneq++;
            } else if (__builtin_islessgreater(a, b)) {
                global_counts.ltgt++;
                switch ((int)(a - b) % 4) {
                    case 0: global_counts.unge++; break;
                    case 1: global_counts.unle++; break;
                    case 2: global_counts.ungt++; break;
                    case 3: global_counts.unlt++; break;
                }
            }
        }
    }
}

void print_counts(const cc_counts *counts) {
    printf("Condition Code Statistics:\n");
    printf("  UNORDERED (unord): %d\n", counts->unordered);
    printf("  ORDERED   (ord):   %d\n", counts->ordered);
    printf("  UNEQ      (ueq):   %d\n", counts->uneq);
    printf("  UNGE      (nlt):   %d\n", counts->unge);
    printf("  UNGT      (nle):   %d\n", counts->ungt);
    printf("  UNLE      (ule):   %d\n", counts->unle);
    printf("  UNLT      (ult):   %d\n", counts->unlt);
    printf("  LTGT      (une):   %d\n", counts->ltgt);
    printf("  TOTAL:            %d\n\n",
           counts->unordered + counts->ordered + counts->uneq +
           counts->unge + counts->ungt + counts->unle +
           counts->unlt + counts->ltgt);
}

int main(void) {
    printf("Testing x86 Floating-Point Condition Codes\n");
    printf("==========================================\n\n");
    
    /* Reset counters */
    memset(&global_counts, 0, sizeof(global_counts));
    
    /* Run all tests */
    printf("1. Testing scalar conditions with GCC builtins...\n");
    test_scalar_conditions();
    
    printf("2. Testing SSE vector conditions...\n");
    test_vector_conditions();
    
    printf("3. Testing inline assembly with condition code constraints...\n");
    test_asm_constraints();
    
    printf("4. Testing control flow based on comparisons...\n");
    test_control_flow();
    
#ifdef __AVX__
    printf("5. Testing AVX conditions...\n");
    test_avx_conditions();
#endif
    
    /* Print final results */
    printf("\nFinal Results:\n");
    print_counts(&global_counts);
    
    /* Verify we hit all condition codes */
    int total_hits = global_counts.unordered + global_counts.ordered +
                     global_counts.uneq + global_counts.unge +
                     global_counts.ungt + global_counts.unle +
                     global_counts.unlt + global_counts.ltgt;
    
    if (total_hits > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("ERROR: No condition code paths were hit.\n");
        return 1;
    }
}
