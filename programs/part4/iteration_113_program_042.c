#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <math.h>

/* Condition code counters */
static int counters[8] = {0};
enum {
    CC_UNORDERED = 0,
    CC_ORDERED,
    CC_UNEQ,
    CC_UNGE,
    CC_UNGT,
    CC_UNLE,
    CC_UNLT,
    CC_LTGT
};

/* Test data arrays */
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    1.0/0.0,          /* +Inf */
    -1.0/0.0,         /* -Inf */
    __builtin_nan(""), /* NaN */
    0.0/0.0           /* Another NaN */
};

static __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(-1.0, 0.0),
    _mm_set_pd(__builtin_inf(), -__builtin_inf()),
    _mm_set_pd(__builtin_nan(""), 3.0)
};

#ifdef __AVX__
static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, 3.0, 4.0),
    _mm256_set_pd(-1.0, 0.0, __builtin_nan(""), __builtin_inf())
};
#endif

/* Test function using GCC builtins */
void test_scalar_conditions(void) {
    int i, j;
    double nan = __builtin_nan("");
    double inf = __builtin_inf();
    
    for (i = 0; i < sizeof(test_scalars)/sizeof(test_scalars[0]); i++) {
        for (j = 0; j < sizeof(test_scalars)/sizeof(test_scalars[0]); j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED: a or b is NaN */
            if (__builtin_isunordered(a, b)) {
                counters[CC_UNORDERED]++;
            }
            
            /* ORDERED: neither is NaN */
            if (__builtin_isordered(a, b)) {
                counters[CC_ORDERED]++;
            }
            
            /* UNEQ: unordered or equal */
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                counters[CC_UNEQ]++;
            }
            
            /* UNGE: unordered or greater-or-equal */
            if (!__builtin_isless(a, b)) {
                counters[CC_UNGE]++;
            }
            
            /* UNGT: unordered or greater */
            if (!__builtin_islessequal(a, b)) {
                counters[CC_UNGT]++;
            }
            
            /* UNLE: unordered or less-or-equal */
            if (!__builtin_isgreater(a, b)) {
                counters[CC_UNLE]++;
            }
            
            /* UNLT: unordered or less */
            if (!__builtin_isgreaterequal(a, b)) {
                counters[CC_UNLT]++;
            }
            
            /* LTGT: less or greater (ordered and not equal) */
            if (__builtin_islessgreater(a, b)) {
                counters[CC_LTGT]++;
            }
        }
    }
}

/* Test function using vector comparisons */
void test_vector_conditions(void) {
    int i, j;
    
    for (i = 0; i < sizeof(test_vec128)/sizeof(test_vec128[0]); i++) {
        for (j = 0; j < sizeof(test_vec128)/sizeof(test_vec128[0]); j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            __m128d cmp_result;
            int mask;
            
            /* Various comparison predicates */
            cmp_result = _mm_cmpord_pd(a, b);  /* ORDERED */
            mask = _mm_movemask_pd(cmp_result);
            if (mask == 0x3) counters[CC_ORDERED]++;
            
            cmp_result = _mm_cmpunord_pd(a, b); /* UNORDERED */
            mask = _mm_movemask_pd(cmp_result);
            if (mask != 0) counters[CC_UNORDERED]++;
            
            cmp_result = _mm_cmpeq_pd(a, b);   /* EQ (part of UNEQ) */
            mask = _mm_movemask_pd(cmp_result);
            if (mask != 0) {
                /* Combine with unordered check for UNEQ */
                __m128d unord = _mm_cmpunord_pd(a, b);
                int unord_mask = _mm_movemask_pd(unord);
                if (unord_mask != 0) counters[CC_UNEQ]++;
            }
            
            cmp_result = _mm_cmpnlt_pd(a, b);  /* UNGE: not less than */
            mask = _mm_movemask_pd(cmp_result);
            if (mask != 0) counters[CC_UNGE]++;
            
            cmp_result = _mm_cmpnle_pd(a, b);  /* UNGT: not less or equal */
            mask = _mm_movemask_pd(cmp_result);
            if (mask != 0) counters[CC_UNGT]++;
            
            cmp_result = _mm_cmple_pd(a, b);   /* LE (part of UNLE) */
            mask = _mm_movemask_pd(cmp_result);
            if (mask != 0) {
                __m128d unord = _mm_cmpunord_pd(a, b);
                int unord_mask = _mm_movemask_pd(unord);
                if (unord_mask != 0) counters[CC_UNLE]++;
            }
            
            cmp_result = _mm_cmplt_pd(a, b);   /* LT (part of UNLT) */
            mask = _mm_movemask_pd(cmp_result);
            if (mask != 0) {
                __m128d unord = _mm_cmpunord_pd(a, b);
                int unord_mask = _mm_movemask_pd(unord);
                if (unord_mask != 0) counters[CC_UNLT]++;
            }
            
            cmp_result = _mm_cmpneq_pd(a, b);  /* NEQ (part of LTGT) */
            mask = _mm_movemask_pd(cmp_result);
            if (mask != 0) {
                __m128d ord = _mm_cmpord_pd(a, b);
                int ord_mask = _mm_movemask_pd(ord);
                if (ord_mask == 0x3) counters[CC_LTGT]++;
            }
        }
    }
}

