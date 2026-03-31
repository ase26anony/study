/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <xmmintrin.h>

/* Force generation of specific condition codes */
#define FORCE_CC_PRINTING(cond, var, src) \
    do { \
        int __cond = (cond); \
        int __var = (var); \
        int __src = (src); \
        /* Use %C to print condition code name */ \
        __asm__ volatile ("cmov%C0 %2, %0" \
                         : "+r"(__var) \
                         : "i"(__cond), "r"(__src) \
                         : "cc"); \
        (var) = __var; \
    } while(0)

/* Alternative using builtin for constant propagation */
#define FORCE_CC_CONST(cond) \
    do { \
        int __dummy = 0; \
        if (__builtin_constant_p(cond)) { \
            __asm__ volatile ("# Condition code: %C0" \
                             : \
                             : "i"(cond) \
                             : "cc"); \
        } \
    } while(0)

int main(void) {
    /* Initialize arrays with mix of normal values and NaN */
    double arr1[256];
    double arr2[256];
    volatile int cc_accumulator = 0;
    
    /* Fill arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
        /* Insert NaN at specific indices to create unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
        if (i % 13 == 0) {
            arr1[i] = __builtin_nan("0xdead");
            arr2[i] = __builtin_nan("0xbeef");
        }
    }
    
    /* Force various floating-point comparisons to generate condition codes */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        volatile double volatile_a = a;
        volatile double volatile_b = b;
        
        /* Perform all standard FP comparisons */
        int lt_result = (volatile_a < volatile_b) ? 1 : 0;
        int le_result = (volatile_a <= volatile_b) ? 1 : 0;
        int gt_result = (volatile_a > volatile_b) ? 1 : 0;
        int ge_result = (volatile_a >= volatile_b) ? 1 : 0;
        int eq_result = (volatile_a == volatile_b) ? 1 : 0;
        int ne_result = (volatile_a != volatile_b) ? 1 : 0;
        
        /* Use results to prevent optimization */
        cc_accumulator += lt_result + le_result + gt_result + ge_result + eq_result + ne_result;
        
        /* Force unordered comparisons */
        int is_unordered = !(a == a) || !(b == b);
        cc_accumulator += is_unordered ? 3 : 1;
        
        /* Generate specific condition codes via inline assembly */
        if (i % 17 == 0) {
            /* UNORDERED */
            FORCE_CC_PRINTING(UNORDERED, cc_accumulator, 42);
            FORCE_CC_CONST(UNORDERED);
        }
        if (i % 19 == 0) {
            /* ORDERED */
            FORCE_CC_PRINTING(ORDERED, cc_accumulator, 43);
            FORCE_CC_CONST(ORDERED);
        }
        if (i % 23 == 0) {
            /* UNEQ */
            FORCE_CC_PRINTING(UNEQ, cc_accumulator, 44);
            FORCE_CC_CONST(UNEQ);
        }
        if (i % 29 == 0) {
            /* UNGE */
            FORCE_CC_PRINTING(UNGE, cc_accumulator, 45);
            FORCE_CC_CONST(UNGE);
        }
        if (i % 31 == 0) {
            /* UNGT */
            FORCE_CC_PRINTING(UNGT, cc_accumulator, 46);
            FORCE_CC_CONST(UNGT);
        }
        if (i % 37 == 0) {
            /* UNLE */
            FORCE_CC_PRINTING(UNLE, cc_accumulator, 47);
            FORCE_CC_CONST(UNLE);
        }
        if (i % 41 == 0) {
            /* UNLT */
            FORCE_CC_PRINTING(UNLT, cc_accumulator, 48);
            FORCE_CC_CONST(UNLT);
        }
        if (i % 43 == 0) {
            /* LTGT */
            FORCE_CC_PRINTING(LTGT, cc_accumulator, 49);
            FORCE_CC_CONST(LTGT);
        }
    }
    
    /* Additional unordered comparison scenarios */
    {
        double nan1 = __builtin_nan("");
        double nan2 = __builtin_nan("0x123");
        double inf = __builtin_inf();
        double normal = 3.14159;
        
        /* Generate various condition codes through control flow */
        volatile int result = 0;
        
        /* UNORDERED case */
        if (!(nan1 == nan1)) {
            result |= 1;
        }
        
        /* ORDERED case */
        if (normal == normal) {
            result |= 2;
        }
        
        /* Mixed comparisons */
        result += (nan1 < normal) ? 4 : 0;    /* UNLT */
        result += (nan1 <= normal) ? 8 : 0;   /* UNLE */
        result += (nan1 > normal) ? 16 : 0;   /* UNGT */
        result += (nan1 >= normal) ? 32 : 0;  /* UNGE */
        result += (nan1 == normal) ? 64 : 0;  /* UNEQ */
        result += (nan1 != normal) ? 128 : 0; /* LTGT */
        
        cc_accumulator += result;
    }
    
    /* Use x86 intrinsics for conditional moves */
    {
        int x = 0, y = 1;
        double a = __builtin_nan("");
        double b = 1.0;
        
        /* This may generate condition code printing */
        if (a < b) {
            x = y;
        }
        
        /* Ternary operator that might use conditional move */
        int z = (a != b) ? x : y;
        cc_accumulator += z;
    }
    
    /* Direct inline assembly with %C format specifier */
    __asm__ volatile (
        "# Testing condition code output:\n"
        "# %C0 = UNORDERED\n"
        "# %C1 = ORDERED\n"
        "# %C2 = UNEQ\n"
        "# %C3 = UNGE\n"
        "# %C4 = UNGT\n"
        "# %C5 = UNLE\n"
        "# %C6 = UNLT\n"
        "# %C7 = LTGT\n"
        :
        : "i"(UNORDERED), "i"(ORDERED), "i"(UNEQ), 
          "i"(UNGE), "i"(UNGT), "i"(UNLE), 
          "i"(UNLT), "i"(LTGT)
    );
    
    printf("Condition code accumulator: %d\n", cc_accumulator);
    return 0;
}
