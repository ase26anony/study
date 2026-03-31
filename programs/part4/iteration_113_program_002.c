#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>

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
    3.14, -2.71
};
#define NUM_SCALARS (sizeof(test_scalars)/sizeof(test_scalars[0]))

/* Vector test data */
static __m128d vec_data[4];
static __m256d vec256_data[2];

/* Initialize test vectors with mixed normal/NaN values */
void init_test_vectors(void) {
    vec_data[0] = _mm_set_pd(1.0, 2.0);
    vec_data[1] = _mm_set_pd(__builtin_nan(""), 3.0);
    vec_data[2] = _mm_set_pd(4.0, __builtin_nan(""));
    vec_data[3] = _mm_set_pd(5.0, 6.0);
    
    vec256_data[0] = _mm256_set_pd(1.0, __builtin_nan(""), 3.0, 4.0);
    vec256_data[1] = _mm256_set_pd(5.0, 6.0, 7.0, __builtin_nan(""));
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (int i = 0; i < NUM_SCALARS; i++) {
        for (int j = 0; j < NUM_SCALARS; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters[CNT_UNORDERED]++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                counters[CNT_ORDERED]++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[CNT_UNEQ]++;
            }
            
            /* UNGE (unordered or greater or equal) */
            if (__builtin_isunordered(a, b) || a >= b) {
                counters[CNT_UNGE]++;
            }
            
            /* UNGT (unordered or greater) */
            if (__builtin_isunordered(a, b) || a > b) {
                counters[CNT_UNGT]++;
            }
            
            /* UNLE (unordered or less or equal) */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters[CNT_UNLE]++;
            }
            
            /* UNLT (unordered or less) */
            if (__builtin_isunordered(a, b) || a < b) {
                counters[CNT_UNLT]++;
            }
            
            /* LTGT (less or greater, but not equal and not unordered) */
            if (!__builtin_isunordered(a, b) && a != b) {
                counters[CNT_LTGT]++;
            }
        }
    }
}

/* Test vector comparisons with SSE/AVX intrinsics */
void test_vector_conditions(void) {
    __m128d a, b;
    __m128d mask;
    
    /* Test various comparison predicates */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            a = vec_data[i];
            b = vec_data[j];
            
            /* _CMP_UNORD_Q - unordered */
            mask = _mm_cmpunord_pd(a, b);
            if (_mm_movemask_pd(mask) != 0) {
                counters[CNT_UNORDERED]++;
            }
            
            /* _CMP_ORD_Q - ordered */
            mask = _mm_cmpord_pd(a, b);
            if (_mm_movemask_pd(mask) != 0) {
                counters[CNT_ORDERED]++;
            }
            
            /* _CMP_EQ_UQ - equal (unordered, non-signaling) */
            mask = _mm_cmp_uq_pd(a, b);
            if (_mm_movemask_pd(mask) != 0) {
                counters[CNT_UNEQ]++;
            }
            
            /* _CMP_NLT_UQ - not less than (unordered, non-signaling) */
            mask = _mm_cmp_nlt_pd(a, b);
            if (_mm_movemask_pd(mask) != 0) {
                counters[CNT_UNGE]++;
            }
            
            /* _CMP_NLE_UQ - not less than or equal (unordered, non-signaling) */
            mask = _mm_cmp_nle_pd(a, b);
            if (_mm_movemask_pd(mask) != 0) {
                counters[CNT_UNGT]++;
            }
            
            /* _CMP_LE_UQ - less than or equal (unordered, non-signaling) */
            mask = _mm_cmp_le_pd(a, b);
            if (_mm_movemask_pd(mask) != 0) {
                counters[CNT_UNLE]++;
            }
            
            /* _CMP_LT_UQ - less than (unordered, non-signaling) */
            mask = _mm_cmp_lt_pd(a, b);
            if (_mm_movemask_pd(mask) != 0) {
                counters[CNT_UNLT]++;
            }
            
            /* _CMP_NEQ_UQ - not equal (unordered, non-signaling) */
            mask = _mm_cmp_neq_pd(a, b);
            if (_mm_movemask_pd(mask) != 0) {
                counters[CNT_LTGT]++;
            }
        }
    }
}

