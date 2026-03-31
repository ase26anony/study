/* gcc -O2 -dp -march=x86-64 -masm=intel -o test_cc test_cc.c */
/* Additional flags for RTL analysis: -fdump-rtl-final -fdump-rtl-expand -m32 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered floating-point comparisons */
static inline int unordered_compare(double a, double b) {
    volatile int result;
    /* All standard FP comparisons to generate different condition codes */
    result = (a < b) ? 1 : 0;   /* UNLT or LT */
    result += (a <= b) ? 1 : 0; /* UNLE or LE */
    result += (a > b) ? 1 : 0;  /* UNGT or GT */
    result += (a >= b) ? 1 : 0; /* UNGE or GE */
    result += (a == b) ? 1 : 0; /* UNEQ or EQ */
    result += (a != b) ? 1 : 0; /* LTGT or NE */
    return result;
}

/* Direct inline assembly with %C constraint to trigger condition code printing */
static inline void force_cc_printing(int cond_code) {
    int src = 42;
    int dst = 0;
    
    /* Use different condition codes to cover all cases */
    switch (cond_code) {
        case 0: /* UNORDERED */
            __asm__ volatile ("cmov%C0 %1, %0\n\t" 
                            : "+r"(dst) : "r"(src), "i"(0) : "cc");
            break;
        case 1: /* ORDERED */
            __asm__ volatile ("cmov%C0 %1, %0\n\t" 
                            : "+r"(dst) : "r"(src), "i"(1) : "cc");
            break;
        case 2: /* UNEQ */
            __asm__ volatile ("cmov%C0 %1, %0\n\t" 
                            : "+r"(dst) : "r"(src), "i"(2) : "cc");
            break;
        case 3: /* UNGE */
            __asm__ volatile ("cmov%C0 %1, %0\n\t" 
                            : "+r"(dst) : "r"(src), "i"(3) : "cc");
            break;
        case 4: /* UNGT */
            __asm__ volatile ("cmov%C0 %1, %0\n\t" 
                            : "+r"(dst) : "r"(src), "i"(4) : "cc");
            break;
        case 5: /* UNLE */
            __asm__ volatile ("cmov%C0 %1, %0\n\t" 
                            : "+r"(dst) : "r"(src), "i"(5) : "cc");
            break;
        case 6: /* UNLT */
            __asm__ volatile ("cmov%C0 %1, %0\n\t" 
                            : "+r"(dst) : "r"(src), "i"(6) : "cc");
            break;
        case 7: /* LTGT */
            __asm__ volatile ("cmov%C0 %1, %0\n\t" 
                            : "+r"(dst) : "r"(src), "i"(7) : "cc");
            break;
    }
    
    /* Prevent optimization */
    __asm__ volatile ("" : : "r"(dst));
}

/* Mixed integer/float conditional moves */
static inline int float_conditional_move(double a, double b) {
    int result = 0;
    
    /* These may generate conditional moves with FP condition codes */
    result = (a < b) ? result | 0x1 : result & ~0x1;
    result = (a <= b) ? result | 0x2 : result & ~0x2;
    result = (a > b) ? result | 0x4 : result & ~0x4;
    result = (a >= b) ? result | 0x8 : result & ~0x8;
    result = (a == b) ? result | 0x10 : result & ~0x10;
    result = (a != b) ? result | 0x20 : result & ~0x20;
    
    return result;
}

int main() {
    /* Create arrays with NaN values at specific positions */
    double arr1[256];
    double arr2[256];
    volatile int cc_accumulator = 0;
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
        /* Insert NaN at specific indices to force unordered comparisons */
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
    
    /* Perform comparisons that should generate various condition codes */
    for (int i = 0; i < 256; i++) {
        /* Force unordered comparisons */
        cc_accumulator += unordered_compare(arr1[i], arr2[i]);
        
        /* Mixed integer/float conditional operations */
        cc_accumulator += float_conditional_move(arr1[i], arr2[i]);
        
        /* Direct control flow based on unordered comparisons */
        if (!(arr1[i] == arr2[i])) {  /* May generate UNEQ or NE */
            cc_accumulator ^= i;
        }
        
        if (arr1[i] < arr2[i]) {  /* May generate UNLT or LT */
            cc_accumulator |= 0x1000;
        }
        
        /* Ordered vs unordered checks */
        if (arr1[i] == arr1[i] && arr2[i] == arr2[i]) {  /* Both are numbers (not NaN) */
            cc_accumulator += 1;  /* ORDERED case */
        } else {
            cc_accumulator -= 1;  /* UNORDERED case */
        }
    }
    
    /* Force all condition code printing variants */
    for (int cc = 0; cc < 8; cc++) {
        force_cc_printing(cc);
    }
    
    /* Additional complex floating expressions */
    double x = __builtin_nan("");
    double y = 3.14159;
    double z = -2.71828;
    
    /* Chain comparisons to force complex condition code generation */
    volatile int complex_result = 0;
    complex_result = (x < y) ? 1 : 0;
    complex_result += (y > z) ? 2 : 0;
    complex_result += (x != x) ? 4 : 0;  /* Always true for NaN - UNORDERED */
    complex_result += (z == z) ? 8 : 0;  /* Always true for normal number - ORDERED */
    
    /* Mixed comparisons */
    complex_result += ((x < y) && (y > z)) ? 16 : 0;  /* May combine condition codes */
    complex_result += ((x != x) || (z == z)) ? 32 : 0; /* UNORDERED or ORDERED */
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(cc_accumulator), "r"(complex_result));
    
    printf("Result: %d (complex: %d)\n", cc_accumulator, complex_result);
    
    return 0;
}
