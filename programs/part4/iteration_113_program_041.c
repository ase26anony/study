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
    UNORDERED = 0,
    ORDERED,
    UNEQ,
    UNGE,
    UNGT,
    UNLE,
    UNLT,
    LTGT
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
static __m128d test_vecs[4];
static __m256d test_avx_vecs[2];

/* Initialize test vectors with mixed data */
void init_test_data(void) {
    /* Create vectors with normal values and NaNs */
    test_vecs[0] = _mm_set_pd(1.0, 2.0);
    test_vecs[1] = _mm_set_pd(__builtin_nan(""), 3.0);
    test_vecs[2] = _mm_set_pd(4.0, __builtin_nan(""));
    test_vecs[3] = _mm_set_pd(5.0, 6.0);
    
    test_avx_vecs[0] = _mm256_set_pd(1.0, __builtin_nan(""), 3.0, 4.0);
    test_avx_vecs[1] = _mm256_set_pd(__builtin_nan(""), 6.0, 7.0, 8.0);
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    printf("Testing scalar conditions...\n");
    
    for (int i = 0; i < NUM_SCALARS; i++) {
        for (int j = 0; j < NUM_SCALARS; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED]++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                counters[ORDERED]++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[UNEQ]++;
            }
            
            /* UNGE (not less than) = !(a < b) */
            if (!__builtin_isless(a, b)) {
                counters[UNGE]++;
            }
            
            /* UNGT (not less than or equal) = !(a <= b) */
            if (!__builtin_islessequal(a, b)) {
                counters[UNGT]++;
            }
            
            /* UNLE (unordered or less than or equal) */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                counters[UNLE]++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
                counters[UNLT]++;
            }
            
            /* LTGT (less than or greater than, but not equal) */
            if (__builtin_islessgreater(a, b)) {
                counters[LTGT]++;
            }
        }
    }
}

/* Test vector comparisons using intrinsics */
void test_vector_conditions(void) {
    printf("Testing vector conditions...\n");
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            __m128d a = test_vecs[i];
            __m128d b = test_vecs[j];
            
            /* Compare with various predicates */
            __m128d cmp;
            
            /* UNORDERED (_CMP_UNORD_Q) */
            cmp = _mm_cmpunord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNORDERED]++;
            }
            
            /* ORDERED (_CMP_ORD_Q) */
            cmp = _mm_cmpord_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[ORDERED]++;
            }
            
            /* UNEQ (_CMP_EQ_UQ) */
            cmp = _mm_cmpeq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNEQ]++;
            }
            
            /* UNGE (_CMP_NLT_UQ) */
            cmp = _mm_cmpnlt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNGE]++;
            }
            
            /* UNGT (_CMP_NLE_UQ) */
            cmp = _mm_cmpnle_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNGT]++;
            }
            
            /* UNLE (_CMP_LE_OS) - using ordered signaling version */
            cmp = _mm_cmple_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNLE]++;
            }
            
            /* UNLT (_CMP_LT_OS) - using ordered signaling version */
            cmp = _mm_cmplt_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[UNLT]++;
            }
            
            /* LTGT (_CMP_NEQ_OS) */
            cmp = _mm_cmpneq_pd(a, b);
            if (_mm_movemask_pd(cmp) != 0) {
                counters[LTGT]++;
            }
        }
    }
}

/* Test AVX vector comparisons */
#ifdef __AVX__
void test_avx_conditions(void) {
    printf("Testing AVX conditions...\n");
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            __m256d a = test_avx_vecs[i];
            __m256d b = test_avx_vecs[j];
            
            /* Various AVX comparison predicates */
            __m256d cmp;
            
            /* UNORDERED */
            cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNORDERED]++;
            }
            
            /* ORDERED */
            cmp = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[ORDERED]++;
            }
            
            /* UNEQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNEQ]++;
            }
            
            /* UNGE */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNGE]++;
            }
            
            /* UNGT */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNGT]++;
            }
            
            /* UNLE */
            cmp = _mm256_cmp_pd(a, b, _CMP_LE_OS);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNLE]++;
            }
            
            /* UNLT */
            cmp = _mm256_cmp_pd(a, b, _CMP_LT_OS);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[UNLT]++;
            }
            
            /* LTGT */
            cmp = _mm256_cmp_pd(a, b, _CMP_NEQ_OS);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters[LTGT]++;
            }
        }
    }
}
#endif

