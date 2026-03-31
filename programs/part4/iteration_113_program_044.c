#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

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
static double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0,
    1.0/0.0,          /* +Inf */
    -1.0/0.0,         /* -Inf */
    0.0/0.0,          /* NaN (quiet) */
    __builtin_nan(""), /* Another NaN */
    DBL_MAX,
    -DBL_MAX,
    DBL_MIN
};

#define TEST_COUNT (sizeof(test_scalars)/sizeof(test_scalars[0]))

/* Scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    for (int i = 0; i < TEST_COUNT; i++) {
        for (int j = 0; j < TEST_COUNT; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED case */
            if (__builtin_isunordered(a, b)) {
                counters[CNT_UNORDERED]++;
            }
            
            /* ORDERED case */
            if (!__builtin_isunordered(a, b)) {
                counters[CNT_ORDERED]++;
            }
            
            /* UNEQ case - unordered or equal */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[CNT_UNEQ]++;
            }
            
            /* UNGE case - not less than (unordered or greater/equal) */
            if (!__builtin_isless(a, b)) {
                counters[CNT_UNGE]++;
            }
            
            /* UNGT case - not less or equal (unordered or greater) */
            if (!__builtin_islessequal(a, b)) {
                counters[CNT_UNGT]++;
            }
            
            /* UNLE case - unordered or less/equal */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters[CNT_UNLE]++;
            }
            
            /* UNLT case - unordered or less than */
            if (__builtin_isunordered(a, b) || a < b) {
                counters[CNT_UNLT]++;
            }
            
            /* LTGT case - less or greater (unordered excluded) */
            if (__builtin_islessgreater(a, b)) {
                counters[CNT_LTGT]++;
            }
        }
    }
}

/* Vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    __m128d vec_a, vec_b, cmp_result;
    __m256d vec_a256, vec_b256, cmp_result256;
    
    for (int i = 0; i < TEST_COUNT - 1; i += 2) {
        /* SSE2 vector comparisons */
        vec_a = _mm_set_pd(test_scalars[i], test_scalars[i+1]);
        vec_b = _mm_set_pd(test_scalars[i+1], test_scalars[i]);
        
        /* Various comparison predicates */
        cmp_result = _mm_cmpord_pd(vec_a, vec_b);   /* ORDERED */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[CNT_ORDERED] += 2;
        }
        
        cmp_result = _mm_cmpunord_pd(vec_a, vec_b); /* UNORDERED */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[CNT_UNORDERED] += 2;
        }
        
        cmp_result = _mm_cmpnlt_pd(vec_a, vec_b);   /* UNLT -> nlt */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[CNT_UNLT] += 2;
        }
        
        cmp_result = _mm_cmpnle_pd(vec_a, vec_b);   /* UNLE -> nle */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[CNT_UNLE] += 2;
        }
        
        cmp_result = _mm_cmpneq_pd(vec_a, vec_b);   /* UNEQ -> ueq */
        if (_mm_movemask_pd(cmp_result) != 0) {
            counters[CNT_UNEQ] += 2;
        }
        
        /* AVX vector comparisons (if available) */
#ifdef __AVX__
        vec_a256 = _mm256_set_pd(test_scalars[i], test_scalars[i+1], 
                                test_scalars[i], test_scalars[i+1]);
        vec_b256 = _mm256_set_pd(test_scalars[i+1], test_scalars[i], 
                                test_scalars[i+1], test_scalars[i]);
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_UNORD_Q); /* UNORDERED */
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[CNT_UNORDERED] += 4;
        }
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_ORD_Q);   /* ORDERED */
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[CNT_ORDERED] += 4;
        }
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NLT_UQ);  /* UNLT -> nlt */
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[CNT_UNLT] += 4;
        }
        
        cmp_result256 = _mm256_cmp_pd(vec_a256, vec_b256, _CMP_NLE_UQ);  /* UNLE -> nle */
        if (_mm256_movemask_pd(cmp_result256) != 0) {
            counters[CNT_UNLE] += 4;
        }
#endif
    }
}

/* Inline assembly with explicit condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
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
    
    /* UNEQ constraint */
    a = 1.0;
    b = 1.0;
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[CNT_UNEQ]++;
    
    /* UNGE constraint (nlt) */
    a = 2.0;
    b = 1.0;
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[CNT_UNGE]++;
    
    /* UNGT constraint (nle) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[CNT_UNGT]++;
    
    /* UNLE constraint (ule) */
    a = 1.0;
    b = 2.0;
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[CNT_UNLE]++;
    
    /* UNLT constraint (ult) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[CNT_UNLT]++;
    
    /* LTGT constraint (une) */
    a = 1.0;
    b = 3.0;
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[CNT_LTGT]++;
}

/* Control flow based on comparison results */
void test_control_flow(void) {
    volatile double a = 1.0;
    volatile double b = __builtin_nan("");
    
    /* Complex control flow to prevent optimization */
    for (int i = 0; i < 100; i++) {
        if (__builtin_isunordered(a, b)) {
            counters[CNT_UNORDERED]++;
            if (!__builtin_isless(a, b)) {
                counters[CNT_UNGE]++;
            }
        } else {
            counters[CNT_ORDERED]++;
            if (__builtin_islessgreater(a, b)) {
                counters[CNT_LTGT]++;
            }
        }
        
        /* Switch on comparison classification */
        int cmp_class = 0;
        if (__builtin_isunordered(a, b)) cmp_class = 1;
        else if (a == b) cmp_class = 2;
        else if (a < b) cmp_class = 3;
        else cmp_class = 4;
        
        switch (cmp_class) {
            case 1: /* UNORDERED */
                counters[CNT_UNORDERED]++;
                break;
            case 2: /* UNEQ */
                counters[CNT_UNEQ]++;
                break;
            case 3: /* UNLT */
                counters[CNT_UNLT]++;
                break;
            case 4: /* UNGT */
                counters[CNT_UNGT]++;
                break;
        }
        
        /* Alternate between NaN and normal values */
        b = (i % 2 == 0) ? __builtin_nan("") : 2.0;
    }
}

int main(void) {
    /* Initialize counters */
    for (int i = 0; i < 8; i++) {
        counters[i] = 0;
    }
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all test functions */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary */
    printf("\nCondition code usage summary:\n");
    printf("UNORDERED (unord): %d\n", counters[CNT_UNORDERED]);
    printf("ORDERED   (ord):   %d\n", counters[CNT_ORDERED]);
    printf("UNEQ      (ueq):   %d\n", counters[CNT_UNEQ]);
    printf("UNGE      (nlt):   %d\n", counters[CNT_UNGE]);
    printf("UNGT      (nle):   %d\n", counters[CNT_UNGT]);
    printf("UNLE      (ule):   %d\n", counters[CNT_UNLE]);
    printf("UNLT      (ult):   %d\n", counters[CNT_UNLT]);
    printf("LTGT      (une):   %d\n", counters[CNT_LTGT]);
    
    /* Verify all condition codes were triggered */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += counters[i];
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
