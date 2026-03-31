/* Condition code test program for i386.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent constant folding */
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

/* Prevent inlining to ensure separate RTL generation */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* Use x87 instruction with UNORDERED condition code */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNORDERED), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
    /* SSE comparison with ORDERED condition */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(ORDERED), "x"(a), "x"(b)
        : "cc"
    );
    return result;
}

__attribute__((noinline))
static int test_uneq(long double a, long double b) {
    int result;
    /* x87 long double comparison with UNEQ */
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNEQ), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    /* Mixed comparison with UNGE */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNGE), "x"(a), "x"(b)
        : "cc"
    );
    return result;
}

__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    /* x87 with UNGT */
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNGT), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    /* SSE with UNLE */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNLE), "x"(a), "x"(b)
        : "cc"
    );
    return result;
}

__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    /* x87 with UNLT */
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNLT), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    /* SSE with LTGT */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(LTGT), "x"(a), "x"(b)
        : "cc"
    );
    return result;
}

/* Function that uses switch to select condition code */
__attribute__((noinline))
static int dispatch_condition(int cond, double a, double b) {
    int result = 0;
    
    /* This switch may generate RTL that needs condition code printing */
    switch (cond) {
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
            /* This might trigger output_operand_lossage */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(cond), "x"(a), "x"(b)  /* Invalid condition code */
                : "cc"
            );
            break;
    }
    return result;
}

/* Helper to generate NaN values */
static double make_nan(void) {
    volatile double zero = 0.0;
    return 0.0 / zero;
}

int main(int argc, char *argv[]) {
    int i, iterations = 100;
    volatile int accumulator = 0;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create test values including NaN, infinity, normal numbers */
    double test_doubles[] = {
        1.0, 2.0, -1.0, 0.0, 
        INFINITY, -INFINITY, make_nan()
    };
    
    long double test_long_doubles[] = {
        1.0L, 2.0L, -1.0L, 0.0L,
        INFINITY, -INFINITY
    };
    
    int num_doubles = sizeof(test_doubles) / sizeof(test_doubles[0]);
    int num_long_doubles = sizeof(test_long_doubles) / sizeof(test_long_doubles[0]);
    
    printf("Testing condition codes for %d iterations\n", iterations);
    
    for (i = 0; i < iterations; i++) {
        /* Mix volatile and non-volatile accesses */
        double d1 = g_d1 + i * 0.1;
        double d2 = g_d2 + i * 0.2;
        long double ld1 = g_ld1 + i * 0.1L;
        long double ld2 = g_ld2 + i * 0.2L;
        
        /* Test all condition code functions */
        accumulator += test_unordered(d1, d2);
        accumulator += test_ordered(d2, d1);
        accumulator += test_uneq(ld1, ld2);
        accumulator += test_unge(d1, d2);
        accumulator += test_ungt(ld2, ld1);
        accumulator += test_unle(d2, d1);
        accumulator += test_unlt(ld1, ld2);
        accumulator += test_ltgt(d1, d2);
        
        /* Also test with array values (including NaN) */
        int idx = i % num_doubles;
        int idx2 = (i + 1) % num_doubles;
        accumulator += test_unordered(test_doubles[idx], test_doubles[idx2]);
        
        idx = i % num_long_doubles;
        idx2 = (i + 2) % num_long_doubles;
        accumulator += test_uneq(test_long_doubles[idx], test_long_doubles[idx2]);
        
        /* Use dispatch function with volatile selector */
        g_selector = (g_selector + 1) & 7;  /* Cycle through valid codes */
        accumulator += dispatch_condition(g_selector, d1, d2);
        
        /* Occasionally test with potentially invalid condition code */
        if (i % 13 == 0) {
            int invalid_cond = 8 + (i % 5);  /* Invalid condition codes */
            accumulator += dispatch_condition(invalid_cond, d1, d2);
        }
        
        /* Mix with regular C comparisons to provide context */
        if (d1 != d2) accumulator++;
        if (ld1 >= ld2) accumulator++;
        if (!isnan(d1)) accumulator++;
    }
    
    printf("Final accumulator: %d\n", accumulator);
    
    /* Force use of all condition codes in one final complex expression */
    {
        double a = g_d1, b = g_d2;
        long double c = g_ld1, d = g_ld2;
        
        /* Complex sequence that might generate various condition codes */
        asm volatile (
            "fldl %1\n\t"
            "fldl %2\n\t"
            "fucomip %%st(1), %%st(0)\n\t"
            "set%c0 %%al\n\t"
            "movzbl %%al, %0\n\t"
            "fstp %%st(0)\n\t"
            "comisd %3, %4\n\t"
            "set%c5 %%cl\n\t"
            "addb %%cl, %%al\n\t"
            "movzbl %%al, %0"
            : "=r"(accumulator)
            : "m"(a), "m"(b), "x"(a), "x"(b), 
              "u"(UNORDERED), "u"(ORDERED)
            : "cc", "st", "eax", "ecx"
        );
    }
    
    return accumulator != 0 ? 0 : 1;
}
