/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force generation of specific condition codes */
#define FORCE_CC_PRINTING(cond, var, src) \
    do { \
        if (__builtin_constant_p(cond)) { \
            asm volatile ("# CC Test: %C0" : : "i"(cond)); \
            asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(cond)); \
        } \
    } while(0)

/* Direct inline assembly with %C constraint */
#define EMIT_CC_NAME(cc) \
    asm volatile ("# Condition Code: %C0" : : "i"(cc))

int main(void) {
    /* Arrays with mix of normal values and NaN */
    volatile double arr1[256];
    volatile double arr2[256];
    volatile int results[6] = {0};  /* For 6 comparison types */
    int accumulator = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
        /* Insert NaN at specific positions to create unordered comparisons */
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
    
    /* Perform various floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        volatile int cmp_result;
        
        /* All standard comparisons - each can produce different condition codes */
        cmp_result = (a < b) ? 1 : 0;   /* May generate UNLT/LT */
        results[0] += cmp_result;
        
        cmp_result = (a <= b) ? 2 : 0;  /* May generate UNLE/LE */
        results[1] += cmp_result;
        
        cmp_result = (a > b) ? 3 : 0;   /* May generate UNGT/GT */
        results[2] += cmp_result;
        
        cmp_result = (a >= b) ? 4 : 0;  /* May generate UNGE/GE */
        results[3] += cmp_result;
        
        cmp_result = (a == b) ? 5 : 0;  /* May generate UNEQ/EQ */
        results[4] += cmp_result;
        
        cmp_result = (a != b) ? 6 : 0;  /* May generate LTGT/NE */
        results[5] += cmp_result;
        
        /* Force conditional move generation with ternary operator */
        int temp = 0;
        temp = (a < b) ? (temp + 1) : (temp - 1);
        temp = (a <= b) ? (temp * 2) : (temp / 2);
        temp = (a > b) ? (temp | 0xFF) : (temp & 0x0F);
        accumulator += temp;
    }
    
    /* Direct inline assembly to trigger condition code name printing */
    int x = 42, y = 100;
    
    /* Test all condition codes from the uncovered block */
    EMIT_CC_NAME(UNORDERED);  /* Should print "unord" */
    EMIT_CC_NAME(ORDERED);    /* Should print "ord" */
    EMIT_CC_NAME(UNEQ);       /* Should print "ueq" */
    EMIT_CC_NAME(UNGE);       /* Should print "nlt" */
    EMIT_CC_NAME(UNGT);       /* Should print "nle" */
    EMIT_CC_NAME(UNLE);       /* Should print "ule" */
    EMIT_CC_NAME(UNLT);       /* Should print "ult" */
    EMIT_CC_NAME(LTGT);       /* Should print "une" */
    
    /* Force conditional moves with different condition codes */
    FORCE_CC_PRINTING(UNORDERED, x, y);
    FORCE_CC_PRINTING(ORDERED, y, x);
    FORCE_CC_PRINTING(UNEQ, x, accumulator);
    FORCE_CC_PRINTING(UNGE, y, results[0]);
    FORCE_CC_PRINTING(UNGT, x, results[1]);
    FORCE_CC_PRINTING(UNLE, y, results[2]);
    FORCE_CC_PRINTING(UNLT, x, results[3]);
    FORCE_CC_PRINTING(LTGT, y, results[4]);
    
    /* Complex floating-point expression to generate RTL with condition codes */
    volatile double sum = 0.0;
    for (int i = 0; i < 256; i++) {
        /* This complex expression should generate various FP comparisons */
        double a = arr1[i];
        double b = arr2[i];
        
        /* Nested comparisons to force condition code generation */
        if (!(a < b) && (a >= b) && (a != b)) {
            sum += 1.0;
        } else if ((a == b) || !(a <= b)) {
            sum -= 1.0;
        } else if (isunordered(a, b)) {
            sum *= 2.0;  /* Explicit unordered check */
        } else if (islessgreater(a, b)) {
            sum /= 2.0;  /* Explicit lessgreater check */
        }
    }
    
    /* Prevent optimization */
    printf("Results: %d %d %d %d %d %d\n", 
           results[0], results[1], results[2], 
           results[3], results[4], results[5]);
    printf("Accumulator: %d\n", accumulator);
    printf("Sum: %f\n", sum);
    printf("x=%d, y=%d\n", x, y);
    
    return 0;
}
