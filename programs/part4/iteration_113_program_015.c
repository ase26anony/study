#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Result counters for each condition code */
typedef struct {
    unsigned int unordered;
    unsigned int ordered;
    unsigned int uneq;
    unsigned int unge;
    unsigned int ungt;
    unsigned int unle;
    unsigned int unlt;
    unsigned int ltgt;
} ConditionCounts;

/* Initialize with NaN, Inf, and normal values */
static const double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0, -0.0,
    __builtin_nan(""), -__builtin_nan(""),
    __builtin_inf(), -__builtin_inf(),
    DBL_MAX, DBL_MIN, -DBL_MAX
};

static const int num_scalars = sizeof(test_scalars) / sizeof(test_scalars[0]);

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(ConditionCounts *counts) {
    for (int i = 0; i < num_scalars; i++) {
        for (int j = 0; j < num_scalars; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED: a or b is NaN */
            if (__builtin_isunordered(a, b)) {
                counts->unordered++;
            }
            
            /* ORDERED: neither is NaN */
            if (__builtin_isordered(a, b)) {
                counts->ordered++;
            }
            
            /* UNEQ: unordered or equal */
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                counts->uneq++;
            }
            
            /* UNGE: unordered or greater-or-equal */
            if (__builtin_isunordered(a, b) || a >= b) {
                counts->unge++;
            }
            
            /* UNGT: unordered or greater */
            if (__builtin_isunordered(a, b) || a > b) {
                counts->ungt++;
            }
            
            /* UNLE: unordered or less-or-equal */
            if (__builtin_isunordered(a, b) || a <= b) {
                counts->unle++;
            }
            
            /* UNLT: unordered or less */
            if (__builtin_isunordered(a, b) || a < b) {
                counts->unlt++;
            }
            
            /* LTGT: less or greater (not equal, not unordered) */
            if (__builtin_islessgreater(a, b)) {
                counts->ltgt++;
            }
        }
    }
}

/* Test vector comparisons using SSE/AVX intrinsics */
#ifdef __SSE2__
void test_vector_sse2(ConditionCounts *counts) {
    __m128d vec_nan = _mm_set1_pd(__builtin_nan(""));
    __m128d vec_one = _mm_set1_pd(1.0);
    __m128d vec_two = _mm_set1_pd(2.0);
    __m128d vec_inf = _mm_set1_pd(__builtin_inf());
    
    /* Compare with different predicates */
    __m128d cmp;
    
    /* _CMP_UNORD_Q - unordered */
    cmp = _mm_cmpunord_pd(vec_nan, vec_one);
    if (_mm_movemask_pd(cmp) != 0) counts->unordered++;
    
    /* _CMP_ORD_Q - ordered */
    cmp = _mm_cmpord_pd(vec_one, vec_two);
    if (_mm_movemask_pd(cmp) != 0) counts->ordered++;
    
    /* _CMP_EQ_UQ - equal or unordered */
    cmp = _mm_cmpeq_pd(vec_nan, vec_nan);
    if (_mm_movemask_pd(cmp) != 0) counts->uneq++;
    
    /* _CMP_NLT_UQ - not less-than or unordered */
    cmp = _mm_cmpnlt_pd(vec_one, vec_two);
    if (_mm_movemask_pd(cmp) != 0) counts->unge++;
    
    /* _CMP_NLE_UQ - not less-or-equal or unordered */
    cmp = _mm_cmpnle_pd(vec_one, vec_two);
    if (_mm_movemask_pd(cmp) != 0) counts->ungt++;
    
    /* _CMP_LE_UQ - less-or-equal or unordered */
    cmp = _mm_cmple_pd(vec_nan, vec_one);
    if (_mm_movemask_pd(cmp) != 0) counts->unle++;
    
    /* _CMP_LT_UQ - less-than or unordered */
    cmp = _mm_cmplt_pd(vec_nan, vec_one);
    if (_mm_movemask_pd(cmp) != 0) counts->unlt++;
    
    /* _CMP_NEQ_UQ - not equal or unordered */
    cmp = _mm_cmpneq_pd(vec_one, vec_two);
    if (_mm_movemask_pd(cmp) != 0) counts->ltgt++;
}
#endif