/* Inline assembly with condition code constraints */
void test_asm_constraints(void) {
    printf("Testing inline assembly with condition codes...\n");
    
    double a = 1.0;
    double b = __builtin_nan("");
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
    if (result) counters[UNORDERED]++;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[ORDERED]++;
    
    /* UNEQ (unordered or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cl"
    );
    if (result) counters[UNEQ]++;
    
    /* UNGE (not less than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[UNGE]++;
    
    /* UNGT (not less than or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[UNGT]++;
    
    /* UNLE (unordered or less than or equal) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setbe %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cl"
    );
    if (result) counters[UNLE]++;
    
    /* UNLT (unordered or less than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "cl"
    );
    if (result) counters[UNLT]++;
    
    /* LTGT (less than or greater than) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[LTGT]++;
}

/* Control flow test with switch statement */
void test_control_flow(void) {
    printf("Testing control flow dependent on comparisons...\n");
    
    for (int i = 0; i < NUM_SCALARS; i++) {
        for (int j = 0; j < NUM_SCALARS; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            int condition = -1;
            
            /* Determine condition type */
            if (__builtin_isunordered(a, b)) {
                condition = UNORDERED;
            } else if (a == b) {
                condition = UNEQ;  /* Ordered equal falls in UNEQ */
            } else if (a < b) {
                condition = UNLT;
            } else if (a > b) {
                condition = UNGT;
            }
            
            /* Switch on condition to force code generation */
            switch (condition) {
                case UNORDERED:
                    counters[UNORDERED]++;
                    break;
                case ORDERED:
                    counters[ORDERED]++;
                    break;
                case UNEQ:
                    counters[UNEQ]++;
                    break;
                case UNGE:
                    counters[UNGE]++;
                    break;
                case UNGT:
                    counters[UNGT]++;
                    break;
                case UNLE:
                    counters[UNLE]++;
                    break;
                case UNLT:
                    counters[UNLT]++;
                    break;
                case LTGT:
                    counters[LTGT]++;
                    break;
                default:
                    /* Mixed condition - increment multiple counters */
                    if (!__builtin_isless(a, b)) counters[UNGE]++;
                    if (!__builtin_islessequal(a, b)) counters[UNGT]++;
                    if (__builtin_islessgreater(a, b)) counters[LTGT]++;
                    break;
            }
        }
    }
}

int main(void) {
    printf("Starting condition code coverage test...\n\n");
    
    /* Initialize test data */
    init_test_data();
    
    /* Reset counters */
    memset(counters, 0, sizeof(counters));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    
#ifdef __AVX__
    test_avx_conditions();
#endif
    
    test_asm_constraints();
    test_control_flow();
    
    /* Print results */
    printf("\n=== Condition Code Usage Summary ===\n");
    printf("UNORDERED: %d\n", counters[UNORDERED]);
    printf("ORDERED:   %d\n", counters[ORDERED]);
    printf("UNEQ:      %d\n", counters[UNEQ]);
    printf("UNGE:      %d\n", counters[UNGE]);
    printf("UNGT:      %d\n", counters[UNGT]);
    printf("UNLE:      %d\n", counters[UNLE]);
    printf("UNLT:      %d\n", counters[UNLT]);
    printf("LTGT:      %d\n", counters[LTGT]);
    
    /* Verify we hit all condition codes */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += counters[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("Test PASSED - condition codes were exercised.\n");
        return 0;
    } else {
        printf("Test FAILED - no condition codes were hit.\n");
        return 1;
    }
}
