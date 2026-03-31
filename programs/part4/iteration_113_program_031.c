#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>

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
} cc_counter;

/* Initialize counters */
static cc_counter counters = {0};

/* Test data arrays */
#define TEST_SIZE 8
static double scalar_data[TEST_SIZE] = {
    1.0, 2.0, -1.0, 0.0,
    __builtin_nan(""), INFINITY, -INFINITY, 3.14
};

static __m128d vec_data[TEST_SIZE/2];
static __m256d avx_data[TEST_SIZE/4];

/* Initialize vector data */
void init_test_data(void) {
    for (int i = 0; i < TEST_SIZE/2; i++) {
        vec_data[i] = _mm_set_pd(scalar_data[2*i+1], scalar_data[2*i]);
    }
    for (int i = 0; i < TEST_SIZE/4; i++) {
        avx_data[i] = _mm256_set_pd(
            scalar_data[4*i+3],
            scalar_data[4*i+2],
            scalar_data[4*i+1],
            scalar_data[4*i]
        );
    }
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    printf("Testing scalar conditions...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = scalar_data[i];
            double b = scalar_data[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                counters.unordered++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                counters.ordered++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || a == b) {
                counters.uneq++;
            }
            
            /* UNGE (unordered or greater or equal) */
            if (__builtin_isunordered(a, b) || a >= b) {
                counters.unge++;
            }
            
            /* UNGT (unordered or greater) */
            if (__builtin_isunordered(a, b) || a > b) {
                counters.ungt++;
            }
            
            /* UNLE (unordered or less or equal) */
            if (__builtin_isunordered(a, b) || a <= b) {
                counters.unle++;
            }
            
            /* UNLT (unordered or less) */
            if (__builtin_isunordered(a, b) || a < b) {
                counters.unlt++;
            }
            
            /* LTGT (less or greater, but not equal and not unordered) */
            if (__builtin_islessgreater(a, b)) {
                counters.ltgt++;
            }
        }
    }
}

/* Test SSE vector comparisons */
void test_sse_conditions(void) {
    printf("Testing SSE vector conditions...\n");
    
    for (int i = 0; i < TEST_SIZE/2; i++) {
        for (int j = 0; j < TEST_SIZE/2; j++) {
            __m128d a = vec_data[i];
            __m128d b = vec_data[j];
            
            /* Compare with various predicates */
            __m128d cmp;
            
            /* _CMP_UNORD_Q - unordered */
            cmp = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unordered += 2; /* Two elements per vector */
            }
            
            /* _CMP_ORD_Q - ordered */
            cmp = _mm_cmp_pd(a, b, _CMP_ORD_Q);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.ordered += 2;
            }
            
            /* _CMP_EQ_UQ - equal or unordered */
            cmp = _mm_cmp_pd(a, b, _CMP_EQ_UQ);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.uneq += 2;
            }
            
            /* _CMP_NLT_UQ - not less than or unordered */
            cmp = _mm_cmp_pd(a, b, _CMP_NLT_UQ);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unge += 2;
            }
            
            /* _CMP_NLE_UQ - not less than or equal or unordered */
            cmp = _mm_cmp_pd(a, b, _CMP_NLE_UQ);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.ungt += 2;
            }
            
            /* _CMP_LE_UQ - less than or equal or unordered */
            cmp = _mm_cmp_pd(a, b, _CMP_LE_UQ);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unle += 2;
            }
            
            /* _CMP_LT_UQ - less than or unordered */
            cmp = _mm_cmp_pd(a, b, _CMP_LT_UQ);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.unlt += 2;
            }
            
            /* _CMP_NEQ_UQ - not equal and unordered */
            cmp = _mm_cmp_pd(a, b, _CMP_NEQ_UQ);
            if (_mm_movemask_pd(cmp) != 0) {
                counters.ltgt += 2;
            }
        }
    }
}

