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
    
    /* Test various condition codes using inline assembly with %C */
    /* UNORDERED */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(16) :);
    
    /* ORDERED */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(23) :);
    
    /* UNEQ */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(24) :);
    
    /* UNGE */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(25) :);
    
    /* UNGT */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(26) :);
    
    /* UNLE */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(27) :);
    
    /* UNLT */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(28) :);
    
    /* LTGT */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(29) :);
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(var));
}

/* Perform all floating-point comparisons to generate condition codes */
static int perform_fp_comparisons(double a, double b) {
    volatile int result = 0;
    
    /* All six standard comparisons - will generate various condition codes */
    result += (a < b) ? 1 : 0;   /* May generate UNLT or LT */
    result += (a <= b) ? 1 : 0;  /* May generate UNLE or LE */
    result += (a > b) ? 1 : 0;   /* May generate UNGT or GT */
    result += (a >= b) ? 1 : 0;  /* May generate UNGE or GE */
    result += (a == b) ? 1 : 0;  /* May generate UNEQ or EQ */
    result += (a != b) ? 1 : 0;  /* May generate LTGT or NE */
    
    /* Ordered/unordered checks */
    result += (!isunordered(a, b)) ? 1 : 0;  /* ORDERED */
    result += (isunordered(a, b)) ? 1 : 0;   /* UNORDERED */
    
    return result;
}

/* Use builtin to force conditional move generation */
static int conditional_move_test(double a, double b) {
    int x = 0, y = 0;
    
    /* These may generate conditional moves with condition codes */
    x = (a < b) ? 100 : 200;
    y = (a != b) ? 300 : 400;
    
    /* Mixed integer/float comparison */
    if (isnan(a) || isnan(b)) {
        x = 500;
    }
    
    return x + y;
}

int main(void) {
    const int SIZE = 256;
    double arr1[SIZE], arr2[SIZE];
    volatile int cc_accumulator = 0;
    
    /* Initialize with NaN values at specific indices */
    init_arrays(arr1, arr2, SIZE);
    
    /* Direct inline assembly tests - should trigger condition code printing */
    inline_asm_cc_tests();
    
    /* Loop through arrays performing all comparisons */
    for (int i = 0; i < SIZE; i++) {
        /* Perform floating-point comparisons */
        cc_accumulator += perform_fp_comparisons(arr1[i], arr2[i]);
        
        /* Conditional move test */
        cc_accumulator += conditional_move_test(arr1[i], arr2[i]);
        
        /* Additional unordered checks */
        if (isunordered(arr1[i], arr2[i])) {
            cc_accumulator += 1000;  /* UNORDERED case */
        } else if (!isunordered(arr1[i], arr2[i])) {
            cc_accumulator += 2000;  /* ORDERED case */
        }
        
        /* Mixed comparisons to generate more condition codes */
        double temp = arr1[i];
        if (temp < arr2[i]) cc_accumulator += 1;
        if (temp > arr2[i]) cc_accumulator += 2;
        if (temp <= arr2[i]) cc_accumulator += 4;
        if (temp >= arr2[i]) cc_accumulator += 8;
        if (temp == arr2[i]) cc_accumulator += 16;
        if (temp != arr2[i]) cc_accumulator += 32;
    }
    
    /* Additional inline assembly with explicit condition codes */
    {
        int x = 0, y = 1;
        /* UNORDERED */
        asm volatile (
            "fucomi %%st(1), %%st\n\t"
            "setp %%al\n\t"
            : : : "cc", "eax"
        );
        
        /* ORDERED */
        asm volatile (
            "fucomip %%st(1), %%st\n\t"
            "setnp %%al\n\t"
            : : : "cc", "eax"
        );
    }
    
    /* Print result to prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    return 0;
}
