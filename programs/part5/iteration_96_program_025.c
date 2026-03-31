/* Condition Code Test Program for i386.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <x86intrin.h>

/* Force generation of various x86 condition codes from floating-point comparisons */
void generate_condition_codes(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Array with mixed NaN and normal values */
    double arr1[256];
    double arr2[256];
    
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 2.0;
        
        /* Insert NaN at specific indices to create unordered comparisons */
        if (i % 7 == 0) arr1[i] = nan_val;
        if (i % 11 == 0) arr2[i] = nan_val;
        if (i % 13 == 0) {
            arr1[i] = inf_val;
            arr2[i] = -inf_val;
        }
    }
    
    volatile int cc_accumulator = 0;
    int result;
    
    /* Loop performing all floating-point comparisons */
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Generate various condition codes through comparisons */
        result = (a < b) ? 1 : 0;      /* May generate UNLT/LT */
        cc_accumulator += result;
        
        result = (a <= b) ? 2 : 0;     /* May generate UNLE/LE */
        cc_accumulator += result;
        
        result = (a > b) ? 3 : 0;      /* May generate UNGT/GT */
        cc_accumulator += result;
        
        result = (a >= b) ? 4 : 0;     /* May generate UNGE/GE */
        cc_accumulator += result;
        
        result = (a == b) ? 5 : 0;     /* May generate UNEQ/EQ */
        cc_accumulator += result;
        
        result = (a != b) ? 6 : 0;     /* May generate LTGT/NE */
        cc_accumulator += result;
        
        /* Ordered/unordered checks */
        result = (!isunordered(a, b)) ? 7 : 0;  /* ORDERED */
        cc_accumulator += result;
        
        result = (isunordered(a, b)) ? 8 : 0;   /* UNORDERED */
        cc_accumulator += result;
    }
    
    /* Direct inline assembly with %C constraint to trigger condition code printing */
    int var1 = 42, var2 = 100, var3 = 0;
    
    /* UNORDERED condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var3)
        : "r"(var1), "i"(16)  /* 16 = UNORDERED */
        : "cc"
    );
    
    /* ORDERED condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var3)
        : "r"(var2), "i"(17)  /* 17 = ORDERED */
        : "cc"
    );
    
    /* UNEQ condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var3)
        : "r"(var1), "i"(18)  /* 18 = UNEQ */
        : "cc"
    );
    
    /* UNGE condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var3)
        : "r"(var2), "i"(19)  /* 19 = UNGE */
        : "cc"
    );
    
    /* UNGT condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var3)
        : "r"(var1), "i"(20)  /* 20 = UNGT */
        : "cc"
    );
    
    /* UNLE condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var3)
        : "r"(var2), "i"(21)  /* 21 = UNLE */
        : "cc"
    );
    
    /* UNLT condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var3)
        : "r"(var1), "i"(22)  /* 22 = UNLT */
        : "cc"
    );
    
    /* LTGT condition code */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var3)
        : "r"(var2), "i"(23)  /* 23 = LTGT */
        : "cc"
    );
    
    /* Prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    printf("Final var3: %d\n", var3);
}

/* Additional function to force complex floating-point control flow */
void complex_fp_control_flow(void) {
    volatile double a = __builtin_nan("");
    volatile double b = 1.0;
    volatile double c = 2.0;
    volatile double d = 3.0;
    
    /* Complex expression that may generate various condition codes */
    for (int i = 0; i < 100; i++) {
        double x = (i % 2) ? a : b;
        double y = (i % 3) ? c : d;
        
        /* Nested comparisons to force different code paths */
        if (x < y) {
            if (x != y) {
                if (!isunordered(x, y)) {
                    /* ORDERED path */
                    asm volatile ("nop" ::: "memory");
                }
            }
        }
        
        /* Switch on comparison results */
        int cmp_result = 0;
        cmp_result |= (x < y) ? 0x1 : 0;
        cmp_result |= (x <= y) ? 0x2 : 0;
        cmp_result |= (x > y) ? 0x4 : 0;
        cmp_result |= (x >= y) ? 0x8 : 0;
        cmp_result |= (x == y) ? 0x10 : 0;
        cmp_result |= (x != y) ? 0x20 : 0;
        cmp_result |= isunordered(x, y) ? 0x40 : 0;
        
        /* Use result to prevent dead code elimination */
        volatile int sink = cmp_result;
        (void)sink;
    }
}

/* Use GCC vector extensions for additional FP comparisons */
void vector_fp_comparisons(void) {
    typedef double v2df __attribute__((vector_size(16)));
    
    v2df vec1 = { __builtin_nan(""), 1.0 };
    v2df vec2 = { 2.0, __builtin_nan("") };
    v2df vec3 = { 3.0, 4.0 };
    
    /* Vector comparisons may generate different code patterns */
    v2df cmp_result;
    
    cmp_result = vec1 < vec2;
    cmp_result = vec1 <= vec2;
    cmp_result = vec1 > vec2;
    cmp_result = vec1 >= vec2;
    cmp_result = vec1 == vec2;
    cmp_result = vec1 != vec2;
    
    /* Prevent optimization */
    volatile v2df sink = cmp_result;
    (void)sink;
}

int main(void) {
    printf("Generating x86 condition codes for coverage testing...\n");
    
    generate_condition_codes();
    complex_fp_control_flow();
    vector_fp_comparisons();
    
    /* Final unordered comparison to ensure UNORDERED code is generated */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("");
    volatile int final_cmp = isunordered(nan1, nan2);
    
    printf("Final unordered check: %d\n", final_cmp);
    printf("Test completed.\n");
    
    return 0;
}
