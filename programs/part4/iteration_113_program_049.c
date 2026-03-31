#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Condition code counters */
static int counters[8] = {0};
enum {
    UNORDERED_IDX = 0,
    ORDERED_IDX = 1,
    UNEQ_IDX = 2,
    UNGE_IDX = 3,
    UNGT_IDX = 4,
    UNLE_IDX = 5,
    UNLT_IDX = 6,
    LTGT_IDX = 7
};

/* Test data arrays */
#define TEST_SIZE 8
static double scalar_test_a[TEST_SIZE];
static double scalar_test_b[TEST_SIZE];
static __m128d vector_test_a[TEST_SIZE];
static __m128d vector_test_b[TEST_SIZE];
static __m256d avx_test_a[TEST_SIZE];
static __m256d avx_test_b[TEST_SIZE];

/* Initialize test data with normal values, infinities, and NaNs */
void init_test_data(void) {
    const double test_values[] = {
        1.0, 2.0, -1.0, -2.0,
        0.0, -0.0, DBL_MAX, -DBL_MAX,
        INFINITY, -INFINITY,
        NAN, -NAN
    };
    
    for (int i = 0; i < TEST_SIZE; i++) {
        double a = test_values[i % 12];
        double b = test_values[(i + 3) % 12];
        
        /* Scalar values */
        scalar_test_a[i] = a;
        scalar_test_b[i] = b;
        
        /* SSE vector values */
        vector_test_a[i] = _mm_set_pd(a, b);
        vector_test_b[i] = _mm_set_pd(b, a);
        
        /* AVX vector values */
        avx_test_a[i] = _mm256_set_pd(a, b, a, b);
        avx_test_b[i] = _mm256_set_pd(b, a, b, a);
    }
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (int i = 0; i < TEST_SIZE; i++) {
        double a = scalar_test_a[i];
        double b = scalar_test_b[i];
        
        /* UNORDERED: a or b is NaN */
        if (__builtin_isunordered(a, b)) {
            counters[UNORDERED_IDX]++;
        }
        
        /* ORDERED: neither a nor b is NaN */
        if (__builtin_isordered(a, b)) {
            counters[ORDERED_IDX]++;
        }
        
        /* UNEQ: unordered or equal */
        if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
            counters[UNEQ_IDX]++;
        }
        
        /* UNGE: not less than (greater or equal or unordered) */
        if (!__builtin_isless(a, b)) {
            counters[UNGE_IDX]++;
        }
        
        /* UNGT: not less or equal (greater or unordered) */
        if (!__builtin_islessequal(a, b)) {
            counters[UNGT_IDX]++;
        }
        
        /* UNLE: not greater than (less or equal or unordered) */
        if (!__builtin_isgreater(a, b)) {
            counters[UNLE_IDX]++;
        }
        
        /* UNLT: not greater or equal (less than or unordered) */
        if (!__builtin_isgreaterequal(a, b)) {
            counters[UNLT_IDX]++;
        }
        
        /* LTGT: less or greater (not equal and ordered) */
        if (__builtin_islessgreater(a, b)) {
            counters[LTGT_IDX]++;
        }
    }
}

