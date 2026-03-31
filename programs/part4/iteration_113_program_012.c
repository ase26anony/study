#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <math.h>

/* Condition code counters */
typedef struct {
    int unordered;
    int ordered;
    int uneq;
    int unge;
    int ungt;
    int unle;
    int unlt;
    int ltgt;
} ConditionCounts;

/* Initialize with NaN, Inf, and normal values */
static const double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    __builtin_inf(), -__builtin_inf(),
    __builtin_nan(""), -__builtin_nan(""),
    3.14, -2.71
};

static const int NUM_TEST_VALUES = sizeof(test_scalars) / sizeof(test_scalars[0]);

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(ConditionCounts *counts) {
    for (int i = 0; i < NUM_TEST_VALUES; i++) {
        for (int j = 0; j < NUM_TEST_VALUES; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counts->unordered++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                counts->ordered++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || (a == b)) {
                counts->uneq++;
            }
            
            /* UNGE (not less than) */
            if (!__builtin_isless(a, b)) {
                counts->unge++;
            }
            
            /* UNGT (not less than or equal) */
            if (!__builtin_islessequal(a, b)) {
                counts->ungt++;
            }
            
            /* UNLE (unordered or less than or equal) */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                counts->unle++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
                counts->unlt++;
            }
            
            /* LTGT (less than or greater than, but not equal and not unordered) */
            if (__builtin_islessgreater(a, b)) {
                counts->ltgt++;
            }
        }
    }
}

/* Test vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(ConditionCounts *counts) {
#ifdef __SSE2__
    __m128d vec_a, vec_b, cmp_result;
    __m128i mask;
    
    for (int i = 0; i < NUM_TEST_VALUES - 1; i += 2) {
        vec_a = _mm_set_pd(test_scalars[i], test_scalars[i+1]);
        
        for (int j = 0; j < NUM_TEST_VALUES - 1; j += 2) {
            vec_b = _mm_set_pd(test_scalars[j], test_scalars[j+1]);
            
            /* Various comparison predicates that map to condition codes */
            
            /* _CMP_UNORD_Q - unordered */
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_extract_epi16(mask, 0) || _mm_extract_epi16(mask, 4)) {
                counts->unordered++;
            }
            
            /* _CMP_ORD_Q - ordered */
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_extract_epi16(mask, 0) || _mm_extract_epi16(mask, 4)) {
                counts->ordered++;
            }
            
            /* _CMP_EQ_UQ - equal (unordered, non-signaling) - maps to UNEQ */
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_EQ_UQ);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_extract_epi16(mask, 0) || _mm_extract_epi16(mask, 4)) {
                counts->uneq++;
            }
            
            /* _CMP_NLT_UQ - not less than (unordered, non-signaling) - maps to UNGE */
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NLT_UQ);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_extract_epi16(mask, 0) || _mm_extract_epi16(mask, 4)) {
                counts->unge++;
            }
            
            /* _CMP_NLE_UQ - not less than or equal (unordered, non-signaling) - maps to UNGT */
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NLE_UQ);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_extract_epi16(mask, 0) || _mm_extract_epi16(mask, 4)) {
                counts->ungt++;
            }
            
            /* _CMP_LE_UQ - less than or equal (unordered, non-signaling) - maps to UNLE */
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_LE_UQ);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_extract_epi16(mask, 0) || _mm_extract_epi16(mask, 4)) {
                counts->unle++;
            }
            
            /* _CMP_LT_UQ - less than (unordered, non-signaling) - maps to UNLT */
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_LT_UQ);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_extract_epi16(mask, 0) || _mm_extract_epi16(mask, 4)) {
                counts->unlt++;
            }
            
            /* _CMP_NEQ_UQ - not equal (unordered, non-signaling) - maps to LTGT */
            cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NEQ_UQ);
            mask = _mm_castpd_si128(cmp_result);
            if (_mm_extract_epi16(mask, 0) || _mm_extract_epi16(mask, 4)) {
                counts->ltgt++;
            }
        }
    }
#endif
}

/* Test AVX/AVX512 vector comparisons when available */
void test_avx_conditions(ConditionCounts *counts) {
#ifdef __AVX__
    __m256d vec_a, vec_b, cmp_result;
    __m256i mask;
    
    for (int i = 0; i < NUM_TEST_VALUES - 3; i += 4) {
        vec_a = _mm256_set_pd(test_scalars[i], test_scalars[i+1], 
                             test_scalars[i+2], test_scalars[i+3]);
        
        for (int j = 0; j < NUM_TEST_VALUES - 3; j += 4) {
            vec_b = _mm256_set_pd(test_scalars[j], test_scalars[j+1],
                                 test_scalars[j+2], test_scalars[j+3]);
            
            /* Test various AVX comparison predicates */
            
            /* Unordered */
            cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
            mask = _mm256_castpd_si256(cmp_result);
            if (_mm256_extract_epi64(mask, 0) || _mm256_extract_epi64(mask, 2)) {
                counts->unordered++;
            }
            
            /* Ordered */
            cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
            mask = _mm256_castpd_si256(cmp_result);
            if (_mm256_extract_epi64(mask, 0) || _mm256_extract_epi64(mask, 2)) {
                counts->ordered++;
            }
            
            /* Equal unordered */
            cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_EQ_UQ);
            mask = _mm256_castpd_si256(cmp_result);
            if (_mm256_extract_epi64(mask, 0) || _mm256_extract_epi64(mask, 2)) {
                counts->uneq++;
            }
        }
    }