/* Test function using inline assembly with condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
    double d = 3.0;
    int result;
    
    /* Force generation of condition code strings in assembly */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[CC_UNORDERED]++;
    
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (c)
        : "al"
    );
    if (result) counters[CC_ORDERED]++;
    
    /* Test various condition codes via constraints */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@ueq"(result)
        : "x"(a), "x"(a)
    );
    if (result) counters[CC_UNEQ]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@unord"(result)
        : "x"(a), "x"(b)
    );
    if (result) counters[CC_UNORDERED]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@nlt"(result)
        : "x"(c), "x"(a)
    );
    if (result) counters[CC_UNGE]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@nle"(result)
        : "x"(c), "x"(a)
    );
    if (result) counters[CC_UNGT]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@ule"(result)
        : "x"(a), "x"(c)
    );
    if (result) counters[CC_UNLE]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@ult"(result)
        : "x"(a), "x"(c)
    );
    if (result) counters[CC_UNLT]++;
    
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        : "=@une"(result)
        : "x"(a), "x"(d)
    );
    if (result) counters[CC_LTGT]++;
}

#ifdef __AVX__
void test_avx_conditions(void) {
    int i, j;
    
    for (i = 0; i < sizeof(test_vec256)/sizeof(test_vec256[0]); i++) {
        for (j = 0; j < sizeof(test_vec256)/sizeof(test_vec256[0]); j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            __m256d cmp_result;
            int mask;
            
            /* AVX comparison predicates */
            cmp_result = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            mask = _mm256_movemask_pd(cmp_result);
            if (mask == 0xF) counters[CC_ORDERED]++;
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CC_UNORDERED]++;
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);  /* UNGE */
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CC_UNGE]++;
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);  /* UNGT */
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CC_UNGT]++;
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);  /* UNEQ */
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CC_UNEQ]++;
        }
    }
}
#endif

/* Control flow test to prevent optimization */
void test_control_flow(void) {
    double values[] = {1.0, __builtin_nan(""), 2.0, __builtin_inf()};
    int i;
    
    for (i = 0; i < 4; i++) {
        double a = values[i];
        double b = values[(i + 1) % 4];
        
        /* Complex control flow based on comparisons */
        if (__builtin_isunordered(a, b)) {
            counters[CC_UNORDERED]++;
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                counters[CC_UNEQ]++;
            }
        } else {
            counters[CC_ORDERED]++;
            if (__builtin_islessgreater(a, b)) {
                counters[CC_LTGT]++;
            }
            if (!__builtin_isless(a, b)) {
                counters[CC_UNGE]++;
                if (!__builtin_islessequal(a, b)) {
                    counters[CC_UNGT]++;
                }
            }
            if (!__builtin_isgreater(a, b)) {
                counters[CC_UNLE]++;
                if (!__builtin_isgreaterequal(a, b)) {
                    counters[CC_UNLT]++;
                }
            }
        }
    }
}

int main(void) {
    /* Initialize counters */
    memset(counters, 0, sizeof(counters));
    
    /* Run all test functions */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX__
    test_avx_conditions();
#endif
    
    /* Print summary */
    printf("Condition code execution summary:\n");
    printf("  UNORDERED: %d\n", counters[CC_UNORDERED]);
    printf("  ORDERED:   %d\n", counters[CC_ORDERED]);
    printf("  UNEQ:      %d\n", counters[CC_UNEQ]);
    printf("  UNGE:      %d\n", counters[CC_UNGE]);
    printf("  UNGT:      %d\n", counters[CC_UNGT]);
    printf("  UNLE:      %d\n", counters[CC_UNLE]);
    printf("  UNLT:      %d\n", counters[CC_UNLT]);
    printf("  LTGT:      %d\n", counters[CC_LTGT]);
    
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
