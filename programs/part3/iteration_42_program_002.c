/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile globals to prevent constant folding */
volatile double g_d1 = 1.0, g_d2 = 2.0;
volatile long double g_ld1 = 3.0L, g_ld2 = 4.0L;
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

/* Force noinline to ensure separate functions */
#define NOINLINE __attribute__((noinline))

/* Test functions for each condition code */
NOINLINE static int test_unordered(double a, double b) {
    int result;
    /* Using x87 instruction with condition code */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNORDERED)
                  : "cc", "st");
    return result;
}

NOINLINE static int test_ordered(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(ORDERED)
                  : "cc", "st");
    return result;
}

NOINLINE static int test_uneq(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNEQ)
                  : "cc", "st");
    return result;
}

NOINLINE static int test_unge(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGE)
                  : "cc", "st");
    return result;
}

NOINLINE static int test_ungt(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGT)
                  : "cc", "st");
    return result;
}

NOINLINE static int test_unle(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLE)
                  : "cc", "st");
    return result;
}

NOINLINE static int test_unlt(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLT)
                  : "cc", "st");
    return result;
}

NOINLINE static int test_ltgt(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(LTGT)
                  : "cc", "st");
    return result;
}

/* Mixed x87 and SSE operations */
NOINLINE static int test_mixed_operations(long double ld, double d) {
    int r1, r2;
    
    /* x87 operation */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(r1)
                  : "u"(UNORDERED)
                  : "cc", "st");
    
    /* SSE operation with different condition code */
    asm volatile ("comisd %1, %2; set%c0 %0"
                  : "=r"(r2)
                  : "x"(d), "x"(ld), "u"(ORDERED)
                  : "cc");
    
    return r1 + r2;
}

/* Function that uses switch to select condition code */
NOINLINE static int test_switch_condition(int cc, double a, double b) {
    int result = 0;
    
    switch (cc) {
        case UNORDERED:
            asm volatile ("comisd %1, %2; set%c0 %0"
                         : "=r"(result)
                         : "x"(a), "x"(b), "u"(UNORDERED)
                         : "cc");
            break;
        case ORDERED:
            asm volatile ("comisd %1, %2; set%c0 %0"
                         : "=r"(result)
                         : "x"(a), "x"(b), "u"(ORDERED)
                         : "cc");
            break;
        case UNEQ:
            asm volatile ("comisd %1, %2; set%c0 %0"
                         : "=r"(result)
                         : "x"(a), "x"(b), "u"(UNEQ)
                         : "cc");
            break;
        case UNGE:
            asm volatile ("comisd %1, %2; set%c0 %0"
                         : "=r"(result)
                         : "x"(a), "x"(b), "u"(UNGE)
                         : "cc");
            break;
        case UNGT:
            asm volatile ("comisd %1, %2; set%c0 %0"
                         : "=r"(result)
                         : "x"(a), "x"(b), "u"(UNGT)
                         : "cc");
            break;
        case UNLE:
            asm volatile ("comisd %1, %2; set%c0 %0"
                         : "=r"(result)
                         : "x"(a), "x"(b), "u"(UNLE)
                         : "cc");
            break;
        case UNLT:
            asm volatile ("comisd %1, %2; set%c0 %0"
                         : "=r"(result)
                         : "x"(a), "x"(b), "u"(UNLT)
                         : "cc");
            break;
        case LTGT:
            asm volatile ("comisd %1, %2; set%c0 %0"
                         : "=r"(result)
                         : "x"(a), "x"(b), "u"(LTGT)
                         : "cc");
            break;
        default:
            /* This should trigger output_operand_lossage for invalid code */
            asm volatile ("comisd %1, %2; set%c0 %0"
                         : "=r"(result)
                         : "x"(a), "x"(b), "u"(cc)  /* Invalid condition code */
                         : "cc");
            break;
    }
    
    return result;
}

/* Function with complex control flow to obscure optimizations */
NOINLINE static int test_complex_flow(double a, double b, int iterations) {
    volatile int sum = 0;
    volatile double x = a;
    volatile double y = b;
    
    for (int i = 0; i < iterations; i++) {
        int cc = i % 8;  /* Cycle through condition codes */
        
        switch (cc) {
            case 0: sum += test_unordered(x, y); break;
            case 1: sum += test_ordered(x, y); break;
            case 2: sum += test_uneq(x, y); break;
            case 3: sum += test_unge(x, y); break;
            case 4: sum += test_ungt(x, y); break;
            case 5: sum += test_unle(x, y); break;
            case 6: sum += test_unlt(x, y); break;
            case 7: sum += test_ltgt(x, y); break;
        }
        
        /* Modify values to prevent constant folding */
        x += 0.1;
        y -= 0.05;
    }
    
    return sum;
}

/* Main function with varied floating-point operations */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int iterations = 100;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize with various values including NaN and Inf */
    double values[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -0.0};
    long double ld_values[] = {1.0L, 2.0L, NAN, INFINITY, -INFINITY, 0.0L, -0.0L};
    
    /* Test all condition code functions */
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            total += test_unordered(values[i], values[j]);
            total += test_ordered(values[i], values[j]);
            total += test_uneq(values[i], values[j]);
            total += test_unge(values[i], values[j]);
            total += test_ungt(values[i], values[j]);
            total += test_unle(values[i], values[j]);
            total += test_unlt(values[i], values[j]);
            total += test_ltgt(values[i], values[j]);
            
            /* Test with long doubles */
            total += test_mixed_operations(ld_values[i], values[j]);
        }
    }
    
    /* Test with switch-based condition code selection */
    volatile int selector = g_selector;
    for (int i = 0; i < 10; i++) {
        /* Test valid condition codes */
        total += test_switch_condition(i % 8, values[i % 7], values[(i + 1) % 7]);
        
        /* Test potentially invalid condition code (8) */
        if (i == 5) {
            total += test_switch_condition(8, values[0], values[1]);
        }
    }
    
    /* Test complex flow control */
    total += test_complex_flow(g_d1, g_d2, iterations);
    
    /* Use result to prevent dead code elimination */
    printf("Total: %d\n", total);
    
    /* Additional assembly output that might trigger printing */
    asm volatile ("# Condition code test complete" : : : "memory");
    
    return total != 0 ? 0 : 1;
}
