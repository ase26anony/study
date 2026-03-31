/* i386_condition_codes.c - Target coverage for x86 condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile globals to prevent constant folding */
volatile double g_d1 = 1.0, g_d2 = 2.0;
volatile long double g_ld1 = 3.0L, g_ld2 = 4.0L;
volatile int g_selector = 0;

/* Condition code constants matching i386.h */
enum x86_cc {
    UNORDERED = 0,
    ORDERED = 1,
    UNEQ = 2,
    UNGE = 3,
    UNGT = 4,
    UNLE = 5,
    UNLT = 6,
    LTGT = 7
};

/* Prevent inlining to ensure separate assembly generation */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* Use %c0 to output condition code name */
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(UNORDERED), "x"(a), "x"(b)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(ORDERED), "x"(a), "x"(b)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_uneq(long double a, long double b) {
    int result;
    /* x87 floating point comparison */
    asm volatile ("fldt %2\n\t"
                  "fldt %1\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "fstp %%st(0)\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(UNEQ), "m"(a), "m"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGE), "x"(a), "x"(b)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    asm volatile ("fldt %2\n\t"
                  "fldt %1\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "fstp %%st(0)\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGT), "m"(a), "m"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLE), "x"(a), "x"(b)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    asm volatile ("fldt %2\n\t"
                  "fldt %1\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "fstp %%st(0)\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLT), "m"(a), "m"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(LTGT), "x"(a), "x"(b)
                  : "cc");
    return result;
}

/* Function that uses switch to select condition code */
__attribute__((noinline))
static int dispatch_condition(int cc, double a, double b) {
    int result = 0;
    
    /* Switch to potentially trigger default case if cc is out of range */
    switch (cc) {
        case UNORDERED:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNORDERED), "x"(a), "x"(b)
                          : "cc");
            break;
        case ORDERED:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(ORDERED), "x"(a), "x"(b)
                          : "cc");
            break;
        case UNEQ:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNEQ), "x"(a), "x"(b)
                          : "cc");
            break;
        case UNGE:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNGE), "x"(a), "x"(b)
                          : "cc");
            break;
        case UNGT:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNGT), "x"(a), "x"(b)
                          : "cc");
            break;
        case UNLE:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNLE), "x"(a), "x"(b)
                          : "cc");
            break;
        case UNLT:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNLT), "x"(a), "x"(b)
                          : "cc");
            break;
        case LTGT:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(LTGT), "x"(a), "x"(b)
                          : "cc");
            break;
        default:
            /* This might trigger output_operand_lossage if cc is invalid */
            asm volatile ("# Invalid condition code %c0"
                          : 
                          : "u"(cc));
            break;
    }
    return result;
}

/* Mixed x87 and SSE operations */
__attribute__((noinline))
static int mixed_fp_operations(double d1, double d2, 
                               long double ld1, long double ld2) {
    int sum = 0;
    
    /* SSE comparisons */
    sum += test_unordered(d1, d2);
    sum += test_ordered(d1 + 1.0, d2 - 1.0);
    sum += test_unge(d1 * 2.0, d2 / 2.0);
    sum += test_unle(d1, d2);
    sum += test_ltgt(d1, d2);
    
    /* x87 comparisons */
    sum += test_uneq(ld1, ld2);
    sum += test_ungt(ld1 + 1.0L, ld2 - 1.0L);
    sum += test_unlt(ld1 * 2.0L, ld2 / 2.0L);
    
    return sum;
}

int main(int argc, char *argv[]) {
    volatile int accumulator = 0;
    int iterations = 100;
    
    /* Use command line argument to control iterations */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Array of test values */
    double d_vals[] = {0.0, 1.0, -1.0, 2.5, -2.5, 0.0/0.0, 1.0/0.0, -1.0/0.0};
    long double ld_vals[] = {0.0L, 1.0L, -1.0L, 3.14L, -3.14L};
    
    int d_count = sizeof(d_vals)/sizeof(d_vals[0]);
    int ld_count = sizeof(ld_vals)/sizeof(ld_vals[0]);
    
    printf("Testing x86 condition codes for %d iterations...\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        /* Use volatile globals mixed with array values */
        double d1 = g_d1 + d_vals[i % d_count];
        double d2 = g_d2 + d_vals[(i + 1) % d_count];
        long double ld1 = g_ld1 + ld_vals[i % ld_count];
        long double ld2 = g_ld2 + ld_vals[(i + 2) % ld_count];
        
        /* Test all condition code functions */
        accumulator += test_unordered(d1, d2);
        accumulator += test_ordered(d2, d1);
        accumulator += test_uneq(ld1, ld2);
        accumulator += test_unge(d1, d2);
        accumulator += test_ungt(ld2, ld1);
        accumulator += test_unle(d1, d2);
        accumulator += test_unlt(ld1, ld2);
        accumulator += test_ltgt(d2, d1);
        
        /* Mixed operations */
        accumulator += mixed_fp_operations(d1, d2, ld1, ld2);
        
        /* Dispatch with varying condition codes */
        int cc = g_selector + (i % 9);  /* 8 valid + 1 potentially invalid */
        accumulator += dispatch_condition(cc, d1, d2);
        
        /* Modify globals to change behavior */
        g_d1 += 0.1;
        g_d2 -= 0.1;
        g_selector ^= 1;
    }
    
    /* Also test with NaN and infinity */
    double nan_val = 0.0/0.0;
    double inf_val = 1.0/0.0;
    long double ld_nan = 0.0L/0.0L;
    long double ld_inf = 1.0L/0.0L;
    
    accumulator += test_unordered(nan_val, 1.0);
    accumulator += test_ordered(inf_val, 1.0);
    accumulator += test_uneq(ld_nan, ld_inf);
    accumulator += test_unge(nan_val, inf_val);
    accumulator += test_ungt(ld_inf, ld_nan);
    accumulator += test_unlt(ld_nan, 0.0L);
    
    printf("Final accumulator: %d\n", accumulator);
    
    /* Force assembly output of condition codes via inline asm with constraints */
    asm volatile ("# Condition code test complete with value %0"
                  : 
                  : "r"(accumulator));
    
    return accumulator != 0 ? 0 : 1;
}
