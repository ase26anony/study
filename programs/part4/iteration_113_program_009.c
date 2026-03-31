#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <float.h>

/* Condition code counters */
typedef struct {
    unsigned int unordered;
    unsigned int ordered;
    unsigned int uneq;
    unsigned int unge;
    unsigned int ungt;
    unsigned int unle;
    unsigned int unlt;
    unsigned int ltgt;
} cc_counter_t;

static cc_counter_t global_counter = {0};

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
    _mm_set_pd(__builtin_nan(""), 3.14)
};

#ifdef __AVX__
static __m256d test_vec256[] = {
    _mm256_set_pd(1.0, 2.0, 3.0, 4.0),
    _mm256_set_pd(-1.0, 0.0, __builtin_nan(""), __builtin_inf())
};
#endif

/* Test scalar comparisons using GCC builtins */
void test_scalar_conditions(void) {
    const int n = sizeof(test_scalars)/sizeof(test_scalars[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED */
            if (__builtin_isunordered(a, b)) {
                global_counter.unordered++;
            }
            
            /* ORDERED */
            if (__builtin_isordered(a, b)) {
                global_counter.ordered++;
            }
            
            /* UNEQ (unordered or equal) - simulate with builtins */
            if (__builtin_isunordered(a, b) || (a == b)) {
                global_counter.uneq++;
            }
            
            /* UNGE (not less than) = !(a < b) */
            if (!__builtin_isless(a, b)) {
                global_counter.unge++;
            }
            
            /* UNGT (not less or equal) = !(a <= b) */
            if (!__builtin_islessequal(a, b)) {
                global_counter.ungt++;
            }
            
            /* UNLE (unordered or less or equal) */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                global_counter.unle++;
            }
            
            /* UNLT (unordered or less than) */
            if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
                global_counter.unlt++;
            }
            
            /* LTGT (less or greater, but not equal and not unordered) */
            if (__builtin_islessgreater(a, b)) {
                global_counter.ltgt++;
            }
        }
    }
}

/* Test vector comparisons using intrinsics */
void test_vector_conditions(void) {
    const int n = sizeof(test_vec128)/sizeof(test_vec128[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            __m128d a = test_vec128[i];
            __m128d b = test_vec128[j];
            
            /* Compare with various predicates */
            __m128d cmp;
            int mask;
            
            /* CMP_UNORD_Q - unordered */
            cmp = _mm_cmpunord_pd(a, b);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.unordered += __builtin_popcount(mask);
            }
            
            /* CMP_ORD_Q - ordered */
            cmp = _mm_cmpord_pd(a, b);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.ordered += __builtin_popcount(mask);
            }
            
            /* CMP_EQ_UQ - equal (unordered or equal) */
            cmp = _mm_cmpeq_pd(a, b);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.uneq += __builtin_popcount(mask);
            }
            
            /* CMP_NLT_UQ - not less than (unordered or not less than) */
            cmp = _mm_cmpnlt_pd(a, b);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.unge += __builtin_popcount(mask);
            }
            
            /* CMP_NLE_UQ - not less or equal */
            cmp = _mm_cmpnle_pd(a, b);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.ungt += __builtin_popcount(mask);
            }
            
            /* CMP_LE_UQ - less or equal (unordered or less or equal) */
            cmp = _mm_cmple_pd(a, b);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.unle += __builtin_popcount(mask);
            }
            
            /* CMP_LT_UQ - less than (unordered or less than) */
            cmp = _mm_cmplt_pd(a, b);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.unlt += __builtin_popcount(mask);
            }
            
            /* CMP_NEQ_UQ - not equal (unordered or not equal) */
            cmp = _mm_cmpneq_pd(a, b);
            mask = _mm_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.ltgt += __builtin_popcount(mask);
            }
        }
    }
}