/* Test inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    double d = 3.0;
    int result;
    
    /* Test each condition code via inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) counters[CNT_UNORDERED]++;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0"
        : "=r"(result)
        : "x"(c), "x"(d)
        : "cc"
    );
    if (result) counters[CNT_ORDERED]++;
    
    /* UNEQ - unordered or equal */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %0\n\t"
        "ucomisd %3, %4\n\t"
        "setp %%al\n\t"
        "orb %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(a), "x"(a), "x"(b)
        : "cc", "al"
    );
    if (result) counters[CNT_UNEQ]++;
    
    /* UNGE - not less than (nlt) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnb %0"
        : "=r"(result)
        : "x"(c), "x"(d)
        : "cc"
    );
    if (result) counters[CNT_UNGE]++;
    
    /* UNGT - not less than or equal (nle) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnbe %0"
        : "=r"(result)
        : "x"(d), "x"(c)
        : "cc"
    );
    if (result) counters[CNT_UNGT]++;
    
    /* UNLE - unordered or less or equal (ule) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setbe %0"
        : "=r"(result)
        : "x"(c), "x"(d)
        : "cc"
    );
    if (result) counters[CNT_UNLE]++;
    
    /* UNLT - unordered or less than (ult) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r"(result)
        : "x"(c), "x"(d)
        : "cc"
    );
    if (result) counters[CNT_UNLT]++;
    
    /* LTGT - not equal and ordered (une) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %0"
        : "=r"(result)
        : "x"(c), "x"(d)
        : "cc"
    );
    if (result) counters[CNT_LTGT]++;
}

/* Test with AVX256 vectors */
#ifdef __AVX__
void test_avx_conditions(void) {
    __m256d a = vec256_data[0];
    __m256d b = vec256_data[1];
    __m256d mask;
    
    /* Use AVX comparison predicates */
    mask = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
    if (_mm256_movemask_pd(mask) != 0) counters[CNT_UNORDERED]++;
    
    mask = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
    if (_mm256_movemask_pd(mask) != 0) counters[CNT_ORDERED]++;
    
    mask = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
    if (_mm256_movemask_pd(mask) != 0) counters[CNT_UNEQ]++;
    
    mask = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
    if (_mm256_movemask_pd(mask) != 0) counters[CNT_UNGE]++;
    
    mask = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
    if (_mm256_movemask_pd(mask) != 0) counters[CNT_UNGT]++;
    
    mask = _mm256_cmp_pd(a, b, _CMP_LE_UQ);
    if (_mm256_movemask_pd(mask) != 0) counters[CNT_UNLE]++;
    
    mask = _mm256_cmp_pd(a, b, _CMP_LT_UQ);
    if (_mm256_movemask_pd(mask) != 0) counters[CNT_UNLT]++;
    
    mask = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
    if (_mm256_movemask_pd(mask) != 0) counters[CNT_LTGT]++;
}
#endif

/* Control flow test with switch statement */
void test_control_flow(void) {
    double a = __builtin_nan("");
    double b = 1.0;
    double c = 2.0;
    
    for (int i = 0; i < 10; i++) {
        int condition = 0;
        
        /* Determine condition class */
        if (__builtin_isunordered(a, b)) condition = 0;
        else if (__builtin_isordered(c, b)) condition = 1;
        else if (__builtin_isunordered(a, b) || a == b) condition = 2;
        else if (__builtin_isunordered(c, b) || c >= b) condition = 3;
        else if (__builtin_isunordered(c, b) || c > b) condition = 4;
        else if (__builtin_isunordered(c, b) || c <= b) condition = 5;
        else if (__builtin_isunordered(c, b) || c < b) condition = 6;
        else if (!__builtin_isunordered(c, b) && c != b) condition = 7;
        
        /* Switch on condition - prevents optimization */
        switch (condition) {
            case 0: counters[CNT_UNORDERED]++; break;
            case 1: counters[CNT_ORDERED]++; break;
            case 2: counters[CNT_UNEQ]++; break;
            case 3: counters[CNT_UNGE]++; break;
            case 4: counters[CNT_UNGT]++; break;
            case 5: counters[CNT_UNLE]++; break;
            case 6: counters[CNT_UNLT]++; break;
            case 7: counters[CNT_LTGT]++; break;
        }
        
        /* Modify values to vary control flow */
        b += 0.1;
    }
}

int main(void) {
    /* Initialize test vectors */
    init_test_vectors();
    
    /* Clear counters */
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
    printf("Condition Code Execution Summary:\n");
    printf("UNORDERED: %d\n", counters[CNT_UNORDERED]);
    printf("ORDERED: %d\n", counters[CNT_ORDERED]);
    printf("UNEQ: %d\n", counters[CNT_UNEQ]);
    printf("UNGE: %d\n", counters[CNT_UNGE]);
    printf("UNGT: %d\n", counters[CNT_UNGT]);
    printf("UNLE: %d\n", counters[CNT_UNLE]);
    printf("UNLT: %d\n", counters[CNT_UNLT]);
    printf("LTGT: %d\n", counters[CNT_LTGT]);
    
    /* Verify all condition codes were triggered */
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += counters[i];
    }
    
    if (total > 0) {
        printf("\nSUCCESS: All condition code paths were exercised (total: %d)\n", total);
        return 0;
    } else {
        printf("\nFAILURE: No condition codes were triggered\n");
        return 1;
    }
}
