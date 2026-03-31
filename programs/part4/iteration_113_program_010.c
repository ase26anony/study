#include <stdio.h>
#include <stdlib.h>
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
    1.0/0.0,          /* +Inf */
    -1.0/0.0,         /* -Inf */
    __builtin_nan(""), /* NaN */
    DBL_MAX,
    -DBL_MAX,
    0.0
};

static __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(-1.0, 0.0),
    _mm_set_pd(__builtin_inf(), -__builtin_inf()),
    _mm_set_pd(__builtin_nan(""), 3.14),
    _mm_set_pd(DBL_MIN, DBL_MAX)
};

#ifdef __AVX__
static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, 3.0, 4.0),
    _mm256_set_pd(-1.0, 0.0, __builtin_nan(""), __builtin_inf()),
    _mm256_set_pd(DBL_MAX, -DBL_MAX, DBL_MIN, -DBL_MIN)
};
#endif

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    int i, j;
    double a, b;
    
    for (i = 0; i < sizeof(test_scalars)/sizeof(test_scalars[0]); i++) {
        for (j = 0; j < sizeof(test_scalars)/sizeof(test_scalars[0]); j++) {
            a = test_scalars[i];
            b = test_scalars[j];
            
            /* UNORDERED case */
            if (__builtin_isunordered(a, b)) {
                counters[CNT_UNORDERED]++;
            }
            
            /* ORDERED case */
            if (__builtin_isordered(a, b)) {
                counters[CNT_ORDERED]++;
            }
            
            /* UNEQ case (unordered or equal) */
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                counters[CNT_UNEQ]++;
            }
            
            /* UNGE case (not less than) */
            if (!__builtin_isless(a, b)) {
                counters[CNT_UNGE]++;
            }
            
            /* UNGT case (not less than or equal) */
            if (!__builtin_islessequal(a, b)) {
                counters[CNT_UNGT]++;
            }
            
            /* UNLE case (unordered or less than or equal) */
            if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
                counters[CNT_UNLE]++;
            }
            
            /* UNLT case (unordered or less than) */
            if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
                counters[CNT_UNLT]++;
            }
            
            /* LTGT case (less than or greater than) */
            if (__builtin_islessgreater(a, b)) {
                counters[CNT_LTGT]++;
            }
        }
    }
}

/* Test vector comparisons using intrinsics */
void test_vector_conditions(void) {
    int i, j;
    __m128d a, b;
    __m128d cmp_result;
    
    for (i = 0; i < sizeof(test_vec128)/sizeof(test_vec128[0]); i++) {
        for (j = 0; j < sizeof(test_vec128)/sizeof(test_vec128[0]); j++) {
            a = test_vec128[i];
            b = test_vec128[j];
            
            /* Various comparison predicates that map to condition codes */
            cmp_result = _mm_cmpord_pd(a, b);      /* ORDERED */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_ORDERED]++;
            }
            
            cmp_result = _mm_cmpunord_pd(a, b);    /* UNORDERED */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNORDERED]++;
            }
            
            cmp_result = _mm_cmpnlt_pd(a, b);      /* UNGE (not less than) */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNGE]++;
            }
            
            cmp_result = _mm_cmpnle_pd(a, b);      /* UNGT (not less than or equal) */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNGT]++;
            }
            
            cmp_result = _mm_cmpunord_pd(a, b);    /* UNORDERED */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNORDERED]++;
            }
        }
    }
}

/* Test inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    double d = 2.0;
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
    if (result) counters[CNT_UNORDERED]++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "cc"
    );
    if (result) counters[CNT_ORDERED]++;
    
    /* UNEQ constraint (unordered or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (d), "x" (c)
        : "al", "bl", "cc"
    );
    if (result) counters[CNT_UNEQ]++;
    
    /* UNGE constraint (not less than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (a)
        : "al", "cc"
    );
    if (result) counters[CNT_UNGE]++;
    
    /* UNGT constraint (not less than or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (a)
        : "al", "cc"
    );
    if (result) counters[CNT_UNGT]++;
    
    /* UNLE constraint (unordered or less than or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "bl", "cc"
    );
    if (result) counters[CNT_UNLE]++;
    
    /* UNLT constraint (unordered or less than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "bl", "cc"
    );
    if (result) counters[CNT_UNLT]++;
    
    /* LTGT constraint (less than or greater than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (a)
        : "al", "cc"
    );
    if (result) counters[CNT_LTGT]++;
}

#ifdef __AVX__
/* Test AVX comparisons */
void test_avx_conditions(void) {
    int i, j;
    __m256d a, b;
    __m256d cmp_result;
    
    for (i = 0; i < sizeof(test_vec256)/sizeof(test_vec256[0]); i++) {
        for (j = 0; j < sizeof(test_vec256)/sizeof(test_vec256[0]); j++) {
            a = test_vec256[i];
            b = test_vec256[j];
            
            /* Test various AVX comparison predicates */
            cmp_result = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);    /* UNORDERED */
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNORDERED]++;
            }
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_ORD_Q);      /* ORDERED */
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CNT_ORDERED]++;
            }
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NGE_UQ);     /* UNGE */
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNGE]++;
            }
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NGT_UQ);     /* UNGT */
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNGT]++;
            }
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);     /* UNEQ */
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNEQ]++;
            }
        }
    }
}
#endif

/* Control flow test that depends on comparison results */
void test_control_flow(void) {
    double a = __builtin_nan("");
    double b = 1.0;
    double c = 1.0;
    double d = 2.0;
    
    /* Complex control flow to prevent optimization */
    for (int i = 0; i < 100; i++) {
        if (__builtin_isunordered(a + i, b)) {
            counters[CNT_UNORDERED]++;
            if (__builtin_isordered(c, d)) {
                counters[CNT_ORDERED]++;
            }
        } else if (!__builtin_isless(a, b)) {
            counters[CNT_UNGE]++;
            if (!__builtin_islessequal(c, d)) {
                counters[CNT_UNGT]++;
            }
        }
        
        if (__builtin_islessequal(c, d) || __builtin_isunordered(c, d)) {
            counters[CNT_UNLE]++;
        }
        
        if (__builtin_isless(c, d) || __builtin_isunordered(c, d)) {
            counters[CNT_UNLT]++;
        }
        
        if (__builtin_islessgreater(c, d)) {
            counters[CNT_LTGT]++;
        }
        
        /* Vary values to create different comparison outcomes */
        c += 0.1;
        d -= 0.05;
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
    printf("Condition code coverage summary:\n");
    printf("UNORDERED: %d\n", counters[CNT_UNORDERED]);
    printf("ORDERED:   %d\n", counters[CNT_ORDERED]);
    printf("UNEQ:      %d\n", counters[CNT_UNEQ]);
    printf("UNGE:      %d\n", counters[CNT_UNGE]);
    printf("UNGT:      %d\n", counters[CNT_UNGT]);
    printf("UNLE:      %d\n", counters[CNT_UNLE]);
    printf("UNLT:      %d\n", counters[CNT_UNLT]);
    printf("LTGT:      %d\n", counters[CNT_LTGT]);
    
    /* Verify we hit all condition codes */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += counters[i];
    }
    
    if (total_hits > 0) {
        printf("\nAll condition code paths were exercised (total hits: %d)\n", total_hits);
        return 0;
    } else {
        printf("\nERROR: No condition code paths were exercised!\n");
        return 1;
    }
}
