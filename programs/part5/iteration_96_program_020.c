/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered floating-point comparisons */
static void generate_unordered_comparisons(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal = 3.14159;
    volatile double zero = 0.0;
    
    /* These comparisons will generate various condition codes */
    volatile int res;
    
    /* UNORDERED: Compare NaN with anything */
    res = (nan_val < normal);
    res = (nan_val > normal);
    res = (nan_val == normal);
    res = (nan_val != normal);
    
    /* ORDERED: Compare two normal numbers */
    res = (normal < inf_val);
    res = (normal > zero);
    
    /* UNEQ: Compare equal values or unordered */
    res = (normal == normal);
    res = (nan_val == nan_val);  /* NaN != NaN, but generates UNEQ check */
    
    /* UNGE: Not less than (>= or unordered) */
    res = (nan_val >= normal);
    res = (normal >= zero);
    
    /* UNGT: Not less than or equal (> or unordered) */
    res = (nan_val > normal);
    res = (inf_val > normal);
    
    /* UNLE: Less than or equal or unordered */
    res = (nan_val <= normal);
    res = (normal <= inf_val);
    
    /* UNLT: Less than or unordered */
    res = (nan_val < normal);
    res = (normal < inf_val);
    
    /* LTGT: Less than or greater than (ordered and not equal) */
    res = (normal < inf_val);
    res = (zero < normal);
    
    /* Prevent optimization */
    asm volatile("" : : "r"(res));
}

/* Use inline assembly with %C constraint to directly trigger printing */
static void inline_asm_condition_codes(void) {
    int var = 0;
    int src = 42;
    
    /* Direct inline assembly with %C constraint */
    /* UNORDERED */
    asm volatile (
        "test %1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(var)
        : "r"(var), "r"(src), "i"(0)  /* 0 = UNORDERED */
        : "cc"
    );
    
    /* ORDERED */
    var = 0;
    asm volatile (
        "test %1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(var)
        : "r"(var), "r"(src), "i"(7)  /* 7 = ORDERED */
        : "cc"
    );
    
    /* UNEQ */
    var = 0;
    asm volatile (
        "test %1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(var)
        : "r"(var), "r"(src), "i"(8)  /* 8 = UNEQ */
        : "cc"
    );
    
    /* UNGE */
    var = 0;
    asm volatile (
        "test %1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(var)
        : "r"(var), "r"(src), "i"(13) /* 13 = UNGE */
        : "cc"
    );
    
    /* UNGT */
    var = 0;
    asm volatile (
        "test %1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(var)
        : "r"(var), "r"(src), "i"(14) /* 14 = UNGT */
        : "cc"
    );
    
    /* UNLE */
    var = 0;
    asm volatile (
        "test %1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(var)
        : "r"(var), "r"(src), "i"(15) /* 15 = UNLE */
        : "cc"
    );
    
    /* UNLT */
    var = 0;
    asm volatile (
        "test %1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(var)
        : "r"(var), "r"(src), "i"(16) /* 16 = UNLT */
        : "cc"
    );
    
    /* LTGT */
    var = 0;
    asm volatile (
        "test %1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(var)
        : "r"(var), "r"(src), "i"(17) /* 17 = LTGT */
        : "cc"
    );
    
    /* Prevent optimization */
    asm volatile("" : : "r"(var));
}

/* Main test with arrays and loops */
int main(void) {
    double arr1[256];
    double arr2[256];
    volatile int cc_accumulator = 0;
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.7;
        
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
    
    /* Perform various floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        volatile int temp;
        
        /* All six standard comparisons */
        temp = (a < b) ? 1 : 0;
        cc_accumulator += temp;
        
        temp = (a <= b) ? 1 : 0;
        cc_accumulator += temp;
        
        temp = (a > b) ? 1 : 0;
        cc_accumulator += temp;
        
        temp = (a >= b) ? 1 : 0;
        cc_accumulator += temp;
        
        temp = (a == b) ? 1 : 0;
        cc_accumulator += temp;
        
        temp = (a != b) ? 1 : 0;
        cc_accumulator += temp;
        
        /* Force conditional move generation using ternary on int */
        int cmov_target = 0;
        cmov_target = (a < b) ? 100 : 200;
        cc_accumulator += cmov_target;
        
        cmov_target = (a != b) ? 300 : 400;
        cc_accumulator += cmov_target;
    }
    
    /* Call functions that generate specific condition codes */
    generate_unordered_comparisons();
    inline_asm_condition_codes();
    
    /* Complex expression that might generate LTGT */
    {
        double x = arr1[0];
        double y = arr2[0];
        volatile int complex_res;
        
        /* This should generate ordered comparison */
        complex_res = (x < y) || (x > y);
        cc_accumulator += complex_res;
        
        /* Force UNORDERED check */
        complex_res = !(x == x) || !(y == y);
        cc_accumulator += complex_res;
    }
    
    /* Use __builtin_constant_p to guard inline assembly */
    if (__builtin_constant_p(cc_accumulator)) {
        /* This won't execute, but forces compiler to consider the asm */
        asm volatile ("nop");
    }
    
    /* Print result to prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    return 0;
}
