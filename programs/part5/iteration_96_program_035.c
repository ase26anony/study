/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered comparisons with NaN values */
static inline double make_nan(void) {
    return __builtin_nan("");
}

/* Mix of inline assembly and floating-point comparisons to trigger
   all condition code name printing */
int main(void) {
    /* Arrays with mix of normal values and NaN */
    double arr1[256];
    double arr2[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 1.3;
        
        /* Insert NaN at specific indices to force unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = make_nan();
        }
        if (i % 11 == 0) {
            arr2[i] = make_nan();
        }
    }
    
    volatile int cc_accumulator = 0;
    int result = 0;
    
    /* Loop performing all floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Perform all six standard comparisons */
        /* Each comparison may generate different condition codes */
        
        // UNORDERED/ORDERED conditions
        if (a != a || b != b) {  // Check for NaN
            cc_accumulator |= 1;
        }
        
        // UNEQ (unordered or equal)
        result = (a == b) ? (result + 1) : (result - 1);
        
        // UNGE (not less than)
        result = (a >= b) ? (result + 2) : (result - 2);
        
        // UNGT (not less than or equal)
        result = (a > b) ? (result + 3) : (result - 3);
        
        // UNLE (unordered or less than or equal)
        result = (a <= b) ? (result + 4) : (result - 4);
        
        // UNLT (unordered or less than)
        result = (a < b) ? (result + 5) : (result - 5);
        
        // LTGT (less than or greater than)
        result = (a != b) ? (result + 6) : (result - 6);
    }
    
    /* Direct inline assembly to trigger condition code printing via %C */
    /* These will directly use the condition code names in assembly output */
    
    int var1 = 42;
    int var2 = 100;
    int var3 = 200;
    
    /* UNORDERED */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var1)
        : "r"(var2), "i"(0)  /* 0 = UNORDERED */
        : "cc"
    );
    
    /* ORDERED */
    asm volatile (
        "cmov%C1 %2, %0\n\t"
        : "+r"(var1)
        : "i"(1), "r"(var3)  /* 1 = ORDERED */
        : "cc"
    );
    
    /* UNEQ */
    asm volatile (
        "cmov%C2 %1, %0\n\t"
        : "+r"(var1)
        : "r"(var2), "i"(2)  /* 2 = UNEQ */
        : "cc"
    );
    
    /* UNGE */
    asm volatile (
        "cmov%C3 %1, %0\n\t"
        : "+r"(var1)
        : "r"(var2), "i"(3)  /* 3 = UNGE */
        : "cc"
    );
    
    /* UNGT */
    asm volatile (
        "cmov%C4 %1, %0\n\t"
        : "+r"(var1)
        : "r"(var2), "i"(4)  /* 4 = UNGT */
        : "cc"
    );
    
    /* UNLE */
    asm volatile (
        "cmov%C5 %1, %0\n\t"
        : "+r"(var1)
        : "r"(var2), "i"(5)  /* 5 = UNLE */
        : "cc"
    );
    
    /* UNLT */
    asm volatile (
        "cmov%C6 %1, %0\n\t"
        : "+r"(var1)
        : "r"(var2), "i"(6)  /* 6 = UNLT */
        : "cc"
    );
    
    /* LTGT */
    asm volatile (
        "cmov%C7 %1, %0\n\t"
        : "+r"(var1)
        : "r"(var2), "i"(7)  /* 7 = LTGT */
        : "cc"
    );
    
    /* Additional floating-point comparisons that may generate condition codes */
    double x = make_nan();
    double y = 3.14159;
    
    /* Force generation of various condition codes through control flow */
    if (x < y) result += 10;    /* May generate UNLT */
    if (x <= y) result += 20;   /* May generate UNLE */
    if (x > y) result += 30;    /* May generate UNGT */
    if (x >= y) result += 40;   /* May generate UNGE */
    if (x == y) result += 50;   /* May generate UNEQ */
    if (x != y) result += 60;   /* May generate LTGT */
    
    /* Mixed integer/float conditional moves */
    int int_result = 0;
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Ternary operator with FP condition on integer target
           May lower to conditional move with condition code */
        int_result = (a < b) ? (int_result + i) : (int_result - i);
        int_result = (a <= b) ? (int_result * 2) : (int_result / 2);
        int_result = (a > b) ? (int_result | 0xFF) : (int_result & 0xFF00);
        int_result = (a >= b) ? (int_result ^ 0xAAAA) : (int_result ^ 0x5555);
        int_result = (a == b) ? (int_result << 2) : (int_result >> 2);
        int_result = (a != b) ? (int_result + 0x1000) : (int_result - 0x1000);
    }
    
    printf("Result: %d, Int result: %d, CC accumulator: %d\n", 
           result, int_result, cc_accumulator);
    
    return (result > 0) ? 0 : 1;
}
