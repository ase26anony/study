#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <math.h>

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
static double test_scalars[TEST_SIZE];
static __m128d test_vec128[TEST_SIZE/2];
static __m256d test_vec256[TEST_SIZE/4];

/* Initialize test data with normal values, Inf, and NaN */
void init_test_data(void) {
    const double inf = 1.0 / 0.0;
    const double neg_inf = -1.0 / 0.0;
    const double nan = 0.0 / 0.0;
    
    /* Scalar test values */
    test_scalars[0] = 1.0;
    test_scalars[1] = 2.0;
    test_scalars[2] = -1.0;
    test_scalars[3] = 0.0;
    test_scalars[4] = inf;
    test_scalars[5] = neg_inf;
    test_scalars[6] = nan;
    test_scalars[7] = __builtin_nan("");
    
    /* 128-bit vector test values */
    for (int i = 0; i < TEST_SIZE/2; i++) {
        test_vec128[i] = _mm_set_pd(test_scalars[2*i+1], test_scalars[2*i]);
    }
    
    /* 256-bit vector test values (if AVX available) */
    for (int i = 0; i < TEST_SIZE/4; i++) {
        test_vec256[i] = _mm256_set_pd(
            test_scalars[4*i+3],
            test_scalars[4*i+2],
            test_scalars[4*i+1],
            test_scalars[4*i]
        );
    }
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    printf("Testing scalar conditions with builtins...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
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
            
            /* UNGE: unordered or greater or equal */
            if (!__builtin_isless(a, b)) {
                counters[CNT_UNGE]++;
            }
            
            /* UNGT: unordered or greater */
            if (__builtin_isgreater(a, b)) {
                counters[CNT_UNGT]++;
            }
            
            /* UNLE: unordered or less or equal */
            if (!__builtin_isgreater(a, b)) {
                counters[CNT_UNLE]++;
            }
            
            /* UNLT: unordered or less */
            if (__builtin_isless(a, b)) {
                counters[CNT_UNLT]++;
            }
            
            /* LTGT: less or greater (ordered and not equal) */
            if (__builtin_islessgreater(a, b)) {
                counters[CNT_LTGT]++;
            }
        }
    }
}

/* Test vector comparisons using intrinsics */
void test_vector_conditions(void) {
    printf("Testing vector conditions with SSE/AVX...\n");
    
    /* SSE2 vector comparisons */
    for (int i = 0; i < TEST_SIZE/2; i++) {
        for (int j = 0; j < TEST_SIZE/2; j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            
            /* Compare with different predicates */
            __m128d cmp_eq = _mm_cmpeq_pd(a, b);
            __m128d cmp_lt = _mm_cmplt_pd(a, b);
            __m128d cmp_le = _mm_cmple_pd(a, b);
            __m128d cmp_unord = _mm_cmpunord_pd(a, b);
            __m128d cmp_neq = _mm_cmpneq_pd(a, b);
            __m128d cmp_nlt = _mm_cmpnlt_pd(a, b);
            __m128d cmp_nle = _mm_cmpnle_pd(a, b);
            __m128d cmp_ord = _mm_cmpord_pd(a, b);
            
            /* Extract masks and update counters based on results */
            int mask_eq = _mm_movemask_pd(cmp_eq);
            int mask_lt = _mm_movemask_pd(cmp_lt);
            int mask_le = _mm_movemask_pd(cmp_le);
            int mask_unord = _mm_movemask_pd(cmp_unord);
            int mask_neq = _mm_movemask_pd(cmp_neq);
            int mask_nlt = _mm_movemask_pd(cmp_nlt);
            int mask_nle = _mm_movemask_pd(cmp_nle);
            int mask_ord = _mm_movemask_pd(cmp_ord);
            
            /* Map to condition codes */
            if (mask_unord) counters[CNT_UNORDERED]++;
            if (mask_ord) counters[CNT_ORDERED]++;
            if (mask_eq || mask_unord) counters[CNT_UNEQ]++;  /* ueq */
            if (mask_nlt) counters[CNT_UNGE]++;  /* nlt */
            if (mask_nle) counters[CNT_UNGT]++;  /* nle */
            if (!mask_nle) counters[CNT_UNLE]++; /* ule */
            if (mask_lt || mask_unord) counters[CNT_UNLT]++; /* ult */
            if (mask_neq && mask_ord) counters[CNT_LTGT]++;  /* une */
        }
    }
    
    /* AVX vector comparisons if compiled with AVX support */
#ifdef __AVX__
    printf("Testing AVX vector conditions...\n");
    
    for (int i = 0; i < TEST_SIZE/4; i++) {
        for (int j = 0; j < TEST_SIZE/4; j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* AVX comparison predicates */
            __m256d cmp_eq = _mm256_cmp_pd(a, b, _CMP_EQ_OQ);
            __m256d cmp_lt = _mm256_cmp_pd(a, b, _CMP_LT_OQ);
            __m256d cmp_le = _mm256_cmp_pd(a, b, _CMP_LE_OQ);
            __m256d cmp_unord = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            __m256d cmp_neq = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            __m256d cmp_nlt = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            __m256d cmp_nle = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            __m256d cmp_ord = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            
            int mask_eq = _mm256_movemask_pd(cmp_eq);
            int mask_lt = _mm256_movemask_pd(cmp_lt);
            int mask_le = _mm256_movemask_pd(cmp_le);
            int mask_unord = _mm256_movemask_pd(cmp_unord);
            int mask_neq = _mm256_movemask_pd(cmp_neq);
            int mask_nlt = _mm256_movemask_pd(cmp_nlt);
            int mask_nle = _mm256_movemask_pd(cmp_nle);
            int mask_ord = _mm256_movemask_pd(cmp_ord);
            
            if (mask_unord) counters[CNT_UNORDERED]++;
            if (mask_ord) counters[CNT_ORDERED]++;
            if (mask_eq || mask_unord) counters[CNT_UNEQ]++;
            if (mask_nlt) counters[CNT_UNGE]++;
            if (mask_nle) counters[CNT_UNGT]++;
            if (!mask_nle) counters[CNT_UNLE]++;
            if (mask_lt || mask_unord) counters[CNT_UNLT]++;
            if (mask_neq && mask_ord) counters[CNT_LTGT]++;
        }
    }
#endif
}

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    printf("Testing inline assembly with condition code constraints...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            int result;
            
            /* UNORDERED constraint */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[CNT_UNORDERED]++;
            
            /* ORDERED constraint */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setnp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[CNT_ORDERED]++;
            
            /* UNEQ constraint (unordered or equal) */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[CNT_UNEQ]++;
            
            /* UNGE constraint (not less than) */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setnb %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[CNT_UNGE]++;
            
            /* UNGT constraint (not less or equal) */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setnbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[CNT_UNGT]++;
            
            /* UNLE constraint (unordered or less or equal) */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setna %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[CNT_UNLE]++;
            
            /* UNLT constraint (unordered or less) */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setb %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters[CNT_UNLT]++;
            
            /* LTGT constraint (not equal and ordered) */
            __asm__ volatile (
                "ucomisd %1, %2\n\t"
                "setne %%al\n\t"
                "setnp %%cl\n\t"
                "andb %%cl, %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cl"
            );
            if (result) counters[CNT_LTGT]++;
        }
    }
}

