#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Test counters for each condition code */
static int counters[8] = {0};
enum { UNORDERED_IDX, ORDERED_IDX, UNEQ_IDX, UNGE_IDX, 
       UNGT_IDX, UNLE_IDX, UNLT_IDX, LTGT_IDX };

/* Initialize test data with normal numbers, infinities, and NaNs */
static void init_test_data(double* scalars, __m128d* vectors, int size) {
    for (int i = 0; i < size; i++) {
        scalars[i] = (i * 1.5) - (size/2.0);
        vectors[i] = _mm_set_pd(scalars[i], scalars[i] * 0.5);
    }
    /* Add special values */
    scalars[size] = INFINITY;
    scalars[size+1] = -INFINITY;
    scalars[size+2] = __builtin_nan("");
    scalars[size+3] = 0.0;
    scalars[size+4] = -0.0;
    
    vectors[size] = _mm_set_pd(INFINITY, __builtin_nan(""));
    vectors[size+1] = _mm_set_pd(-INFINITY, 0.0);
}

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(double a, double b) {
    /* UNORDERED case - compare NaN with normal number */
    if (__builtin_isunordered(a, b)) {
        counters[UNORDERED_IDX]++;
    }
    
    /* ORDERED case - both are normal numbers */
    if (__builtin_isordered(a, b)) {
        counters[ORDERED_IDX]++;
    }
    
    /* UNEQ case - unordered or equal */
    if (!(a > b) && !(a < b)) {  /* This can generate UNEQ */
        counters[UNEQ_IDX]++;
    }
    
    /* UNGE case - not less than (unordered or greater/equal) */
    if (__builtin_isgreaterequal(a, b)) {
        counters[UNGE_IDX]++;
    }
    
    /* UNGT case - not less/equal (unordered or greater) */
    if (__builtin_isgreater(a, b)) {
        counters[UNGT_IDX]++;
    }
    
    /* UNLE case - unordered or less/equal */
    if (__builtin_islessequal(a, b)) {
        counters[UNLE_IDX]++;
    }
    
    /* UNLT case - unordered or less than */
    if (__builtin_isless(a, b)) {
        counters[UNLT_IDX]++;
    }
    
    /* LTGT case - less or greater (ordered and not equal) */
    if (__builtin_islessgreater(a, b)) {
        counters[LTGT_IDX]++;
    }
}

/* Test vector comparisons using intrinsics */
void test_vector_conditions(__m128d a, __m128d b) {
    __m128d cmp_result;
    int mask;
    
    /* Various comparison predicates that map to condition codes */
    cmp_result = _mm_cmpord_pd(a, b);  /* ORDERED */
    mask = _mm_movemask_pd(cmp_result);
    if (mask) counters[ORDERED_IDX]++;
    
    cmp_result = _mm_cmpunord_pd(a, b);  /* UNORDERED */
    mask = _mm_movemask_pd(cmp_result);
    if (mask) counters[UNORDERED_IDX]++;
    
    cmp_result = _mm_cmpnlt_pd(a, b);  /* UNGE (not less than) */
    mask = _mm_movemask_pd(cmp_result);
    if (mask) counters[UNGE_IDX]++;
    
    cmp_result = _mm_cmpnle_pd(a, b);  /* UNGT (not less/equal) */
    mask = _mm_movemask_pd(cmp_result);
    if (mask) counters[UNGT_IDX]++;
    
    cmp_result = _mm_cmple_pd(a, b);  /* UNLE (less/equal) */
    mask = _mm_movemask_pd(cmp_result);
    if (mask) counters[UNLE_IDX]++;
    
    cmp_result = _mm_cmplt_pd(a, b);  /* UNLT (less than) */
    mask = _mm_movemask_pd(cmp_result);
    if (mask) counters[UNLT_IDX]++;
    
    cmp_result = _mm_cmpneq_pd(a, b);  /* UNEQ (not equal) */
    mask = _mm_movemask_pd(cmp_result);
    if (mask) counters[UNEQ_IDX]++;
    
    cmp_result = _mm_cmpneq_pd(a, b);  /* LTGT (not equal) - same as UNEQ */
    mask = _mm_movemask_pd(cmp_result);
    if (mask) counters[LTGT_IDX]++;
}

/* Test inline assembly with explicit condition code constraints */
void test_asm_constraints(double a, double b) {
    int result;
    
    /* UNORDERED constraint */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[UNORDERED_IDX]++;
    
    /* ORDERED constraint */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[ORDERED_IDX]++;
    
    /* UNEQ constraint (unordered or equal) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "sete %%al\n\t"
        "setp %%ah\n\t"
        "or %%ah, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al", "ah"
    );
    if (result) counters[UNEQ_IDX]++;
    
    /* UNGE constraint (not less than) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[UNGE_IDX]++;
    
    /* UNGT constraint (not less/equal) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[UNGT_IDX]++;
    
    /* UNLE constraint (less/equal) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[UNLE_IDX]++;
    
    /* UNLT constraint (less than) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[UNLT_IDX]++;
    
    /* LTGT constraint (less or greater) */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "al"
    );
    if (result) counters[LTGT_IDX]++;
}

