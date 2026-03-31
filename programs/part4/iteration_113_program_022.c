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
    printf("Testing scalar conditions...\n");
    
    for (int i = 0; i < NUM_SCALARS; i++) {
        for (int j = 0; j < NUM_SCALARS; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED: a or b is NaN */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED]++;
            }
            
            /* ORDERED: neither is NaN */
            if (__builtin_isordered(a, b)) {
                counters[ORDERED]++;
            }
            
            /* UNEQ: unordered or equal */
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                counters[UNEQ]++;
            }
            
            /* UNGE: unordered or greater-or-equal */
            if (__builtin_isunordered(a, b) || a >= b) {
                counters[UNGE]++;
            }
            
            /* UNGT: unordered or greater */
            if (__builtin_isunordered(a, b) || a > b) {
                counters[UNGT]++;
            }
            
            /* UNLE: unordered or less-or-equal */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters[UNLE]++;
            }
            
            /* UNLT: unordered or less */
            if (__builtin_isunordered(a, b) || a < b) {
                counters[UNLT]++;
            }
            
            /* LTGT: less or greater (ordered and not equal) */
            if (__builtin_islessgreater(a, b)) {
                counters[LTGT]++;
            }
        }
    }
}

/* Test vector comparisons with SSE/AVX */
void test_vector_conditions(void) {
    printf("Testing vector conditions...\n");
    
    /* SSE2 vector comparisons */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            __m128d a = vec_data[i];
            __m128d b = vec_data[j];
            
            /* Compare with various predicates */
            __m128d cmp_unord = _mm_cmpunord_pd(a, b);   /* UNORDERED */
            __m128d cmp_eq = _mm_cmpeq_pd(a, b);         /* EQ */
            __m128d cmp_lt = _mm_cmplt_pd(a, b);         /* LT */
            __m128d cmp_le = _mm_cmple_pd(a, b);         /* LE */
            __m128d cmp_gt = _mm_cmpgt_pd(a, b);         /* GT */
            __m128d cmp_ge = _mm_cmpge_pd(a, b);         /* GE */
            __m128d cmp_neq = _mm_cmpneq_pd(a, b);       /* NEQ */
            __m128d cmp_nlt = _mm_cmpnlt_pd(a, b);       /* NLT (UNGE) */
            __m128d cmp_nle = _mm_cmpnle_pd(a, b);       /* NLE (UNGT) */
            
            /* Extract masks and update counters */
            int mask_unord = _mm_movemask_pd(cmp_unord);
            int mask_eq = _mm_movemask_pd(cmp_eq);
            int mask_lt = _mm_movemask_pd(cmp_lt);
            int mask_gt = _mm_movemask_pd(cmp_gt);
            int mask_nlt = _mm_movemask_pd(cmp_nlt);
            int mask_nle = _mm_movemask_pd(cmp_nle);
            
            /* Update counters based on vector lane results */
            for (int lane = 0; lane < 2; lane++) {
                if (mask_unord & (1 << lane)) counters[UNORDERED]++;
                if (!(mask_unord & (1 << lane))) counters[ORDERED]++;
                if ((mask_unord | mask_eq) & (1 << lane)) counters[UNEQ]++;
                if (mask_nlt & (1 << lane)) counters[UNGE]++;
                if (mask_nle & (1 << lane)) counters[UNGT]++;
                if ((mask_unord | mask_lt | mask_eq) & (1 << lane)) counters[UNLE]++;
                if ((mask_unord | mask_lt) & (1 << lane)) counters[UNLT]++;
                if ((mask_lt | mask_gt) & (1 << lane)) counters[LTGT]++;
            }
        }
    }
    
    /* AVX vector comparisons if available */
