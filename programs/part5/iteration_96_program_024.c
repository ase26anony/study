/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force generation of specific condition codes */
#define FORCE_CC_PRINTING 1

/* Mix of normal values and NaN */
static double arr1[256];
static double arr2[256];

/* Volatile to prevent optimization */
volatile int cc_accumulator = 0;

/* Initialize arrays with mix of values and NaN */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        /* Every 7th element is NaN, others are normal values */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
            arr2[i] = (i % 14 == 0) ? __builtin_nan("") : i * 1.5;
        } else if (i % 11 == 0) {
            arr1[i] = i * 2.3;
            arr2[i] = __builtin_nan("");
        } else {
            arr1[i] = i * 1.5;
            arr2[i] = i * 1.5 + 0.1;
        }
    }
}

/* Direct inline assembly with %C constraint to trigger printing */
void direct_cc_printing(void) {
    int var = 42;
    int src = 99;
    
    /* UNORDERED condition */
    asm volatile ("# UNORDERED condition code\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(0)  /* 0 = UNORDERED */
                  : "memory");
    
    /* ORDERED condition */
    asm volatile ("# ORDERED condition code\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(7)  /* 7 = ORDERED */
                  : "memory");
    
    /* UNEQ condition */
    asm volatile ("# UNEQ condition code\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(1)  /* 1 = UNEQ */
                  : "memory");
    
    /* UNGE condition */
    asm volatile ("# UNGE condition code\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(5)  /* 5 = UNGE */
                  : "memory");
    
    /* UNGT condition */
    asm volatile ("# UNGT condition code\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(6)  /* 6 = UNGT */
                  : "memory");
    
    /* UNLE condition */
    asm volatile ("# UNLE condition code\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(3)  /* 3 = UNLE */
                  : "memory");
    
    /* UNLT condition */
    asm volatile ("# UNLT condition code\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(2)  /* 2 = UNLT */
                  : "memory");
    
    /* LTGT condition */
    asm volatile ("# LTGT condition code\n\t"
                  "cmov%C0 %1, %0"
                  : "+r"(var)
                  : "r"(src), "i"(4)  /* 4 = LTGT */
                  : "memory");
}

/* Generate floating-point comparisons that produce various condition codes */
void generate_fp_comparisons(void) {
    volatile double a, b;
    volatile int result;
    
    /* Test 1: NaN vs NaN (unordered) */
    a = __builtin_nan("");
    b = __builtin_nan("");
    
    /* All comparisons with NaN should be false except != */
    result = (a < b) ? 1 : 0;   /* UNORDERED/LT */
    cc_accumulator += result;
    result = (a <= b) ? 1 : 0;  /* UNORDERED/LE */
    cc_accumulator += result;
    result = (a > b) ? 1 : 0;   /* UNORDERED/GT */
    cc_accumulator += result;
    result = (a >= b) ? 1 : 0;  /* UNORDERED/GE */
    cc_accumulator += result;
    result = (a == b) ? 1 : 0;  /* UNORDERED/EQ */
    cc_accumulator += result;
    result = (a != b) ? 1 : 0;  /* UNORDERED/NEQ - This is true! */
    cc_accumulator += result;
    
    /* Test 2: NaN vs normal number */
    a = __builtin_nan("");
    b = 3.14159;
    
    result = (a < b) ? 1 : 0;   /* UNORDERED */
    cc_accumulator += result;
    result = (a <= b) ? 1 : 0;  /* UNORDERED */
    cc_accumulator += result;
    result = (a > b) ? 1 : 0;   /* UNORDERED */
    cc_accumulator += result;
    result = (a >= b) ? 1 : 0;  /* UNORDERED */
    cc_accumulator += result;
    result = (a == b) ? 1 : 0;  /* UNORDERED */
    cc_accumulator += result;
    result = (a != b) ? 1 : 0;  /* UNORDERED - This is true! */
    cc_accumulator += result;
    
    /* Test 3: Normal numbers with various relations */
    a = 1.0;
    b = 2.0;
    
    result = (a < b) ? 1 : 0;   /* LT */
    cc_accumulator += result;
    result = (a <= b) ? 1 : 0;  /* LE */
    cc_accumulator += result;
    result = (a > b) ? 1 : 0;   /* GT */
    cc_accumulator += result;
    result = (a >= b) ? 1 : 0;  /* GE */
    cc_accumulator += result;
    result = (a == b) ? 1 : 0;  /* EQ */
    cc_accumulator += result;
    result = (a != b) ? 1 : 0;  /* NEQ */
    cc_accumulator += result;
    
    /* Test 4: Equal normal numbers */
    a = 2.5;
    b = 2.5;
    
    result = (a < b) ? 1 : 0;   /* EQ/GE/LE but not LT */
    cc_accumulator += result;
    result = (a <= b) ? 1 : 0;  /* LE/EQ */
    cc_accumulator += result;
    result = (a > b) ? 1 : 0;   /* EQ/GE/LE but not GT */
    cc_accumulator += result;
    result = (a >= b) ? 1 : 0;  /* GE/EQ */
    cc_accumulator += result;
    result = (a == b) ? 1 : 0;  /* EQ */
    cc_accumulator += result;
    result = (a != b) ? 1 : 0;  /* NEQ */
    cc_accumulator += result;
}

/* Loop through arrays generating mixed condition codes */
void array_comparison_loop(void) {
    int temp_result;
    
    for (int i = 0; i < 256; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Generate all comparison types, forcing condition code evaluation */
        temp_result = (a < b) ? (i & 1) : (i & 2);
        cc_accumulator ^= temp_result;
        
        temp_result = (a <= b) ? (i & 3) : (i & 4);
        cc_accumulator ^= temp_result;
        
        temp_result = (a > b) ? (i & 5) : (i & 6);
        cc_accumulator ^= temp_result;
        
        temp_result = (a >= b) ? (i & 7) : (i & 8);
        cc_accumulator ^= temp_result;
        
        temp_result = (a == b) ? (i & 9) : (i & 10);
        cc_accumulator ^= temp_result;
        
        temp_result = (a != b) ? (i & 11) : (i & 12);
        cc_accumulator ^= temp_result;
        
        /* Use ternary operator to potentially generate conditional moves */
        int x = (a < b) ? i : -i;
        int y = (a <= b) ? x : -x;
        int z = (a > b) ? y : -y;
        cc_accumulator += z;
    }
}

/* Additional test with mixed integer/float operations */
void mixed_operations(void) {
    volatile float f1, f2;
    volatile double d1, d2;
    volatile int i1, i2;
    
    f1 = __builtin_nanf("");
    f2 = 1.0f;
    d1 = __builtin_nan("");
    d2 = 2.0;
    
    /* Mixed float/double comparisons */
    i1 = (f1 < f2) ? 100 : 200;
    i2 = (d1 > d2) ? 300 : 400;
    
    cc_accumulator += i1 + i2;
    
    /* Force ordered/unordered checks */
    if (!(f1 == f1)) {  /* NaN check - unordered */
        cc_accumulator += 1000;
    }
    
    if (d2 == d2) {     /* Normal number - ordered */
        cc_accumulator += 2000;
    }
}

int main(void) {
    init_arrays();
    
    /* Direct condition code printing via inline assembly */
    direct_cc_printing();
    
    /* Generate various floating-point comparisons */
    generate_fp_comparisons();
    
    /* Process arrays with mixed NaN/normal values */
    array_comparison_loop();
    
    /* Additional mixed operations */
    mixed_operations();
    
    /* Print result to prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    return 0;
}
