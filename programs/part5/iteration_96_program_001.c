/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <x86intrin.h>

/* Force generation of specific condition codes */
#define FORCE_CC_PRINTING(cond, var, src) \
    do { \
        if (__builtin_constant_p(cond)) { \
            __asm__ volatile ("cmov%C0 %1, %0" \
                             : "+r"(var) \
                             : "r"(src), "i"(cond) \
                             : "cc"); \
        } \
    } while(0)

/* Direct inline assembly with %C format specifier */
#define EMIT_CC_NAME(cond) \
    do { \
        int dummy = 0; \
        __asm__ volatile ("# BEGIN CC: %C0" \
                         : \
                         : "i"(cond) \
                         : "memory"); \
        __asm__ volatile ("# END CC" ::: "memory"); \
    } while(0)

int main(void) {
    /* Create arrays with mix of normal values and NaN */
    volatile double arr1[256];
    volatile double arr2[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = (i * 1.5) - 128.0;
        arr2[i] = (i * 0.75) + 64.0;
        
        /* Insert NaN values at specific indices to create unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
        if (i % 13 == 0) {
            arr1[i] = __builtin_nan("");
            arr2[i] = __builtin_nan("");
        }
    }
    
    /* Volatile accumulator to prevent optimization */
    volatile int cc_accumulator = 0;
    int temp_result = 0;
    
    /* Loop performing all floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Perform all standard FP comparisons */
        int lt_result  = (a < b)  ? 1 : 0;
        int le_result  = (a <= b) ? 1 : 0;
        int gt_result  = (a > b)  ? 1 : 0;
        int ge_result  = (a >= b) ? 1 : 0;
        int eq_result  = (a == b) ? 1 : 0;
        int ne_result  = (a != b) ? 1 : 0;
        
        /* Use ternary operators to force potential conditional move generation */
        cc_accumulator += lt_result ? (i & 1) : (i & 2);
        cc_accumulator += le_result ? (i & 4) : (i & 8);
        cc_accumulator += gt_result ? (i & 16) : (i & 32);
        cc_accumulator += ge_result ? (i & 64) : (i & 128);
        cc_accumulator += eq_result ? (i & 256) : (i & 512);
        cc_accumulator += ne_result ? (i & 1024) : (i & 2048);
        
        /* Force unordered comparisons */
        int is_unordered = (a != a) || (b != b);
        cc_accumulator += is_unordered ? 1 : 0;
        
        /* Create control flow that depends on unordered comparisons */
        if (is_unordered) {
            temp_result ^= i;
        }
        
        /* Mixed integer/float conditional moves */
        int int_var = i;
        int int_src = i * 2;
        
        /* Direct inline assembly to trigger condition code printing */
        if (i % 17 == 0) {
            /* UNORDERED */
            EMIT_CC_NAME(UNORDERED);
            FORCE_CC_PRINTING(UNORDERED, int_var, int_src);
        }
        if (i % 19 == 0) {
            /* ORDERED */
            EMIT_CC_PRINTING(ORDERED, int_var, int_src);
        }
        if (i % 23 == 0) {
            /* UNEQ */
            EMIT_CC_PRINTING(UNEQ, int_var, int_src);
        }
        if (i % 29 == 0) {
            /* UNGE */
            EMIT_CC_PRINTING(UNGE, int_var, int_src);
        }
        if (i % 31 == 0) {
            /* UNGT */
            EMIT_CC_PRINTING(UNGT, int_var, int_src);
        }
        if (i % 37 == 0) {
            /* UNLE */
            EMIT_CC_PRINTING(UNLE, int_var, int_src);
        }
        if (i % 41 == 0) {
            /* UNLT */
            EMIT_CC_PRINTING(UNLT, int_var, int_src);
        }
        if (i % 43 == 0) {
            /* LTGT */
            EMIT_CC_PRINTING(LTGT, int_var, int_src);
        }
    }
    
    /* Additional unordered comparison scenarios */
    double nan1 = __builtin_nan("");
    double nan2 = __builtin_nan("");
    double normal = 42.0;
    
    /* Create complex expressions that might generate various condition codes */
    volatile double complex_expr = (nan1 < normal) ? 1.0 : 
                                   (normal > nan2) ? 2.0 :
                                   (nan1 == nan2) ? 3.0 :
                                   (normal != nan1) ? 4.0 : 5.0;
    
    /* Use __builtin_isfinite to generate ordered checks */
    int ordered_check = __builtin_isfinite(nan1) + __builtin_isfinite(normal);
    cc_accumulator += ordered_check;
    
    /* Print results to prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    printf("Temp result: %d\n", temp_result);
    printf("Complex expr: %f\n", complex_expr);
    
    return 0;
}
