#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Condition code counters for verification */
static int counters[8] = {0};
enum {
    CC_UNORDERED = 0,
    CC_ORDERED,
    CC_UNEQ,
    CC_UNGE,
    CC_UNGT,
    CC_UNLE,
    CC_UNLT,
    CC_LTGT
};

/* Test data with normal numbers, infinities, and NaNs */
static const double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0, -0.0,
    INFINITY, -INFINITY,
    __builtin_nan(""), -__builtin_nan(""),
    DBL_MAX, DBL_MIN, -DBL_MAX
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
    int n = sizeof(test_scalars) / sizeof(test_scalars[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED case */
            if (__builtin_isunordered(a, b)) {
                counters[CC_UNORDERED]++;
                /* Control flow to prevent optimization */
                if (counters[CC_UNORDERED] % 2) {
                    printf("U");
                }
            }
            
            /* ORDERED case */
            if (!__builtin_isunordered(a, b)) {
                counters[CC_ORDERED]++;
            }
            
            /* UNEQ case (unordered or equal) */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[CC_UNEQ]++;
            }
            
            /* UNGE case (not less than) */
            if (!(a < b)) {
                counters[CC_UNGE]++;
            }
            
            /* UNGT case (not less or equal) */
            if (!(a <= b)) {
                counters[CC_UNGT]++;
            }
            
            /* UNLE case (unordered or less or equal) */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters[CC_UNLE]++;
            }
            
            /* UNLT case (unordered or less than) */
            if (__builtin_isunordered(a, b) || a < b) {
                counters[CC_UNLT]++;
            }
            
            /* LTGT case (less or greater) */
            if (a < b || a > b) {
                counters[CC_LTGT]++;
            }
        }
    }
}

/* Test vector comparisons with SSE/AVX */
void test_vector_conditions(void) {
    int n = sizeof(test_vec128) / sizeof(test_vec128[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            
            /* Various comparison predicates */
            __m128d cmp;
            
            /* UNORDERED: _CMP_UNORD_Q */
            cmp = _mm_cmpunord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CC_UNORDERED]++;
            }
            
            /* ORDERED: _CMP_ORD_Q */
            cmp = _mm_cmpord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CC_ORDERED]++;
            }
            
            /* UNEQ: _CMP_EQ_UQ */
            cmp = _mm_cmpeq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CC_UNEQ]++;
            }
            
            /* UNGE: _CMP_NLT_UQ */
            cmp = _mm_cmpnlt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CC_UNGE]++;
            }
            
            /* UNGT: _CMP_NLE_UQ */
            cmp = _mm_cmpnle_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CC_UNGT]++;
            }
            
            /* UNLE: _CMP_LE_OS */
            cmp = _mm_cmple_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CC_UNLE]++;
            }
            
            /* UNLT: _CMP_LT_OS */
            cmp = _mm_cmplt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CC_UNLT]++;
            }
            
            /* LTGT: _CMP_NEQ_OS */
            cmp = _mm_cmpneq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[CC_LTGT]++;
            }
        }
    }
}

#ifdef __AVX__
void test_avx_conditions(void) {
    int n = sizeof(test_vec256) / sizeof(test_vec256[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* AVX comparison with various predicates */
            __m256d cmp;
            
            /* UNORDERED */
            cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CC_UNORDERED]++;
            }
            
            /* ORDERED */
            cmp = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CC_ORDERED]++;
            }
            
            /* UNEQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CC_UNEQ]++;
            }
            
            /* UNGE */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CC_UNGE]++;
            }
            
            /* UNGT */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CC_UNGT]++;
            }
            
            /* UNLE */
            cmp = _mm256_cmp_pd(a, b, _CMP_LE_OS);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CC_UNLE]++;
            }
            
            /* UNLT */
            cmp = _mm256_cmp_pd(a, b, _CMP_LT_OS);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CC_UNLT]++;
            }
            
            /* LTGT */
            cmp = _mm256_cmp_pd(a, b, _CMP_NEQ_OS);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[CC_LTGT]++;
            }
        }
    }
}
#endif

/* Inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
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
    if (result) counters[CC_UNORDERED]++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=@ord" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) counters[CC_ORDERED]++;
    
    /* UNEQ constraint */
    a = 1.0;
    b = 1.0;
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=@ueq" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) counters[CC_UNEQ]++;
    
    /* UNGE constraint (not less than) */
    a = 2.0;
    b = 1.0;
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=@nlt" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) counters[CC_UNGE]++;
    
    /* UNGT constraint (not less or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=@nle" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) counters[CC_UNGT]++;
    
    /* UNLE constraint */
    a = 1.0;
    b = 2.0;
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=@ule" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) counters[CC_UNLE]++;
    
    /* UNLT constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=@ult" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) counters[CC_UNLT]++;
    
    /* LTGT constraint (not equal) */
    a = 1.0;
    b = 2.0;
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=@une" (result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    if (result) counters[CC_LTGT]++;
}

/* Control flow intensive test to prevent optimization */
void test_control_flow(void) {
    double values[] = {1.0, __builtin_nan(""), INFINITY, -INFINITY, 0.0};
    int n = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow based on comparisons */
            if (__builtin_isunordered(a, b)) {
                counters[CC_UNORDERED]++;
                if (a != a) {  /* Always true for NaN */
                    printf(".");  /* Side effect */
                }
            } else if (__builtin_isgreater(a, b)) {
                counters[CC_UNGT]++;  /* Not less or equal */
            } else if (__builtin_isless(a, b)) {
                counters[CC_UNLT]++;
            } else if (__builtin_isgreaterequal(a, b)) {
                counters[CC_UNGE]++;
            } else if (__builtin_islessequal(a, b)) {
                counters[CC_UNLE]++;
            } else if (__builtin_islessgreater(a, b)) {
                counters[CC_LTGT]++;
            } else {
                counters[CC_UNEQ]++;  /* Equal or both NaN */
            }
        }
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Reset counters */
    memset(counters, 0, sizeof(counters));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX__
    test_avx_conditions();
#endif
    
    /* Print summary */
    printf("\n\nCondition Code Summary:\n");
    printf("UNORDERED (unord): %d\n", counters[CC_UNORDERED]);
    printf("ORDERED   (ord):   %d\n", counters[CC_ORDERED]);
    printf("UNEQ      (ueq):   %d\n", counters[CC_UNEQ]);
    printf("UNGE      (nlt):   %d\n", counters[CC_UNGE]);
    printf("UNGT      (nle):   %d\n", counters[CC_UNGT]);
    printf("UNLE      (ule):   %d\n", counters[CC_UNLE]);
    printf("UNLT      (ult):   %d\n", counters[CC_UNLT]);
    printf("LTGT      (une):   %d\n", counters[CC_LTGT]);
    
    /* Verify all condition codes were triggered */
    int total = 0;
    int all_nonzero = 1;
    for (int i = 0; i < 8; i++) {
        total += counters[i];
        if (counters[i] == 0) {
            printf("Warning: Condition code %d was not triggered\n", i);
            all_nonzero = 0;
        }
    }
    
    printf("\nTotal comparisons: %d\n", total);
    printf("All condition codes triggered: %s\n", 
           all_nonzero ? "YES" : "NO");
    
    return all_nonzero ? 0 : 1;
}