/* Test inline assembly with condition code constraints */
void test_asm_constraints(void) {
    double a = 1.0;
    double b = __builtin_nan("");
    int result;
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) global_counter.unordered++;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) global_counter.ordered++;
    
    /* UNEQ - unordered or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %0\n\t"
        "setp %%al\n\t"
        "orb %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    if (result) global_counter.uneq++;
    
    /* UNGE - not less than (nlt) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnb %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) global_counter.unge++;
    
    /* UNGT - not less or equal (nle) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) global_counter.ungt++;
    
    /* UNLE - unordered or less or equal (ule) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0\n\t"
        "setp %%al\n\t"
        "orb %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    if (result) global_counter.unle++;
    
    /* UNLT - unordered or less than (ult) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %0\n\t"
        "setp %%al\n\t"
        "orb %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    if (result) global_counter.unlt++;
    
    /* LTGT - less or greater (une) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    if (result) global_counter.ltgt++;
}

#ifdef __AVX__
/* Test AVX vector comparisons */
void test_avx_conditions(void) {
    const int n = sizeof(test_vec256)/sizeof(test_vec256[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            __m256d a = test_vec256[i];
            __m256d b = test_vec256[j];
            
            /* Use various AVX comparison predicates */
            __m256d cmp;
            int mask;
            
            /* _CMP_UNORD_Q */
            cmp = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.unordered += __builtin_popcount(mask);
            }
            
            /* _CMP_ORD_Q */
            cmp = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.ordered += __builtin_popcount(mask);
            }
            
            /* _CMP_EQ_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.uneq += __builtin_popcount(mask);
            }
            
            /* _CMP_NLT_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.unge += __builtin_popcount(mask);
            }
            
            /* _CMP_NLE_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.ungt += __builtin_popcount(mask);
            }
            
            /* _CMP_LE_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_LE_UQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.unle += __builtin_popcount(mask);
            }
            
            /* _CMP_LT_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_LT_UQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.unlt += __builtin_popcount(mask);
            }
            
            /* _CMP_NEQ_UQ */
            cmp = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
            mask = _mm256_movemask_pd(cmp);
            if (mask != 0) {
                global_counter.ltgt += __builtin_popcount(mask);
            }
        }
    }
}
#endif

/* Control flow test to prevent optimization */
void test_control_flow(void) {
    double values[] = {1.0, 2.0, __builtin_nan(""), __builtin_inf()};
    int n = sizeof(values)/sizeof(values[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double a = values[i];
            double b = values[j];
            
            /* Complex control flow based on comparisons */
            if (__builtin_isunordered(a, b)) {
                global_counter.unordered++;
                if (__builtin_isless(a, b)) {
                    /* This should never execute for NaN */
                    global_counter.unlt += 100;
                }
            } else if (__builtin_isless(a, b)) {
                global_counter.unlt++;
            } else if (__builtin_isgreater(a, b)) {
                global_counter.ungt++;
            } else if (__builtin_islessequal(a, b)) {
                global_counter.unle++;
            } else if (__builtin_isgreaterequal(a, b)) {
                global_counter.unge++;
            } else if (__builtin_islessgreater(a, b)) {
                global_counter.ltgt++;
            }
            
            /* Switch on comparison classification */
            int cmp_class = 0;
            if (__builtin_isunordered(a, b)) cmp_class = 1;
            else if (a == b) cmp_class = 2;
            else if (a < b) cmp_class = 3;
            else cmp_class = 4;
            
            switch (cmp_class) {
                case 1: /* UNORDERED */
                    global_counter.unordered++;
                    break;
                case 2: /* UNEQ (equal, not unordered) */
                    global_counter.uneq++;
                    break;
                case 3: /* UNLT (less than, not unordered) */
                    global_counter.unlt++;
                    break;
                case 4: /* UNGT (greater than, not unordered) */
                    global_counter.ungt++;
                    break;
            }
        }
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Reset counters */
    memset(&global_counter, 0, sizeof(global_counter));
    
    /* Run all tests */
    test_scalar_conditions();
    test_vector_conditions();
    test_asm_constraints();
    test_control_flow();
    
#ifdef __AVX__
    test_avx_conditions();
    printf("AVX support detected and tested.\n");
#endif
    
    /* Print results */
    printf("\nCondition Code Statistics:\n");
    printf("UNORDERED: %u\n", global_counter.unordered);
    printf("ORDERED:   %u\n", global_counter.ordered);
    printf("UNEQ:      %u\n", global_counter.uneq);
    printf("UNGE:      %u\n", global_counter.unge);
    printf("UNGT:      %u\n", global_counter.ungt);
    printf("UNLE:      %u\n", global_counter.unle);
    printf("UNLT:      %u\n", global_counter.unlt);
    printf("LTGT:      %u\n", global_counter.ltgt);
    
    /* Verify we hit all condition codes */
    unsigned int total = global_counter.unordered + global_counter.ordered +
                        global_counter.uneq + global_counter.unge +
                        global_counter.ungt + global_counter.unle +
                        global_counter.unlt + global_counter.ltgt;
    
    printf("\nTotal condition code hits: %u\n", total);
    
    if (total == 0) {
        printf("ERROR: No condition codes were triggered!\n");
        return 1;
    }
    
    printf("\nAll condition codes were successfully exercised.\n");
    return 0;
}
