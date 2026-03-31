/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

/* Force unordered comparisons with NaN */
static double get_nan(void) {
    return __builtin_nan("");
}

/* Mix of normal values and NaN */
static void init_arrays(double arr1[256], double arr2[256]) {
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 2.0;
        
        /* Insert NaN at strategic positions to force unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = get_nan();
        }
        if (i % 11 == 0) {
            arr2[i] = get_nan();
        }
        if (i % 13 == 0) {
            arr1[i] = get_nan();
            arr2[i] = get_nan();
        }
    }
}

/* Direct inline assembly with %C constraint to trigger condition code printing */
static void emit_condition_codes(void) {
    int var = 0;
    int src = 42;
    
    /* UNORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(16)  /* 16 = UNORDERED */
                  : "cc");
    
    /* ORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(17)  /* 17 = ORDERED */
                  : "cc");
    
    /* UNEQ */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(18)  /* 18 = UNEQ */
                  : "cc");
    
    /* UNGE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(19)  /* 19 = UNGE */
                  : "cc");
    
    /* UNGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(20)  /* 20 = UNGT */
                  : "cc");
    
    /* UNLE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(21)  /* 21 = UNLE */
                  : "cc");
    
    /* UNLT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(22)  /* 22 = UNLT */
                  : "cc");
    
    /* LTGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(23)  /* 23 = LTGT */
                  : "cc");
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(var));
}

/* Force generation of all condition codes through floating-point comparisons */
static int test_floating_comparisons(void) {
    double arr1[256], arr2[256];
    volatile int cc_accumulator = 0;
    
    init_arrays(arr1, arr2);
    
    /* Perform all six standard FP comparisons, some will be unordered due to NaN */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        
        /* Each comparison can generate different condition codes */
        int result;
        
        /* < (LT) - can generate UNLT or LTGT */
        result = (a < b) ? 1 : 0;
        cc_accumulator += result;
        
        /* <= (LE) - can generate UNLE or LTGT */
        result = (a <= b) ? 2 : 0;
        cc_accumulator += result;
        
        /* > (GT) - can generate UNGT or LTGT */
        result = (a > b) ? 3 : 0;
        cc_accumulator += result;
        
        /* >= (GE) - can generate UNGE or LTGT */
        result = (a >= b) ? 4 : 0;
        cc_accumulator += result;
        
        /* == (EQ) - can generate UNEQ or LTGT */
        result = (a == b) ? 5 : 0;
        cc_accumulator += result;
        
        /* != (NE) - can generate UNORDERED or LTGT */
        result = (a != b) ? 6 : 0;
        cc_accumulator += result;
        
        /* Ordered comparison - can generate ORDERED */
        result = (!isunordered(a, b)) ? 7 : 0;
        cc_accumulator += result;
        
        /* Unordered comparison - can generate UNORDERED */
        result = (isunordered(a, b)) ? 8 : 0;
        cc_accumulator += result;
    }
    
    return cc_accumulator;
}

/* Use ternary operators to force conditional move generation */
static int test_conditional_moves(void) {
    double arr1[256], arr2[256];
    int result = 0;
    
    init_arrays(arr1, arr2);
    
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* These ternary operations may generate conditional moves */
        int cmp_lt = (a < b) ? i : -i;
        int cmp_le = (a <= b) ? i*2 : -i*2;
        int cmp_gt = (a > b) ? i*3 : -i*3;
        int cmp_ge = (a >= b) ? i*4 : -i*4;
        int cmp_eq = (a == b) ? i*5 : -i*5;
        int cmp_ne = (a != b) ? i*6 : -i*6;
        
        result += cmp_lt + cmp_le + cmp_gt + cmp_ge + cmp_eq + cmp_ne;
    }
    
    return result;
}

/* Mixed integer/float comparisons */
static int test_mixed_comparisons(void) {
    float farr[128];
    double darr[128];
    int iarr[128];
    int result = 0;
    
    for (int i = 0; i < 128; i++) {
        farr[i] = i * 0.5f;
        darr[i] = i * 0.25;
        iarr[i] = i;
        
        if (i % 5 == 0) farr[i] = __builtin_nanf("");
        if (i % 9 == 0) darr[i] = get_nan();
    }
    
    for (int i = 0; i < 128; i++) {
        /* Mixed type comparisons */
        volatile float f1 = farr[i];
        volatile double d1 = darr[i];
        volatile int i1 = iarr[i];
        
        /* Float-double comparison */
        int r1 = (f1 < d1) ? 1 : 0;
        
        /* Float-int comparison (promotion) */
        int r2 = (f1 < i1) ? 2 : 0;
        
        /* Double-int comparison */
        int r3 = (d1 > i1) ? 3 : 0;
        
        result += r1 + r2 + r3;
    }
    
    return result;
}

int main(void) {
    int result1, result2, result3;
    
    /* Direct trigger of condition code printing */
    emit_condition_codes();
    
    /* Force various FP comparisons */
    result1 = test_floating_comparisons();
    
    /* Force conditional move generation */
    result2 = test_conditional_moves();
    
    /* Mixed type comparisons */
    result3 = test_mixed_comparisons();
    
    /* Prevent optimization */
    printf("Results: %d %d %d\n", result1, result2, result3);
    
    return 0;
}