/* Control flow test that depends on comparison results */
void test_control_flow(void) {
    printf("Testing control flow dependent on comparisons...\n");
    
    volatile double a = 1.0;
    volatile double b = __builtin_nan("");
    volatile double c = 2.0;
    volatile double d = 3.0;
    
    /* Complex control flow to prevent optimization */
    for (int i = 0; i < 100; i++) {
        if (__builtin_isunordered(a, b)) {
            counters[CNT_UNORDERED]++;
            if (__builtin_isordered(c, d)) {
                counters[CNT_ORDERED]++;
            }
        }
        
        if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
            counters[CNT_UNEQ]++;
        }
        
        if (!__builtin_isless(c, d)) {
            counters[CNT_UNGE]++;
            if (__builtin_isgreater(c, d)) {
                counters[CNT_UNGT]++;
            }
        }
        
        if (!__builtin_isgreater(c, d)) {
            counters[CNT_UNLE]++;
            if (__builtin_isless(c, d)) {
                counters[CNT_UNLT]++;
            }
        }
        
        if (__builtin_islessgreater(c, d)) {
            counters[CNT_LTGT]++;
        }
        
        /* Mix in some vector operations */
        __m128d v1 = _mm_set_pd(a, b);
        __m128d v2 = _mm_set_pd(c, d);
        __m128d cmp = _mm_cmpunord_pd(v1, v2);
        int mask = _mm_movemask_pd(cmp);
        
        switch (mask) {
            case 0:
                counters[CNT_ORDERED]++;
                break;
            case 1:
            case 2:
            case 3:
                counters[CNT_UNORDERED]++;
                break;
        }
    }
}

int main(void) {
    printf("=== Testing x86 Floating-Point Condition Codes ===\n\n");
    
    /* Initialize test data */
    init_test_data();
    
    /* Reset counters */
    memset(counters, 0, sizeof(counters));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print results */
    printf("\n=== Condition Code Usage Summary ===\n");
    printf("UNORDERED (unord): %d\n", counters[CNT_UNORDERED]);
    printf("ORDERED   (ord):   %d\n", counters[CNT_ORDERED]);
    printf("UNEQ      (ueq):   %d\n", counters[CNT_UNEQ]);
    printf("UNGE      (nlt):   %d\n", counters[CNT_UNGE]);
    printf("UNGT      (nle):   %d\n", counters[CNT_UNGT]);
    printf("UNLE      (ule):   %d\n", counters[CNT_UNLE]);
    printf("UNLT      (ult):   %d\n", counters[CNT_UNLT]);
    printf("LTGT      (une):   %d\n", counters[CNT_LTGT]);
    
    /* Verify all condition codes were triggered */
    int all_triggered = 1;
    for (int i = 0; i < 8; i++) {
        if (counters[i] == 0) {
            printf("WARNING: Condition code %d was not triggered!\n", i);
            all_triggered = 0;
        }
    }
    
    if (all_triggered) {
        printf("\nSUCCESS: All condition codes were triggered!\n");
    } else {
        printf("\nWARNING: Some condition codes were not triggered.\n");
    }
    
    return all_triggered ? 0 : 1;
}
