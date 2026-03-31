/* Condition Code Coverage Test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <x86intrin.h>

/* Force generation of all condition codes through floating-point comparisons */
#define FORCE_CC_PRINTING 1

/* Mix of normal values and NaN */
static void init_arrays(double *arr1, double *arr2, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 2.0;
        
        /* Insert NaN at specific indices to create unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
    }
}

/* Direct inline assembly with %C constraint to trigger condition code printing */
static void direct_cc_asm(void) {
    int var = 0;
    int src = 42;
    
    /* UNORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(0) /* 0 = UNORDERED */
                  : "cc");
    
    /* ORDERED */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(7) /* 7 = ORDERED */
                  : "cc");
    
    /* UNEQ */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(1) /* 1 = UNEQ */
                  : "cc");
    
    /* UNGE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(2) /* 2 = UNGE */
                  : "cc");
    
    /* UNGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(3) /* 3 = UNGT */
                  : "cc");
    
    /* UNLE */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(4) /* 4 = UNLE */
                  : "cc");
    
    /* UNLT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(5) /* 5 = UNLT */
                  : "cc");
    
    /* LTGT */
    asm volatile ("cmov%C0 %1, %0" 
                  : "+r"(var) 
                  : "r"(src), "i"(6) /* 6 = LTGT */
                  : "cc");
    
    /* Prevent optimization */
    volatile int sink = var;
    (void)sink;
}

/* Generate condition codes through floating-point comparisons */
static int generate_condition_codes(double a, double b) {
    volatile int result = 0;
    
    /* All six standard comparisons that can generate various condition codes */
    result += (a < b) ? 1 : 0;   /* May generate UNLT or LTGT */
    result += (a <= b) ? 2 : 0;  /* May generate UNLE */
    result += (a > b) ? 4 : 0;   /* May generate UNGT */
    result += (a >= b) ? 8 : 0;  /* May generate UNGE */
    result += (a == b) ? 16 : 0; /* May generate UNEQ or ORDERED */
    result += (a != b) ? 32 : 0; /* May generate UNORDERED or LTGT */
    
    return result;
}

/* Use ternary operators to force conditional move generation */
static int ternary_with_fp_cond(double a, double b) {
    int x = 0;
    
    /* These may lower to conditional moves with condition code names */
    x = (a < b) ? (x + 1) : (x - 1);
    x = (a <= b) ? (x * 2) : (x / 2);
    x = (a > b) ? (x | 0xFF) : (x & 0x0F);
    x = (a >= b) ? (x ^ 0xAA) : (x ^ 0x55);
    x = (a == b) ? (x << 2) : (x >> 2);
    x = (a != b) ? ~x : x;
    
    return x;
}

/* Mixed integer/float comparisons */
static int mixed_comparisons(double fp_val, int int_val) {
    int result = 0;
    
    /* Compare float with integer - may generate different condition codes */
    result = (fp_val < int_val) ? result + 1 : result;
    result = (fp_val > int_val) ? result * 2 : result;
    result = (fp_val == int_val) ? result | 0x80 : result;
    
    return result;
}

int main(void) {
    const int ARRAY_SIZE = 256;
    double arr1[ARRAY_SIZE];
    double arr2[ARRAY_SIZE];
    volatile int cc_accumulator = 0;
    
    /* Initialize with mix of normal values and NaN */
    init_arrays(arr1, arr2, ARRAY_SIZE);
    
    /* Direct assembly to trigger condition code printing */
    direct_cc_asm();
    
    /* Loop through arrays generating various condition codes */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Force unordered comparisons with NaN values */
        cc_accumulator += generate_condition_codes(arr1[i], arr2[i]);
        
        /* Use ternary operators that may generate conditional moves */
        cc_accumulator += ternary_with_fp_cond(arr1[i], arr2[i]);
        
        /* Mixed type comparisons */
        cc_accumulator += mixed_comparisons(arr1[i], i);
        
        /* Additional unordered checks */
        if (isunordered(arr1[i], arr2[i])) {
            cc_accumulator |= 0x1000;  /* UNORDERED */
        }
        if (islessgreater(arr1[i], arr2[i])) {
            cc_accumulator |= 0x2000;  /* LTGT */
        }
    }
    
    /* Prevent dead code elimination */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    /* Additional inline assembly with %C in different contexts */
    {
        int x = 0, y = 1;
        double d1 = __builtin_nan("");
        double d2 = 3.14;
        
        /* Compare and use result in asm */
        int cmp_result = (d1 < d2);
        asm volatile ("test %1, %1\n\t"
                      "set%C0 %0"
                      : "=r"(x)
                      : "r"(cmp_result), "i"(0) /* UNORDERED */
                      : "cc");
        
        /* Another with ORDERED */
        cmp_result = (d2 == d2);  /* d2 == d2 is true and ordered */
        asm volatile ("test %1, %1\n\t"
                      "set%C0 %0"
                      : "=r"(y)
                      : "r"(cmp_result), "i"(7) /* ORDERED */
                      : "cc");
        
        printf("Unordered test: %d, Ordered test: %d\n", x, y);
    }
    
    return 0;
}
