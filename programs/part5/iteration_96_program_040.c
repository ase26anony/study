/* Compile with: gcc -O2 -dp -march=x86-64 -masm=intel -o test_cc test_cc.c */
/* Also try: gcc -O3 -fno-trapping-math -dP -march=native -fdump-rtl-final test_cc.c */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force generation of various condition codes through floating-point comparisons */
void generate_condition_codes(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* These comparisons should generate different condition codes */
    volatile int result;
    
    /* UNORDERED: Compare NaN with anything */
    result = (nan_val < normal1);
    result = (normal1 < nan_val);
    
    /* ORDERED: Compare two normal numbers */
    result = (normal1 < normal2);
    result = (normal2 > normal1);
    
    /* UNEQ: Unordered or equal - NaN == NaN is false, but we need unordered */
    result = (nan_val == nan_val);  /* false, but may generate UNEQ */
    
    /* UNGE: Not less than (unordered, greater, or equal) */
    result = !(nan_val < normal1);
    result = !(normal1 < nan_val);
    
    /* UNGT: Not less than or equal */
    result = !(nan_val <= normal1);
    
    /* UNLE: Unordered or less than or equal */
    result = (nan_val <= normal1);
    
    /* UNLT: Unordered or less than */
    result = (nan_val < normal1);
    
    /* LTGT: Less than or greater than (ordered and not equal) */
    result = (normal1 < normal2) || (normal1 > normal2);
    
    /* Prevent optimization */
    asm volatile("" : : "r"(result));
}

/* Direct inline assembly to trigger %C format specifier */
void direct_asm_condition_codes(void) {
    int var = 42;
    int src = 99;
    
    /* Use different condition codes with %C constraint */
    asm volatile (
        /* UNORDERED */
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(0)  /* 0 = UNORDERED */
    );
    
    asm volatile (
        /* ORDERED */
        "cmov%C1 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(1)  /* 1 = ORDERED */
    );
    
    asm volatile (
        /* UNEQ */
        "cmov%C2 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(2)  /* 2 = UNEQ */
    );
    
    asm volatile (
        /* UNGE */
        "cmov%C3 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(3)  /* 3 = UNGE */
    );
    
    asm volatile (
        /* UNGT */
        "cmov%C4 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(4)  /* 4 = UNGT */
    );
    
    asm volatile (
        /* UNLE */
        "cmov%C5 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(5)  /* 5 = UNLE */
    );
    
    asm volatile (
        /* UNLT */
        "cmov%C6 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(6)  /* 6 = UNLT */
    );
    
    asm volatile (
        /* LTGT */
        "cmov%C7 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(7)  /* 7 = LTGT */
    );
    
    printf("Direct asm result: %d\n", var);
}

/* Complex loop with mixed NaN and normal values */
void complex_nan_comparisons(void) {
    #define ARRAY_SIZE 256
    double arr1[ARRAY_SIZE];
    double arr2[ARRAY_SIZE];
    volatile int cc_accumulator = 0;
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.1;
        
        /* Insert NaN at specific indices */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
        if (i % 13 == 0) {
            arr1[i] = __builtin_inf();
        }
        if (i % 17 == 0) {
            arr2[i] = -__builtin_inf();
        }
    }
    
    /* Perform all types of comparisons */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        
        /* Use ternary operators to force conditional evaluation */
        int cmp_lt = (a < b) ? 1 : 0;
        int cmp_le = (a <= b) ? 2 : 0;
        int cmp_gt = (a > b) ? 3 : 0;
        int cmp_ge = (a >= b) ? 4 : 0;
        int cmp_eq = (a == b) ? 5 : 0;
        int cmp_ne = (a != b) ? 6 : 0;
        
        /* Force use of results to prevent optimization */
        cc_accumulator += cmp_lt + cmp_le + cmp_gt + cmp_ge + cmp_eq + cmp_ne;
        
        /* Create control flow that depends on unordered comparisons */
        if (a != b) {
            cc_accumulator |= 1;
        }
        if (!(a < b)) {
            cc_accumulator |= 2;
        }
        if (!(a > b)) {
            cc_accumulator |= 4;
        }
    }
    
    printf("Complex comparisons accumulator: %d\n", cc_accumulator);
}

/* Use GCC builtins for conditional moves */
void builtin_cmov_examples(void) {
    double a = __builtin_nan("");
    double b = 3.14;
    int result1, result2;
    
    /* These should generate condition code checks */
    result1 = (a < b) ? 100 : 200;
    result2 = (b > a) ? 300 : 400;
    
    /* Mixed integer/float conditional moves */
    int x = 0;
    float f1 = __builtin_nanf("");
    float f2 = 2.5f;
    
    x = (f1 < f2) ? 555 : 666;
    x = (f1 == f2) ? 777 : 888;
    x = (f1 != f2) ? 999 : 111;
    
    printf("Builtin cmov results: %d, %d, %d\n", result1, result2, x);
}

/* Main function that combines all approaches */
int main(void) {
    printf("=== Generating condition codes for i386.cc coverage ===\n");
    
    /* Approach 1: Direct floating-point comparisons */
    generate_condition_codes();
    
    /* Approach 2: Direct inline assembly with %C constraint */
    direct_asm_condition_codes();
    
    /* Approach 3: Complex loops with NaN values */
    complex_nan_comparisons();
    
    /* Approach 4: GCC builtins and ternary operators */
    builtin_cmov_examples();
    
    printf("=== Condition code generation complete ===\n");
    
    return 0;
}
