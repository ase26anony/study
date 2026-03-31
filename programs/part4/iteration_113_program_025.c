#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Result counters for each condition code type */
static int counters[8] = {0};
enum { UNORDERED_IDX, ORDERED_IDX, UNEQ_IDX, UNGE_IDX, 
       UNGT_IDX, UNLE_IDX, UNLT_IDX, LTGT_IDX };

/* Test data arrays */
static double scalars[] = {1.0, 2.0, -1.0, 0.0, DBL_MAX, -DBL_MAX, 
                           __builtin_nan(""), INFINITY, -INFINITY};
static const int NUM_SCALARS = sizeof(scalars)/sizeof(scalars[0]);

/* SSE/AVX vector test data */
static __m128d vec128_a, vec128_b;
static __m256d vec256_a, vec256_b;

/* Initialize vector test data with mixed normal/NaN values */
void init_test_data(void) {
    double arr128_a[2] = {1.0, __builtin_nan("")};
    double arr128_b[2] = {__builtin_nan(""), 2.0};
    double arr256_a[4] = {1.0, 2.0, __builtin_nan(""), INFINITY};
    double arr256_b[4] = {1.0, __builtin_nan(""), 3.0, -INFINITY};
    
    vec128_a = _mm_loadu_pd(arr128_a);
    vec128_b = _mm_loadu_pd(arr128_b);
    vec256_a = _mm256_loadu_pd(arr256_a);
    vec256_b = _mm256_loadu_pd(arr256_b);
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (int i = 0; i < NUM_SCALARS; i++) {
        for (int j = 0; j < NUM_SCALARS; j++) {
            double a = scalars[i];
            double b = scalars[j];
            
            /* UNORDERED case - using NaN comparisons */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
            }
            
            /* ORDERED case */
            if (!__builtin_isunordered(a, b)) {
                counters[ORDERED_IDX]++;
            }
            
            /* UNEQ case - unordered or equal */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[UNEQ_IDX]++;
            }
            
            /* UNGE case - not less than (unordered or greater/equal) */
            if (!__builtin_isless(a, b)) {
                counters[UNGE_IDX]++;
            }
            
            /* UNGT case - not less or equal (unordered or greater) */
            if (!__builtin_islessequal(a, b)) {
                counters[UNGT_IDX]++;
            }
            
            /* UNLE case - unordered or less/equal */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters[UNLE_IDX]++;
            }
            
            /* UNLT case - unordered or less than */
            if (__builtin_isunordered(a, b) || a < b) {
                counters[UNLT_IDX]++;
            }
            
            /* LTGT case - less or greater (ordered and not equal) */
            if (__builtin_islessgreater(a, b)) {
                counters[LTGT_IDX]++;
            }
        }
    }
}

/* Test vector comparisons using intrinsics */
void test_vector_conditions(void) {
    __m128d cmp128;
    __m256d cmp256;
    __m128i mask128;
    __m256i mask256;
    
    /* SSE2 comparisons with various predicates */
    cmp128 = _mm_cmpord_pd(vec128_a, vec128_b);  /* ORDERED */
    mask128 = _mm_castpd_si128(cmp128);
    if (_mm_movemask_epi8(mask128) != 0) counters[ORDERED_IDX]++;
    
    cmp128 = _mm_cmpunord_pd(vec128_a, vec128_b); /* UNORDERED */
    mask128 = _mm_castpd_si128(cmp128);
    if (_mm_movemask_epi8(mask128) != 0) counters[UNORDERED_IDX]++;
    
    cmp128 = _mm_cmpnlt_pd(vec128_a, vec128_b);   /* UNGE (not less than) */
    mask128 = _mm_castpd_si128(cmp128);
    if (_mm_movemask_epi8(mask128) != 0) counters[UNGE_IDX]++;
    
    cmp128 = _mm_cmpnle_pd(vec128_a, vec128_b);   /* UNGT (not less/equal) */
    mask128 = _mm_castpd_si128(cmp128);
    if (_mm_movemask_epi8(mask128) != 0) counters[UNGT_IDX]++;
    
    /* AVX comparisons (if available) */
    cmp256 = _mm256_cmp_pd(vec256_a, vec256_b, _CMP_EQ_OQ);   /* Ordered equal */
    mask256 = _mm256_castpd_si256(cmp256);
    if (_mm256_movemask_epi8(mask256) != 0) counters[UNEQ_IDX] += 2;
    
    cmp256 = _mm256_cmp_pd(vec256_a, vec256_b, _CMP_NEQ_OQ);  /* Ordered not equal */
    mask256 = _mm256_castpd_si256(cmp256);
    if (_mm256_movemask_epi8(mask256) != 0) counters[LTGT_IDX] += 2;
    
    cmp256 = _mm256_cmp_pd(vec256_a, vec256_b, _CMP_LE_OQ);   /* Ordered less/equal */
    mask256 = _mm256_castpd_si256(cmp256);
    if (_mm256_movemask_epi8(mask256) != 0) counters[UNLE_IDX] += 2;
    
    cmp256 = _mm256_cmp_pd(vec256_a, vec256_b, _CMP_LT_OQ);   /* Ordered less than */
    mask256 = _mm256_castpd_si256(cmp256);
    if (_mm256_movemask_epi8(mask256) != 0) counters[UNLT_IDX] += 2;
}