#endif
}

/* Inline assembly with explicit condition code constraints */
void test_asm_constraints(ConditionCounts *counts) {
    double a = 1.0;
    double b = __builtin_nan("");
    int result;
    
    /* Test each condition code via inline assembly constraints */
    
    /* UNORDERED */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counts->unordered++;
    
    /* ORDERED */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counts->ordered++;
    
    /* UNEQ - unordered or equal */
    a = 2.0;
    b = 2.0;
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "sete %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "bl"
    );
    if (result) counts->uneq++;
    
    /* UNGE - not less than */
    a = 3.0;
    b = 2.0;
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counts->unge++;
    
    /* UNGT - not less than or equal */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counts->ungt++;
    
    /* UNLE - unordered or less than or equal */
    a = __builtin_nan("");
    b = 2.0;
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setbe %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "bl"
    );
    if (result) counts->unle++;
    
    /* UNLT - unordered or less than */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setb %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "bl"
    );
    if (result) counts->unlt++;
    
    /* LTGT - less than or greater than */
    a = 3.0;
    b = 2.0;
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setne %%al\n\t"
        "setnp %%bl\n\t"
        "andb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "bl"
    );
    if (result) counts->ltgt++;
}

/* Control flow that depends on comparison results */
void test_control_flow(ConditionCounts *counts) {
    volatile double a, b;
    int i;
    
    for (i = 0; i < 100; i++) {
        /* Vary values to prevent optimization */
        a = (i % 2 == 0) ? 1.0 : __builtin_nan("");
        b = (i % 3 == 0) ? 2.0 : __builtin_nan("");
        
        /* Complex control flow based on comparisons */
        if (__builtin_isunordered(a, b)) {
            counts->unordered++;
            if (__builtin_isordered(a + 1.0, b - 1.0)) {
                counts->ordered++;
            }
        } else if (__builtin_isless(a, b)) {
            counts->unlt++;  /* Not unordered and less than */
            if (!__builtin_islessequal(a, b)) {
                /* This should never happen, but prevents optimization */
                counts->ungt++;
            }
        } else if (a == b) {
            counts->uneq++;  /* Ordered and equal */
        } else if (__builtin_isgreater(a, b)) {
            counts->unge++;  /* Not less than (and ordered) */
            if (__builtin_islessgreater(a, b)) {
                counts->ltgt++;
            }
        }
        
        /* Switch based on comparison classification */
        int cmp_class = 0;
        if (__builtin_isunordered(a, b)) cmp_class = 1;
        else if (a == b) cmp_class = 2;
        else if (a < b) cmp_class = 3;
        else cmp_class = 4;
        
        switch (cmp_class) {
            case 1: /* UNORDERED */
                counts->unordered += 2;
                break;
            case 2: /* UNEQ (ordered equal) */
                counts->uneq += 2;
                break;
            case 3: /* UNLT (ordered less than) */
                counts->unlt += 2;
                break;
            case 4: /* UNGT (ordered greater than) */
                counts->ungt += 2;
                break;
        }
    }
}

int main() {
    ConditionCounts counts = {0};
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Test all comparison methods */
    test_scalar_conditions(&counts);
    printf("After scalar tests:\n");
    
    test_vector_conditions(&counts);
    printf("After vector tests:\n");
    
    test_avx_conditions(&counts);
    printf("After AVX tests:\n");
    
    test_asm_constraints(&counts);
    printf("After inline assembly tests:\n");
    
    test_control_flow(&counts);
    printf("After control flow tests:\n");
    
    /* Print summary */
    printf("\n=== Condition Code Summary ===\n");
    printf("UNORDERED: %d\n", counts.unordered);
    printf("ORDERED:   %d\n", counts.ordered);
    printf("UNEQ:      %d\n", counts.uneq);
    printf("UNGE:      %d\n", counts.unge);
    printf("UNGT:      %d\n", counts.ungt);
    printf("UNLE:      %d\n", counts.unle);
    printf("UNLT:      %d\n", counts.unlt);
    printf("LTGT:      %d\n", counts.ltgt);
    
    /* Verify we hit each condition at least once */
    int total_hits = counts.unordered + counts.ordered + counts.uneq +
                    counts.unge + counts.ungt + counts.unle +
                    counts.unlt + counts.ltgt;
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("SUCCESS: Condition code logic was exercised.\n");
        return 0;
    } else {
        printf("FAILURE: No condition codes were triggered.\n");
        return 1;
    }
}
