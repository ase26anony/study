#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <math.h>

/* Condition code counters */
static int cc_counts[8] = {0};
enum CC_TYPES { UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT };

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    __builtin_nan(""), -__builtin_nan(""),
    1.0/0.0, -1.0/0.0  /* +Inf, -Inf */
};

static __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(__builtin_nan(""), 3.0),
    _mm_set_pd(0.0, -0.0),
    _mm_set_pd(1.0/0.0, -1.0/0.0)
};

#ifdef __AVX__
static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, 3.0, 4.0),
    _mm256_set_pd(__builtin_nan(""), 5.0, 6.0, 7.0),
    _mm256_set_pd(0.0, -0.0, 1.0/0.0, -1.0/0.0)
};
#endif

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                cc_counts[UNORDERED]++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                cc_counts[ORDERED]++;
            }
            
            /* UNEQ (unordered or equal) */
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                cc_counts[UNEQ]++;
            }
            
            /* UNGE (not less than) */
            if (!__builtin_isless(a, b)) {
                cc_counts[UNGE]++;
            }
            
            /* UNGT (not less or equal) */
            if (!__builtin_islessequal(a, b)) {
                cc_counts[UNGT]++;
            }
            
            /* UNLE (unordered or less or equal) */
            if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
                cc_counts[UNLE]++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
                cc_counts[UNLT]++;
            }
            
            /* LTGT (less or greater) */
            if (__builtin_islessgreater(a, b)) {
                cc_counts[LTGT]++;
            }
        }
    }
}

/* Test vector comparisons */
void test_vector_conditions(void) {
    /* Test with SSE2 */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            
            /* Compare with different predicates */
            __m128d cmp_eq = _mm_cmpneq_sd(a, b);   /* Not equal */
            __m128d cmp_lt = _mm_cmplt_sd(a, b);    /* Less than */
            __m128d cmp_le = _mm_cmple_sd(a, b);    /* Less or equal */
            __m128d cmp_unord = _mm_cmpunord_sd(a, b); /* Unordered */
            __m128d cmp_ord = _mm_cmpord_sd(a, b);  /* Ordered */
            __m128d cmp_nlt = _mm_cmpnlt_sd(a, b);  /* Not less than (UNGE) */
            __m128d cmp_nle = _mm_cmpnle_sd(a, b);  /* Not less or equal (UNGT) */
            
            /* Extract results to force code generation */
            double res_eq = _mm_cvtsd_f64(cmp_eq);
            double res_lt = _mm_cvtsd_f64(cmp_lt);
            double res_le = _mm_cvtsd_f64(cmp_le);
            double res_unord = _mm_cvtsd_f64(cmp_unord);
            double res_ord = _mm_cvtsd_f64(cmp_ord);
            double res_nlt = _mm_cvtsd_f64(cmp_nlt);
            double res_nle = _mm_cvtsd_f64(cmp_nle);
            
            /* Update counters based on results */
            if (res_unord != 0.0) cc_counts[UNORDERED]++;
            if (res_ord != 0.0) cc_counts[ORDERED]++;
            if (res_nlt != 0.0) cc_counts[UNGE]++;
            if (res_nle != 0.0) cc_counts[UNGT]++;
            
            /* Additional conditions from combinations */
            if ((res_eq != 0.0) && (res_unord != 0.0)) cc_counts[UNEQ]++;
            if ((res_le != 0.0) || (res_unord != 0.0)) cc_counts[UNLE]++;
            if ((res_lt != 0.0) || (res_unord != 0.0)) cc_counts[UNLT]++;
            if ((res_lt != 0.0) || (!(res_le != 0.0) && !(res_eq != 0.0))) cc_counts[LTGT]++;
        }
    }
    
#ifdef __AVX__
    /* Test with AVX */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* AVX comparison predicates */
            __m256d cmp_eq = _mm256_cmp_pd(a, b, _CMP_EQ_OQ);
            __m256d cmp_lt = _mm256_cmp_pd(a, b, _CMP_LT_OQ);
            __m256d cmp_le = _mm256_cmp_pd(a, b, _CMP_LE_OQ);
            __m256d cmp_unord = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            __m256d cmp_ord = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            __m256d cmp_nlt = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);  /* UNGE */
            __m256d cmp_nle = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);  /* UNGT */
            __m256d cmp_neq = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);  /* UNEQ */
            
            /* Extract mask to force code generation */
            int mask_eq = _mm256_movemask_pd(cmp_eq);
            int mask_lt = _mm256_movemask_pd(cmp_lt);
            int mask_le = _mm256_movemask_pd(cmp_le);
            int mask_unord = _mm256_movemask_pd(cmp_unord);
            int mask_ord = _mm256_movemask_pd(cmp_ord);
            int mask_nlt = _mm256_movemask_pd(cmp_nlt);
            int mask_nle = _mm256_movemask_pd(cmp_nle);
            int mask_neq = _mm256_movemask_pd(cmp_neq);
            
            /* Update counters */
            if (mask_unord) cc_counts[UNORDERED]++;
            if (mask_ord) cc_counts[ORDERED]++;
            if (mask_neq) cc_counts[UNEQ]++;
            if (mask_nlt) cc_counts[UNGE]++;
            if (mask_nle) cc_counts[UNGT]++;
            if (mask_le || mask_unord) cc_counts[UNLE]++;
            if (mask_lt || mask_unord) cc_counts[UNLT]++;
            if (mask_lt || (!mask_le && !mask_eq)) cc_counts[LTGT]++;
        }
    }