/* Test SSE vector comparisons */
void test_sse_conditions(void) {
    for (int i = 0; i < TEST_SIZE; i++) {
        __m128d a = vector_test_a[i];
        __m128d b = vector_test_b[i];
        
        /* Generate comparison masks for different predicates */
        __m128d cmp_unord = _mm_cmpunord_pd(a, b);  /* UNORDERED */
        __m128d cmp_ord = _mm_cmpord_pd(a, b);      /* ORDERED */
        __m128d cmp_eq = _mm_cmpeq_pd(a, b);        /* EQ */
        __m128d cmp_lt = _mm_cmplt_pd(a, b);        /* LT */
        __m128d cmp_le = _mm_cmple_pd(a, b);        /* LE */
        __m128d cmp_gt = _mm_cmpgt_pd(a, b);        /* GT */
        __m128d cmp_ge = _mm_cmpge_pd(a, b);        /* GE */
        __m128d cmp_neq = _mm_cmpneq_pd(a, b);      /* NEQ */
        
        /* Extract masks and update counters */
        int mask_unord = _mm_movemask_pd(cmp_unord);
        int mask_ord = _mm_movemask_pd(cmp_ord);
        
        /* Control flow based on comparison results */
        if (mask_unord) {
            counters[UNORDERED_IDX] += __builtin_popcount(mask_unord);
        }
        if (mask_ord) {
            counters[ORDERED_IDX] += __builtin_popcount(mask_ord);
        }
        
        /* Combine masks for other condition codes */
        __m128d cmp_ueq = _mm_or_pd(cmp_unord, cmp_eq);  /* UNEQ */
        __m128d cmp_nlt = _mm_or_pd(cmp_unord, cmp_ge);  /* UNGE (nlt) */
        __m128d cmp_nle = _mm_or_pd(cmp_unord, cmp_gt);  /* UNGT (nle) */
        __m128d cmp_ule = _mm_or_pd(cmp_unord, cmp_le);  /* UNLE */
        __m128d cmp_ult = _mm_or_pd(cmp_unord, cmp_lt);  /* UNLT */
        __m128d cmp_une = _mm_and_pd(cmp_ord, cmp_neq);  /* LTGT (une) */
        
        /* Update counters based on combined masks */
        counters[UNEQ_IDX] += __builtin_popcount(_mm_movemask_pd(cmp_ueq));
        counters[UNGE_IDX] += __builtin_popcount(_mm_movemask_pd(cmp_nlt));
        counters[UNGT_IDX] += __builtin_popcount(_mm_movemask_pd(cmp_nle));
        counters[UNLE_IDX] += __builtin_popcount(_mm_movemask_pd(cmp_ule));
        counters[UNLT_IDX] += __builtin_popcount(_mm_movemask_pd(cmp_ult));
        counters[LTGT_IDX] += __builtin_popcount(_mm_movemask_pd(cmp_une));
    }
}

/* Test AVX vector comparisons */
#ifdef __AVX__
void test_avx_conditions(void) {
    for (int i = 0; i < TEST_SIZE; i++) {
        __m256d a = avx_test_a[i];
        __m256d b = avx_test_b[i];
        
        /* AVX comparison intrinsics */
        __m256d cmp_unord = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);  /* UNORDERED */
        __m256d cmp_ord = _mm256_cmp_pd(a, b, _CMP_ORD_Q);      /* ORDERED */
        __m256d cmp_eq = _mm256_cmp_pd(a, b, _CMP_EQ_OQ);       /* EQ */
        __m256d cmp_lt = _mm256_cmp_pd(a, b, _CMP_LT_OQ);       /* LT */
        __m256d cmp_le = _mm256_cmp_pd(a, b, _CMP_LE_OQ);       /* LE */
        __m256d cmp_gt = _mm256_cmp_pd(a, b, _CMP_GT_OQ);       /* GT */
        __m256d cmp_ge = _mm256_cmp_pd(a, b, _CMP_GE_OQ);       /* GE */
        __m256d cmp_neq = _mm256_cmp_pd(a, b, _CMP_NEQ_OQ);     /* NEQ */
        
        /* Extract masks */
        int mask_unord = _mm256_movemask_pd(cmp_unord);
        int mask_ord = _mm256_movemask_pd(cmp_ord);
        
        /* Update counters */
        counters[UNORDERED_IDX] += __builtin_popcount(mask_unord);
        counters[ORDERED_IDX] += __builtin_popcount(mask_ord);
        
        /* Additional AVX-specific predicates */
        __m256d cmp_ueq = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);      /* UNEQ */
        __m256d cmp_nlt = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);     /* UNGE (nlt) */
        __m256d cmp_nle = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);     /* UNGT (nle) */
        __m256d cmp_ule = _mm256_cmp_pd(a, b, _CMP_LE_UQ);      /* UNLE */
        __m256d cmp_ult = _mm256_cmp_pd(a, b, _CMP_LT_UQ);      /* UNLT */
        __m256d cmp_une = _mm256_cmp_pd(a, b, _CMP_NEQ_OQ);     /* LTGT (une) */
        
        counters[UNEQ_IDX] += __builtin_popcount(_mm256_movemask_pd(cmp_ueq));
        counters[UNGE_IDX] += __builtin_popcount(_mm256_movemask_pd(cmp_nlt));
        counters[UNGT_IDX] += __builtin_popcount(_mm256_movemask_pd(cmp_nle));
        counters[UNLE_IDX] += __builtin_popcount(_mm256_movemask_pd(cmp_ule));
        counters[UNLT_IDX] += __builtin_popcount(_mm256_movemask_pd(cmp_ult));
        counters[LTGT_IDX] += __builtin_popcount(_mm256_movemask_pd(cmp_une));
    }
}
#endif

