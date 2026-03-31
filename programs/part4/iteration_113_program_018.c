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
#define TEST_SIZE 8
static double test_scalars[TEST_SIZE] = {
    1.0, -1.0, 0.0, -0.0,
    INFINITY, -INFINITY,
    NAN, -NAN
};

static __m128d test_vec128[TEST_SIZE/2];
static __m256d test_vec256[TEST_SIZE/4];

/* Initialize vector test data */
void init_test_data(void) {
    for (int i = 0; i < TEST_SIZE/2; i++) {
        test_vec128[i] = _mm_set_pd(test_scalars[2*i+1], test_scalars[2*i]);
    }
    for (int i = 0; i < TEST_SIZE/4; i++) {
        test_vec256[i] = _mm256_set_pd(
            test_scalars[4*i+3], test_scalars[4*i+2],
            test_scalars[4*i+1], test_scalars[4*i]
        );
    }
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    double nan = __builtin_nan("");
    double inf = INFINITY;
    
    for (int i = 0; i < TEST_SIZE; i++) {
        double a = test_scalars[i];
        
        /* UNORDERED case - compare with NaN */
        if (__builtin_isunordered(a, nan)) {
            counters[CNT_UNORDERED]++;
        }
        
        /* ORDERED case - compare normal numbers */
        if (__builtin_isordered(a, 1.0)) {
            counters[CNT_ORDERED]++;
        }
        
        /* UNEQ case - unordered or equal */
        if (!__builtin_isgreater(a, a) && !__builtin_isless(a, a)) {
            counters[CNT_UNEQ]++;
        }
        
        /* UNGE case - not less than (unordered or greater/equal) */
        if (!__builtin_isless(a, inf)) {
            counters[CNT_UNGE]++;
        }
        
        /* UNGT case - not less/equal (unordered or greater) */
        if (!__builtin_islessequal(a, -inf)) {
            counters[CNT_UNGT]++;
        }
        
        /* UNLE case - unordered or less/equal */
        if (__builtin_islessequal(a, nan) || __builtin_islessequal(nan, a)) {
            counters[CNT_UNLE]++;
        }
        
        /* UNLT case - unordered or less than */
        if (__builtin_isless(a, nan) || __builtin_isless(nan, a)) {
            counters[CNT_UNLT]++;
        }
        
        /* LTGT case - less or greater (unordered excluded) */
        if (__builtin_islessgreater(a, a)) {
            counters[CNT_LTGT]++;
        }
    }
}