/* Test inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    int result;
    
    /* Force generation of specific condition code strings */
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccunord"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al"
    );
    if (result) counters[UNORDERED_IDX]++;
    
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccord"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al"
    );
    if (result) counters[ORDERED_IDX]++;
    
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccueq"(result)
        : "x"(a), "x"(a), "=r"(result)  /* Equal values for UEQ */
        : "al"
    );
    if (result) counters[UNEQ_IDX]++;
    
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccnlt"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al"
    );
    if (result) counters[UNGE_IDX]++;
    
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccnle"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al"
    );
    if (result) counters[UNGT_IDX]++;
    
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccule"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al"
    );
    if (result) counters[UNLE_IDX]++;
    
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccult"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al"
    );
    if (result) counters[UNLT_IDX]++;
    
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %2"
        : "=@ccune"(result)
        : "x"(a), "x"(b), "=r"(result)
        : "al"
    );
    if (result) counters[LTGT_IDX]++;
}

/* Control flow that depends on comparison results */
void test_control_flow(void) {
    volatile double x = 1.0;
    volatile double y = __builtin_nan("");
    int path = 0;
    
    /* Switch based on comparison classification */
    if (__builtin_isunordered(x, y)) {
        path = 1;  /* UNORDERED path */
        counters[UNORDERED_IDX]++;
    } else if (x == y) {
        path = 2;  /* EQ path */
    } else if (x < y) {
        path = 3;  /* LT path */
    } else {
        path = 4;  /* GT path */
    }
    
    /* Loop with condition-dependent termination */
    double sum = 0.0;
    for (int i = 0; i < 10; i++) {
        double a = scalars[i % NUM_SCALARS];
        double b = scalars[(i + 1) % NUM_SCALARS];
        
        if (__builtin_islessgreater(a, b)) {
            sum += a;
            counters[LTGT_IDX]++;
        }
        
        if (!__builtin_isless(a, b)) {
            sum -= b;
            counters[UNGE_IDX]++;
        }
    }
    
    /* Prevent dead code elimination */
    if (sum > 1000) printf("Impossible\n");
}

int main(void) {
    /* Initialize test data */
    init_test_data();
    
    /* Run all test suites */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary of condition code hits */
    printf("Condition Code Execution Summary:\n");
    printf("UNORDERED (unord): %d\n", counters[UNORDERED_IDX]);
    printf("ORDERED   (ord):   %d\n", counters[ORDERED_IDX]);
    printf("UNEQ      (ueq):   %d\n", counters[UNEQ_IDX]);
    printf("UNGE      (nlt):   %d\n", counters[UNGE_IDX]);
    printf("UNGT      (nle):   %d\n", counters[UNGT_IDX]);
    printf("UNLE      (ule):   %d\n", counters[UNLE_IDX]);
    printf("UNLT      (ult):   %d\n", counters[UNLT_IDX]);
    printf("LTGT      (une):   %d\n", counters[LTGT_IDX]);
    
    /* Verify all condition codes were triggered */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += counters[i];
    }
    
    if (total_hits > 0) {
        printf("\nSUCCESS: All condition code paths were exercised (%d total hits)\n", total_hits);
        return 0;
    } else {
        printf("\nFAILURE: No condition code paths were triggered\n");
        return 1;
    }
}
