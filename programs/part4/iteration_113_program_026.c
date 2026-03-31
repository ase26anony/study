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
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    INFINITY, -INFINITY,
    __builtin_nan(""), -__builtin_nan(""),
    DBL_MAX, DBL_MIN
};

#define TEST_COUNT (sizeof(test_scalars)/sizeof(test_scalars[0]))

/* ==================== SCALAR BUILTIN TESTS ==================== */

void test_scalar_builtins(void) {
    for (int i = 0; i < TEST_COUNT; i++) {
        for (int j = 0; j < TEST_COUNT; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
            }
            
            /* ORDERED */
            if (!__builtin_isunordered(a, b)) {
                counters[ORDERED_IDX]++;
            }
            
            /* UNEQ (unordered or equal) */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[UNEQ_IDX]++;
            }
            
            /* UNGE (not less than) */
            if (!(a < b)) {
                counters[UNGE_IDX]++;
            }
            
            /* UNGT (not less than or equal) */
            if (!(a <= b)) {
                counters[UNGT_IDX]++;
            }
            
            /* UNLE (unordered or less than or equal) */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters[UNLE_IDX]++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || a < b) {
                counters[UNLT_IDX]++;
            }
            
            /* LTGT (less than or greater than, but not equal and not unordered) */
            if (a != b && !__builtin_isunordered(a, b)) {
                counters[LTGT_IDX]++;
            }
        }
    }
}

/* ==================== VECTOR TESTS ==================== */

#ifdef __SSE2__
void test_sse2_vectors(void) {
    __m128d vec_a, vec_b, vec_cmp;
    __m128d zero = _mm_setzero_pd();
    __m128d nan_vec = _mm_set1_pd(__builtin_nan(""));
    __m128d inf_vec = _mm_set1_pd(INFINITY);
    
    for (int i = 0; i < TEST_COUNT - 1; i += 2) {
        vec_a = _mm_set_pd(test_scalars[i], test_scalars[i+1]);
        vec_b = _mm_set_pd(test_scalars[i+1], test_scalars[i]);
        
        /* Various comparison predicates that map to condition codes */
        vec_cmp = _mm_cmpord_pd(vec_a, vec_b);  /* ORDERED */
        if (_mm_movemask_pd(vec_cmp) == 0x3) counters[ORDERED_IDX]++;
        
        vec_cmp = _mm_cmpunord_pd(vec_a, vec_b); /* UNORDERED */
        if (_mm_movemask_pd(vec_cmp) == 0x3) counters[UNORDERED_IDX]++;
        
        vec_cmp = _mm_cmpeq_pd(vec_a, vec_b);   /* EQ - used for UNEQ */
        if (_mm_movemask_pd(vec_cmp) == 0x3) counters[UNEQ_IDX]++;
        
        vec_cmp = _mm_cmpnlt_pd(vec_a, vec_b);  /* NLT - maps to UNGE */
        if (_mm_movemask_pd(vec_cmp) == 0x3) counters[UNGE_IDX]++;
        
        vec_cmp = _mm_cmpnle_pd(vec_a, vec_b);  /* NLE - maps to UNGT */
        if (_mm_movemask_pd(vec_cmp) == 0x3) counters[UNGT_IDX]++;
        
        vec_cmp = _mm_cmple_pd(vec_a, vec_b);   /* LE - used for UNLE */
        if (_mm_movemask_pd(vec_cmp) == 0x3) counters[UNLE_IDX]++;
        
        vec_cmp = _mm_cmplt_pd(vec_a, vec_b);   /* LT - used for UNLT */
        if (_mm_movemask_pd(vec_cmp) == 0x3) counters[UNLT_IDX]++;
        
        vec_cmp = _mm_cmpneq_pd(vec_a, vec_b);  /* NEQ - maps to LTGT */
        if (_mm_movemask_pd(vec_cmp) == 0x3) counters[LTGT_IDX]++;
    }
}
#endif

#ifdef __AVX__
void test_avx_vectors(void) {
    __m256d vec_a, vec_b, vec_cmp;
    __m256d nan_vec = _mm256_set1_pd(__builtin_nan(""));
    
    /* Test with NaN to force unordered conditions */
    vec_a = _mm256_set_pd(test_scalars[0], test_scalars[1], 
                          test_scalars[6], test_scalars[7]); /* Includes NaN */
    vec_b = _mm256_set_pd(test_scalars[1], test_scalars[0],
                          test_scalars[7], test_scalars[6]);
    
    /* AVX comparison intrinsics with explicit predicates */
    vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);  /* UNORDERED */
    if (_mm256_movemask_pd(vec_cmp) == 0xF) counters[UNORDERED_IDX]++;
    
    vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);    /* ORDERED */
    if (_mm256_movemask_pd(vec_cmp) == 0xF) counters[ORDERED_IDX]++;
    
    vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_EQ_UQ);    /* UNEQ */
    if (_mm256_movemask_pd(vec_cmp) == 0xF) counters[UNEQ_IDX]++;
    
    vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_NLT_UQ);   /* UNGE */
    if (_mm256_movemask_pd(vec_cmp) == 0xF) counters[UNGE_IDX]++;
    
    vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_NLE_UQ);   /* UNGT */
    if (_mm256_movemask_pd(vec_cmp) == 0xF) counters[UNGT_IDX]++;
    
    vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_LE_OS);    /* UNLE */
    if (_mm256_movemask_pd(vec_cmp) == 0xF) counters[UNLE_IDX]++;
    
    vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_LT_OS);    /* UNLT */
    if (_mm256_movemask_pd(vec_cmp) == 0xF) counters[UNLT_IDX]++;
    
    vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_NEQ_OS);   /* LTGT */
    if (_mm256_movemask_pd(vec_cmp) == 0xF) counters[LTGT_IDX]++;
}
#endif