#ifdef __AVX__
    printf("Testing AVX vector conditions...\n");
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            __m256d a = vec256_data[i];
            __m256d b = vec256_data[j];
            
            /* AVX comparison predicates */
            __m256d cmp_unord = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            __m256d cmp_eq = _mm256_cmp_pd(a, b, _CMP_EQ_OQ);
            __m256d cmp_lt = _mm256_cmp_pd(a, b, _CMP_LT_OQ);
            __m256d cmp_le = _mm256_cmp_pd(a, b, _CMP_LE_OQ);
            __m256d cmp_gt = _mm256_cmp_pd(a, b, _CMP_GT_OQ);
            __m256d cmp_ge = _mm256_cmp_pd(a, b, _CMP_GE_OQ);
            __m256d cmp_neq = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            __m256d cmp_nlt = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);  /* UNGE */
            __m256d cmp_nle = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);  /* UNGT */
            
            int mask_unord = _mm256_movemask_pd(cmp_unord);
            int mask_eq = _mm256_movemask_pd(cmp_eq);
            int mask_lt = _mm256_movemask_pd(cmp_lt);
            int mask_gt = _mm256_movemask_pd(cmp_gt);
            int mask_nlt = _mm256_movemask_pd(cmp_nlt);
            int mask_nle = _mm256_movemask_pd(cmp_nle);
            
            for (int lane = 0; lane < 4; lane++) {
                if (mask_unord & (1 << lane)) counters[UNORDERED]++;
                if (!(mask_unord & (1 << lane))) counters[ORDERED]++;
                if ((mask_unord | mask_eq) & (1 << lane)) counters[UNEQ]++;
                if (mask_nlt & (1 << lane)) counters[UNGE]++;
                if (mask_nle & (1 << lane)) counters[UNGT]++;
                if ((mask_unord | mask_lt | mask_eq) & (1 << lane)) counters[UNLE]++;
                if ((mask_unord | mask_lt) & (1 << lane)) counters[UNLT]++;
                if ((mask_lt | mask_gt) & (1 << lane)) counters[LTGT]++;
            }
        }
    }
#endif
}

/* Inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    printf("Testing inline assembly with condition codes...\n");
    
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    double d = 2.0;
    int result;
    
    /* Test various condition codes via inline assembly */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[UNORDERED]++;
    
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (d)
        : "al"
    );
    if (result) counters[ORDERED]++;
    
    /* Test with explicit condition code strings in constraints */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@ueq"(result)
        : "x"(a), "x"(c)
    );
    if (result) counters[UNEQ]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@unord"(result)
        : "x"(a), "x"(b)
    );
    if (result) counters[UNORDERED]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@nlt"(result)
        : "x"(c), "x"(a)
    );
    if (result) counters[UNGE]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@nle"(result)
        : "x"(c), "x"(a)
    );
    if (result) counters[UNGT]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@ule"(result)
        : "x"(a), "x"(c)
    );
    if (result) counters[UNLE]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@ult"(result)
        : "x"(a), "x"(c)
    );
    if (result) counters[UNLT]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@une"(result)
        : "x"(a), "x"(c)
    );
    if (result) counters[LTGT]++;
}

/* Control flow that depends on comparison results */
void test_control_flow(void) {
    printf("Testing control flow dependent on comparisons...\n");
    
    volatile double vals[] = {1.0, __builtin_nan(""), 2.0, 0.0};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            volatile double x = vals[i];
            volatile double y = vals[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(x, y)) {
                counters[UNORDERED]++;
                if (__builtin_isless(x, y)) {
                    /* This path should rarely be taken with NaN */
                    counters[UNLT]--;
                }
            } else if (__builtin_isless(x, y)) {
                counters[UNLT]++;
                if (__builtin_isgreater(x, y)) {
                    counters[LTGT] += 2;
                }
            } else if (__builtin_isgreater(x, y)) {
                counters[LTGT]++;
            } else if (__builtin_islessequal(x, y)) {
                counters[UNLE]++;
            } else if (__builtin_isgreaterequal(x, y)) {
                counters[UNGE]++;
            }
            
            /* Switch statement based on comparison classification */
            int cmp_class = -1;
            if (__builtin_isunordered(x, y)) {
                cmp_class = UNORDERED;
            } else if (x == y) {
                cmp_class = UNEQ;
            } else if (x < y) {
                cmp_class = UNLT;
            } else if (x > y) {
                cmp_class = LTGT;  /* Using LTGT for ordered greater */
            }
            
            switch (cmp_class) {
                case UNORDERED:
                    counters[UNORDERED] += 2;
                    break;
                case UNEQ:
                    counters[UNEQ] += 2;
                    break;
                case UNLT:
                    counters[UNLT] += 2;
                    break;
                case LTGT:
                    counters[LTGT] += 2;
                    break;
                default:
                    counters[ORDERED] += 2;
                    break;
            }
        }
    }
}

int main(void) {
    printf("=== Testing x86 Floating-Point Condition Codes ===\n\n");
    
    /* Initialize test vectors */
    init_test_vectors();
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary */
    printf("\n=== Condition Code Summary ===\n");
    printf("UNORDERED: %d\n", counters[UNORDERED]);
    printf("ORDERED:   %d\n", counters[ORDERED]);
    printf("UNEQ:      %d\n", counters[UNEQ]);
    printf("UNGE:      %d\n", counters[UNGE]);
    printf("UNGT:      %d\n", counters[UNGT]);
    printf("UNLE:      %d\n", counters[UNLE]);
    printf("UNLT:      %d\n", counters[UNLT]);
    printf("LTGT:      %d\n", counters[LTGT]);
    
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
