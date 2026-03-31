/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <x86intrin.h>

/* Force unordered floating-point comparisons */
static inline int unordered_compare(double a, double b) {
    volatile int result;
    /* All standard comparisons that can generate different condition codes */
    result = (a < b) ? 1 : 0;   /* LT or UNLT */
    result += (a <= b) ? 2 : 0; /* LE or UNLE */
    result += (a > b) ? 4 : 0;  /* GT or UNGT */
    result += (a >= b) ? 8 : 0; /* GE or UNGE */
    result += (a == b) ? 16 : 0;/* EQ or UNEQ */
    result += (a != b) ? 32 : 0;/* NEQ or LTGT */
    return result;
}

/* Direct inline assembly with %C constraint */
static inline void emit_condition_codes(void) {
    int var = 0;
    int src = 42;
    
    /* UNORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(16) /* UNORDERED code */
                  : "cc");
    
    /* ORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(23) /* ORDERED code */
                  : "cc");
    
    /* UNEQ */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(18) /* UNEQ code */
                  : "cc");
    
    /* UNGE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(13) /* UNGE code */
                  : "cc");
    
    /* UNGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(14) /* UNGT code */
                  : "cc");
    
    /* UNLE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(19) /* UNLE code */
                  : "cc");
    
    /* UNLT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(12) /* UNLT code */
                  : "cc");
    
    /* LTGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(20) /* LTGT code */
                  : "cc");
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(var));
}

/* Mixed integer/float conditional moves */
static inline int float_conditional_move(double a, double b) {
    int result = 0;
    
    /* These may generate condition code names in RTL */
    result = (a < b) ? result | 0x1 : result & ~0x1;
    result = (a <= b) ? result | 0x2 : result & ~0x2;
    result = (a > b) ? result | 0x4 : result & ~0x4;
    result = (a >= b) ? result | 0x8 : result & ~0x8;
    result = (a == b) ? result | 0x10 : result & ~0x10;
    result = (a != b) ? result | 0x20 : result & ~0x20;
    
    return result;
}

int main(void) {
    double arr1[256];
    double arr2[256];
    volatile int cc_accumulator = 0;
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
        /* Insert NaN at specific positions to force unordered comparisons */
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
    
    /* Force various floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        /* Regular comparisons that may generate condition codes */
        cc_accumulator += unordered_compare(arr1[i], arr2[i]);
        
        /* Mixed integer/float conditional operations */
        cc_accumulator += float_conditional_move(arr1[i], arr2[i]);
        
        /* Direct unordered checks */
        volatile int is_unordered = isnan(arr1[i]) || isnan(arr2[i]);
        volatile int is_ordered = !is_unordered;
        
        /* Control flow based on unordered comparisons */
        if (arr1[i] != arr2[i]) {
            cc_accumulator |= 0x100;
        }
        if (!(arr1[i] < arr2[i])) {
            cc_accumulator |= 0x200;
        }
        if (!(arr1[i] > arr2[i])) {
            cc_accumulator |= 0x400;
        }
    }
    
    /* Emit condition codes via inline assembly */
    emit_condition_codes();
    
    /* Additional unordered scenarios */
    double nan1 = __builtin_nan("");
    double nan2 = __builtin_nan("0x1234");
    double normal = 3.14159;
    
    /* All possible unordered comparisons */
    volatile int r1 = (nan1 < normal);
    volatile int r2 = (nan1 <= normal);
    volatile int r3 = (nan1 > normal);
    volatile int r4 = (nan1 >= normal);
    volatile int r5 = (nan1 == normal);
    volatile int r6 = (nan1 != normal);
    
    volatile int r7 = (normal < nan1);
    volatile int r8 = (normal <= nan1);
    volatile int r9 = (normal > nan1);
    volatile int r10 = (normal >= nan1);
    volatile int r11 = (normal == nan1);
    volatile int r12 = (normal != nan1);
    
    volatile int r13 = (nan1 < nan2);
    volatile int r14 = (nan1 <= nan2);
    volatile int r15 = (nan1 > nan2);
    volatile int r16 = (nan1 >= nan2);
    volatile int r17 = (nan1 == nan2);
    volatile int r18 = (nan1 != nan2);
    
    /* Use results to prevent optimization */
    cc_accumulator += r1 + r2 + r3 + r4 + r5 + r6 +
                     r7 + r8 + r9 + r10 + r11 + r12 +
                     r13 + r14 + r15 + r16 + r17 + r18;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(cc_accumulator));
    
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    return cc_accumulator != 0 ? 0 : 1;
}
