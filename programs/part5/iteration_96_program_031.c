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
        arr2[i] = (i + 1) * 1.3;
        
        /* Insert NaN values at specific indices to create unordered comparisons */
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

/* Direct inline assembly with %C constraint to trigger condition code printing */
static void direct_cc_asm(void) {
    int var = 0;
    int src = 42;
    
    /* UNORDERED condition */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(0)  /* 0 = UNORDERED */
        : "cc"
    );
    
    /* ORDERED condition */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(7)  /* 7 = ORDERED */
        : "cc"
    );
    
    /* UNEQ condition */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(1)  /* 1 = UNEQ */
        : "cc"
    );
    
    /* UNGE condition */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(5)  /* 5 = UNGE */
        : "cc"
    );
    
    /* UNGT condition */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(6)  /* 6 = UNGT */
        : "cc"
    );
    
    /* UNLE condition */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(2)  /* 2 = UNLE */
        : "cc"
    );
    
    /* UNLT condition */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(3)  /* 3 = UNLT */
        : "cc"
    );
    
    /* LTGT condition */
    asm volatile (
        "cmov%C0 %1, %0\n\t"
        : "+r"(var)
        : "r"(src), "i"(4)  /* 4 = LTGT */
        : "cc"
    );
}

/* Force unordered floating-point comparisons */
static volatile int cc_accumulator = 0;

static void perform_fp_comparisons(double a, double b) {
    volatile int result;
    
    /* All six standard comparisons - will generate various condition codes */
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
static void generate_cmov(double a, double b) {
    int x = 0, y = 100;
    int res;
    
    /* These may expand to conditional moves with condition codes */
    res = (a < b) ? x : y;
    cc_accumulator += res;
    
    res = (a > b) ? x : y;
    cc_accumulator += res;
    
    res = (a == b) ? x : y;
    cc_accumulator += res;
    
    res = (a != b) ? x : y;
    cc_accumulator += res;
}

int main(void) {
    double arr1[256], arr2[256];
    
    /* Initialize with mix of normal values and NaN */
    init_arrays(arr1, arr2);
    
    /* Direct inline assembly to trigger condition code printing */
    direct_cc_asm();
    
    /* Loop through arrays performing all types of comparisons */
    for (int i = 0; i < 256; i++) {
        perform_fp_comparisons(arr1[i], arr2[i]);
        generate_cmov(arr1[i], arr2[i]);
        
        /* Additional unordered checks */
        if (isunordered(arr1[i], arr2[i])) {
            cc_accumulator += i;
        }
        
        /* Mixed relational operators to generate different codes */
        volatile double tmp = arr1[i];
        if (tmp < arr2[i]) cc_accumulator += 1;
        if (tmp <= arr2[i]) cc_accumulator += 2;
        if (tmp > arr2[i]) cc_accumulator += 3;
        if (tmp >= arr2[i]) cc_accumulator += 4;
        if (tmp == arr2[i]) cc_accumulator += 5;
        if (tmp != arr2[i]) cc_accumulator += 6;
    }
    
    /* Prevent optimization */
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    /* Additional inline assembly with different condition codes */
    {
        int a = 10, b = 20;
        asm volatile (
            "testl %1, %1\n\t"
            "set%C0 %b2\n\t"
            "movzbl %b2, %0\n\t"
            : "=r"(a)
            : "r"(b), "q"(a), "i"(0)  /* UNORDERED */
            : "cc"
        );
        
        asm volatile (
            "testl %1, %1\n\t"
            "set%C0 %b2\n\t"
            "movzbl %b2, %0\n\t"
            : "=r"(a)
            : "r"(b), "q"(a), "i"(7)  /* ORDERED */
            : "cc"
        );
    }
    
    return 0;
}
