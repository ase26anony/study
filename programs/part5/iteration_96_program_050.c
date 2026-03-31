/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered comparisons with NaN */
static double get_nan(void) {
    return __builtin_nan("");
}

/* Inline assembly to directly trigger %C format specifier */
static void emit_condition_codes(void) {
    int var = 0;
    int src = 42;
    
    /* Direct inline assembly with %C constraint */
    /* UNORDERED */
    asm volatile ("# UNORDERED test\n\t"
                  "cmov%C0 %1, %0\n\t" 
                  : "+r"(var) : "r"(src), "i"(16) /* 16 = UNORDERED */);
    
    /* ORDERED */
    asm volatile ("# ORDERED test\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) : "r"(src), "i"(17) /* 17 = ORDERED */);
    
    /* UNEQ */
    asm volatile ("# UNEQ test\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) : "r"(src), "i"(18) /* 18 = UNEQ */);
    
    /* UNGE */
    asm volatile ("# UNGE test\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) : "r"(src), "i"(19) /* 19 = UNGE */);
    
    /* UNGT */
    asm volatile ("# UNGT test\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) : "r"(src), "i"(20) /* 20 = UNGT */);
    
    /* UNLE */
    asm volatile ("# UNLE test\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) : "r"(src), "i"(21) /* 21 = UNLE */);
    
    /* UNLT */
    asm volatile ("# UNLT test\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) : "r"(src), "i"(22) /* 22 = UNLT */);
    
    /* LTGT */
    asm volatile ("# LTGT test\n\t"
                  "cmov%C0 %1, %0\n\t"
                  : "+r"(var) : "r"(src), "i"(23) /* 23 = LTGT */);
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(var));
}

/* Generate various floating-point comparisons */
static int fp_comparisons(double a, double b, volatile int *accum) {
    int result = 0;
    
    /* All six standard comparisons - will generate various condition codes */
    result += (a < b) ? 1 : 0;   /* May generate UNLT or LT */
    result += (a <= b) ? 2 : 0;  /* May generate UNLE or LE */
    result += (a > b) ? 4 : 0;   /* May generate UNGT or GT */
    result += (a >= b) ? 8 : 0;  /* May generate UNGE or GE */
    result += (a == b) ? 16 : 0; /* May generate UNEQ or EQ */
    result += (a != b) ? 32 : 0; /* May generate LTGT or NE */
    
    /* Force unordered checks */
    int is_unordered = isnan(a) || isnan(b);
    result += is_unordered ? 64 : 0;
    
    /* Use ternary to potentially generate conditional moves */
    int cmp_result = (a < b) ? 100 : 
                     (a > b) ? 200 : 
                     (a == b) ? 300 : 400;
    
    *accum += result + cmp_result;
    return result;
}

int main(void) {
    /* Arrays with mix of normal values and NaN */
    double arr1[256];
    double arr2[256];
    volatile int condition_accumulator = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.1;
        
        /* Insert NaN at specific indices to force unordered comparisons */
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
    
    /* Perform comparisons that should generate various condition codes */
    for (int i = 0; i < 256; i++) {
        fp_comparisons(arr1[i], arr2[i], &condition_accumulator);
        
        /* Additional unordered checks */
        volatile double nan_val = get_nan();
        volatile double normal_val = 42.0;
        
        /* These should generate UNORDERED/ORDERED codes */
        int unordered_check = (arr1[i] != arr1[i]) ? 1 : 0; /* NaN != NaN is true */
        int ordered_check = (arr1[i] == arr1[i]) ? 1 : 0;   /* NaN == NaN is false */
        
        condition_accumulator += unordered_check + ordered_check;
        
        /* Mixed comparisons with NaN */
        condition_accumulator += (arr1[i] < nan_val) ? 1000 : 0;
        condition_accumulator += (nan_val > arr2[i]) ? 2000 : 0;
        condition_accumulator += (arr1[i] >= nan_val) ? 3000 : 0;
        condition_accumulator += (nan_val <= arr2[i]) ? 4000 : 0;
    }
    
    /* Directly trigger condition code printing via inline assembly */
    emit_condition_codes();
    
    /* Additional inline assembly with floating point comparisons */
    double x = get_nan();
    double y = 3.14;
    int res1, res2;
    
    asm volatile (
        "# Floating point comparison to generate condition codes\n\t"
        "comisd %1, %2\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0\n\t"
        : "=r"(res1) : "x"(x), "x"(y) : "al"
    );
    
    asm volatile (
        "# Another FP comparison\n\t"
        "comisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0\n\t"
        : "=r"(res2) : "x"(y), "x"(x) : "al"
    );
    
    condition_accumulator += res1 + res2;
    
    /* Use __builtin_constant_p to ensure asm isn't eliminated */
    if (__builtin_constant_p(condition_accumulator)) {
        asm volatile ("# Constant path\n\t" : : );
    }
    
    /* Prevent dead code elimination */
    printf("Condition accumulator: %d\n", condition_accumulator);
    
    /* Additional printf to use result */
    if (condition_accumulator > 0) {
        printf("Tests completed successfully.\n");
    }
    
    return 0;
}
