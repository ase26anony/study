/* Condition Code Test Program for i386.cc coverage */
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
                            : "r"(src), "i"(cond)); \
        } \
    } while(0)

/* Direct inline assembly with %C format specifier */
#define EMIT_CC_NAME(cc) \
    __asm__ volatile ("# CC: %C0" :: "i"(cc))

int main(void) {
    /* Arrays with mixed normal and NaN values */
    volatile double arr1[256];
    volatile double arr2[256];
    volatile int cc_accumulator = 0;
    int result = 0;
    
    /* Initialize arrays */
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
    
    /* Force generation of all condition codes through comparisons */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        int temp;
        
        /* Generate UNORDERED (unord) - true if either operand is NaN */
        if (!(a == a) || !(b == b)) {
            cc_accumulator |= 1;
            EMIT_CC_NAME(UNORDERED);
        }
        
        /* Generate ORDERED (ord) - true if both operands are not NaN */
        if (a == a && b == b) {
            cc_accumulator |= 2;
            EMIT_CC_NAME(ORDERED);
        }
        
        /* Generate UNEQ (ueq) - unordered or equal */
        if (a != a || b != b || a == b) {
            cc_accumulator |= 4;
            EMIT_CC_NAME(UNEQ);
        }
        
        /* Generate UNGE (nlt) - unordered or greater than or equal */
        if (a != a || b != b || a >= b) {
            cc_accumulator |= 8;
            EMIT_CC_NAME(UNGE);
        }
        
        /* Generate UNGT (nle) - unordered or greater than */
        if (a != a || b != b || a > b) {
            cc_accumulator |= 16;
            EMIT_CC_NAME(UNGT);
        }
        
        /* Generate UNLE (ule) - unordered or less than or equal */
        if (a != a || b != b || a <= b) {
            cc_accumulator |= 32;
            EMIT_CC_NAME(UNLE);
        }
        
        /* Generate UNLT (ult) - unordered or less than */
        if (a != a || b != b || a < b) {
            cc_accumulator |= 64;
            EMIT_CC_NAME(UNLT);
        }
        
        /* Generate LTGT (une) - less than or greater than (ordered, not equal) */
        if (a == a && b == b && a != b) {
            cc_accumulator |= 128;
            EMIT_CC_NAME(LTGT);
        }
        
        /* Use ternary operators to force conditional move generation */
        temp = (a < b) ? 1 : 0;
        result += temp;
        
        temp = (a <= b) ? 2 : 0;
        result += temp;
        
        temp = (a > b) ? 4 : 0;
        result += temp;
        
        temp = (a >= b) ? 8 : 0;
        result += temp;
        
        temp = (a == b) ? 16 : 0;
        result += temp;
        
        temp = (a != b) ? 32 : 0;
        result += temp;
    }
    
    /* Direct inline assembly with %C constraint for each condition code */
    int x = 0, y = 1;
    
    FORCE_CC_PRINTING(UNORDERED, x, y);
    FORCE_CC_PRINTING(ORDERED, x, y);
    FORCE_CC_PRINTING(UNEQ, x, y);
    FORCE_CC_PRINTING(UNGE, x, y);
    FORCE_CC_PRINTING(UNGT, x, y);
    FORCE_CC_PRINTING(UNLE, x, y);
    FORCE_CC_PRINTING(UNLT, x, y);
    FORCE_CC_PRINTING(LTGT, x, y);
    
    /* Additional floating-point comparisons to ensure RTL generation */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("0x123");
    volatile double normal = 3.14159;
    
    /* Complex expression that should generate various condition codes */
    volatile int complex_result = 0;
    for (int i = 0; i < 100; i++) {
        double val = i * 0.1;
        
        /* Mix ordered and unordered comparisons */
        if ((val < normal) && (nan1 == nan1)) {
            complex_result++;
        }
        
        if ((val >= nan2) || (normal != normal)) {
            complex_result--;
        }
        
        if (!(val <= nan1) && (normal == normal)) {
            complex_result += 2;
        }
    }
    
    /* Prevent optimization */
    printf("Result: %d, CC Accumulator: %d, Complex: %d\n", 
           result, cc_accumulator, complex_result);
    
    return 0;
}
