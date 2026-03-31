#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Condition code counters */
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

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    1.0/0.0,          /* +Inf */
    -1.0/0.0,         /* -Inf */
    __builtin_nan(""), /* NaN */
    0.0/0.0           /* Another NaN */
};

static __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(-1.0, 0.0),
    _mm_set_pd(__builtin_inf(), -__builtin_inf()),
    _mm_set_pd(__builtin_nan(""), 3.14)
};

#ifdef __AVX__
static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, 3.0, 4.0),
    _mm256_set_pd(-1.0, 0.0, __builtin_nan(""), __builtin_inf())
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
            
            /* UNORDERED: a or b is NaN */
            if (__builtin_isunordered(a, b)) {
                counters[CC_UNORDERED]++;
            }
            
            /* ORDERED: neither is NaN */
            if (__builtin_isordered(a, b)) {
                counters[CC_ORDERED]++;
            }
            
            /* UNEQ: unordered or equal */
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                counters[CC_UNEQ]++;
            }
            
            /* UNGE: unordered or greater or equal */
            if (!__builtin_isless(a, b)) {
                counters[CC_UNGE]++;
            }
            
            /* UNGT: unordered or greater */
            if (!__builtin_islessequal(a, b)) {
                counters[CC_UNGT]++;
            }
            
            /* UNLE: unordered or less or equal */
            if (!__builtin_isgreater(a, b)) {
                counters[CC_UNLE]++;
            }
            
            /* UNLT: unordered or less */
            if (!__builtin_isgreaterequal(a, b)) {
                counters[CC_UNLT]++;
            }
            
            /* LTGT: less or greater (ordered and not equal) */
            if (__builtin_islessgreater(a, b)) {
                counters[CC_LTGT]++;
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
                counters[CC_ORDERED]++;
            }
            
            cmp_result = _mm_cmpunord_pd(a, b);    /* UNORDERED */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CC_UNORDERED]++;
            }
            
            cmp_result = _mm_cmpnge_pd(a, b);      /* UNGE: not greater or equal */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CC_UNGE]++;
            }
            
            cmp_result = _mm_cmpngt_pd(a, b);      /* UNGT: not greater */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CC_UNGT]++;
            }
            
            cmp_result = _mm_cmpnle_pd(a, b);      /* UNLE: not less or equal */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CC_UNLE]++;
            }
            
            cmp_result = _mm_cmpnlt_pd(a, b);      /* UNLT: not less */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CC_UNLT]++;
            }
            
            cmp_result = _mm_cmpneq_pd(a, b);      /* UNEQ: not equal */
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CC_UNEQ]++;
            }
        }
    }
}

/* Test inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    int result;
    
    /* UNORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[CC_UNORDERED]++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al"
    );
    if (result) counters[CC_ORDERED]++;
    
    /* UNEQ constraint via cmpsd */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movmskpd %1, %0"
        : "=r" (result)
        : "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b)), "i" (_CMP_EQ_UQ)
        : "cc"
    );
    if (result) counters[CC_UNEQ]++;
    
    /* UNGE constraint (nlt) */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movmskpd %1, %0"
        : "=r" (result)
        : "x" (_mm_set1_pd(3.0)), "x" (_mm_set1_pd(2.0)), "i" (_CMP_NLT_UQ)
        : "cc"
    );
    if (result) counters[CC_UNGE]++;
    
    /* UNGT constraint (nle) */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movmskpd %1, %0"
        : "=r" (result)
        : "x" (_mm_set1_pd(3.0)), "x" (_mm_set1_pd(2.0)), "i" (_CMP_NLE_UQ)
        : "cc"
    );
    if (result) counters[CC_UNGT]++;
    
    /* UNLE constraint */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movmskpd %1, %0"
        : "=r" (result)
        : "x" (_mm_set1_pd(1.0)), "x" (_mm_set1_pd(2.0)), "i" (_CMP_LE_UQ)
        : "cc"
    );
    if (result) counters[CC_UNLE]++;
    
    /* UNLT constraint */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movmskpd %1, %0"
        : "=r" (result)
        : "x" (_mm_set1_pd(1.0)), "x" (_mm_set1_pd(2.0)), "i" (_CMP_LT_UQ)
        : "cc"
    );
    if (result) counters[CC_UNLT]++;
    
    /* LTGT constraint (une) */
    __asm__ volatile (
        "cmpsd %2, %1, %3\n\t"
        "movmskpd %1, %0"
        : "=r" (result)
        : "x" (_mm_set1_pd(1.0)), "x" (_mm_set1_pd(2.0)), "i" (_CMP_NEQ_UQ)
        : "cc"
    );
    if (result) counters[CC_LTGT]++;
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
            cmp_result = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CC_UNORDERED]++;
            }
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CC_ORDERED]++;
            }
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NGE_UQ);
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CC_UNGE]++;
            }
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NGT_UQ);
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CC_UNGT]++;
            }
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CC_UNLE]++;
            }
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CC_UNLT]++;
            }
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CC_UNEQ]++;
            }
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NEQ_OQ);
            if (_mm256_movemask_pd(cmp_result) != 0) {
                counters[CC_LTGT]++;
            }
        }
    }
}
#endif

/* Control flow based on comparison results */
void test_control_flow(void) {
    double values[] = {1.0, __builtin_nan(""), 2.0, 0.0};
    int i;
    
    for (i = 0; i < sizeof(values)/sizeof(values[0]); i++) {
        double a = values[i];
        double b = values[(i + 1) % 4];
        
        /* Complex control flow to prevent optimization */
        if (__builtin_isunordered(a, b)) {
            counters[CC_UNORDERED]++;
            if (!__builtin_isordered(a, b)) {
                counters[CC_ORDERED]--;
            }
        } else if (__builtin_islessgreater(a, b)) {
            counters[CC_LTGT]++;
            if (__builtin_isgreaterequal(a, b)) {
                counters[CC_UNLT]--;
            }
        } else if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
            counters[CC_UNEQ]++;
            switch (i % 3) {
                case 0:
                    counters[CC_UNGE] += __builtin_isless(a, b) ? 0 : 1;
                    break;
                case 1:
                    counters[CC_UNGT] += __builtin_islessequal(a, b) ? 0 : 1;
                    break;
                case 2:
                    counters[CC_UNLE] += __builtin_isgreater(a, b) ? 0 : 1;
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
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX__
    test_avx_conditions();
    printf("AVX extensions enabled\n");
#endif
    
    /* Print results */
    printf("\nCondition code usage summary:\n");
    printf("UNORDERED (unord): %d\n", counters[CC_UNORDERED]);
    printf("ORDERED   (ord):   %d\n", counters[CC_ORDERED]);
    printf("UNEQ      (ueq):   %d\n", counters[CC_UNEQ]);
    printf("UNGE      (nlt):   %d\n", counters[CC_UNGE]);
    printf("UNGT      (nle):   %d\n", counters[CC_UNGT]);
    printf("UNLE      (ule):   %d\n", counters[CC_UNLE]);
    printf("UNLT      (ult):   %d\n", counters[CC_UNLT]);
    printf("LTGT      (une):   %d\n", counters[CC_LTGT]);
    
    /* Verify all condition codes were triggered */
    int all_triggered = 1;
    for (int i = 0; i < 8; i++) {
        if (counters[i] == 0) {
            printf("Warning: Condition code %d was not triggered\n", i);
            all_triggered = 0;
        }
    }
    
    if (all_triggered) {
        printf("\nAll condition code paths were exercised successfully!\n");
    }
    
    return 0;
}
