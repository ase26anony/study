/* Condition Code Coverage Test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force generation of all x86 condition codes from floating-point comparisons */
#ifdef __GNUC__
#define FORCE_CC_PRINTING 1
#endif

/* Mix of normal values and NaN */
static void init_arrays(double arr1[256], double arr2[256]) {
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
            arr1[i] = __builtin_nan("");
            arr2[i] = __builtin_nan("");
        }
    }
}

/* Direct inline assembly with %C constraint to trigger condition code printing */
static void direct_cc_asm(void) {
    int var = 0;
    int src = 42;
    
    /* UNORDERED condition code */
    asm volatile ("# UNORDERED condition code\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) 
                  : "r"(src), "i"(16)  /* 16 = UNORDERED */
                  : "cc");
    
    /* ORDERED condition code */
    asm volatile ("# ORDERED condition code\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) 
                  : "r"(src), "i"(17)  /* 17 = ORDERED */
                  : "cc");
    
    /* UNEQ condition code */
    asm volatile ("# UNEQ condition code\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) 
                  : "r"(src), "i"(18)  /* 18 = UNEQ */
                  : "cc");
    
    /* UNGE condition code */
    asm volatile ("# UNGE condition code\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) 
                  : "r"(src), "i"(19)  /* 19 = UNGE */
                  : "cc");
    
    /* UNGT condition code */
    asm volatile ("# UNGT condition code\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) 
                  : "r"(src), "i"(20)  /* 20 = UNGT */
                  : "cc");
    
    /* UNLE condition code */
    asm volatile ("# UNLE condition code\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) 
                  : "r"(src), "i"(21)  /* 21 = UNLE */
                  : "cc");
    
    /* UNLT condition code */
    asm volatile ("# UNLT condition code\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) 
                  : "r"(src), "i"(22)  /* 22 = UNLT */
                  : "cc");
    
    /* LTGT condition code */
    asm volatile ("# LTGT condition code\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) 
                  : "r"(src), "i"(23)  /* 23 = LTGT */
                  : "cc");
}

/* Generate floating-point comparisons that produce various condition codes */
static int generate_fp_comparisons(double a, double b) {
    volatile int result = 0;
    
    /* Each comparison should generate different condition codes */
    result += (a < b) ? 1 : 0;    /* May generate UNLT or LT */
    result += (a <= b) ? 2 : 0;   /* May generate UNLE or LE */
    result += (a > b) ? 4 : 0;    /* May generate UNGT or GT */
    result += (a >= b) ? 8 : 0;   /* May generate UNGE or GE */
    result += (a == b) ? 16 : 0;  /* May generate UNEQ or EQ */
    result += (a != b) ? 32 : 0;  /* May generate LTGT or NE */
    
    return result;
}

/* Use ternary operators to force conditional move generation */
static int ternary_with_fp_cond(double a, double b) {
    int x = 0;
    
    /* These may lower to conditional moves with condition code names */
    x = (a < b) ? (x + 1) : (x - 1);
    x = (a <= b) ? (x * 2) : (x / 2);
    x = (a > b) ? (x | 0xFF) : (x & 0x0F);
    x = (a >= b) ? (x ^ 0xAA) : (x ^ 0x55);
    x = (a == b) ? (x << 2) : (x >> 2);
    x = (a != b) ? ~x : x;
    
    return x;
}

int main(void) {
    double arr1[256], arr2[256];
    volatile int cc_accumulator = 0;
    int ternary_results = 0;
    
    /* Initialize with mix of normal values and NaN */
    init_arrays(arr1, arr2);
    
    /* Direct assembly to trigger condition code printing */
    direct_cc_asm();
    
    /* Loop through arrays generating various floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        /* Generate comparisons that may produce unordered results */
        cc_accumulator += generate_fp_comparisons(arr1[i], arr2[i]);
        
        /* Use ternary operators to potentially generate conditional moves */
        ternary_results ^= ternary_with_fp_cond(arr1[i], arr2[i]);
        
        /* Additional complex expression to prevent optimization */
        double tmp = arr1[i] * arr2[i];
        if (!isnan(tmp)) {
            cc_accumulator += (int)tmp;
        }
    }
    
    /* Force side effects to prevent dead code elimination */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    printf("Ternary results: %d\n", ternary_results);
    
    /* Additional unordered comparisons in control flow */
    volatile double nan_val = __builtin_nan("");
    volatile double normal_val = 3.14159;
    
    if (nan_val < normal_val) {
        printf("Unordered comparison path 1\n");
    }
    
    if (normal_val >= nan_val) {
        printf("Unordered comparison path 2\n");
    }
    
    if (nan_val == nan_val) {  /* This is false for NaN */
        printf("This should not print\n");
    }
    
    if (nan_val != nan_val) {  /* This is true for NaN */
        printf("NaN != NaN is true (unordered)\n");
    }
    
    return 0;
}
