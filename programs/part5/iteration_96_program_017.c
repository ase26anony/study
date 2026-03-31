/* Condition Code Coverage Test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force generation of various x86 condition codes */
#define FORCE_CC_PRINTING 1

/* Mix of normal values and NaN */
static void init_arrays(double arr1[256], double arr2[256]) {
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.5;
        
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
    
    /* UNORDERED condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(16)  /* 16 = UNORDERED */
                  : "cc");
    
    /* ORDERED condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(17)  /* 17 = ORDERED */
                  : "cc");
    
    /* UNEQ condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(18)  /* 18 = UNEQ */
                  : "cc");
    
    /* UNGE condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(19)  /* 19 = UNGE */
                  : "cc");
    
    /* UNGT condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(20)  /* 20 = UNGT */
                  : "cc");
    
    /* UNLE condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(21)  /* 21 = UNLE */
                  : "cc");
    
    /* UNLT condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(22)  /* 22 = UNLT */
                  : "cc");
    
    /* LTGT condition */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(23)  /* 23 = LTGT */
                  : "cc");
}

/* Generate floating-point comparisons that produce various condition codes */
static int generate_fp_comparisons(double arr1[256], double arr2[256]) {
    volatile int cc_accumulator = 0;
    int result = 0;
    
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Force generation of different condition codes through comparisons */
        
        /* UNORDERED: a or b is NaN */
        if (!(a < b) && !(a >= b)) {
            cc_accumulator |= 1;
        }
        
        /* ORDERED: neither is NaN */
        if (a == a && b == b) {
            cc_accumulator |= 2;
        }
        
        /* UNEQ: unordered or equal */
        if (!(a != b)) {
            result = (a == b) ? i : result;
        }
        
        /* UNGE: unordered or greater or equal */
        if (!(a < b)) {
            result = (a >= b) ? i : result;
        }
        
        /* UNGT: unordered or greater */
        if (!(a <= b)) {
            result = (a > b) ? i : result;
        }
        
        /* UNLE: unordered or less or equal */
        if (!(a > b)) {
            result = (a <= b) ? i : result;
        }
        
        /* UNLT: unordered or less */
        if (!(a >= b)) {
            result = (a < b) ? i : result;
        }
        
        /* LTGT: less or greater (but not equal, not unordered) */
        if (a != b && a == a && b == b) {
            result = (a < b || a > b) ? i : result;
        }
        
        /* All six standard comparisons to generate base condition codes */
        volatile int lt = (a < b);
        volatile int le = (a <= b);
        volatile int gt = (a > b);
        volatile int ge = (a >= b);
        volatile int eq = (a == b);
        volatile int ne = (a != b);
        
        /* Use results to prevent optimization */
        cc_accumulator += lt + le + gt + ge + eq + ne;
    }
    
    return cc_accumulator + result;
}

/* Use ternary operators to potentially generate conditional moves */
static int ternary_with_fp_conditions(double arr1[256], double arr2[256]) {
    int sum = 0;
    
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Ternary operations with FP conditions - may generate CMOV */
        int val1 = (a < b) ? i : -i;
        int val2 = (a <= b) ? val1 : -val1;
        int val3 = (a > b) ? val2 : -val2;
        int val4 = (a >= b) ? val3 : -val3;
        int val5 = (a == b) ? val4 : -val4;
        int val6 = (a != b) ? val5 : -val5;
        
        sum += val6;
    }
    
    return sum;
}

int main(void) {
    double arr1[256];
    double arr2[256];
    
    /* Initialize with mix of normal values and NaN */
    init_arrays(arr1, arr2);
    
    /* Direct assembly to trigger condition code printing */
    direct_cc_asm();
    
    /* Generate FP comparisons for various condition codes */
    int cc_result = generate_fp_comparisons(arr1, arr2);
    
    /* Use ternary operators that may generate conditional moves */
    int ternary_result = ternary_with_fp_conditions(arr1, arr2);
    
    /* Prevent optimization and use results */
    printf("Condition code accumulator: %d\n", cc_result);
    printf("Ternary result: %d\n", ternary_result);
    
    /* Additional unordered comparisons in control flow */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("");
    volatile double normal = 3.14159;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    if (nan1 == nan2) {
        printf("Unexpected: nan1 == nan2\n");
    }
    
    if (normal == normal) {
        printf("Expected: normal == normal\n");
    }
    
    if (!(nan1 < normal) && !(nan1 >= normal)) {
        printf("Unordered comparison detected\n");
    }
    
    return (cc_result + ternary_result) > 0 ? 0 : 1;
}
