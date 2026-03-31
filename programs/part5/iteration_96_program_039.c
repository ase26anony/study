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
    
    /* Test various condition codes via inline assembly */
    /* UNORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(16) /* 16 = UNORDERED */
                  : "cc");
    
    /* ORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(17) /* 17 = ORDERED */
                  : "cc");
    
    /* UNEQ */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(18) /* 18 = UNEQ */
                  : "cc");
    
    /* UNGE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(19) /* 19 = UNGE */
                  : "cc");
    
    /* UNGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(20) /* 20 = UNGT */
                  : "cc");
    
    /* UNLE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(21) /* 21 = UNLE */
                  : "cc");
    
    /* UNLT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(22) /* 22 = UNLT */
                  : "cc");
    
    /* LTGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(23) /* 23 = LTGT */
                  : "cc");
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(var));
}

/* Perform floating-point comparisons that generate various condition codes */
static volatile int cc_accumulator = 0;

static void fp_comparison_tests(double a, double b) {
    volatile int result;
    
    /* < comparison - may generate UNLT or LT */
    result = (a < b) ? 1 : 0;
    cc_accumulator += result;
    
    /* <= comparison - may generate UNLE or LE */
    result = (a <= b) ? 2 : 0;
    cc_accumulator += result;
    
    /* > comparison - may generate UNGT or GT */
    result = (a > b) ? 3 : 0;
    cc_accumulator += result;
    
    /* >= comparison - may generate UNGE or GE */
    result = (a >= b) ? 4 : 0;
    cc_accumulator += result;
    
    /* == comparison - may generate UNEQ or EQ */
    result = (a == b) ? 5 : 0;
    cc_accumulator += result;
    
    /* != comparison - may generate LTGT or NE */
    result = (a != b) ? 6 : 0;
    cc_accumulator += result;
    
    /* Ordered comparison */
    result = (!isunordered(a, b)) ? 7 : 0;
    cc_accumulator += result;
    
    /* Unordered comparison */
    result = (isunordered(a, b)) ? 8 : 0;
    cc_accumulator += result;
}

/* Use builtin to generate conditional moves based on FP comparisons */
static void conditional_move_tests(double a, double b) {
    int x = 0, y = 100;
    int res;
    
    /* These may generate cmov instructions with condition codes */
    res = (a < b) ? x : y;
    cc_accumulator += res;
    
    res = (a <= b) ? x : y;
    cc_accumulator += res;
    
    res = (a > b) ? x : y;
    cc_accumulator += res;
    
    res = (a >= b) ? x : y;
    cc_accumulator += res;
    
    res = (a == b) ? x : y;
    cc_accumulator += res;
    
    res = (a != b) ? x : y;
    cc_accumulator += res;
}

int main(void) {
    const int SIZE = 256;
    double arr1[SIZE], arr2[SIZE];
    
    /* Initialize arrays with mix of normal values and NaN */
    init_arrays(arr1, arr2, SIZE);
    
    /* Direct inline assembly tests - should trigger %C printing */
    inline_asm_cc_tests();
    
    /* Loop through arrays performing various comparisons */
    for (int i = 0; i < SIZE; i++) {
        fp_comparison_tests(arr1[i], arr2[i]);
        conditional_move_tests(arr1[i], arr2[i]);
        
        /* Additional unordered scenarios */
        if (__builtin_constant_p(i)) {
            /* Force different optimization paths */
            double nan_val = __builtin_nan("");
            double inf_val = __builtin_inf();
            
            /* Comparisons that should generate UNORDERED/ORDERED codes */
            volatile int r1 = (arr1[i] == nan_val) ? 1 : 0;
            volatile int r2 = (arr1[i] != nan_val) ? 1 : 0;
            volatile int r3 = (arr1[i] < inf_val) ? 1 : 0;
            volatile int r4 = (inf_val > arr2[i]) ? 1 : 0;
            
            cc_accumulator += r1 + r2 + r3 + r4;
        }
    }
    
    /* Additional explicit tests for each condition code */
    double nan1 = __builtin_nan("");
    double nan2 = __builtin_nan("");
    double normal = 3.14159;
    
    /* UNORDERED: nan1 vs nan2 */
    asm volatile ("# UNORDERED test begin");
    volatile int unord_test = (isunordered(nan1, nan2)) ? 1 : 0;
    cc_accumulator += unord_test;
    
    /* ORDERED: normal vs normal */
    asm volatile ("# ORDERED test begin");
    volatile int ord_test = (!isunordered(normal, normal)) ? 1 : 0;
    cc_accumulator += ord_test;
    
    /* UNEQ: nan1 vs nan1 */
    asm volatile ("# UNEQ test begin");
    volatile int uneq_test = (nan1 == nan1) ? 1 : 0;  /* NaN == NaN is false, but may generate UNEQ */
    cc_accumulator += uneq_test;
    
    /* Mixed comparisons to trigger other codes */
    asm volatile ("# Mixed comparisons begin");
    for (int i = 0; i < 16; i++) {
        double val1 = (i % 3 == 0) ? __builtin_nan("") : i * 1.1;
        double val2 = (i % 5 == 0) ? __builtin_nan("") : i * 0.9;
        
        /* All six relational operators */
        volatile int cmp1 = (val1 < val2) ? 1 : 0;
        volatile int cmp2 = (val1 <= val2) ? 1 : 0;
        volatile int cmp3 = (val1 > val2) ? 1 : 0;
        volatile int cmp4 = (val1 >= val2) ? 1 : 0;
        volatile int cmp5 = (val1 == val2) ? 1 : 0;
        volatile int cmp6 = (val1 != val2) ? 1 : 0;
        
        cc_accumulator += cmp1 + cmp2 + cmp3 + cmp4 + cmp5 + cmp6;
    }
    
    /* Print result to prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    return 0;
}