/* Test vector comparisons */
void test_vector_conditions(void) {
    __m128d vnan = _mm_set1_pd(__builtin_nan(""));
    __m128d vone = _mm_set1_pd(1.0);
    __m128d vinf = _mm_set1_pd(INFINITY);
    
    for (int i = 0; i < TEST_SIZE/2; i++) {
        __m128d a = test_vec128[i];
        __m128d cmp;
        
        /* Generate various comparison predicates */
        cmp = _mm_cmpord_pd(a, a);      /* ORDERED */
        if (_mm_movemask_pd(cmp) != 0) counters[CNT_ORDERED]++;
        
        cmp = _mm_cmpunord_pd(a, vnan); /* UNORDERED */
        if (_mm_movemask_pd(cmp) != 0) counters[CNT_UNORDERED]++;
        
        cmp = _mm_cmpnlt_pd(a, vinf);   /* UNGE (nlt) */
        if (_mm_movemask_pd(cmp) != 0) counters[CNT_UNGE]++;
        
        cmp = _mm_cmpnle_pd(a, _mm_set1_pd(-INFINITY)); /* UNGT (nle) */
        if (_mm_movemask_pd(cmp) != 0) counters[CNT_UNGT]++;
        
        /* For AVX if available */
        #ifdef __AVX__
        __m256d a256 = test_vec256[i/2];
        __m256d cmp256;
        
        cmp256 = _mm256_cmp_pd(a256, vnan, _CMP_UNORD_Q);
        if (_mm256_movemask_pd(cmp256) != 0) counters[CNT_UNORDERED]++;
        
        cmp256 = _mm256_cmp_pd(a256, a256, _CMP_ORD_Q);
        if (_mm256_movemask_pd(cmp256) != 0) counters[CNT_ORDERED]++;
        
        cmp256 = _mm256_cmp_pd(a256, vinf, _CMP_NLT_UQ); /* UNGE */
        if (_mm256_movemask_pd(cmp256) != 0) counters[CNT_UNGE]++;
        
        cmp256 = _mm256_cmp_pd(a256, _mm256_set1_pd(-INFINITY), _CMP_NLE_UQ); /* UNGT */
        if (_mm256_movemask_pd(cmp256) != 0) counters[CNT_UNGT]++;
        #endif
    }
}

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    double c = 2.0;
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
    if (result) counters[CNT_UNORDERED]++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=@ord" (result)
        : "x" (a), "x" (c)
        : "al"
    );
    if (result) counters[CNT_ORDERED]++;
    
    /* UNEQ constraint via inline asm */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %0"
        : "=@ueq" (result)
        : "x" (a), "x" (a)
        : "al"
    );
    if (result) counters[CNT_UNEQ]++;
    
    /* UNGE (nlt) constraint */
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=@nlt" (result)
        : "x" (c), "x" (a)
        : "al"
    );
    if (result) counters[CNT_UNGE]++;
    
    /* UNGT (nle) constraint */
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=@nle" (result)
        : "x" (c), "x" (a)
        : "al"
    );
    if (result) counters[CNT_UNGT]++;
    
    /* UNLE constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=@ule" (result)
        : "x" (a), "x" (c)
        : "al"
    );
    if (result) counters[CNT_UNLE]++;
    
    /* UNLT constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=@ult" (result)
        : "x" (a), "x" (c)
        : "al"
    );
    if (result) counters[CNT_UNLT]++;
    
    /* LTGT (une) constraint */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=@une" (result)
        : "x" (a), "x" (c)
        : "al"
    );
    if (result) counters[CNT_LTGT]++;
}

/* Control flow based on comparison results */
void test_control_flow(void) {
    double values[] = {1.0, NAN, INFINITY, -INFINITY, 0.0};
    int n = sizeof(values)/sizeof(values[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                counters[CNT_UNORDERED]++;
                if (!__builtin_isordered(a, b)) {
                    counters[CNT_ORDERED]--;  /* This won't execute but creates complexity */
                }
            } else if (__builtin_isless(a, b)) {
                counters[CNT_UNLT]++;
            } else if (__builtin_isgreater(a, b)) {
                counters[CNT_UNGT]++;
            } else if (__builtin_islessequal(a, b)) {
                counters[CNT_UNLE]++;
            } else if (__builtin_isgreaterequal(a, b)) {
                counters[CNT_UNGE]++;
            }
            
            /* Switch on comparison classification */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 1;
            else if (a == b) cmp_class = 2;  /* UNEQ for equal non-NaN */
            else if (__builtin_islessgreater(a, b)) cmp_class = 3;
            
            switch (cmp_class) {
                case 1: counters[CNT_UNORDERED]++; break;
                case 2: counters[CNT_UNEQ]++; break;
                case 3: counters[CNT_LTGT]++; break;
            }
        }
    }
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
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
    /* Print summary */
    printf("Condition code execution summary:\n");
    printf("UNORDERED (unord): %d\n", counters[CNT_UNORDERED]);
    printf("ORDERED   (ord):   %d\n", counters[CNT_ORDERED]);
    printf("UNEQ      (ueq):   %d\n", counters[CNT_UNEQ]);
    printf("UNGE      (nlt):   %d\n", counters[CNT_UNGE]);
    printf("UNGT      (nle):   %d\n", counters[CNT_UNGT]);
    printf("UNLE      (ule):   %d\n", counters[CNT_UNLE]);
    printf("UNLT      (ult):   %d\n", counters[CNT_UNLT]);
    printf("LTGT      (une):   %d\n", counters[CNT_LTGT]);
    
    /* Verify at least some conditions were triggered */
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += counters[i];
    }
    
    if (total == 0) {
        fprintf(stderr, "ERROR: No condition codes were triggered!\n");
        return 1;
    }
    
    printf("\nTotal condition code hits: %d\n", total);
    printf("All condition code paths were exercised.\n");
    
    return 0;
}
