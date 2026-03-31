/* test_i386_condcodes.c - Target coverage for i386.cc condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent constant folding */
volatile double g_d1 = 1.0;
volatile double g_d2 = 2.0;
volatile long double g_ld1 = 3.0L;
volatile long double g_ld2 = 4.0L;
volatile int g_selector = 0;

/* Condition code constants matching i386.h */
#define UNORDERED 0
#define ORDERED   1
#define UNEQ      2
#define UNGE      3
#define UNGT      4
#define UNLE      5
#define UNLT      6
#define LTGT      7

/* Function to test UNORDERED condition code */
static int __attribute__((noinline)) test_unordered(double a, double b) {
    int result;
    /* Using x87 instruction with UNORDERED condition */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNORDERED)
        : "cc", "st"
    );
    return result;
}

/* Function to test ORDERED condition code */
static int __attribute__((noinline)) test_ordered(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(ORDERED)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNEQ condition code */
static int __attribute__((noinline)) test_uneq(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNEQ)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNGE condition code */
static int __attribute__((noinline)) test_unge(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNGE)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNGT condition code */
static int __attribute__((noinline)) test_ungt(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNGT)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNLE condition code */
static int __attribute__((noinline)) test_unle(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNLE)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNLT condition code */
static int __attribute__((noinline)) test_unlt(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNLT)
        : "cc", "st"
    );
    return result;
}

/* Function to test LTGT condition code */
static int __attribute__((noinline)) test_ltgt(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(LTGT)
        : "cc", "st"
    );
    return result;
}

/* SSE version using comisd instruction */
static int __attribute__((noinline)) test_sse_unordered(double a, double b) {
    int result;
    asm volatile (
        "comisd %1, %2\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNORDERED)
        : "cc"
    );
    return result;
}

/* Mixed x87 and SSE operations */
static int __attribute__((noinline)) test_mixed_fp(long double a, double b) {
    int result1, result2;
    
    /* x87 operation */
    asm volatile (
        "fldt %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result1)
        : "m"(a), "m"(b), "u"(UNORDERED)
        : "cc", "st"
    );
    
    /* SSE operation */
    double d_a = (double)a;
    asm volatile (
        "comisd %1, %2\n\t"
        "set%c0 %0"
        : "=r"(result2)
        : "x"(d_a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    
    return result1 & result2;
}

/* Helper function that uses switch to select condition code */
static int __attribute__((noinline)) test_cond_switch(double a, double b, int cond_code) {
    int result = 0;
    
    switch (cond_code) {
        case UNORDERED:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNORDERED)
                : "cc", "st"
            );
            break;
        case ORDERED:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(ORDERED)
                : "cc", "st"
            );
            break;
        case UNEQ:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNEQ)
                : "cc", "st"
            );
            break;
        case UNGE:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNGE)
                : "cc", "st"
            );
            break;
        case UNGT:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNGT)
                : "cc", "st"
            );
            break;
        case UNLE:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNLE)
                : "cc", "st"
            );
            break;
        case UNLT:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNLT)
                : "cc", "st"
            );
            break;
        case LTGT:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(LTGT)
                : "cc", "st"
            );
            break;
        default:
            /* This might trigger output_operand_lossage if cond_code is invalid */
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(cond_code)  /* Potentially invalid condition code */
                : "cc", "st"
            );
            break;
    }
    
    return result;
}

/* Function with regular C comparisons mixed with inline asm */
static int __attribute__((noinline)) test_mixed_comparisons(double a, double b) {
    int result = 0;
    
    /* Regular C comparison */
    if (a != b) {
        result |= 1;
    }
    
    /* Inline asm with condition code */
    int asm_result;
    asm volatile (
        "comisd %1, %2\n\t"
        "set%c0 %0"
        : "=r"(asm_result)
        : "x"(a), "x"(b), "u"(UNORDERED)
        : "cc"
    );
    result |= (asm_result << 1);
    
    /* Another regular comparison */
    if (a >= b) {
        result |= 4;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int total = 0;
    int iterations = 100;
    
    /* Parse iteration count from command line */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create arrays with volatile values */
    double d_vals[8];
    long double ld_vals[8];
    
    for (int i = 0; i < 8; i++) {
        d_vals[i] = g_d1 + i * 0.5;
        ld_vals[i] = g_ld1 + i * 0.5L;
    }
    
    /* Main test loop */
    for (int i = 0; i < iterations; i++) {
        int idx1 = i % 8;
        int idx2 = (i + 1) % 8;
        
        /* Test all condition code functions */
        total += test_unordered(d_vals[idx1], d_vals[idx2]);
        total += test_ordered(d_vals[idx1], d_vals[idx2]);
        total += test_uneq(d_vals[idx1], d_vals[idx2]);
        total += test_unge(d_vals[idx1], d_vals[idx2]);
        total += test_ungt(d_vals[idx1], d_vals[idx2]);
        total += test_unle(d_vals[idx1], d_vals[idx2]);
        total += test_unlt(d_vals[idx1], d_vals[idx2]);
        total += test_ltgt(d_vals[idx1], d_vals[idx2]);
        
        /* Test SSE version */
        total += test_sse_unordered(d_vals[idx1], d_vals[idx2]);
        
        /* Test mixed x87/SSE */
        total += test_mixed_fp(ld_vals[idx1], d_vals[idx2]);
        
        /* Test with switch statement */
        int cond = i % 9;  /* 8 valid codes + 1 potentially invalid */
        total += test_cond_switch(d_vals[idx1], d_vals[idx2], cond);
        
        /* Test mixed comparisons */
        total += test_mixed_comparisons(d_vals[idx1], d_vals[idx2]);
        
        /* Modify values slightly */
        d_vals[idx1] += 0.1;
        ld_vals[idx1] += 0.1L;
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int final_result = total;
    
    printf("Final result: %d\n", final_result);
    
    /* Try to trigger the default case with an invalid condition code */
    if (argc > 2 && strcmp(argv[2], "test-invalid") == 0) {
        double a = 1.0, b = 2.0;
        int invalid_result;
        
        /* This might trigger output_operand_lossage */
        asm volatile (
            "fldl %1\n\t"
            "fldl %2\n\t"
            "fucomip %%st(1), %%st(0)\n\t"
            "set%c0 %0\n\t"
            "fstp %%st(0)"
            : "=r"(invalid_result)
            : "m"(a), "m"(b), "u"(999)  /* Invalid condition code */
            : "cc", "st"
        );
        
        printf("Invalid cond test: %d\n", invalid_result);
    }
    
    return 0;
}
