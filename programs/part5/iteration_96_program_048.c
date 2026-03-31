/* Condition Code Test Program for i386.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <x86intrin.h>

/* Force generation of various condition codes */
#define FORCE_CC_PRINTING 1

/* Mix of normal values and NaN */
static void init_arrays(double arr1[256], double arr2[256]) {
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.1;
        
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
}

/* Direct inline assembly with %C constraint to trigger condition code printing */
static void direct_cc_asm(void) {
    int var = 42;
    int src = 99;
    
    /* UNORDERED condition */
    asm volatile ("# UNORDERED test begin"
                  "\n\tcmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(0)  /* 0 = UNORDERED */
                  : "cc");
    
    /* ORDERED condition */
    asm volatile ("# ORDERED test begin"
                  "\n\tcmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(7)  /* 7 = ORDERED */
                  : "cc");
    
    /* UNEQ condition */
    asm volatile ("# UNEQ test begin"
                  "\n\tcmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(6)  /* 6 = UNEQ */
                  : "cc");
    
    /* UNGE condition */
    asm volatile ("# UNGE test begin"
                  "\n\tcmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(5)  /* 5 = UNGE */
                  : "cc");
    
    /* UNGT condition */
    asm volatile ("# UNGT test begin"
                  "\n\tcmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(4)  /* 4 = UNGT */
                  : "cc");
    
    /* UNLE condition */
    asm volatile ("# UNLE test begin"
                  "\n\tcmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(3)  /* 3 = UNLE */
                  : "cc");
    
    /* UNLT condition */
    asm volatile ("# UNLT test begin"
                  "\n\tcmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(2)  /* 2 = UNLT */
                  : "cc");
    
    /* LTGT condition */
    asm volatile ("# LTGT test begin"
                  "\n\tcmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(1)  /* 1 = LTGT */
                  : "cc");
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(var));
}

/* Generate floating-point comparisons that produce various condition codes */
static int generate_fp_comparisons(double arr1[256], double arr2[256]) {
    volatile int cc_accumulator = 0;
    int result = 0;
    
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Generate all standard comparisons - each produces different condition codes */
        int lt = (a < b) ? 1 : 0;      /* May generate UNLT/UNORDERED */
        int le = (a <= b) ? 2 : 0;     /* May generate UNLE/UNORDERED */
        int gt = (a > b) ? 4 : 0;      /* May generate UNGT/UNORDERED */
        int ge = (a >= b) ? 8 : 0;     /* May generate UNGE/UNORDERED */
        int eq = (a == b) ? 16 : 0;    /* May generate UNEQ/UNORDERED */
        int ne = (a != b) ? 32 : 0;    /* May generate LTGT/UNORDERED */
        
        /* Use volatile to prevent optimization */
        cc_accumulator = lt + le + gt + ge + eq + ne;
        
        /* Force conditional move generation through ternary operators */
        result += (a < b) ? (i * 2) : (i * 3);
        result += (a <= b) ? (i * 5) : (i * 7);
        result += (a > b) ? (i * 11) : (i * 13);
        result += (a >= b) ? (i * 17) : (i * 19);
        result += (a == b) ? (i * 23) : (i * 29);
        result += (a != b) ? (i * 31) : (i * 37);
        
        /* Ordered/unordered checks */
        int ordered = !isunordered(a, b) ? 64 : 0;
        int unordered = isunordered(a, b) ? 128 : 0;
        cc_accumulator += ordered + unordered;
        
        /* Use builtin to check for constant propagation */
        if (__builtin_constant_p(i)) {
            asm volatile ("# Constant propagation check %0" : : "r"(i));
        }
    }
    
    return result + cc_accumulator;
}

/* Mixed integer/float conditional moves */
static void mixed_conditional_moves(void) {
    double f1 = __builtin_nan("");
    double f2 = 3.14159;
    int i1, i2, i3;
    
    /* These may generate condition code checks */
    i1 = (f1 < f2) ? 100 : 200;
    i2 = (f1 == f2) ? 300 : 400;
    i3 = (!isunordered(f1, f2)) ? 500 : 600;
    
    /* Force usage */
    asm volatile ("" : : "r"(i1), "r"(i2), "r"(i3));
}

/* Additional unordered comparison scenarios */
static void special_nan_comparisons(void) {
    volatile double nan1 = __builtin_nan("0x1");
    volatile double nan2 = __builtin_nan("0x2");
    volatile double normal = 42.0;
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    
    /* These should generate various UN* condition codes */
    volatile int r1 = (nan1 < normal);
    volatile int r2 = (nan1 <= normal);
    volatile int r3 = (nan1 > normal);
    volatile int r4 = (nan1 >= normal);
    volatile int r5 = (nan1 == normal);
    volatile int r6 = (nan1 != normal);
    
    /* NaN vs NaN comparisons */
    volatile int r7 = (nan1 < nan2);
    volatile int r8 = (nan1 <= nan2);
    volatile int r9 = (nan1 > nan2);
    volatile int r10 = (nan1 >= nan2);
    volatile int r11 = (nan1 == nan2);
    volatile int r12 = (nan1 != nan2);
    
    /* Infinity comparisons */
    volatile int r13 = (inf < neg_inf);
    volatile int r14 = (inf <= neg_inf);
    volatile int r15 = (inf > neg_inf);
    volatile int r16 = (inf >= neg_inf);
    
    /* Prevent optimization */
    asm volatile ("" : : "m"(r1), "m"(r2), "m"(r3), "m"(r4),
                       "m"(r5), "m"(r6), "m"(r7), "m"(r8),
                       "m"(r9), "m"(r10), "m"(r11), "m"(r12),
                       "m"(r13), "m"(r14), "m"(r15), "m"(r16));
}

int main(void) {
    double arr1[256];
    double arr2[256];
    
    /* Initialize arrays with mix of normal and NaN values */
    init_arrays(arr1, arr2);
    
    /* Direct assembly to trigger condition code printing */
    direct_cc_asm();
    
    /* Generate floating-point comparisons */
    int result = generate_fp_comparisons(arr1, arr2);
    
    /* Mixed integer/float conditional moves */
    mixed_conditional_moves();
    
    /* Special NaN comparisons */
    special_nan_comparisons();
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
