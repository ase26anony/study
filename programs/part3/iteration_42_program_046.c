/* test_condition_codes.c - Target uncovered lines in i386.cc */
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
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNORDERED)
        : "cc", "st"
    );
    return result;
}

/* Function to test ORDERED condition code */
static int __attribute__((noinline)) test_ordered(double a, double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(ORDERED)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNEQ condition code */
static int __attribute__((noinline)) test_uneq(double a, double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNEQ)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNGE condition code */
static int __attribute__((noinline)) test_unge(double a, double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNGE)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNGT condition code */
static int __attribute__((noinline)) test_ungt(double a, double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNGT)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNLE condition code */
static int __attribute__((noinline)) test_unle(double a, double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNLE)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNLT condition code */
static int __attribute__((noinline)) test_unlt(double a, double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNLT)
        : "cc", "st"
    );
    return result;
}

/* Function to test LTGT condition code */
static int __attribute__((noinline)) test_ltgt(double a, double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(LTGT)
        : "cc", "st"
    );
    return result;
}

/* Mixed x87 and SSE operations */
static int __attribute__((noinline)) test_mixed_operations(long double ld, double d) {
    int result1, result2;
    
    /* x87 operation with UNORDERED */
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result1)
        : "u"(UNORDERED), "m"(ld), "m"(g_ld2)
        : "cc", "st"
    );
    
    /* SSE operation with ORDERED */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c1 %1"
        : "=r"(result2)
        : "u"(ORDERED), "x"(d), "x"(g_d2)
        : "cc"
    );
    
    return result1 + result2;
}

/* Function that uses a switch to select condition code */
static int __attribute__((noinline)) test_switch_condition(int cc, double a, double b) {
    int result = 0;
    
    switch (cc) {
        case UNORDERED:
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNORDERED), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case ORDERED:
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(ORDERED), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case UNEQ:
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNEQ), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case UNGE:
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNGE), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case UNGT:
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNGT), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case UNLE:
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNLE), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case UNLT:
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNLT), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case LTGT:
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(LTGT), "x"(a), "x"(b)
                : "cc"
            );
            break;
        default:
            /* This might trigger output_operand_lossage if an invalid code is passed */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(cc), "x"(a), "x"(b)  /* Invalid condition code */
                : "cc"
            );
            break;
    }
    
    return result;
}

/* Function with complex control flow to obscure optimization */
static int __attribute__((noinline)) test_complex_flow(double a, double b, int iterations) {
    volatile int sum = 0;
    volatile double x = a;
    volatile double y = b;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix regular C comparisons with inline assembly */
        if (x != y) {
            sum += test_unordered(x, y);
        }
        
        if (x >= y) {
            sum += test_ordered(x, y);
        }
        
        /* Modify values to prevent constant folding */
        x = sin(x + 0.1);
        y = cos(y + 0.1);
        
        /* Use different condition codes based on loop index */
        switch (i % 8) {
            case 0: sum += test_uneq(x, y); break;
            case 1: sum += test_unge(x, y); break;
            case 2: sum += test_ungt(x, y); break;
            case 3: sum += test_unle(x, y); break;
            case 4: sum += test_unlt(x, y); break;
            case 5: sum += test_ltgt(x, y); break;
            case 6: sum += test_unordered(x, y); break;
            case 7: sum += test_ordered(x, y); break;
        }
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    volatile int total = 0;
    int iterations = 100;
    
    /* Parse iterations from command line if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize test values from volatile sources */
    double d1 = g_d1;
    double d2 = g_d2;
    long double ld1 = g_ld1;
    long double ld2 = g_ld2;
    
    printf("Testing x86 condition code printing logic...\n");
    
    /* Test each condition code function directly */
    total += test_unordered(d1, d2);
    total += test_ordered(d2, d1);
    total += test_uneq(d1, d1);
    total += test_unge(d2, d1);
    total += test_ungt(d2, d1);
    total += test_unle(d1, d2);
    total += test_unlt(d1, d2);
    total += test_ltgt(d1, d2);
    
    /* Test mixed x87 and SSE operations */
    total += test_mixed_operations(ld1, d1);
    
    /* Test with switch statement */
    for (int i = 0; i < 8; i++) {
        total += test_switch_condition(i, d1 + i, d2 + i);
    }
    
    /* Try to trigger default case with potentially invalid code */
    if (argc > 2) {
        int invalid_cc = atoi(argv[2]);
        total += test_switch_condition(invalid_cc, d1, d2);
    }
    
    /* Test complex control flow */
    total += test_complex_flow(d1, d2, iterations);
    
    /* Use the result to prevent dead code elimination */
    printf("Accumulated result: %d\n", total);
    
    /* Additional test with NaN values to trigger UNORDERED */
    double nan_val = 0.0 / 0.0;
    total += test_unordered(nan_val, d1);
    total += test_ordered(d1, nan_val);
    
    printf("Final result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