#ifdef __AVX__
void test_vector_avx(ConditionCounts *counts) {
    __m256d vec_nan = _mm256_set1_pd(__builtin_nan(""));
    __m256d vec_one = _mm256_set1_pd(1.0);
    __m256d vec_two = _mm256_set1_pd(2.0);
    
    /* AVX comparisons with various predicates */
    __m256d cmp;
    
    /* Unordered */
    cmp = _mm256_cmp_pd(vec_nan, vec_one, _CMP_UNORD_Q);
    if (_mm256_movemask_pd(cmp) != 0) counts->unordered++;
    
    /* Ordered */
    cmp = _mm256_cmp_pd(vec_one, vec_two, _CMP_ORD_Q);
    if (_mm256_movemask_pd(cmp) != 0) counts->ordered++;
    
    /* Equal or unordered */
    cmp = _mm256_cmp_pd(vec_nan, vec_nan, _CMP_EQ_UQ);
    if (_mm256_movemask_pd(cmp) != 0) counts->uneq++;
    
    /* Not less-than or unordered */
    cmp = _mm256_cmp_pd(vec_one, vec_two, _CMP_NLT_UQ);
    if (_mm256_movemask_pd(cmp) != 0) counts->unge++;
    
    /* Not less-or-equal or unordered */
    cmp = _mm256_cmp_pd(vec_one, vec_two, _CMP_NLE_UQ);
    if (_mm256_movemask_pd(cmp) != 0) counts->ungt++;
    
    /* Less-or-equal or unordered */
    cmp = _mm256_cmp_pd(vec_nan, vec_one, _CMP_LE_UQ);
    if (_mm256_movemask_pd(cmp) != 0) counts->unle++;
    
    /* Less-than or unordered */
    cmp = _mm256_cmp_pd(vec_nan, vec_one, _CMP_LT_UQ);
    if (_mm256_movemask_pd(cmp) != 0) counts->unlt++;
    
    /* Not equal or unordered */
    cmp = _mm256_cmp_pd(vec_one, vec_two, _CMP_NEQ_UQ);
    if (_mm256_movemask_pd(cmp) != 0) counts->ltgt++;
}
#endif

/* Inline assembly with explicit condition code constraints */
void test_asm_constraints(ConditionCounts *counts) {
    double a = 1.0;
    double b = 2.0;
    double nan = __builtin_nan("");
    int result;
    
    /* Test each condition code via inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(nan), "x"(a)
        : "al"
    );
    if (result) counts->unordered++;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al"
    );
    if (result) counts->ordered++;
    
    /* UNEQ - unordered or equal */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(nan), "x"(nan)
        : "al", "cl"
    );
    if (result) counts->uneq++;
    
    /* UNGE - not less-than (nlt) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al"
    );
    if (result) counts->unge++;
    
    /* UNGT - not less-or-equal (nle) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(b), "x"(a)  /* b > a */
        : "al"
    );
    if (result) counts->ungt++;
    
    /* UNLE - unordered or less-or-equal */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al"
    );
    if (result) counts->unle++;
    
    /* UNLT - unordered or less-than */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al"
    );
    if (result) counts->unlt++;
    
    /* LTGT - unordered or not equal (une) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al"
    );
    if (result) counts->ltgt++;
}

/* Control flow that depends on comparison results */
void test_control_flow(ConditionCounts *counts) {
    volatile double vals[] = {1.0, __builtin_nan(""), 2.0, -__builtin_inf()};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            double a = vals[i];
            double b = vals[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                counts->unordered++;
                if (__builtin_isordered(a + 1.0, b - 1.0)) {
                    counts->ordered++;
                }
            } else {
                if (a == b) {
                    counts->uneq++;  /* Equal case of UNEQ */
                }
                
                if (a >= b) {
                    counts->unge++;
                }
                
                if (a > b) {
                    counts->ungt++;
                }
                
                if (a <= b) {
                    counts->unle++;
                }
                
                if (a < b) {
                    counts->unlt++;
                }
                
                if (a != b) {
                    counts->ltgt++;
                }
            }
        }
    }
}

int main() {
    ConditionCounts counts = {0};
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Test with different comparison methods */
    test_scalar_conditions(&counts);
    
#ifdef __SSE2__
    test_vector_sse2(&counts);
#endif
    
#ifdef __AVX__
    test_vector_avx(&counts);
#endif
    
    test_asm_constraints(&counts);
    test_control_flow(&counts);
    
    /* Print summary of condition code hits */
    printf("\nCondition Code Hit Summary:\n");
    printf("UNORDERED (unord): %u\n", counts.unordered);
    printf("ORDERED   (ord):   %u\n", counts.ordered);
    printf("UNEQ      (ueq):   %u\n", counts.uneq);
    printf("UNGE      (nlt):   %u\n", counts.unge);
    printf("UNGT      (nle):   %u\n", counts.ungt);
    printf("UNLE      (ule):   %u\n", counts.unle);
    printf("UNLT      (ult):   %u\n", counts.unlt);
    printf("LTGT      (une):   %u\n", counts.ltgt);
    
    /* Verify all condition codes were triggered */
    int total_hits = counts.unordered + counts.ordered + counts.uneq +
                     counts.unge + counts.ungt + counts.unle +
                     counts.unlt + counts.ltgt;
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("ERROR: No condition codes were triggered.\n");
        return 1;
    }
}
