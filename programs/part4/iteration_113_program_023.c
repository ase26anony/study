#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Result counters for each condition code type */
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

/* Test data with normal numbers, infinities, and NaNs */
static const double test_scalars[] = {
    1.0, 2.0, -1.0, -2.0,
    0.0, -0.0, DBL_MAX, -DBL_MAX,
    INFINITY, -INFINITY,
    __builtin_nan(""), -__builtin_nan(""),
    3.14, -3.14
};

static const int num_scalars = sizeof(test_scalars) / sizeof(test_scalars[0]);

/* Vector test data */
static __m128d vec_data[8];
static __m256d vec256_data[4];

/* Initialize vector test data */
void init_vector_data(void) {
    for (int i = 0; i < 8; i++) {
        double a = test_scalars[i * 2];
        double b = test_scalars[i * 2 + 1];
        vec_data[i] = _mm_set_pd(a, b);
    }
    
    for (int i = 0; i < 4; i++) {
        double a = test_scalars[i * 4];
        double b = test_scalars[i * 4 + 1];
        double c = test_scalars[i * 4 + 2];
        double d = test_scalars[i * 4 + 3];
        vec256_data[i] = _mm256_set_pd(a, b, c, d);
    }
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (int i = 0; i < num_scalars; i++) {
        for (int j = 0; j < num_scalars; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED - using __builtin_isunordered */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
            }
            
            /* ORDERED - using !__builtin_isunordered */
            if (!__builtin_isunordered(a, b)) {
                counters[ORDERED_IDX]++;
            }
            
            /* UNEQ - unordered or equal */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[UNEQ_IDX]++;
            }
            
            /* UNGE - unordered or greater-or-equal */
            if (__builtin_isunordered(a, b) || a >= b) {
                counters[UNGE_IDX]++;
            }
            
            /* UNGT - unordered or greater */
            if (__builtin_isunordered(a, b) || a > b) {
                counters[UNGT_IDX]++;
            }
            
            /* UNLE - unordered or less-or-equal */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters[UNLE_IDX]++;
            }
            
            /* UNLT - unordered or less */
            if (__builtin_isunordered(a, b) || a < b) {
                counters[UNLT_IDX]++;
            }
            
            /* LTGT - less or greater (but not equal, not unordered) */
            if (!__builtin_isunordered(a, b) && a != b) {
                counters[LTGT_IDX]++;
            }
        }
    }
}

/* Test vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    /* SSE2 vector comparisons */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            __m128d a = vec_data[i];
            __m128d b = vec_data[j];
            
            /* Compare with different predicates */
            __m128d cmp_unord = _mm_cmpunord_pd(a, b);  /* UNORDERED */
            __m128d cmp_eq = _mm_cmpeq_pd(a, b);        /* EQ */
            __m128d cmp_lt = _mm_cmplt_pd(a, b);        /* LT */
            __m128d cmp_le = _mm_cmple_pd(a, b);        /* LE */
            __m128d cmp_neq = _mm_cmpneq_pd(a, b);      /* NEQ */
            __m128d cmp_nlt = _mm_cmpnlt_pd(a, b);      /* NLT (UNGE) */
            __m128d cmp_nle = _mm_cmpnle_pd(a, b);      /* NLE (UNGT) */
            
            /* Extract results to affect control flow */
            double res_unord[2], res_eq[2], res_lt[2];
            _mm_storeu_pd(res_unord, cmp_unord);
            _mm_storeu_pd(res_eq, cmp_eq);
            _mm_storeu_pd(res_lt, cmp_lt);
            
            /* Force control flow based on comparison results */
            if (res_unord[0] != 0.0 || res_unord[1] != 0.0) {
                counters[UNORDERED_IDX]++;
            }
            
            if (res_eq[0] != 0.0 && !__builtin_isunordered(res_eq[0], 0.0)) {
                counters[UNEQ_IDX]++;  /* EQ case contributes to UNEQ */
            }
            
            if (res_lt[0] != 0.0) {
                counters[UNLT_IDX]++;  /* LT case contributes to UNLT */
            }
        }
    }
    
    /* AVX vector comparisons if available */
#ifdef __AVX__
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            __m256d a = vec256_data[i];
            __m256d b = vec256_data[j];
            
            /* AVX comparison predicates */
            __m256d cmp_ord = _mm256_cmp_pd(a, b, _CMP_ORD_Q);   /* ORDERED */
            __m256d cmp_unord = _mm256_cmp_pd(a, b, _CMP_UNORD_Q); /* UNORDERED */
            __m256d cmp_eq_uq = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);  /* EQ (unordered quiet) */
            __m256d cmp_nge_us = _mm256_cmp_pd(a, b, _CMP_NGE_US); /* NGE (unordered signaling) */
            __m256d cmp_ngt_us = _mm256_cmp_pd(a, b, _CMP_NGT_US); /* NGT (unordered signaling) */
            __m256d cmp_false_oq = _mm256_cmp_pd(a, b, _CMP_FALSE_OQ); /* FALSE (ordered quiet) */
            
            /* Extract and use results */
            double res_ord[4], res_unord[4];
            _mm256_storeu_pd(res_ord, cmp_ord);
            _mm256_storeu_pd(res_unord, cmp_unord);
            
            for (int k = 0; k < 4; k++) {
                if (res_ord[k] != 0.0) {
                    counters[ORDERED_IDX]++;
                }
                if (res_unord[k] != 0.0) {
                    counters[UNORDERED_IDX]++;
                }
            }
        }
    }
