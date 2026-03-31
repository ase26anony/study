/* test_i386_condcodes.c - Target coverage for i386.cc lines 13992-14017 */
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

/* Function prototypes */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static int test_mixed_fpu(long double a, long double b) __attribute__((noinline));
static int test_switch_cond(int cc, double a, double b) __attribute__((noinline));
static void force_asm_output(void) __attribute__((noinline));

/* Test functions for each condition code */
static int test_unordered(double a, double b) {
    int result;
    /* Using x87 instruction with UNORDERED condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNORDERED)
        : "cc", "st"
    );
    return result;
}

static int test_ordered(double a, double b) {
    int result;
    /* Using SSE instruction with ORDERED condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    return result;
}

static int test_uneq(double a, double b) {
    int result;
    /* Mixed x87/SSE with UNEQ condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNEQ)
        : "cc", "st"
    );
    return result;
}

static int test_unge(double a, double b) {
    int result;
    /* Using UNGE condition (nlt) */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNGE)
        : "cc"
    );
    return result;
}

static int test_ungt(double a, double b) {
    int result;
    /* Using UNGT condition (nle) */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNGT)
        : "cc"
    );
    return result;
}

static int test_unle(double a, double b) {
    int result;
    /* Using UNLE condition (ule) */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNLE)
        : "cc"
    );
    return result;
}

static int test_unlt(double a, double b) {
    int result;
    /* Using UNLT condition (ult) */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNLT)
        : "cc"
    );
    return result;
}

static int test_ltgt(double a, double b) {
    int result;
    /* Using LTGT condition (une) */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(LTGT)
        : "cc"
    );
    return result;
}

/* Test with long double (x87) operations */
static int test_mixed_fpu(long double a, long double b) {
    int result1, result2;
    
    /* First comparison with UNORDERED */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result1)
        : "m"(a), "m"(b), "u"(UNORDERED)
        : "cc", "st"
    );
    
    /* Second comparison with ORDERED */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result2)
        : "m"(a), "m"(b), "u"(ORDERED)
        : "cc", "st"
    );
    
    return result1 | result2;
}

/* Function that uses a switch to select condition code */
static int test_switch_cond(int cc, double a, double b) {
    int result = 0;
    
    /* Volatile to prevent optimization */
    volatile int local_cc = cc;
    
    switch (local_cc & 0x7) {
        case UNORDERED:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNORDERED)
                : "cc"
            );
            break;
        case ORDERED:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(ORDERED)
                : "cc"
            );
            break;
        case UNEQ:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNEQ)
                : "cc"
            );
            break;
        case UNGE:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGE)
                : "cc"
            );
            break;
        case UNGT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGT)
                : "cc"
            );
            break;
        case UNLE:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLE)
                : "cc"
            );
            break;
        case UNLT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLT)
                : "cc"
            );
            break;
        case LTGT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(LTGT)
                : "cc"
            );
            break;
        default:
            /* This should trigger output_operand_lossage for invalid condition */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(8)  /* Invalid condition code */
                : "cc"
            );
            break;
    }
    
    return result;
}

/* Force assembly output through various means */
static void force_asm_output(void) {
    /* Create a format string that might trigger assembly output */
    const char* fmt = "Result: %d\n";
    
    /* Use builtin printf to potentially trigger RTL output */
    volatile int dummy = 0;
    __builtin_printf(fmt, dummy);
    
    /* Create a complex floating-point expression */
    volatile double x = g_d1;
    volatile double y = g_d2;
    double z = x * y - x / y + sqrt(x);
    
    /* Use the result in inline asm */
    asm volatile (
        "movsd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=m"(z)
        : "m"(z), "m"(x)
        : "xmm0"
    );
}

int main(int argc, char *argv[]) {
    int i, iterations = 100;
    volatile int total = 0;
    
    /* Parse iteration count from command line */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize arrays with volatile values */
    double darray[8];
    long double ldarray[8];
    
    for (i = 0; i < 8; i++) {
        darray[i] = g_d1 + i * 0.5;
        ldarray[i] = g_ld1 + i * 0.5L;
    }
    
    /* Main test loop */
    for (i = 0; i < iterations; i++) {
        int idx = i % 8;
        int idx2 = (i + 1) % 8;
        
        /* Call all test functions */
        total += test_unordered(darray[idx], darray[idx2]);
        total += test_ordered(darray[idx], darray[idx2]);
        total += test_uneq(darray[idx], darray[idx2]);
        total += test_unge(darray[idx], darray[idx2]);
        total += test_ungt(darray[idx], darray[idx2]);
        total += test_unle(darray[idx], darray[idx2]);
        total += test_unlt(darray[idx], darray[idx2]);
        total += test_ltgt(darray[idx], darray[idx2]);
        
        /* Test with long double */
        total += test_mixed_fpu(ldarray[idx], ldarray[idx2]);
        
        /* Test switch-based condition selection */
        total += test_switch_cond(i & 0x7, darray[idx], darray[idx2]);
        
        /* Occasionally test with invalid condition code */
        if ((i % 13) == 0) {
            total += test_switch_cond(8, darray[idx], darray[idx2]);  /* Invalid */
        }
        
        /* Mix with regular C comparisons */
        if (darray[idx] != darray[idx2]) total++;
        if (darray[idx] >= darray[idx2]) total++;
        
        /* Force some NaN comparisons */
        double nan_val = 0.0 / 0.0;
        if (darray[idx] == nan_val) total--;  /* Always false, but creates NaN usage */
        
        /* Update volatile selector */
        g_selector = i;
    }
    
    /* Force assembly output */
    force_asm_output();
    
    /* Print result to prevent dead code elimination */
    printf("Final total: %d\n", total);
    
    return 0;
}
