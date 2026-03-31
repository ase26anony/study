/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force generation of specific condition codes */
#define FORCE_CC_PRINTING(cond) \
    do { \
        int __result; \
        asm volatile ("cmov%C0 %1, %0" \
                      : "+r"(__result) \
                      : "r"(1), "i"(cond)); \
    } while(0)

/* Alternative inline assembly with %C constraint */
#define EMIT_CONDITION_CODE(cc_name, var) \
    asm volatile ("# Condition: " cc_name "\n\t" \
                  "set%C0 %1" \
                  : "=r"(var) \
                  : "i"(cc_name) : "memory")

int main(void) {
    /* Arrays with mixed normal values and NaN */
    double arr1[256];
    double arr2[256];
    volatile int cc_accumulator = 0;
    
    /* Initialize arrays with pattern: normal values and NaNs */
    for (int i = 0; i < 256; i++) {
        if (i % 7 == 0) {
            /* Insert NaN values to trigger unordered comparisons */
            arr1[i] = __builtin_nan("");
            arr2[i] = (i % 14 == 0) ? __builtin_nan("") : i * 1.5;
        } else {
            arr1[i] = i * 1.5;
            arr2[i] = (i - 1) * 1.5;
        }
    }
    
    /* Force unordered comparisons that generate UNORDERED/ORDERED codes */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        int cmp_result;
        
        /* Generate all condition codes through comparisons */
        if (a < b)  cc_accumulator |= 0x01;   /* LT */
        if (a <= b) cc_accumulator |= 0x02;   /* LE */
        if (a > b)  cc_accumulator |= 0x04;   /* GT */
        if (a >= b) cc_accumulator |= 0x08;   /* GE */
        if (a == b) cc_accumulator |= 0x10;   /* EQ */
        if (a != b) cc_accumulator |= 0x20;   /* NEQ */
        
        /* Ternary operators that may generate conditional moves */
        cmp_result = (a < b) ? 1 : 0;
        cc_accumulator += cmp_result;
        
        cmp_result = (a != b) ? 2 : 0;
        cc_accumulator += cmp_result;
        
        /* Mixed integer/float comparison */
        if (!(a >= b)) cc_accumulator |= 0x40;  /* NLT -> UNGE */
        if (!(a <= b)) cc_accumulator |= 0x80;  /* NLE -> UNGT */
    }
    
    /* Direct inline assembly to trigger condition code printing */
    /* These should appear in assembly output with -dp or -dP */
    
    /* UNORDERED */
    asm volatile ("# UNORDERED test\n\t"
                  "fucomip %%st(1), %%st\n\t"
                  "set%C0 %%al"
                  : 
                  : "i"(UNORDERED)
                  : "cc", "eax");
    
    /* ORDERED */
    asm volatile ("# ORDERED test\n\t"
                  "fucomip %%st(1), %%st\n\t"
                  "set%C0 %%al"
                  : 
                  : "i"(ORDERED)
                  : "cc", "eax");
    
    /* UNEQ */
    asm volatile ("# UNEQ test\n\t"
                  "fucomip %%st(1), %%st\n\t"
                  "set%C0 %%bl"
                  : 
                  : "i"(UNEQ)
                  : "cc", "ebx");
    
    /* UNGE */
    asm volatile ("# UNGE test\n\t"
                  "fucomip %%st(1), %%st\n\t"
                  "set%C0 %%cl"
                  : 
                  : "i"(UNGE)
                  : "cc", "ecx");
    
    /* UNGT */
    asm volatile ("# UNGT test\n\t"
                  "fucomip %%st(1), %%st\n\t"
                  "set%C0 %%dl"
                  : 
                  : "i"(UNGT)
                  : "cc", "edx");
    
    /* UNLE */
    asm volatile ("# UNLE test\n\t"
                  "fucomip %%st(1), %%st\n\t"
                  "set%C0 %%al"
                  : 
                  : "i"(UNLE)
                  : "cc", "eax");
    
    /* UNLT */
    asm volatile ("# UNLT test\n\t"
                  "fucomip %%st(1), %%st\n\t"
                  "set%C0 %%bl"
                  : 
                  : "i"(UNLT)
                  : "cc", "ebx");
    
    /* LTGT */
    asm volatile ("# LTGT test\n\t"
                  "fucomip %%st(1), %%st\n\t"
                  "set%C0 %%cl"
                  : 
                  : "i"(LTGT)
                  : "cc", "ecx");
    
    /* Complex expression that forces multiple condition codes */
    double x = __builtin_nan("");
    double y = 3.14159;
    double z = 2.71828;
    
    /* This should generate various condition code checks */
    volatile int complex_result = 0;
    if ((x < y) || (y >= z) || (x != x) || (z == y)) {
        complex_result = 1;
    }
    
    /* Use __builtin_constant_p to ensure asm isn't eliminated */
    if (__builtin_constant_p(0)) {
        /* This block won't execute, but compiler must consider it */
        FORCE_CC_PRINTING(UNORDERED);
        FORCE_CC_PRINTING(ORDERED);
        FORCE_CC_PRINTING(UNEQ);
    }
    
    /* Prevent dead code elimination */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    printf("Complex result: %d\n", complex_result);
    
    /* Additional NaN comparisons to ensure unordered handling */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("0x1234");
    volatile double normal = 42.0;
    
    /* These comparisons with NaN should generate UNORDERED codes */
    volatile int nan_cmp1 = (nan1 < normal);
    volatile int nan_cmp2 = (normal > nan2);
    volatile int nan_cmp3 = (nan1 == nan2);
    volatile int nan_cmp4 = (nan1 != normal);
    
    printf("NaN comparisons: %d %d %d %d\n", 
           nan_cmp1, nan_cmp2, nan_cmp3, nan_cmp4);
    
    return 0;
}
