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
#define TEST_SIZE 8
static double scalars[TEST_SIZE];
static __m128d vectors[TEST_SIZE];
static __m256d vectors256[TEST_SIZE/2];

/* Initialize test data with normal values, infinities, and NaNs */
void init_test_data(void) {
    scalars[0] = 1.0;
    scalars[1] = 2.0;
    scalars[2] = -1.0;
    scalars[3] = 0.0;
    scalars[4] = __builtin_nan("");
    scalars[5] = INFINITY;
    scalars[6] = -INFINITY;
    scalars[7] = DBL_MAX;
    
    for (int i = 0; i < TEST_SIZE; i++) {
        vectors[i] = _mm_set_pd(scalars[i], scalars[(i+1)%TEST_SIZE]);
    }
    
    for (int i = 0; i < TEST_SIZE/2; i++) {
        vectors256[i] = _mm256_set_pd(
            scalars[i*2],
            scalars[i*2+1],
            scalars[(i*2+2)%TEST_SIZE],
            scalars[(i*2+3)%TEST_SIZE]
        );
    }
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = scalars[i];
            double b = scalars[j];
            
            /* UNORDERED: a or b is NaN */
            if (__builtin_isunordered(a, b)) {
                counters[CNT_UNORDERED]++;
            }
            
            /* ORDERED: neither a nor b is NaN */
            if (__builtin_isordered(a, b)) {
                counters[CNT_ORDERED]++;
            }
            
            /* UNEQ: unordered or equal */
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                counters[CNT_UNEQ]++;
            }
            
            /* UNGE: unordered or greater-or-equal */
            if (__builtin_isgreaterequal(a, b) || __builtin_isunordered(a, b)) {
                counters[CNT_UNGE]++;
            }
            
            /* UNGT: unordered or greater */
            if (__builtin_isgreater(a, b) || __builtin_isunordered(a, b)) {
                counters[CNT_UNGT]++;
            }
            
            /* UNLE: unordered or less-or-equal */
            if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
                counters[CNT_UNLE]++;
            }
            
            /* UNLT: unordered or less */
            if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
                counters[CNT_UNLT]++;
            }
            
            /* LTGT: less or greater (ordered, not equal) */
            if (__builtin_islessgreater(a, b)) {
                counters[CNT_LTGT]++;
            }
        }
    }
}

/* Test vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    __m128d zero = _mm_set1_pd(0.0);
    __m128d nan_vec = _mm_set1_pd(__builtin_nan(""));
    
    for (int i = 0; i < TEST_SIZE; i++) {
        /* SSE comparisons */
        __m128d cmp;
        int mask;
        
        /* Compare for unordered */
        cmp = _mm_cmpunord_pd(vectors[i], zero);
        mask = _mm_movemask_pd(cmp);
        if (mask) counters[CNT_UNORDERED] += __builtin_popcount(mask);
        
        /* Compare for ordered */
        cmp = _mm_cmpord_pd(vectors[i], zero);
        mask = _mm_movemask_pd(cmp);
        if (mask) counters[CNT_ORDERED] += __builtin_popcount(mask);
        
        /* Compare for not-less-than (UNGE) */
        cmp = _mm_cmpnlt_pd(vectors[i], zero);
        mask = _mm_movemask_pd(cmp);
        if (mask) counters[CNT_UNGE] += __builtin_popcount(mask);
        
        /* Compare for not-less-or-equal (UNGT) */
        cmp = _mm_cmpnle_pd(vectors[i], zero);
        mask = _mm_movemask_pd(cmp);
        if (mask) counters[CNT_UNGT] += __builtin_popcount(mask);
        
        /* Compare unordered or less-or-equal */
        __m128d unord = _mm_cmpunord_pd(vectors[i], zero);
        __m128d le = _mm_cmple_pd(vectors[i], zero);
        cmp = _mm_or_pd(unord, le);
        mask = _mm_movemask_pd(cmp);
        if (mask) counters[CNT_UNLE] += __builtin_popcount(mask);
        
        /* Compare unordered or less-than */
        __m128d lt = _mm_cmplt_pd(vectors[i], zero);
        cmp = _mm_or_pd(unord, lt);
        mask = _mm_movemask_pd(cmp);
        if (mask) counters[CNT_UNLT] += __builtin_popcount(mask);
        
        /* Compare not-equal (LTGT for ordered comparisons) */
        cmp = _mm_cmpneq_pd(vectors[i], zero);
        mask = _mm_movemask_pd(cmp);
        if (mask) counters[CNT_LTGT] += __builtin_popcount(mask);
    }
    