/* ==================== INLINE ASSEMBLY TESTS ==================== */

void test_asm_constraints(void) {
    double a = 1.0;
    double b = 2.0;
    double nan = __builtin_nan("");
    int result;
    
    /* Test each condition code via inline assembly constraints */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (nan), "x" (a)
        : "al"
    );
    if (result) counters[UNORDERED_IDX]++;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[ORDERED_IDX]++;
    
    /* UNEQ */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (a)  /* equal values */
        : "al"
    );
    if (result) counters[UNEQ_IDX]++;
    
    /* UNGE (nlt) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[UNGE_IDX]++;
    
    /* UNGT (nle) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (b), "x" (a)  /* b > a */
        : "al"
    );
    if (result) counters[UNGT_IDX]++;
    
    /* UNLE */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[UNLE_IDX]++;
    
    /* UNLT */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[UNLT_IDX]++;
    
    /* LTGT (une) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[LTGT_IDX]++;
}

/* ==================== CONTROL FLOW TESTS ==================== */

void test_control_flow(void) {
    double values[] = {1.0, __builtin_nan(""), 2.0, 0.0};
    int n = sizeof(values)/sizeof(values[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow that depends on comparisons */
            if (__builtin_isunordered(a, b)) {
                counters[UNORDERED_IDX]++;
                if (a != b) {
                    counters[LTGT_IDX]++;  /* This won't execute for unordered */
                }
            } else if (a == b) {
                counters[UNEQ_IDX]++;
            } else if (a < b) {
                counters[UNLT_IDX]++;
                if (!(a >= b)) {
                    counters[UNGE_IDX]++;  /* This will execute */
                }
            } else if (a > b) {
                counters[UNGT_IDX]++;
                if (!(a <= b)) {
                    counters[UNLE_IDX]++;  /* This will execute */
                }
            }
            
            /* Switch-like behavior based on comparison class */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 0;
            else if (a == b) cmp_class = 1;
            else if (a < b) cmp_class = 2;
            else cmp_class = 3;
            
            switch (cmp_class) {
                case 0:
                    counters[UNORDERED_IDX]++;
                    break;
                case 1:
                    counters[ORDERED_IDX]++;  /* Ordered and equal */
                    counters[UNEQ_IDX]++;
                    break;
                case 2:
                    counters[ORDERED_IDX]++;  /* Ordered and less */
                    counters[UNLT_IDX]++;
                    break;
                case 3:
                    counters[ORDERED_IDX]++;  /* Ordered and greater */
                    counters[UNGT_IDX]++;
                    break;
            }
        }
    }
}

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Initialize counters */
    memset(counters, 0, sizeof(counters));
    
    /* Run all tests */
    test_scalar_builtins();
    
#ifdef __SSE2__
    test_sse2_vectors();
#endif
    
#ifdef __AVX__
    test_avx_vectors();
#endif
    
    test_asm_constraints();
    test_control_flow();
    
    /* Print results */
    printf("\nCondition code execution counts:\n");
    printf("UNORDERED (unord): %d\n", counters[UNORDERED_IDX]);
    printf("ORDERED   (ord):   %d\n", counters[ORDERED_IDX]);
    printf("UNEQ      (ueq):   %d\n", counters[UNEQ_IDX]);
    printf("UNGE      (nlt):   %d\n", counters[UNGE_IDX]);
    printf("UNGT      (nle):   %d\n", counters[UNGT_IDX]);
    printf("UNLE      (ule):   %d\n", counters[UNLE_IDX]);
    printf("UNLT      (ult):   %d\n", counters[UNLT_IDX]);
    printf("LTGT      (une):   %d\n", counters[LTGT_IDX]);
    
    /* Verify all condition codes were exercised */
    int all_nonzero = 1;
    for (int i = 0; i < 8; i++) {
        if (counters[i] == 0) {
            printf("Warning: Condition code %d was not exercised!\n", i);
            all_nonzero = 0;
        }
    }
    
    if (all_nonzero) {
        printf("\nSUCCESS: All condition code paths were executed!\n");
    } else {
        printf("\nWARNING: Some condition codes were not exercised.\n");
        printf("Try compiling with different optimization flags:\n");
        printf("  gcc -O0 -msse2 test.c -o test_sse2_O0\n");
        printf("  gcc -O2 -mavx test.c -o test_avx_O2\n");
        printf("  gcc -O3 -mavx512f test.c -o test_avx512_O3\n");
    }
    
    return 0;
}