/* Test AVX vector comparisons */
#ifdef __AVX__
void test_avx_conditions(void) {
    printf("Testing AVX vector conditions...\n");
    
    for (int i = 0; i < TEST_SIZE/4; i++) {
        for (int j = 0; j < TEST_SIZE/4; j++) {
            __m256d a = avx_data[i];
            __m256d b = avx_data[j];
            
            /* Compare with various predicates */
            __m256d cmp;
            
            /* _CMP_UNORD_Q - unordered */
            cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters.unordered += 4; /* Four elements per vector */
            }
            
            /* _CMP_ORD_Q - ordered */
            cmp = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters.ordered += 4;
            }
            
            /* _CMP_EQ_UQ - equal or unordered */
            cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters.uneq += 4;
            }
            
            /* _CMP_NLT_UQ - not less than or unordered */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters.unge += 4;
            }
            
            /* _CMP_NLE_UQ - not less than or equal or unordered */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters.ungt += 4;
            }
            
            /* _CMP_LE_UQ - less than or equal or unordered */
            cmp = _mm256_cmp_pd(a, b, _CMP_LE_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters.unle += 4;
            }
            
            /* _CMP_LT_UQ - less than or unordered */
            cmp = _mm256_cmp_pd(a, b, _CMP_LT_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters.unlt += 4;
            }
            
            /* _CMP_NEQ_UQ - not equal and unordered */
            cmp = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            if (_mm256_movemask_pd(cmp) != 0) {
                counters.ltgt += 4;
            }
        }
    }
}
#endif

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    printf("Testing inline assembly constraints...\n");
    
    for (int i = 0; i < TEST_SIZE; i++) {
        for (int j = 0; j < TEST_SIZE; j++) {
            double a = scalar_data[i];
            double b = scalar_data[j];
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
            if (result) counters.unordered++;
            
            /* ORDERED constraint */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al"
            );
            if (result) counters.ordered++;
            
            /* UNEQ constraint */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "sete %%al\n\t"
                "setp %%cl\n\t"
                "orb %%cl, %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cl"
            );
            if (result) counters.uneq++;
            
            /* UNGE constraint (nlt) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setae %%al\n\t"
                "setp %%cl\n\t"
                "orb %%cl, %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cl"
            );
            if (result) counters.unge++;
            
            /* UNGT constraint (nle) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "seta %%al\n\t"
                "setp %%cl\n\t"
                "orb %%cl, %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cl"
            );
            if (result) counters.ungt++;
            
            /* UNLE constraint */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setbe %%al\n\t"
                "setp %%cl\n\t"
                "orb %%cl, %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cl"
            );
            if (result) counters.unle++;
            
            /* UNLT constraint */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setb %%al\n\t"
                "setp %%cl\n\t"
                "orb %%cl, %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cl"
            );
            if (result) counters.unlt++;
            
            /* LTGT constraint (une) */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setne %%al\n\t"
                "setnp %%cl\n\t"
                "andb %%cl, %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cl"
            );
            if (result) counters.ltgt++;
        }
    }
}

/* Print summary of condition code hits */
void print_summary(void) {
    printf("\n=== Condition Code Summary ===\n");
    printf("UNORDERED (unord): %d\n", counters.unordered);
    printf("ORDERED (ord): %d\n", counters.ordered);
    printf("UNEQ (ueq): %d\n", counters.uneq);
    printf("UNGE (nlt): %d\n", counters.unge);
    printf("UNGT (nle): %d\n", counters.ungt);
    printf("UNLE (ule): %d\n", counters.unle);
    printf("UNLT (ult): %d\n", counters.unlt);
    printf("LTGT (une): %d\n", counters.ltgt);
    
    int total = counters.unordered + counters.ordered + counters.uneq +
                counters.unge + counters.ungt + counters.unle +
                counters.unlt + counters.ltgt;
    printf("Total condition code hits: %d\n", total);
}

int main(void) {
    printf("Starting condition code coverage test...\n");
    
    /* Initialize test data */
    init_test_data();
    
    /* Run all test functions */
    test_scalar_conditions();
    test_sse_conditions();
    
#ifdef __AVX__
    test_avx_conditions();
#else
    printf("AVX not available, skipping AVX tests...\n");
#endif
    
    test_asm_constraints();
    
    /* Print results */
    print_summary();
    
    /* Verify we hit all condition codes */
    if (counters.unordered > 0 && counters.ordered > 0 &&
        counters.uneq > 0 && counters.unge > 0 &&
        counters.ungt > 0 && counters.unle > 0 &&
        counters.unlt > 0 && counters.ltgt > 0) {
        printf("\nSUCCESS: All condition code paths were exercised!\n");
        return 0;
    } else {
        printf("\nWARNING: Some condition codes were not hit!\n");
        return 1;
    }
}
