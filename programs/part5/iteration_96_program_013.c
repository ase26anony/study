/* Compile with: gcc -O2 -dp -march=x86-64 -masm=intel -o cc_test cc_test.c */
/* For RTL dumps: gcc -O2 -fdump-rtl-final -fdump-rtl-expand -m32 cc_test.c */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Force unordered comparisons by mixing NaN values */
static inline double get_nan(void) {
    return __builtin_nan("");
}

/* Inline assembly that directly uses %C format specifier */
#define EMIT_COND_CODE(cond, var, src) \
    __asm__ volatile ("cmov%C0 %1, %0\n\t" \
                      : "+r"(var) \
                      : "r"(src), "i"(cond) \
                      : "cc")

/* Another inline assembly pattern that triggers condition code printing */
#define EMIT_FCMOV(cond, dst, src) \
    __asm__ volatile ("fcmov%C0 %1, %0\n\t" \
                      : "+t"(dst) \
                      : "u"(src), "i"(cond) \
                      : "cc")

int main(void) {
    /* Arrays with mix of normal values and NaN */
    double arr1[256];
    double arr2[256];
    volatile int cc_accumulator = 0;
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.1;
        
        /* Insert NaN at specific indices to create unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = get_nan();
        }
        if (i % 11 == 0) {
            arr2[i] = get_nan();
        }
    }
    
    /* Perform all floating-point comparisons to generate various condition codes */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        int temp;
        
        /* Each comparison should generate different condition codes */
        
        /* UNORDERED and ORDERED codes */
        if (a != a || b != b) {  /* Check for NaN */
            /* This may generate UNORDERED condition */
            temp = (a < b) ? 1 : 0;
            cc_accumulator += temp;
        }
        
        /* UNEQ (unordered or equal) */
        temp = (a == b) ? 2 : 0;
        cc_accumulator += temp;
        
        /* UNGE (unordered or greater or equal) */
        temp = (a >= b) ? 3 : 0;
        cc_accumulator += temp;
        
        /* UNGT (unordered or greater) */
        temp = (a > b) ? 4 : 0;
        cc_accumulator += temp;
        
        /* UNLE (unordered or less or equal) */
        temp = (a <= b) ? 5 : 0;
        cc_accumulator += temp;
        
        /* UNLT (unordered or less) */
        temp = (a < b) ? 6 : 0;
        cc_accumulator += temp;
        
        /* LTGT (less, greater, or unordered but not equal) */
        temp = (a != b) ? 7 : 0;
        cc_accumulator += temp;
        
        /* Use ternary operators to force conditional move generation */
        result = (a < b) ? result | 0x1 : result & ~0x1;
        result = (a <= b) ? result | 0x2 : result & ~0x2;
        result = (a > b) ? result | 0x4 : result & ~0x4;
        result = (a >= b) ? result | 0x8 : result & ~0x8;
        result = (a == b) ? result | 0x10 : result & ~0x10;
        result = (a != b) ? result | 0x20 : result & ~0x20;
    }
    
    /* Direct inline assembly to trigger condition code printing */
    /* These use the %C format specifier which should hit the uncovered code */
    
    int x = 42;
    int y = 100;
    
    /* UNORDERED */
    if (__builtin_constant_p(0)) {
        EMIT_COND_CODE(UNORDERED, x, y);
    }
    
    /* ORDERED */
    EMIT_COND_CODE(ORDERED, x, y + 1);
    
    /* UNEQ */
    EMIT_COND_CODE(UNEQ, x, y + 2);
    
    /* UNGE */
    EMIT_COND_CODE(UNGE, x, y + 3);
    
    /* UNGT */
    EMIT_COND_CODE(UNGT, x, y + 4);
    
    /* UNLE */
    EMIT_COND_CODE(UNLE, x, y + 5);
    
    /* UNLT */
    EMIT_COND_CODE(UNLT, x, y + 6);
    
    /* LTGT */
    EMIT_COND_CODE(LTGT, x, y + 7);
    
    /* Floating point conditional moves */
    double fp1 = 3.14;
    double fp2 = 2.71;
    
    EMIT_FCMOV(UNORDERED, fp1, fp2);
    EMIT_FCMOV(ORDERED, fp1, fp2 + 1.0);
    
    /* Complex expression that forces RTL with condition codes */
    volatile double test_val = arr1[0];
    for (int i = 1; i < 256; i++) {
        /* This complex conditional should generate various condition codes */
        if ((arr1[i] < arr2[i]) && !(arr1[i] != arr2[i]) || (arr1[i] >= arr2[i])) {
            test_val += arr1[i];
        } else if (arr1[i] == arr2[i] || arr1[i] <= arr2[i]) {
            test_val -= arr2[i];
        }
    }
    
    /* Prevent optimization */
    printf("Result: %d, Accumulator: %d, Test: %f\n", result, cc_accumulator, test_val);
    printf("x = %d, fp1 = %f\n", x, fp1);
    
    return 0;
}
