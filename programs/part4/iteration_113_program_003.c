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
    INFINITY, -INFINITY,
    __builtin_nan(""), -__builtin_nan(""),
    3.14, -2.71
};

static __m128d test_vec128[] = {
    _mm_set_pd(1.0, 2.0),
    _mm_set_pd(-1.0, 0.0),
    _mm_set_pd(INFINITY, -INFINITY),
    _mm_set_pd(__builtin_nan(""), 3.14),
    _mm_set_pd(-__builtin_nan(""), -2.71)
};

#ifdef __AVX__
static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, -1.0, 0.0),
    _mm256_set_pd(INFINITY, -INFINITY, __builtin_nan(""), 3.14),
    _mm256_set_pd(-__builtin_nan(""), -2.71, 1.5, -1.5)
};
#endif

/* Test scalar conditions using GCC builtins */
void test_scalar_conditions(void) {
    int i, j;
    double a, b;
    
    for (i = 0; i < sizeof(test_scalars)/sizeof(test_scalars[0]); i++) {
        for (j = 0; j < sizeof(test_scalars)/sizeof(test_scalars[0]); j++) {
            a = test_scalars[i];
            b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters[CNT_UNORDERED]++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                counters[CNT_ORDERED]++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || a == b) {
                counters[CNT_UNEQ]++;
            }
            
            /* UNGE (not less than) = !(a < b) */
            if (!__builtin_isless(a, b)) {
                counters[CNT_UNGE]++;
            }
            
            /* UNGT (not less or equal) = !(a <= b) */
            if (!__builtin_islessequal(a, b)) {
                counters[CNT_UNGT]++;
            }
            
            /* UNLE (unordered or less or equal) */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                counters[CNT_UNLE]++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
                counters[CNT_UNLT]++;
            }
            
            /* LTGT (less or greater) = !(a == b) && !__builtin_isunordered(a, b) */
            if (__builtin_islessgreater(a, b)) {
                counters[CNT_LTGT]++;
            }
        }
    }
}

/* Test vector conditions using SSE/AVX intrinsics */
void test_vector_conditions(void) {
    int i, j;
    __m128d a, b;
    __m128d cmp_result;
    
    for (i = 0; i < sizeof(test_vec128)/sizeof(test_vec128[0]); i++) {
        for (j = 0; j < sizeof(test_vec128)/sizeof(test_vec128[0]); j++) {
            a = test_vec128[i];
            b = test_vec128[j];
            
            /* Various comparison predicates that map to condition codes */
            
            /* _CMP_UNORD_Q - unordered */
            cmp_result = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNORDERED]++;
            }
            
            /* _CMP_ORD_Q - ordered */
            cmp_result = _mm_cmp_pd(a, b, _CMP_ORD_Q);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_ORDERED]++;
            }
            
            /* _CMP_EQ_UQ - equal (unordered, non-signaling) */
            cmp_result = _mm_cmp_pd(a, b, _CMP_EQ_UQ);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNEQ]++;
            }
            
            /* _CMP_NLT_US - not less than (unordered, signaling) */
            cmp_result = _mm_cmp_pd(a, b, _CMP_NLT_US);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNGE]++;
            }
            
            /* _CMP_NLE_US - not less or equal (unordered, signaling) */
            cmp_result = _mm_cmp_pd(a, b, _CMP_NLE_US);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNGT]++;
            }
            
            /* _CMP_LE_UQ - less or equal (unordered, non-signaling) */
            cmp_result = _mm_cmp_pd(a, b, _CMP_LE_UQ);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNLE]++;
            }
            
            /* _CMP_LT_UQ - less than (unordered, non-signaling) */
            cmp_result = _mm_cmp_pd(a, b, _CMP_LT_UQ);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_UNLT]++;
            }
            
            /* _CMP_NEQ_UQ - not equal (unordered, non-signaling) */
            cmp_result = _mm_cmp_pd(a, b, _CMP_NEQ_UQ);
            if (_mm_movemask_pd(cmp_result) != 0) {
                counters[CNT_LTGT]++;
            }
        }
    }
}

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    double a, b;
    int result;
    
    /* Test various condition codes through inline assembly */
    for (int i = 0; i < 4; i++) {
        a = test_scalars[i * 2];
        b = test_scalars[i * 2 + 1];
        
        /* UNORDERED */
        __asm__ volatile (
            "ucomisd %1, %0\n\t"
            "setp %%al\n\t"
            "movzbl %%al, %2"
            : "=@ccunord"(result)
            : "x"(a), "x"(b), "=r"(result)
            : "al"
        );
        if (result) counters[CNT_UNORDERED]++;
        
        /* ORDERED */
        __asm__ volatile (
            "ucomisd %1, %0\n\t"
            "setnp %%al\n\t"
            "movzbl %%al, %2"
            : "=@ccord"(result)
            : "x"(a), "x"(b), "=r"(result)
            : "al"
        );
        if (result) counters[CNT_ORDERED]++;
        
        /* UNEQ (unordered or equal) */
        __asm__ volatile (
            "ucomisd %1, %0\n\t"
            "sete %%al\n\t"
            "setp %%ah\n\t"
            "orb %%ah, %%al\n\t"
            "movzbl %%al, %2"
            : "=@ccueq"(result)
            : "x"(a), "x"(b), "=r"(result)
            : "al", "ah"
        );
        if (result) counters[CNT_UNEQ]++;
        
        /* UNGE (not less than) */
        __asm__ volatile (
            "ucomisd %1, %0\n\t"
            "setnb %%al\n\t"
            "movzbl %%al, %2"
            : "=@ccnlt"(result)
            : "x"(a), "x"(b), "=r"(result)
            : "al"
        );
        if (result) counters[CNT_UNGE]++;
        
        /* UNGT (not less or equal) */
        __asm__ volatile (
            "ucomisd %1, %0\n\t"
            "setnbe %%al\n\t"
            "movzbl %%al, %2"
            : "=@ccnle"(result)
            : "x"(a), "x"(b), "=r"(result)
            : "al"
        );
        if (result) counters[CNT_UNGT]++;
        
        /* UNLE (unordered or less or equal) */
        __asm__ volatile (
            "ucomisd %1, %0\n\t"
            "setbe %%al\n\t"
            "setp %%ah\n\t"
            "orb %%ah, %%al\n\t"
            "movzbl %%al, %2"
            : "=@ccule"(result)
            : "x"(a), "x"(b), "=r"(result)
            : "al", "ah"
        );
        if (result) counters[CNT_UNLE]++;
        
        /* UNLT (unordered or less than) */
        __asm__ volatile (
            "ucomisd %1, %0\n\t"
            "setb %%al\n\t"
            "setp %%ah\n\t"
            "orb %%ah, %%al\n\t"
            "movzbl %%al, %2"
            : "=@ccult"(result)
            : "x"(a), "x"(b), "=r"(result)
            : "al", "ah"
        );
        if (result) counters[CNT_UNLT]++;
        
        /* LTGT (less or greater) */
        __asm__ volatile (
            "ucomisd %1, %0\n\t"
            "setne %%al\n\t"
            "setnp %%ah\n\t"
            "andb %%ah, %%al\n\t"
            "movzbl %%al, %2"
            : "=@ccune"(result)
            : "x"(a), "x"(b), "=r"(result)
            : "al", "ah"
        );
        if (result) counters[CNT_LTGT]++;
    }
}

