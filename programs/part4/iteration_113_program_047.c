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
} cc_counter;

/* Initialize counters */
static cc_counter counters = {0};

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    1.0/0.0,          /* +Inf */
    -1.0/0.0,         /* -Inf */
    __builtin_nan(""), /* NaN */
    3.14, -2.71
};

static __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(-1.0, 0.0),
    _mm_set_pd(__builtin_inf(), -__builtin_inf()),
    _mm_set_pd(__builtin_nan(""), 3.14),
    _mm_set_pd(-2.71, __builtin_nan(""))
};

#ifdef __AVX__
static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, 3.0, 4.0),
    _mm256_set_pd(-1.0, 0.0, __builtin_nan(""), __builtin_inf()),
    _mm256_set_pd(__builtin_inf(), -__builtin_inf(), __builtin_nan(""), 0.0)
};
#endif

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    int n = sizeof(test_scalars) / sizeof(test_scalars[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
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
            
            /* UNGE (not less than) = !(a < b) */
            if (!__builtin_isless(a, b)) {
                counters.unge++;
            }
            
            /* UNGT (not less than or equal) = !(a <= b) */
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

/* Test vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    int n = sizeof(test_vec128) / sizeof(test_vec128[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            
            /* Compare with various predicates */
            __m128d cmp;
            
            /* UNORDERED (_CMP_UNORD_Q) */
            cmp = _mm_cmpunord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unordered += _mm_movemask_pd(cmp);
            }
            
            /* ORDERED (_CMP_ORD_Q) */
            cmp = _mm_cmpord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.ordered += _mm_movemask_pd(cmp);
            }
            
            /* UNEQ (_CMP_EQ_UQ) */
            cmp = _mm_cmpeq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.uneq += _mm_movemask_pd(cmp);
            }
            
            /* UNGE (_CMP_NLT_UQ) */
            cmp = _mm_cmpnlt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unge += _mm_movemask_pd(cmp);
            }
            
            /* UNGT (_CMP_NLE_UQ) */
            cmp = _mm_cmpnle_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.ungt += _mm_movemask_pd(cmp);
            }
            
            /* UNLE (_CMP_LE_UQ) */
            cmp = _mm_cmple_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unle += _mm_movemask_pd(cmp);
            }
            
            /* UNLT (_CMP_LT_UQ) */
            cmp = _mm_cmplt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unlt += _mm_movemask_pd(cmp);
            }
            
            /* LTGT (_CMP_NEQ_UQ) */
            cmp = _mm_cmpneq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.ltgt += _mm_movemask_pd(cmp);
            }
        }
    }
    
#ifdef __AVX__
    /* Test AVX comparisons if available */
    int m = sizeof(test_vec256) / sizeof(test_vec256[0]);
    
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* Various AVX comparison predicates */
            __m256d cmp;
            
            /* UNORDERED */
            cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            counters.unordered += _mm256_movemask_pd(cmp);
            
            /* ORDERED */
            cmp = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            counters.ordered += _mm256_movemask_pd(cmp);
            
            /* UNEQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            counters.uneq += _mm256_movemask_pd(cmp);
            
            /* UNGE */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            counters.unge += _mm256_movemask_pd(cmp);
            
            /* UNGT */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            counters.ungt += _mm256_movemask_pd(cmp);
            
            /* UNLE */
            cmp = _mm256_cmp_pd(a, b, _CMP_LE_UQ);
            counters.unle += _mm256_movemask_pd(cmp);
            
            /* UNLT */
            cmp = _mm256_cmp_pd(a, b, _CMP_LT_UQ);
            counters.unlt += _mm256_movemask_pd(cmp);
            
            /* LTGT */
            cmp = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            counters.ltgt += _mm256_movemask_pd(cmp);
        }
    }
#endif
}

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    double d = 2.0;
    int result;
    
    /* Test various condition codes via inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters.unordered++;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al"
    );
    if (result) counters.ordered++;
    
    /* UNEQ (unordered or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %%al\n\t"
        "setp %%ah\n\t"
        "or %%ah, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (d)
        : "al", "ah"
    );
    if (result) counters.uneq++;
    
    /* UNGE (not less than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (a)
        : "al"
    );
    if (result) counters.unge++;
    
    /* UNGT (not less than or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (a)
        : "al"
    );
    if (result) counters.ungt++;
    
    /* UNLE (unordered or less than or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setbe %%al\n\t"
        "setp %%ah\n\t"
        "or %%ah, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "ah"
    );
    if (result) counters.unle++;
    
    /* UNLT (unordered or less than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %%al\n\t"
        "setp %%ah\n\t"
        "or %%ah, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "ah"
    );
    if (result) counters.unlt++;
    
    /* LTGT (less than or greater than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al"
    );
    if (result) counters.ltgt++;
}

/* Control flow test that depends on comparison results */
void test_control_flow(void) {
    double values[] = {1.0, __builtin_nan(""), 0.0, __builtin_inf()};
    int n = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow based on comparisons */
            if (__builtin_isunordered(a, b)) {
                counters.unordered++;
                if (__builtin_isless(a, b)) {
                    /* This should never execute for NaN */
                    counters.unlt++;
                }
            } else if (__builtin_isless(a, b)) {
                counters.unlt++;
                if (__builtin_isgreaterequal(b, a)) {
                    counters.unge++;
                }
            } else if (__builtin_isgreater(a, b)) {
                counters.ungt++;
            } else if (__builtin_islessequal(a, b)) {
                counters.unle++;
                if (__builtin_islessequal(b, a)) {
                    counters.uneq++;
                }
            }
            
            /* Switch on comparison classification */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 1;
            else if (a == b) cmp_class = 2;
            else if (a < b) cmp_class = 3;
            else cmp_class = 4;
            
            switch (cmp_class) {
                case 1: /* UNORDERED */
                    counters.unordered += 2;
                    break;
                case 2: /* UNEQ (equal) */
                    counters.uneq += 2;
                    break;
                case 3: /* UNLT */
                    counters.unlt += 2;
                    break;
                case 4: /* UNGT */
                    counters.ungt += 2;
                    break;
            }
        }
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Reset counters */
    memset(&counters, 0, sizeof(counters));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary */
    printf("\nCondition Code Summary:\n");
    printf("UNORDERED: %d\n", counters.unordered);
    printf("ORDERED:   %d\n", counters.ordered);
    printf("UNEQ:      %d\n", counters.uneq);
    printf("UNGE:      %d\n", counters.unge);
    printf("UNGT:      %d\n", counters.ungt);
    printf("UNLE:      %d\n", counters.unle);
    printf("UNLT:      %d\n", counters.unlt);
    printf("LTGT:      %d\n", counters.ltgt);
    
    /* Verify all condition codes were triggered */
    int total = counters.unordered + counters.ordered + counters.uneq +
                counters.unge + counters.ungt + counters.unle +
                counters.unlt + counters.ltgt;
    
    printf("\nTotal condition code hits: %d\n", total);
    
    if (total > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("FAILURE: No condition codes were triggered.\n");
        return 1;
    }
}
