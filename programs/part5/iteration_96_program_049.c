/* Condition Code Test Program for i386.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <x86intrin.h>

/* Force generation of various condition codes */
#define FORCE_CC_PRINTING 1

/* Mix of normal values and NaN */
static void init_arrays(double arr1[256], double arr2[256]) {
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
        /* Insert NaN at specific indices to create unordered comparisons */
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

/* Direct inline assembly with %C constraint to trigger printing */
static void direct_cc_printing(void) {
    int var = 42;
    int src = 99;
    
    /* UNORDERED condition */
    asm volatile ("# UNORDERED test\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(UNORDERED));
    
    /* ORDERED condition */
    asm volatile ("# ORDERED test\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(ORDERED));
    
    /* UNEQ condition */
    asm volatile ("# UNEQ test\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(UNEQ));
    
    /* UNGE condition */
    asm volatile ("# UNGE test\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(UNGE));
    
    /* UNGT condition */
    asm volatile ("# UNGT test\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(UNGT));
    
    /* UNLE condition */
    asm volatile ("# UNLE test\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(UNLE));
    
    /* UNLT condition */
    asm volatile ("# UNLT test\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(UNLT));
    
    /* LTGT condition */
    asm volatile ("# LTGT test\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(LTGT));
    
    (void)var; /* Prevent unused variable warning */
}

/* Generate condition codes through floating-point comparisons */
static volatile int cc_accumulator = 0;

static void generate_fp_condition_codes(double a, double b) {
    volatile int result;
    
    /* Generate all standard FP comparisons */
    result = (a < b) ? 1 : 0;   /* May generate UNLT or LT */
    cc_accumulator += result;
    
    result = (a <= b) ? 2 : 0;  /* May generate UNLE or LE */
    cc_accumulator += result;
    
    result = (a > b) ? 3 : 0;   /* May generate UNGT or GT */
    cc_accumulator += result;
    
    result = (a >= b) ? 4 : 0;  /* May generate UNGE or GE */
    cc_accumulator += result;
    
    result = (a == b) ? 5 : 0;  /* May generate UNEQ or EQ */
    cc_accumulator += result;
    
    result = (a != b) ? 6 : 0;  /* May generate LTGT or NE */
    cc_accumulator += result;
    
    /* Ordered/unordered checks */
    result = (!isunordered(a, b)) ? 7 : 0;  /* ORDERED */
    cc_accumulator += result;
    
    result = (isunordered(a, b)) ? 8 : 0;   /* UNORDERED */
    cc_accumulator += result;
}

/* Use builtin to generate conditional moves */
static void generate_cmov_instructions(void) {
    double a = __builtin_nan("");
    double b = 3.14159;
    int x = 0, y = 0;
    
    /* Force generation of condition codes for cmov */
    if (__builtin_constant_p(0)) {
        /* These won't execute but force compiler to consider them */
        asm volatile ("# CMOV block\n\t"
                      "cmov%C0 %1, %0" : "+r"(x) : "r"(y), "i"(UNORDERED));
        asm volatile ("cmov%C0 %1, %0" : "+r"(x) : "r"(y), "i"(ORDERED));
        asm volatile ("cmov%C0 %1, %0" : "+r"(x) : "r"(y), "i"(UNEQ));
    }
    
    /* Real comparisons that generate condition codes */
    volatile double v = a;
    volatile double w = b;
    
    /* This should generate UNORDERED comparisons */
    int r1 = (v < w) ? 100 : 200;
    int r2 = (v == w) ? 300 : 400;
    int r3 = (v != w) ? 500 : 600;
    
    cc_accumulator += r1 + r2 + r3;
}

int main(void) {
    double arr1[256], arr2[256];
    
    /* Initialize with mix of normal values and NaN */
    init_arrays(arr1, arr2);
    
    /* Direct condition code printing via inline assembly */
    direct_cc_printing();
    
    /* Generate condition codes through FP comparisons */
    for (int i = 0; i < 256; i++) {
        generate_fp_condition_codes(arr1[i], arr2[i]);
        
        /* Additional mixed comparisons */
        if (i % 3 == 0) {
            double temp = arr1[i];
            arr1[i] = arr2[i];
            arr2[i] = temp;
        }
    }
    
    /* Generate cmov instructions */
    generate_cmov_instructions();
    
    /* Complex expression to force various condition codes */
    for (int i = 0; i < 256; i += 4) {
        double a = arr1[i];
        double b = arr2[i];
        double c = arr1[i+1];
        double d = arr2[i+1];
        
        /* Complex floating-point expression */
        volatile int res = ((a < b) && (c > d)) ? 1 : 
                          ((a == b) || (c != d)) ? 2 :
                          (!isunordered(a, c) && isunordered(b, d)) ? 3 : 4;
        
        cc_accumulator += res;
        
        /* Use ternary with FP condition on integer target */
        int target = (a < b) ? i : (i * 2);
        cc_accumulator += target;
    }
    
    /* Prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    /* Additional unordered comparisons */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("");
    volatile double normal = 42.0;
    
    /* These should generate UNORDERED condition codes */
    int test1 = (nan1 < normal) ? 1 : 0;
    int test2 = (normal > nan2) ? 1 : 0;
    int test3 = (nan1 == nan2) ? 1 : 0;
    int test4 = (nan1 != normal) ? 1 : 0;
    
    printf("Unordered test results: %d %d %d %d\n", test1, test2, test3, test4);
    
    return cc_accumulator != 0 ? 0 : 1;
}
