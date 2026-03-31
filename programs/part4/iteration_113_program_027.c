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
    CNT_UNORDERED = 0,
    CNT_ORDERED,
    CNT_UNEQ,
    CNT_UNGE,
    CNT_UNGT,
    CNT_UNLE,
    CNT_UNLT,
    CNT_LTGT
};

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    __builtin_nan(""), -__builtin_nan(""),
    1.0/0.0, -1.0/0.0,  /* +Inf, -Inf */
    DBL_MAX, DBL_MIN
};

static __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(__builtin_nan(""), 3.0),
    _mm_set_pd(4.0, __builtin_nan("")),
    _mm_set_pd(__builtin_nan(""), __builtin_nan("")),
    _mm_set_pd(1.0/0.0, -1.0/0.0)
};

static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, 3.0, 4.0),
    _mm256_set_pd(__builtin_nan(""), 5.0, 6.0, 7.0),
    _mm256_set_pd(8.0, __builtin_nan(""), 9.0, 10.0)
};

/* Test function 1: Scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (size_t i = 0; i < sizeof(test_scalars)/sizeof(test_scalars[0]); i++) {
        for (size_t j = 0; j < sizeof(test_scalars)/sizeof(test_scalars[0]); j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters[CNT_UNORDERED]++;
            }
            
            /* ORDERED */
            if (!__builtin_isunordered(a, b)) {
                counters[CNT_ORDERED]++;
            }
            
            /* UNEQ (unordered or equal) */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[CNT_UNEQ]++;
            }
            
            /* UNGE (not less than) */
            if (!(a < b)) {
                counters[CNT_UNGE]++;
            }
            
            /* UNGT (not less than or equal) */
            if (!(a <= b)) {
                counters[CNT_UNGT]++;
            }
            
            /* UNLE (unordered or less than or equal) */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters[CNT_UNLE]++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || a < b) {
                counters[CNT_UNLT]++;
            }
            
            /* LTGT (less than or greater than, but not equal and not unordered) */
            if (a != b && !__builtin_isunordered(a, b)) {
                counters[CNT_LTGT]++;
            }
        }
    }
}

/* Test function 2: Vector comparisons */
void test_vector_conditions(void) {
    /* Test with SSE2 */
    for (size_t i = 0; i < sizeof(test_vec128)/sizeof(test_vec128[0]); i++) {
        for (size_t j = 0; j < sizeof(test_vec128)/sizeof(test_vec128[0]); j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            
            /* Compare with various predicates */
            __m128d cmp;
            
            /* _CMP_UNORD_Q - unordered */
            cmp = _mm_cmpunord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CNT_UNORDERED]++;
            }
            
            /* _CMP_ORD_Q - ordered */
            cmp = _mm_cmpord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CNT_ORDERED]++;
            }
            
            /* _CMP_EQ_UQ - equal (unordered) */
            cmp = _mm_cmpeq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CNT_UNEQ]++;
            }
            
            /* _CMP_NLT_UQ - not less than (unordered) */
            cmp = _mm_cmpnlt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CNT_UNGE]++;
            }
            
            /* _CMP_NLE_UQ - not less than or equal (unordered) */
            cmp = _mm_cmpnle_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CNT_UNGT]++;
            }
            
            /* _CMP_LE_UQ - less than or equal (unordered) */
            cmp = _mm_cmple_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CNT_UNLE]++;
            }
            
            /* _CMP_LT_UQ - less than (unordered) */
            cmp = _mm_cmplt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CNT_UNLT]++;
            }
            
            /* _CMP_NEQ_UQ - not equal (unordered) */
            cmp = _mm_cmpneq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CNT_LTGT]++;
            }
        }
    }
    
    /* Test with AVX if available */
#ifdef __AVX__
    for (size_t i = 0; i < sizeof(test_vec256)/sizeof(test_vec256[0]); i++) {
        for (size_t j = 0; j < sizeof(test_vec256)/sizeof(test_vec256[0]); j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* AVX comparisons with similar predicates */
            __m256d cmp;
            
            cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CNT_UNORDERED]++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CNT_ORDERED]++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CNT_UNEQ]++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CNT_UNGE]++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CNT_UNGT]++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_LE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CNT_UNLE]++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_LT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CNT_UNLT]++;
            }
            
            cmp = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CNT_LTGT]++;
            }
        }
    }
