/* Condition Code Test Program for i386.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

/* Force generation of specific condition codes */
#define FORCE_CC_PRINTING 1

/* Mix of normal values and NaN patterns */
static void init_arrays(double *arr1, double *arr2, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
        /* Insert NaN at strategic positions to create unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
        if (i % 13 == 0) {
            arr1[i] = __builtin_nan("0xdead");
            arr2[i] = __builtin_nan("0xbeef");
        }
    }
}

/* Direct inline assembly to trigger %C format specifier */
static void direct_cc_printing(void) {
    int var = 42;
    int src = 99;
    
    /* UNORDERED condition code */
    asm volatile (
        "# UNORDERED condition code\n\t"
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(0)  /* 0 = UNORDERED */
        : "cc"
    );
    
    /* ORDERED condition code */
    var = 42;
    asm volatile (
        "# ORDERED condition code\n\t"
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(7)  /* 7 = ORDERED */
        : "cc"
    );
    
    /* UNEQ condition code */
    var = 42;
    asm volatile (
        "# UNEQ condition code\n\t"
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(6)  /* 6 = UNEQ */
        : "cc"
    );
    
    /* UNGE condition code */
    var = 42;
    asm volatile (
        "# UNGE condition code\n\t"
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(13) /* 13 = UNGE */
        : "cc"
    );
    
    /* UNGT condition code */
    var = 42;
    asm volatile (
        "# UNGT condition code\n\t"
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(14) /* 14 = UNGT */
        : "cc"
    );
    
    /* UNLE condition code */
    var = 42;
    asm volatile (
        "# UNLE condition code\n\t"
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(15) /* 15 = UNLE */
        : "cc"
    );
    
    /* UNLT condition code */
    var = 42;
    asm volatile (
        "# UNLT condition code\n\t"
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(12) /* 12 = UNLT */
        : "cc"
    );
    
    /* LTGT condition code */
    var = 42;
    asm volatile (
        "# LTGT condition code\n\t"
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(4)  /* 4 = LTGT */
        : "cc"
    );
}

/* Complex floating-point comparisons to generate condition codes */
static int perform_fp_comparisons(double *arr1, double *arr2, int size) {
    volatile int cc_accumulator = 0;
    int temp_result = 0;
    
    for (int i = 0; i < size; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Generate all standard FP comparisons */
        volatile int lt = (a < b) ? 1 : 0;
        volatile int le = (a <= b) ? 1 : 0;
        volatile int gt = (a > b) ? 1 : 0;
        volatile int ge = (a >= b) ? 1 : 0;
        volatile int eq = (a == b) ? 1 : 0;
        volatile int ne = (a != b) ? 1 : 0;
        
        /* Force conditional move generation through ternary operators */
        temp_result = (a < b) ? (temp_result + 1) : (temp_result - 1);
        temp_result = (a <= b) ? (temp_result * 2) : (temp_result / 2);
        temp_result = (a > b) ? (temp_result | 0xFF) : (temp_result & 0x0F);
        temp_result = (a >= b) ? (temp_result << 2) : (temp_result >> 2);
        temp_result = (a == b) ? (temp_result ^ 0xAA) : (temp_result ^ 0x55);
        temp_result = (a != b) ? (temp_result + 0x10) : (temp_result - 0x10);
        
        /* Accumulate comparison results to prevent optimization */
        cc_accumulator += lt + le + gt + ge + eq + ne;
        
        /* Special handling for NaN comparisons */
        if (isnan(a) || isnan(b)) {
            /* Force unordered comparisons */
            volatile int unordered = !(a == a) || !(b == b);
            cc_accumulator += unordered ? 100 : -100;
        }
    }
    
    return cc_accumulator + temp_result;
}

/* Mixed integer/float conditional operations */
static void mixed_conditional_ops(void) {
    double f1 = __builtin_nan("");
    double f2 = 3.14159;
    int i1 = 42, i2 = 99;
    
    /* These may generate conditional moves with FP condition codes */
    int result1 = (f1 < f2) ? i1 : i2;
    int result2 = (f1 <= f2) ? i1 : i2;
    int result3 = (f1 > f2) ? i1 : i2;
    int result4 = (f1 >= f2) ? i1 : i2;
    int result5 = (f1 == f2) ? i1 : i2;
    int result6 = (f1 != f2) ? i1 : i2;
    
    /* Use results to prevent optimization */
    volatile int dummy = result1 + result2 + result3 + result4 + result5 + result6;
    (void)dummy;
}

/* Function with FP comparisons in control flow */
static void fp_control_flow(double *arr, int size) {
    for (int i = 1; i < size; i++) {
        /* Complex conditional with FP comparisons */
        if (arr[i] < arr[i-1] || arr[i] > arr[i-1] * 2.0) {
            arr[i] = arr[i-1];
        } else if (arr[i] == arr[i-1] || arr[i] != arr[i-1] * 0.5) {
            arr[i] = -arr[i];
        }
        
        /* Nested comparisons */
        if ((arr[i] <= 0.0 && arr[i] >= -1.0) || (arr[i] >= 100.0 && arr[i] <= 200.0)) {
            arr[i] = sqrt(fabs(arr[i]));
        }
    }
}

int main(void) {
    const int ARRAY_SIZE = 256;
    double arr1[ARRAY_SIZE];
    double arr2[ARRAY_SIZE];
    
    /* Initialize with mixed normal and NaN values */
    init_arrays(arr1, arr2, ARRAY_SIZE);
    
    /* Direct condition code printing via inline assembly */
    direct_cc_printing();
    
    /* Perform floating-point comparisons to generate condition codes */
    int cc_result = perform_fp_comparisons(arr1, arr2, ARRAY_SIZE);
    
    /* Mixed integer/float conditional operations */
    mixed_conditional_ops();
    
    /* FP comparisons in control flow */
    fp_control_flow(arr1, ARRAY_SIZE);
    fp_control_flow(arr2, ARRAY_SIZE);
    
    /* Prevent optimization of results */
    volatile int final_result = cc_result;
    
    printf("Condition code test completed. Result: %d\n", final_result);
    
    return 0;
}