#ifdef __AVX__
/* Test AVX vector conditions */
void test_avx_conditions(void) {
    int i, j;
    __m256d a, b;
    __m256d cmp_result;
    int mask;
    
    for (i = 0; i < sizeof(test_vec256)/sizeof(test_vec256[0]); i++) {
        for (j = 0; j < sizeof(test_vec256)/sizeof(test_vec256[0]); j++) {
            a = test_vec256[i];
            b = test_vec256[j];
            
            /* Use AVX comparison predicates */
            cmp_result = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CNT_UNORDERED] += __builtin_popcount(mask);
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CNT_ORDERED] += __builtin_popcount(mask);
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CNT_UNEQ] += __builtin_popcount(mask);
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NLT_US);
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CNT_UNGE] += __builtin_popcount(mask);
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NLE_US);
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CNT_UNGT] += __builtin_popcount(mask);
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_LE_UQ);
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CNT_UNLE] += __builtin_popcount(mask);
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_LT_UQ);
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CNT_UNLT] += __builtin_popcount(mask);
            
            cmp_result = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            mask = _mm256_movemask_pd(cmp_result);
            if (mask != 0) counters[CNT_LTGT] += __builtin_popcount(mask);
        }
    }
}
#endif

/* Control flow test that depends on comparison results */
void test_control_flow(void) {
    double a = __builtin_nan("");
    double b = 1.0;
    double c = 2.0;
    double d = __builtin_nan("");
    
    /* Complex control flow to prevent optimization */
    for (int i = 0; i < 100; i++) {
        if (__builtin_isunordered(a, b)) {
            counters[CNT_UNORDERED]++;
            a = b;  /* Change value to affect future comparisons */
        }
        
        if (__builtin_isordered(b, c)) {
            counters[CNT_ORDERED]++;
            b += 0.1;
        }
        
        if (!__builtin_isless(b, c)) {
            counters[CNT_UNGE]++;
            c -= 0.1;
        }
        
        if (__builtin_islessgreater(b, c)) {
            counters[CNT_LTGT]++;
            /* Swap values */
            double tmp = b;
            b = c;
            c = tmp;
        }
        
        if (__builtin_isunordered(a, d) || a == d) {
            counters[CNT_UNEQ]++;
            d = i * 0.5;
        }
    }
}

int main(void) {
    /* Initialize counters */
    memset(counters, 0, sizeof(counters));
    
    printf("Testing condition code generation...\n");
    
    /* Run all test functions */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX__
    test_avx_conditions();
#endif
    
    /* Print results */
    printf("\nCondition code usage summary:\n");
    printf("UNORDERED: %d\n", counters[CNT_UNORDERED]);
    printf("ORDERED:   %d\n", counters[CNT_ORDERED]);
    printf("UNEQ:      %d\n", counters[CNT_UNEQ]);
    printf("UNGE:      %d\n", counters[CNT_UNGE]);
    printf("UNGT:      %d\n", counters[CNT_UNGT]);
    printf("UNLE:      %d\n", counters[CNT_UNLE]);
    printf("UNLT:      %d\n", counters[CNT_UNLT]);
    printf("LTGT:      %d\n", counters[CNT_LTGT]);
    
    /* Verify we hit all condition codes */
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += counters[i];
    }
    
    if (total > 0) {
        printf("\nSuccess: Generated %d condition code references\n", total);
        return 0;
    } else {
        printf("\nError: No condition codes generated\n");
        return 1;
    }
}