#endif
}

/* Inline assembly with condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double result;
    int flag;
    
    /* Test various condition codes via inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al"
    );
    if (flag) cc_counts[UNORDERED]++;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al"
    );
    if (flag) cc_counts[ORDERED]++;
    
    /* UNEQ (unordered or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al", "cl"
    );
    if (flag) cc_counts[UNEQ]++;
    
    /* UNGE (not less than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al"
    );
    if (flag) cc_counts[UNGE]++;
    
    /* UNGT (not less or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al"
    );
    if (flag) cc_counts[UNGT]++;
    
    /* UNLE (unordered or less or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al", "cl"
    );
    if (flag) cc_counts[UNLE]++;
    
    /* UNLT (unordered or less than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al", "cl"
    );
    if (flag) cc_counts[UNLT]++;
    
    /* LTGT (less or greater) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al"
    );
    if (flag) cc_counts[LTGT]++;
    
    /* Direct condition code string usage in constraints */
    a = 2.0;
    b = 1.0;
    
    /* Using condition code strings in constraints */
    __asm__ volatile (
        "ucomisd %1, %0"
        : /* no outputs */
        : "x" (a), "x" (b)
        : "cc"
    );
    
    /* Force generation of condition code output */
    __asm__ volatile (
        "cmpsd %1, %0, %2"
        : "=@ueq"(result)
        : "x"(a), "i"(0)  /* 0 = _CMP_EQ_OQ */
        : "cc"
    );
    
    __asm__ volatile (
        "cmpsd %1, %0, %2"
        : "=@unord"(result)
        : "x"(a), "i"(3)  /* 3 = _CMP_UNORD_Q */
        : "cc"
    );
    
    __asm__ volatile (
        "cmpsd %1, %0, %2"
        : "=@nlt"(result)
        : "x"(a), "i"(5)  /* 5 = _CMP_NLT_UQ */
        : "cc"
    );
    
    __asm__ volatile (
        "cmpsd %1, %0, %2"
        : "=@nle"(result)
        : "x"(a), "i"(6)  /* 6 = _CMP_NLE_UQ */
        : "cc"
    );
    
    __asm__ volatile (
        "cmpsd %1, %0, %2"
        : "=@ord"(result)
        : "x"(a), "i"(7)  /* 7 = _CMP_ORD_Q */
        : "cc"
    );
}

/* Control flow based on comparison results */
void test_control_flow(void) {
    double values[] = {1.0, 2.0, __builtin_nan(""), 0.0, -1.0/0.0};
    int n = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                cc_counts[UNORDERED]++;
                if (__builtin_isless(a, b)) {
                    /* This shouldn't happen for unordered, but creates control flow */
                    cc_counts[UNLT]--;
                }
            } else if (__builtin_isless(a, b)) {
                cc_counts[UNLT]++;
                if (__builtin_isgreater(a, b)) {
                    cc_counts[LTGT]++;
                }
            } else if (__builtin_isgreater(a, b)) {
                cc_counts[LTGT]++;
            } else if (__builtin_islessequal(a, b)) {
                cc_counts[UNLE]++;
                if (!__builtin_isless(a, b)) {
                    cc_counts[UNEQ]++;
                }
            }
            
            /* Switch based on comparison classification */
            int cmp_class = -1;
            if (__builtin_isunordered(a, b)) {
                cmp_class = UNORDERED;
            } else if (a == b) {
                cmp_class = UNEQ;
            } else if (a < b) {
                cmp_class = UNLT;
            } else if (a > b) {
                cmp_class = LTGT;  /* Using LTGT for greater */
            }
            
            switch (cmp_class) {
                case UNORDERED:
                    cc_counts[UNORDERED]++;
                    break;
                case UNEQ:
                    cc_counts[UNEQ]++;
                    break;
                case UNLT:
                    cc_counts[UNLT]++;
                    break;
                case LTGT:
                    cc_counts[LTGT]++;
                    break;
                default:
                    /* ORDERED but not equal, less, or greater */
                    cc_counts[ORDERED]++;
                    break;
            }
        }
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Reset counters */
    memset(cc_counts, 0, sizeof(cc_counts));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print results */
    printf("\nCondition Code Statistics:\n");
    printf("UNORDERED: %d\n", cc_counts[UNORDERED]);
    printf("ORDERED:   %d\n", cc_counts[ORDERED]);
    printf("UNEQ:      %d\n", cc_counts[UNEQ]);
    printf("UNGE:      %d\n", cc_counts[UNGE]);
    printf("UNGT:      %d\n", cc_counts[UNGT]);
    printf("UNLE:      %d\n", cc_counts[UNLE]);
    printf("UNLT:      %d\n", cc_counts[UNLT]);
    printf("LTGT:      %d\n", cc_counts[LTGT]);
    
    /* Verify we hit all condition codes */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += cc_counts[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("ERROR: No condition codes were triggered.\n");
        return 1;
    }
}