/* Inline assembly with condition code constraints */
void test_asm_constraints(void) {
    for (int i = 0; i < TEST_SIZE; i++) {
        double a = scalar_test_a[i];
        double b = scalar_test_b[i];
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
        if (result) counters[UNORDERED_IDX]++;
        
        /* ORDERED constraint */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnp %%al\n\t"
            "movzbl %%al, %0"
            : "=@ord" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) counters[ORDERED_IDX]++;
        
        /* UNEQ constraint */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "sete %%al\n\t"
            "setp %%cl\n\t"
            "orb %%cl, %%al\n\t"
            "movzbl %%al, %0"
            : "=@ueq" (result)
            : "x" (a), "x" (b)
            : "al", "cl"
        );
        if (result) counters[UNEQ_IDX]++;
        
        /* UNGE (nlt) constraint */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnb %%al\n\t"
            "movzbl %%al, %0"
            : "=@nlt" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) counters[UNGE_IDX]++;
        
        /* UNGT (nle) constraint */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setnbe %%al\n\t"
            "movzbl %%al, %0"
            : "=@nle" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) counters[UNGT_IDX]++;
        
        /* UNLE constraint */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setbe %%al\n\t"
            "movzbl %%al, %0"
            : "=@ule" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) counters[UNLE_IDX]++;
        
        /* UNLT constraint */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setb %%al\n\t"
            "movzbl %%al, %0"
            : "=@ult" (result)
            : "x" (a), "x" (b)
            : "al"
        );
        if (result) counters[UNLT_IDX]++;
        
        /* LTGT (une) constraint */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "setne %%al\n\t"
            "setnp %%cl\n\t"
            "andb %%cl, %%al\n\t"
            "movzbl %%al, %0"
            : "=@une" (result)
            : "x" (a), "x" (b)
            : "al", "cl"
        );
        if (result) counters[LTGT_IDX]++;
    }
}

/* Print summary of condition code hits */
void print_summary(void) {
    const char *names[8] = {
        "UNORDERED", "ORDERED", "UNEQ", "UNGE",
        "UNGT", "UNLE", "UNLT", "LTGT"
    };
    
    printf("Condition Code Summary:\n");
    printf("=======================\n");
    for (int i = 0; i < 8; i++) {
        printf("%-10s: %d\n", names[i], counters[i]);
    }
    
    /* Verify we hit all condition codes */
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += (counters[i] > 0);
    }
    printf("\nCondition codes triggered: %d/8\n", total);
}

int main(void) {
    /* Initialize test data */
    init_test_data();
    
    /* Reset counters */
    for (int i = 0; i < 8; i++) {
        counters[i] = 0;
    }
    
    /* Run all tests */
    test_scalar_conditions();
    test_sse_conditions();
    
#ifdef __AVX__
    test_avx_conditions();
#endif
    
    test_asm_constraints();
    
    /* Print results */
    print_summary();
    
    /* Return success if we triggered at least some condition codes */
    int any_hit = 0;
    for (int i = 0; i < 8; i++) {
        if (counters[i] > 0) {
            any_hit = 1;
            break;
        }
    }
    
    return any_hit ? 0 : 1;
}
