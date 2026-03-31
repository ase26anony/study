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
} cc_counters;

/* Global counters to prevent optimization */
volatile cc_counters global_counters = {0};

/* Test data with various floating-point values */
static const double test_scalars[] = {
    1.0, 2.0, -1.0, 0.0, -0.0,
    __builtin_nan(""), -__builtin_nan(""),
    INFINITY, -INFINITY,
    DBL_MAX, DBL_MIN,
    3.14159265358979323846
};

static const size_t num_scalars = sizeof(test_scalars) / sizeof(test_scalars[0]);

/* Test UNORDERED and ORDERED condition codes */
void test_unordered_ordered(void) {
    for (size_t i = 0; i < num_scalars; i++) {
        for (size_t j = 0; j < num_scalars; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNORDERED: true if either operand is NaN */
            if (__builtin_isunordered(a, b)) {
                global_counters.unordered++;
            }
            
            /* ORDERED: false if either operand is NaN */
            if (__builtin_isordered(a, b)) {
                global_counters.ordered++;
            }
        }
    }
}

/* Test UNEQ, UNGE, UNGT, UNLE, UNLT condition codes */
void test_unequal_comparisons(void) {
    for (size_t i = 0; i < num_scalars; i++) {
        for (size_t j = 0; j < num_scalars; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* UNEQ: unordered or equal */
            if (!(a > b) && !(a < b)) {  /* Equivalent to !(a > b) && !(a < b) */
                global_counters.uneq++;
            }
            
            /* UNGE: unordered or greater or equal */
            if (!(a < b)) {  /* Equivalent to !(a < b) */
                global_counters.unge++;
            }
            
            /* UNGT: unordered or greater */
            if (!(a <= b)) {  /* Equivalent to !(a <= b) */
                global_counters.ungt++;
            }
            
            /* UNLE: unordered or less or equal */
            if (!(a > b)) {  /* Equivalent to !(a > b) */
                global_counters.unle++;
            }
            
            /* UNLT: unordered or less */
            if (!(a >= b)) {  /* Equivalent to !(a >= b) */
                global_counters.unlt++;
            }
        }
    }
}

/* Test LTGT (unordered or not equal) condition code */
void test_ltgt_comparisons(void) {
    for (size_t i = 0; i < num_scalars; i++) {
        for (size_t j = 0; j < num_scalars; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            
            /* LTGT: less, greater, or unordered (but not equal) */
            if (__builtin_islessgreater(a, b)) {
                global_counters.ltgt++;
            }
        }
    }
}

/* Vector comparisons using SSE/AVX intrinsics */
void test_vector_conditions(void) {
#ifdef __SSE2__
    __m128d vec_a, vec_b, vec_cmp;
    __m128d zero = _mm_setzero_pd();
    __m128d nan_vec = _mm_set1_pd(__builtin_nan(""));
    __m128d inf_vec = _mm_set1_pd(INFINITY);
    
    /* Test various vector comparisons */
    for (size_t i = 0; i < num_scalars - 1; i += 2) {
        vec_a = _mm_set_pd(test_scalars[i], test_scalars[i+1]);
        vec_b = _mm_set_pd(test_scalars[(i+1) % num_scalars], 
                          test_scalars[(i+2) % num_scalars]);
        
        /* Compare unordered (CMP_UNORD) */
        vec_cmp = _mm_cmpunord_pd(vec_a, vec_b);
        if (_mm_movemask_pd(vec_cmp) != 0) {
            global_counters.unordered++;
        }
        
        /* Compare ordered (CMP_ORD) */
        vec_cmp = _mm_cmpord_pd(vec_a, vec_b);
        if (_mm_movemask_pd(vec_cmp) != 0) {
            global_counters.ordered++;
        }
        
        /* Compare not less than (CMP_NLT) */
        vec_cmp = _mm_cmpnlt_pd(vec_a, vec_b);
        if (_mm_movemask_pd(vec_cmp) != 0) {
            global_counters.unge++;  /* nlt corresponds to UNGE */
        }
        
        /* Compare not less than or equal (CMP_NLE) */
        vec_cmp = _mm_cmpnle_pd(vec_a, vec_b);
        if (_mm_movemask_pd(vec_cmp) != 0) {
            global_counters.ungt++;  /* nle corresponds to UNGT */
        }
        
        /* Compare unordered or less than or equal (CMP_ULE) */
        vec_cmp = _mm_cmpule_pd(vec_a, vec_b);
        if (_mm_movemask_pd(vec_cmp) != 0) {
            global_counters.unle++;
        }
        
        /* Compare unordered or less than (CMP_ULT) */
        vec_cmp = _mm_cmpult_pd(vec_a, vec_b);
        if (_mm_movemask_pd(vec_cmp) != 0) {
            global_counters.unlt++;
        }
        
        /* Compare not equal (CMP_NEQ) */
        vec_cmp = _mm_cmpneq_pd(vec_a, vec_b);
        if (_mm_movemask_pd(vec_cmp) != 0) {
            global_counters.ltgt++;  /* neq corresponds to LTGT */
        }
    }
#endif
}

/* Inline assembly with explicit condition code constraints */
void test_asm_condition_codes(void) {
    for (size_t i = 0; i < num_scalars; i++) {
        for (size_t j = 0; j < num_scalars; j++) {
            double a = test_scalars[i];
            double b = test_scalars[j];
            int result;
            
            /* UNORDERED condition code */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) global_counters.unordered++;
            
            /* ORDERED condition code */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnp %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) global_counters.ordered++;
            
            /* UNEQ condition code */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "sete %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) global_counters.uneq++;
            
            /* UNGE (nlt) condition code */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnb %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) global_counters.unge++;
            
            /* UNGT (nle) condition code */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setnbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) global_counters.ungt++;
            
            /* UNLE condition code */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setbe %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) global_counters.unle++;
            
            /* UNLT condition code */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setb %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) global_counters.unlt++;
            
            /* LTGT (une) condition code */
            __asm__ volatile (
                "ucomisd %2, %1\n\t"
                "setne %%al\n\t"
                "movzbl %%al, %0"
                : "=r" (result)
                : "x" (a), "x" (b)
                : "al", "cc"
            );
            if (result) global_counters.ltgt++;
        }
    }
}