#ifdef __AVX__
    /* AVX comparisons */
    __m256d zero256 = _mm256_set1_pd(0.0);
    
    for (int i = 0; i < TEST_SIZE/2; i++) {
        __m256d cmp256;
        int mask256;
        
        /* AVX unordered comparison */
        cmp256 = _mm256_cmp_pd(vectors256[i], zero256, _CMP_UNORD_Q);
        mask256 = _mm256_movemask_pd(cmp256);
        if (mask256) counters[CNT_UNORDERED] += __builtin_popcount(mask256);
        
        /* AVX ordered comparison */
        cmp256 = _mm256_cmp_pd(vectors256[i], zero256, _CMP_ORD_Q);
        mask256 = _mm256_movemask_pd(cmp256);
        if (mask256) counters[CNT_ORDERED] += __builtin_popcount(mask256);
        
        /* AVX not-less-than (UNGE) */
        cmp256 = _mm256_cmp_pd(vectors256[i], zero256, _CMP_NLT_UQ);
        mask256 = _mm256_movemask_pd(cmp256);
        if (mask256) counters[CNT_UNGE] += __builtin_popcount(mask256);
        
        /* AVX not-less-or-equal (UNGT) */
        cmp256 = _mm256_cmp_pd(vectors256[i], zero256, _CMP_NLE_UQ);
        mask256 = _mm256_movemask_pd(cmp256);
        if (mask256) counters[CNT_UNGT] += __builtin_popcount(mask256);
    }
#endif
}

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    int result;
    
    /* Force generation of condition code strings via inline assembly */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al"
    );
    if (result) counters[CNT_UNORDERED]++;
    
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(c)
        : "al"
    );
    if (result) counters[CNT_ORDERED]++;
    
    /* Test with explicit condition code constraints */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        : "=@ueq"(result)
        : "x"(a), "x"(a)
    );
    if (result) counters[CNT_UNEQ]++;
    
    /* More assembly tests with different condition codes */
    double d = 3.0;
    double e = 2.0;
    
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setae %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(d), "x"(e)
        : "al"
    );
    if (result) counters[CNT_UNGE]++;
    
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(d), "x"(e)
        : "al"
    );
    if (result) counters[CNT_UNGT]++;
}

/* Control flow dependent on comparison results */
void test_control_flow(void) {
    double test_vals[] = {1.0, __builtin_nan(""), INFINITY, -INFINITY};
    int n = sizeof(test_vals)/sizeof(test_vals[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = test_vals[i];
            double b = test_vals[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                counters[CNT_UNORDERED]++;
                for (int k = 0; k < 3; k++) {
                    if (__builtin_isordered(a + k, b - k)) {
                        counters[CNT_ORDERED]++;
                    }
                }
            } else if (__builtin_islessgreater(a, b)) {
                counters[CNT_LTGT]++;
                switch ((int)(a - b)) {
                    case 0:
                        counters[CNT_UNEQ]++;
                        break;
                    default:
                        if (__builtin_isgreaterequal(a, b)) {
                            counters[CNT_UNGE]++;
                        }
                        if (__builtin_islessequal(a, b)) {
                            counters[CNT_UNLE]++;
                        }
                }
            }
            
            /* Nested comparisons */
            while (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
                counters[CNT_UNLT]++;
                a += 1.0;
                if (a >= b && !__builtin_isunordered(a, b)) {
                    counters[CNT_UNGE]++;
                    break;
                }
            }
        }
    }
}

int main(void) {
    /* Initialize test data */
    init_test_data();
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary of condition code hits */
    printf("Condition code execution summary:\n");
    printf("UNORDERED: %d\n", counters[CNT_UNORDERED]);
    printf("ORDERED: %d\n", counters[CNT_ORDERED]);
    printf("UNEQ: %d\n", counters[CNT_UNEQ]);
    printf("UNGE: %d\n", counters[CNT_UNGE]);
    printf("UNGT: %d\n", counters[CNT_UNGT]);
    printf("UNLE: %d\n", counters[CNT_UNLE]);
    printf("UNLT: %d\n", counters[CNT_UNLT]);
    printf("LTGT: %d\n", counters[CNT_LTGT]);
    
    /* Verify all condition codes were exercised */
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += counters[i];
    }
    
    if (total > 0) {
        printf("\nAll condition code paths exercised successfully.\n");
        return 0;
    } else {
        printf("\nERROR: No condition code paths were exercised.\n");
        return 1;
    }
}