#endif
}

/* Test inline assembly with condition code constraints */
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
    if (result) counters[UNORDERED_IDX]++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (d)
        : "al", "cc"
    );
    if (result) counters[ORDERED_IDX]++;
    
    /* UNEQ constraint - unordered or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "sete %%dl\n\t"
        "or %%dl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (d)
        : "al", "dl", "cc"
    );
    if (result) counters[UNEQ_IDX]++;
    
    /* UNGE constraint - unordered or not less than */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "setnb %%dl\n\t"
        "or %%dl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "dl", "cc"
    );
    if (result) counters[UNGE_IDX]++;
    
    /* UNGT constraint - unordered or greater than */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "setnbe %%dl\n\t"
        "or %%dl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (c), "x" (a)
        : "al", "dl", "cc"
    );
    if (result) counters[UNGT_IDX]++;
    
    /* UNLE constraint - unordered or less or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "setna %%dl\n\t"
        "or %%dl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "dl", "cc"
    );
    if (result) counters[UNLE_IDX]++;
    
    /* UNLT constraint - unordered or less than */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "setb %%dl\n\t"
        "or %%dl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "dl", "cc"
    );
    if (result) counters[UNLT_IDX]++;
    
    /* LTGT constraint - not equal and ordered */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "setne %%dl\n\t"
        "andn %%dl, %%al, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al", "dl", "cc"
    );
    if (result) counters[LTGT_IDX]++;
}

/* Complex control flow based on comparison results */
void test_control_flow_conditions(void) {
    double values[] = {1.0, __builtin_nan(""), 2.0, -__builtin_nan(""), 0.0};
    int n = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Switch-like structure based on comparison classification */
            int cmp_class = 0;
            
            if (__builtin_isunordered(a, b)) {
                cmp_class = 1;  /* UNORDERED */
            } else if (a == b) {
                cmp_class = 2;  /* EQ */
            } else if (a < b) {
                cmp_class = 3;  /* LT */
            } else {
                cmp_class = 4;  /* GT */
            }
            
            /* Force different code paths */
            switch (cmp_class) {
                case 1:  /* UNORDERED */
                    counters[UNORDERED_IDX]++;
                    if (__builtin_isgreater(a, b) || __builtin_isless(a, b)) {
                        counters[LTGT_IDX]++;  /* This shouldn't happen for NaN */
                    }
                    break;
                case 2:  /* EQ */
                    counters[UNEQ_IDX]++;
                    counters[ORDERED_IDX]++;
                    break;
                case 3:  /* LT */
                    counters[UNLT_IDX]++;
                    counters[ORDERED_IDX]++;
                    if (!__builtin_islessequal(a, b)) {
                        counters[UNLE_IDX]++;  /* Should not happen */
                    }
                    break;
                case 4:  /* GT */
                    counters[UNGT_IDX]++;
                    counters[ORDERED_IDX]++;
                    if (!__builtin_isgreaterequal(a, b)) {
                        counters[UNGE_IDX]++;  /* Should not happen */
                    }
                    break;
            }
            
            /* Additional comparisons to hit more condition codes */
            if (__builtin_islessgreater(a, b)) {
                counters[LTGT_IDX]++;
            }
            
            if (__builtin_islessequal(a, b)) {
                counters[UNLE_IDX]++;
            }
            
            if (__builtin_isgreaterequal(a, b)) {
                counters[UNGE_IDX]++;
            }
        }
    }
}

int main(void) {
    /* Initialize test data */
    init_vector_data();
    
    /* Clear counters */
    memset(counters, 0, sizeof(counters));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow_conditions();
    
    /* Print summary */
    printf("Condition code execution summary:\n");
    printf("UNORDERED: %d\n", counters[UNORDERED_IDX]);
    printf("ORDERED: %d\n", counters[ORDERED_IDX]);
    printf("UNEQ: %d\n", counters[UNEQ_IDX]);
    printf("UNGE: %d\n", counters[UNGE_IDX]);
    printf("UNGT: %d\n", counters[UNGT_IDX]);
    printf("UNLE: %d\n", counters[UNLE_IDX]);
    printf("UNLT: %d\n", counters[UNLT_IDX]);
    printf("LTGT: %d\n", counters[LTGT_IDX]);
    
    /* Verify all condition codes were hit */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += counters[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits == 0) {
        fprintf(stderr, "ERROR: No condition codes were triggered!\n");
        return 1;
    }
    
    /* Check if all condition code types were hit */
    int all_hit = 1;
    for (int i = 0; i < 8; i++) {
        if (counters[i] == 0) {
            printf("Warning: Condition code %d was not hit\n", i);
            all_hit = 0;
        }
    }
    
    if (all_hit) {
        printf("SUCCESS: All condition code types were triggered!\n");
    }
    
    return 0;
}