/* AVX/AVX512 vector comparisons if available */
#ifdef __AVX__
void test_avx_conditions(void) {
    __m256d vec_a, vec_b, vec_cmp;
    __m256d nan_vec = _mm256_set1_pd(__builtin_nan(""));
    
    for (size_t i = 0; i < num_scalars - 3; i += 4) {
        vec_a = _mm256_set_pd(test_scalars[i], test_scalars[i+1],
                             test_scalars[i+2], test_scalars[i+3]);
        vec_b = _mm256_set_pd(test_scalars[(i+1) % num_scalars],
                             test_scalars[(i+2) % num_scalars],
                             test_scalars[(i+3) % num_scalars],
                             test_scalars[(i+4) % num_scalars]);
        
        /* CMP_UNORD_Q */
        vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
        if (_mm256_movemask_pd(vec_cmp) != 0) {
            global_counters.unordered++;
        }
        
        /* CMP_ORD_Q */
        vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
        if (_mm256_movemask_pd(vec_cmp) != 0) {
            global_counters.ordered++;
        }
        
        /* CMP_NLT_UQ */
        vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_NLT_UQ);
        if (_mm256_movemask_pd(vec_cmp) != 0) {
            global_counters.unge++;
        }
        
        /* CMP_NLE_UQ */
        vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_NLE_UQ);
        if (_mm256_movemask_pd(vec_cmp) != 0) {
            global_counters.ungt++;
        }
        
        /* CMP_ULE_OQ */
        vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_ULE_OQ);
        if (_mm256_movemask_pd(vec_cmp) != 0) {
            global_counters.unle++;
        }
        
        /* CMP_ULT_OQ */
        vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_ULT_OQ);
        if (_mm256_movemask_pd(vec_cmp) != 0) {
            global_counters.unlt++;
        }
        
        /* CMP_NEQ_OQ */
        vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_NEQ_OQ);
        if (_mm256_movemask_pd(vec_cmp) != 0) {
            global_counters.ltgt++;
        }
        
        /* CMP_NEQ_UQ for UNEQ */
        vec_cmp = _mm256_cmp_pd(vec_a, vec_b, _CMP_NEQ_UQ);
        if (_mm256_movemask_pd(vec_cmp) == 0) {  /* Note: inverted logic for equality */
            global_counters.uneq++;
        }
    }
}
#endif

/* Control flow based on comparison results */
void test_control_flow_conditions(void) {
    double special_values[] = {1.0, -1.0, 0.0, INFINITY, -INFINITY, __builtin_nan("")};
    
    for (size_t i = 0; i < sizeof(special_values)/sizeof(special_values[0]); i++) {
        for (size_t j = 0; j < sizeof(special_values)/sizeof(special_values[0]); j++) {
            double a = special_values[i];
            double b = special_values[j];
            
            /* Complex control flow to prevent optimization */
            if (__builtin_isunordered(a, b)) {
                global_counters.unordered++;
                if (__builtin_isordered(a, b)) {
                    /* This should never happen, but creates control flow */
                    global_counters.ordered++;
                }
            } else {
                global_counters.ordered++;
                
                if (__builtin_islessgreater(a, b)) {
                    global_counters.ltgt++;
                    
                    if (!(a > b)) {
                        global_counters.unle++;
                        if (!(a < b)) {
                            global_counters.uneq++;
                        }
                    }
                    
                    if (!(a < b)) {
                        global_counters.unge++;
                    }
                    
                    if (!(a >= b)) {
                        global_counters.unlt++;
                    }
                    
                    if (!(a <= b)) {
                        global_counters.ungt++;
                    }
                }
            }
        }
    }
}

int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Reset counters */
    memset((void*)&global_counters, 0, sizeof(global_counters));
    
    /* Execute all test functions */
    test_unordered_ordered();
    test_unequal_comparisons();
    test_ltgt_comparisons();
    test_vector_conditions();
    test_asm_condition_codes();
    test_control_flow_conditions();
    
#ifdef __AVX__
    test_avx_conditions();
#endif
    
    /* Print summary of condition code hits */
    printf("\nCondition Code Summary:\n");
    printf("UNORDERED: %u\n", global_counters.unordered);
    printf("ORDERED:   %u\n", global_counters.ordered);
    printf("UNEQ:      %u\n", global_counters.uneq);
    printf("UNGE:      %u\n", global_counters.unge);
    printf("UNGT:      %u\n", global_counters.ungt);
    printf("UNLE:      %u\n", global_counters.unle);
    printf("UNLT:      %u\n", global_counters.unlt);
    printf("LTGT:      %u\n", global_counters.ltgt);
    
    /* Verify all condition codes were triggered */
    int all_triggered = (global_counters.unordered > 0 &&
                        global_counters.ordered > 0 &&
                        global_counters.uneq > 0 &&
                        global_counters.unge > 0 &&
                        global_counters.ungt > 0 &&
                        global_counters.unle > 0 &&
                        global_counters.unlt > 0 &&
                        global_counters.ltgt > 0);
    
    if (all_triggered) {
        printf("\nSUCCESS: All condition codes were triggered!\n");
        return 0;
    } else {
        printf("\nWARNING: Some condition codes may not have been triggered\n");
        return 1;
    }
}
