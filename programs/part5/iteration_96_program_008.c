/* Compile with: gcc -O2 -dp -march=x86-64 -masm=intel -o test_cc test_cc.c */
/* Also try: gcc -O3 -fno-trapping-math -dP -march=native -fdump-rtl-final test_cc.c */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered comparisons by mixing NaN values */
static void init_arrays(double *arr1, double *arr2, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
        /* Insert NaN at specific indices to force unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
    }
}

/* Direct inline assembly with %C constraint to trigger condition code printing */
static void inline_asm_cc_tests(void) {
    int var = 0;
    int src = 42;
    
    /* Test various condition codes using inline assembly */
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

/* Force generation of condition codes through floating-point comparisons */
static int fp_comparison_tests(double *arr1, double *arr2, int size) {
    volatile int cc_accumulator = 0;
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Perform all six standard floating-point comparisons */
        /* Each comparison can generate different condition codes */
        
        /* Less than - can generate UNLT or LT */
        result = (a < b) ? (result + 1) : (result - 1);
        
        /* Less than or equal - can generate UNLE or LE */
        result = (a <= b) ? (result + 2) : (result - 2);
        
        /* Greater than - can generate UNGT or GT */
        result = (a > b) ? (result + 3) : (result - 3);
        
        /* Greater than or equal - can generate UNGE or GE */
        result = (a >= b) ? (result + 4) : (result - 4);
        
        /* Equal - can generate UNEQ or EQ */
        result = (a == b) ? (result + 5) : (result - 5);
        
        /* Not equal - can generate LTGT or NE */
        result = (a != b) ? (result + 6) : (result - 6);
        
        /* Force unordered checks with explicit isnan */
        if (isunordered(a, b)) {
            cc_accumulator |= 1;  /* UNORDERED */
        }
        if (!isunordered(a, b)) {
            cc_accumulator |= 2;  /* ORDERED */
        }
        
        /* Mixed comparisons to trigger various condition codes */
        volatile double cmp1 = (a < b) ? 1.0 : 0.0;
        volatile double cmp2 = (a > b) ? 1.0 : 0.0;
        volatile double cmp3 = (a == b) ? 1.0 : 0.0;
        volatile double cmp4 = (a != b) ? 1.0 : 0.0;
        
        /* Use results to prevent dead code elimination */
        cc_accumulator += (int)cmp1 + (int)cmp2 + (int)cmp3 + (int)cmp4;
    }
    
    return result + cc_accumulator;
}

/* Additional tests with conditional moves based on FP comparisons */
static void conditional_move_tests(double a, double b) {
    int x = 0, y = 100;
    
    /* These may generate conditional moves with condition codes */
    int r1 = (a < b) ? x : y;
    int r2 = (a <= b) ? x : y;
    int r3 = (a > b) ? x : y;
    int r4 = (a >= b) ? x : y;
    int r5 = (a == b) ? x : y;
    int r6 = (a != b) ? x : y;
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6));
}

/* Test with volatile doubles to force actual comparisons */
static void volatile_fp_tests(void) {
    volatile double v1 = __builtin_nan("");
    volatile double v2 = 3.14159;
    volatile double v3 = 2.71828;
    volatile double v4 = __builtin_nan("0xdead");
    
    /* These comparisons with NaN should generate UNORDERED codes */
    volatile int c1 = (v1 < v2);
    volatile int c2 = (v1 > v3);
    volatile int c3 = (v1 == v4);
    volatile int c4 = (v2 != v1);
    volatile int c5 = (v2 <= v1);
    volatile int c6 = (v3 >= v1);
    
    /* Use results */
    asm volatile ("" : : "r"(c1), "r"(c2), "r"(c3), "r"(c4), "r"(c5), "r"(c6));
}

int main(void) {
    const int SIZE = 256;
    double arr1[SIZE];
    double arr2[SIZE];
    
    /* Initialize arrays with mix of normal values and NaN */
    init_arrays(arr1, arr2, SIZE);
    
    /* Direct inline assembly tests - these should directly trigger %C printing */
    inline_asm_cc_tests();
    
    /* Floating-point comparison tests */
    int result = fp_comparison_tests(arr1, arr2, SIZE);
    
    /* Additional conditional move tests */
    conditional_move_tests(arr1[0], arr2[0]);
    conditional_move_tests(__builtin_nan(""), arr2[1]);
    conditional_move_tests(arr1[2], __builtin_nan(""));
    
    /* Volatile FP tests */
    volatile_fp_tests();
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
