/* gcc -O2 -dp -march=x86-64 -masm=intel -fdump-rtl-final -fno-trapping-math -o cc_test cc_test.c */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

/* Force generation of various condition codes through floating-point comparisons */
void generate_condition_codes(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* These comparisons should generate different condition codes */
    volatile int res;
    
    /* UNORDERED: Compare NaN with anything */
    res = (nan_val < normal1);
    res = (nan_val > normal1);
    res = (nan_val == normal1);
    res = (nan_val != normal1);
    
    /* ORDERED: Compare two normal numbers */
    res = (normal1 < normal2);
    res = (normal1 > normal2);
    res = (normal1 == normal2);
    res = (normal1 != normal2);
    
    /* UNEQ: Compare NaN with NaN (unordered equal) */
    res = (nan_val == nan_val);
    
    /* UNGE: Not less than (unordered) */
    res = !(nan_val < normal1);
    
    /* UNGT: Not less than or equal (unordered) */
    res = !(nan_val <= normal1);
    
    /* UNLE: Unordered or less than or equal */
    res = (nan_val <= normal1);
    
    /* UNLT: Unordered or less than */
    res = (nan_val < normal1);
    
    /* LTGT: Less than or greater than (ordered) */
    res = (normal1 < normal2) || (normal1 > normal2);
}

/* Direct inline assembly to trigger %C format specifier */
void inline_asm_condition_codes(void) {
    int var = 0;
    int src = 42;
    
    /* Use various condition codes with %C in inline assembly */
    asm volatile (
        "# UNORDERED condition\n"
        "cmov%C0 %1, %0\n"
        : "+r"(var)
        : "r"(src), "i"(0)  /* 0 = UNORDERED */
        : "cc"
    );
    
    asm volatile (
        "# ORDERED condition\n"
        "cmov%C0 %1, %0\n"
        : "+r"(var)
        : "r"(src), "i"(7)  /* 7 = ORDERED */
        : "cc"
    );
    
    asm volatile (
        "# UNEQ condition\n"
        "cmov%C0 %1, %0\n"
        : "+r"(var)
        : "r"(src), "i"(1)  /* 1 = UNEQ */
        : "cc"
    );
    
    asm volatile (
        "# UNGE condition\n"
        : "+r"(var)
        : "r"(src), "i"(2)  /* 2 = UNGE */
        : "cc"
    );
    
    asm volatile (
        "# UNGT condition\n"
        : "+r"(var)
        : "r"(src), "i"(3)  /* 3 = UNGT */
        : "cc"
    );
    
    asm volatile (
        "# UNLE condition\n"
        : "+r"(var)
        : "r"(src), "i"(4)  /* 4 = UNLE */
        : "cc"
    );
    
    asm volatile (
        "# UNLT condition\n"
        : "+r"(var)
        : "r"(src), "i"(5)  /* 5 = UNLT */
        : "cc"
    );
    
    asm volatile (
        "# LTGT condition\n"
        : "+r"(var)
        : "r"(src), "i"(6)  /* 6 = LTGT */
        : "cc"
    );
    
    /* Prevent optimization */
    printf("Inline asm result: %d\n", var);
}

/* Complex floating-point comparisons with arrays */
void array_comparisons(void) {
    #define ARRAY_SIZE 256
    double arr1[ARRAY_SIZE];
    double arr2[ARRAY_SIZE];
    volatile int condition_acc = 0;
    
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
            arr2[i] = -__builtin_inf();
        }
    }
    
    /* Perform all six standard floating-point comparisons */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        
        /* Each comparison should potentially generate different condition codes */
        int lt_result = (a < b) ? 1 : 0;
        int le_result = (a <= b) ? 1 : 0;
        int gt_result = (a > b) ? 1 : 0;
        int ge_result = (a >= b) ? 1 : 0;
        int eq_result = (a == b) ? 1 : 0;
        int ne_result = (a != b) ? 1 : 0;
        
        /* Use ternary operators to force potential conditional move generation */
        condition_acc += lt_result;
        condition_acc += le_result;
        condition_acc += gt_result;
        condition_acc += ge_result;
        condition_acc += eq_result;
        condition_acc += ne_result;
        
        /* Complex expression that might generate LTGT */
        int ltgt_candidate = ((a < b) || (a > b)) ? 1 : 0;
        condition_acc += ltgt_candidate;
        
        /* Ordered/unordered checks */
        int ordered_check = (a == a && b == b) ? 1 : 0;  /* Both are numbers */
        int unordered_check = (a != a || b != b) ? 1 : 0; /* At least one is NaN */
        condition_acc += ordered_check;
        condition_acc += unordered_check;
    }
    
    printf("Condition accumulator: %d\n", condition_acc);
}

/* Use GCC builtins for conditional moves */
void builtin_cmov_examples(void) {
    double a = __builtin_nan("");
    double b = 3.14;
    int result1, result2;
    
    /* These might expand to instructions using condition codes */
    result1 = (a < b) ? 100 : 200;
    result2 = (a != a) ? 300 : 400;  /* Check for NaN */
    
    /* Force use of results */
    printf("CMOV results: %d %d\n", result1, result2);
    
    /* Mixed integer/float comparisons */
    float f1 = 1.0f;
    float f2 = 2.0f;
    int i1 = (f1 < f2) ? 1 : 0;
    int i2 = (f1 != f2) ? 1 : 0;
    
    printf("Float comparisons: %d %d\n", i1, i2);
}

/* Function that uses switch on comparison results to force code generation */
void switch_on_comparisons(void) {
    volatile double vals[] = {1.0, 2.0, __builtin_nan(""), __builtin_inf()};
    int results[4] = {0};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* This switch might generate various condition codes */
            if (vals[i] < vals[j]) {
                results[0]++;
            } else if (vals[i] > vals[j]) {
                results[1]++;
            } else if (vals[i] == vals[j]) {
                results[2]++;
            } else {
                /* Unordered case */
                results[3]++;
            }
        }
    }
    
    printf("Comparison counts: %d %d %d %d\n", 
           results[0], results[1], results[2], results[3]);
}

int main(void) {
    printf("Generating condition codes for coverage...\n");
    
    /* Call all functions to trigger different code paths */
    generate_condition_codes();
    inline_asm_condition_codes();
    array_comparisons();
    builtin_cmov_examples();
    switch_on_comparisons();
    
    return 0;
}
