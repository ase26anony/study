/* Condition Code Test Program for i386.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <xmmintrin.h>

/* Force generation of specific condition codes */
#define FORCE_CC_PRINTING 1

/* Mix of normal values and NaN */
static void init_arrays(double arr1[256], double arr2[256]) {
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
        /* Insert NaN at specific positions to create unordered comparisons */
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

/* Direct inline assembly with %C constraint */
static void inline_asm_cc_tests(void) {
    int var = 0;
    int src = 42;
    
    /* Test various condition codes directly */
    asm volatile (
        "# UNORDERED test\n"
        "cmov%C0 %1, %0\n"
        : "+r"(var)
        : "r"(src), "i"(0)  /* UNORDERED = 0 in some contexts */
        : "cc"
    );
    
    asm volatile (
        "# ORDERED test\n"
        "cmov%C1 %1, %0\n"
        : "+r"(var)
        : "r"(src), "i"(1)  /* ORDERED = 1 */
        : "cc"
    );
    
    asm volatile (
        "# UNEQ test\n"
        "cmov%C2 %1, %0\n"
        : "+r"(var)
        : "r"(src), "i"(2)  /* UNEQ = 2 */
        : "cc"
    );
    
    asm volatile (
        "# UNGE test\n"
        "cmov%C3 %1, %0\n"
        : "+r"(var)
        : "r"(src), "i"(3)  /* UNGE = 3 */
        : "cc"
    );
    
    asm volatile (
        "# UNGT test\n"
        "cmov%C4 %1, %0\n"
        : "+r"(var)
        : "r"(src), "i"(4)  /* UNGT = 4 */
        : "cc"
    );
    
    asm volatile (
        "# UNLE test\n"
        "cmov%C5 %1, %0\n"
        : "+r"(var)
        : "r"(src), "i"(5)  /* UNLE = 5 */
        : "cc"
    );
    
    asm volatile (
        "# UNLT test\n"
        "cmov%C6 %1, %0\n"
        : "+r"(var)
        : "r"(src), "i"(6)  /* UNLT = 6 */
        : "cc"
    );
    
    asm volatile (
        "# LTGT test\n"
        "cmov%C7 %1, %0\n"
        : "+r"(var)
        : "r"(src), "i"(7)  /* LTGT = 7 */
        : "cc"
    );
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(var));
}

/* Generate floating-point comparisons that produce various condition codes */
static int fp_comparison_tests(double a, double b) {
    volatile int result = 0;
    
    /* All six standard comparisons - will generate different condition codes */
    result += (a < b) ? 1 : 0;    /* LT - may become UNLT with NaN */
    result += (a <= b) ? 2 : 0;   /* LE - may become UNLE with NaN */
    result += (a > b) ? 4 : 0;    /* GT - may become UNGT with NaN */
    result += (a >= b) ? 8 : 0;   /* GE - may become UNGE with NaN */
    result += (a == b) ? 16 : 0;  /* EQ - may become UNEQ with NaN */
    result += (a != b) ? 32 : 0;  /* NEQ - may become LTGT with NaN */
    
    /* Ordered/unordered checks */
    result += (isunordered(a, b)) ? 64 : 0;   /* UNORDERED */
    result += (isordered(a, b)) ? 128 : 0;    /* ORDERED */
    
    return result;
}

/* Use ternary operators to force conditional move generation */
static int ternary_conditional_moves(double a, double b) {
    int x = 0, y = 0, z = 0;
    
    /* These may compile to conditional moves with condition codes */
    x = (a < b) ? 100 : 200;
    y = (a > b) ? 300 : 400;
    z = (a == b) ? 500 : 600;
    
    /* Unordered comparisons */
    x += (isunordered(a, b)) ? 700 : 800;
    y += (isordered(a, b)) ? 900 : 1000;
    
    return x + y + z;
}

int main(void) {
    double arr1[256], arr2[256];
    volatile int cc_accumulator = 0;
    
    /* Initialize with mix of normal values and NaN */
    init_arrays(arr1, arr2);
    
    /* Direct inline assembly tests */
    inline_asm_cc_tests();
    
    /* Loop through arrays performing comparisons */
    for (int i = 0; i < 256; i++) {
        /* Force floating-point comparisons */
        cc_accumulator += fp_comparison_tests(arr1[i], arr2[i]);
        
        /* Force potential conditional move generation */
        cc_accumulator += ternary_conditional_moves(arr1[i], arr2[i]);
        
        /* Additional unordered scenarios */
        if (isunordered(arr1[i], arr2[i])) {
            cc_accumulator |= (1 << (i % 16));
        }
        
        /* Mixed relational comparisons */
        volatile double tmp = arr1[i];
        if (tmp < arr2[i]) cc_accumulator++;
        if (tmp > arr2[i]) cc_accumulator++;
        if (tmp <= arr2[i]) cc_accumulator++;
        if (tmp >= arr2[i]) cc_accumulator++;
        if (tmp == arr2[i]) cc_accumulator++;
        if (tmp != arr2[i]) cc_accumulator++;
    }
    
    /* Additional explicit NaN comparisons */
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double normal = 3.14159;
    
    /* These should generate various condition codes */
    cc_accumulator += (nan_val < normal) ? 1 : 0;
    cc_accumulator += (nan_val > normal) ? 2 : 0;
    cc_accumulator += (nan_val == normal) ? 4 : 0;
    cc_accumulator += (nan_val != normal) ? 8 : 0;
    cc_accumulator += (normal < inf_val) ? 16 : 0;
    cc_accumulator += (normal > -inf_val) ? 32 : 0;
    
    /* Print result to prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    return 0;
}