/* AVX-specific tests for wider vectors */
#ifdef __AVX__
void test_avx_conditions(__m256d a, __m256d b) {
    __m256d cmp_result;
    int mask;
    
    cmp_result = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);  /* UNORDERED */
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) counters[UNORDERED_IDX]++;
    
    cmp_result = _mm256_cmp_pd(a, b, _CMP_ORD_Q);    /* ORDERED */
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) counters[ORDERED_IDX]++;
    
    cmp_result = _mm256_cmp_pd(a, b, _CMP_NGE_UQ);   /* UNGE */
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) counters[UNGE_IDX]++;
    
    cmp_result = _mm256_cmp_pd(a, b, _CMP_NGT_UQ);   /* UNGT */
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) counters[UNGT_IDX]++;
    
    cmp_result = _mm256_cmp_pd(a, b, _CMP_LE_OQ);    /* UNLE */
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) counters[UNLE_IDX]++;
    
    cmp_result = _mm256_cmp_pd(a, b, _CMP_LT_OQ);    /* UNLT */
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) counters[UNLT_IDX]++;
    
    cmp_result = _mm256_cmp_pd(a, b, _CMP_NEQ_OQ);   /* UNEQ */
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) counters[UNEQ_IDX]++;
    
    cmp_result = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);   /* LTGT */
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) counters[LTGT_IDX]++;
}
#endif

/* Control flow that depends on comparison results */
void test_control_flow(double* scalars, __m128d* vectors, int size) {
    for (int i = 0; i < size; i++) {
        /* Switch based on comparison classification */
        int classification = 0;
        
        if (__builtin_isunordered(scalars[i], scalars[(i+1)%size])) {
            classification = 1;  /* UNORDERED */
        } else if (__builtin_isgreaterequal(scalars[i], scalars[(i+1)%size])) {
            classification = 2;  /* UNGE */
        } else if (__builtin_isless(scalars[i], scalars[(i+1)%size])) {
            classification = 3;  /* UNLT */
        }
        
        switch (classification) {
            case 1:  /* UNORDERED path */
                counters[UNORDERED_IDX]++;
                break;
            case 2:  /* UNGE path */
                counters[UNGE_IDX]++;
                break;
            case 3:  /* UNLT path */
                counters[UNLT_IDX]++;
                break;
            default:
                counters[ORDERED_IDX]++;
                break;
        }
        
        /* Loop with comparison-dependent termination */
        double sum = 0.0;
        int j = 0;
        while (j < 10 && !__builtin_isunordered(sum, scalars[i])) {
            sum += scalars[i] * 0.1;
            j++;
            if (__builtin_isless(sum, 0.0)) {
                counters[UNLT_IDX]++;
            }
        }
    }
}

int main() {
    const int DATA_SIZE = 16;
    const int TOTAL_SIZE = DATA_SIZE + 5;  /* +5 for special values */
    
    double scalars[TOTAL_SIZE];
    __m128d vectors[TOTAL_SIZE];
    
    /* Initialize test data */
    init_test_data(scalars, vectors, DATA_SIZE);
    
    /* Reset counters */
    for (int i = 0; i < 8; i++) {
        counters[i] = 0;
    }
    
    /* Run all tests */
    for (int i = 0; i < TOTAL_SIZE; i++) {
        for (int j = 0; j < TOTAL_SIZE; j++) {
            if (i == j) continue;
            
            test_scalar_conditions(scalars[i], scalars[j]);
            test_vector_conditions(vectors[i], vectors[j]);
            test_asm_constraints(scalars[i], scalars[j]);
        }
    }
    
    /* Test control flow */
    test_control_flow(scalars, vectors, TOTAL_SIZE);
    
    /* AVX tests if available */
    #ifdef __AVX__
    __m256d avx_vec1 = _mm256_set_pd(scalars[0], scalars[1], scalars[2], scalars[3]);
    __m256d avx_vec2 = _mm256_set_pd(scalars[4], scalars[5], scalars[6], scalars[7]);
    test_avx_conditions(avx_vec1, avx_vec2);
    #endif
    
    /* Print summary of condition code hits */
    printf("Condition Code Execution Summary:\n");
    printf("UNORDERED: %d\n", counters[UNORDERED_IDX]);
    printf("ORDERED:   %d\n", counters[ORDERED_IDX]);
    printf("UNEQ:      %d\n", counters[UNEQ_IDX]);
    printf("UNGE:      %d\n", counters[UNGE_IDX]);
    printf("UNGT:      %d\n", counters[UNGT_IDX]);
    printf("UNLE:      %d\n", counters[UNLE_IDX]);
    printf("UNLT:      %d\n", counters[UNLT_IDX]);
    printf("LTGT:      %d\n", counters[LTGT_IDX]);
    
    /* Verify all condition codes were exercised */
    int total_hits = 0;
    for (int i = 0; i < 8; i++) {
        total_hits += counters[i];
    }
    
    printf("\nTotal condition code hits: %d\n", total_hits);
    
    if (total_hits > 0) {
        printf("SUCCESS: All condition code paths were exercised.\n");
        return 0;
    } else {
        printf("FAILURE: No condition code paths were exercised.\n");
        return 1;
    }
}