#endif
}

/* Test function 3: Inline assembly with condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    double d = 2.0;
    int result;
    
    /* Force generation of condition code strings through inline asm */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al"
    );
    if (result) counters[CNT_UNORDERED]++;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(c), "x"(d)
        : "al"
    );
    if (result) counters[CNT_ORDERED]++;
    
    /* UNEQ - unordered or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setpe %%al\n\t"
        "sete %%ah\n\t"
        "orb %%ah, %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(d), "x"(d)  /* equal values */
        : "al", "ah"
    );
    if (result) counters[CNT_UNEQ]++;
    
    /* UNGE - not less than (nlt) */
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(c), "x"(a)  /* 2.0 > 1.0 */
        : "al"
    );
    if (result) counters[CNT_UNGE]++;
    
    /* UNGT - not less than or equal (nle) */
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(c), "x"(a)  /* 2.0 > 1.0 */
        : "al"
    );
    if (result) counters[CNT_UNGT]++;
    
    /* UNLE - unordered or less than or equal (ule) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(c)  /* 1.0 < 2.0 */
        : "al"
    );
    if (result) counters[CNT_UNLE]++;
    
    /* UNLT - unordered or less than (ult) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(c)  /* 1.0 < 2.0 */
        : "al"
    );
    if (result) counters[CNT_UNLT]++;
    
    /* LTGT - less than or greater than (une) */
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(c), "x"(a)  /* 2.0 != 1.0 */
        : "al"
    );
    if (result) counters[CNT_LTGT]++;
}

/* Test function 4: Control flow dependent on comparisons */
void test_control_flow(void) {
    volatile double vals[] = {1.0, __builtin_nan(""), 2.0, 0.0};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            double a = vals[i];
            double b = vals[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                counters[CNT_UNORDERED]++;
                if (!__builtin_isless(a, b)) {
                    counters[CNT_UNGE]++;
                }
            } else {
                counters[CNT_ORDERED]++;
                if (__builtin_isless(a, b)) {
                    counters[CNT_UNLT]++;
                } else if (__builtin_isgreater(a, b)) {
                    counters[CNT_UNGT]++;
                } else {
                    counters[CNT_UNEQ]++;
                }
            }
            
            /* Switch statement based on comparison results */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 1;
            else if (a < b) cmp_class = 2;
            else if (a > b) cmp_class = 3;
            else cmp_class = 4;
            
            switch (cmp_class) {
                case 1: /* UNORDERED */
                    counters[CNT_UNORDERED]++;
                    break;
                case 2: /* UNLT */
                    counters[CNT_UNLT]++;
                    if (a <= b) counters[CNT_UNLE]++;
                    break;
                case 3: /* UNGT */
                    counters[CNT_UNGT]++;
                    if (a >= b) counters[CNT_UNGE]++;
                    break;
                case 4: /* UNEQ */
                    counters[CNT_UNEQ]++;
                    if (a != b) counters[CNT_LTGT]++;  /* Won't execute */
                    break;
            }
        }
    }
}

int main(void) {
    /* Initialize counters */
    memset(counters, 0, sizeof(counters));
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all test functions */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print results */
    printf("\nCondition code usage summary:\n");
    printf("UNORDERED (unord): %d\n", counters[CNT_UNORDERED]);
    printf("ORDERED   (ord):   %d\n", counters[CNT_ORDERED]);
    printf("UNEQ      (ueq):   %d\n", counters[CNT_UNEQ]);
    printf("UNGE      (nlt):   %d\n", counters[CNT_UNGE]);
    printf("UNGT      (nle):   %d\n", counters[CNT_UNGT]);
    printf("UNLE      (ule):   %d\n", counters[CNT_UNLE]);
    printf("UNLT      (ult):   %d\n", counters[CNT_UNLT]);
    printf("LTGT      (une):   %d\n", counters[CNT_LTGT]);
    
    /* Verify we hit all condition codes */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += counters[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("ERROR: No condition codes were triggered.\n");
        return 1;
    }
}
