/* test_i386_condcodes.c - Test x86 floating-point condition code printing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double g_d1 = 1.0;
volatile double g_d2 = 2.0;
volatile long double g_ld1 = 3.0L;
volatile long double g_ld2 = 4.0L;
volatile int g_selector = 0;

/* Function prototypes */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static int test_condition(int cond, double a, double b) __attribute__((noinline));

/* Test UNORDERED condition code */
static int test_unordered(double a, double b)
{
    int result;
    /* Use x87 instruction with UNORDERED condition */
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

/* Test ORDERED condition code */
static int test_ordered(double a, double b)
{
    int result;
    /* Use SSE instruction with ORDERED condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    return result;
}

/* Test UNEQ condition code */
static int test_uneq(double a, double b)
{
    int result;
    /* Mix x87 and regular comparison */
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

/* Test UNGE condition code */
static int test_unge(double a, double b)
{
    int result;
    /* Use SSE with UNGE (nlt) */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNGE)
        : "cc"
    );
    return result;
}

/* Test UNGT condition code */
static int test_ungt(double a, double b)
{
    int result;
    /* Use x87 with UNGT (nle) */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNGT)
        : "cc", "st"
    );
    return result;
}

/* Test UNLE condition code */
static int test_unle(double a, double b)
{
    int result;
    /* Use SSE with UNLE (ule) */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNLE)
        : "cc"
    );
    return result;
}

/* Test UNLT condition code */
static int test_unlt(double a, double b)
{
    int result;
    /* Use x87 with UNLT (ult) */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNLT)
        : "cc", "st"
    );
    return result;
}

/* Test LTGT condition code */
static int test_ltgt(double a, double b)
{
    int result;
    /* Use SSE with LTGT (une) */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(LTGT)
        : "cc"
    );
    return result;
}

/* Test long double with various condition codes */
static int test_longdouble_cond(int cond, long double a, long double b)
{
    int result;
    /* Force x87 usage with long double */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c3 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(cond)
        : "cc", "st"
    );
    return result;
}

/* Generic condition tester - may trigger default case with invalid input */
static int test_condition(int cond, double a, double b)
{
    int result = 0;
    
    /* Switch to potentially create RTL patterns that need condition code printing */
    switch (cond) {
        case UNORDERED:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c3 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNORDERED)
                : "cc"
            );
            break;
        case ORDERED:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c3 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(ORDERED)
                : "cc"
            );
            break;
        case UNEQ:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c3 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNEQ)
                : "cc"
            );
            break;
        case UNGE:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c3 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGE)
                : "cc"
            );
            break;
        case UNGT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c3 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGT)
                : "cc"
            );
            break;
        case UNLE:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c3 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLE)
                : "cc"
            );
            break;
        case UNLT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c3 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLT)
                : "cc"
            );
            break;
        case LTGT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c3 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(LTGT)
                : "cc"
            );
            break;
        default:
            /* This might trigger output_operand_lossage for invalid condition */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c3 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cond)  /* Potentially invalid condition */
                : "cc"
            );
            break;
    }
    return result;
}

/* Create NaN values for testing unordered comparisons */
static double make_nan()
{
    union {
        uint64_t i;
        double d;
    } u;
    u.i = 0x7FF8000000000001ULL; /* Quiet NaN */
    return u.d;
}

int main(int argc, char *argv[])
{
    volatile int sum = 0;
    int i, iterations;
    
    /* Parse iterations from command line or use default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    /* Create test values including NaN */
    double dvals[8];
    long double ldvals[8];
    double nan_val = make_nan();
    
    for (i = 0; i < 8; i++) {
        dvals[i] = (i * 1.5) - 3.0;
        ldvals[i] = (i * 2.0L) - 4.0L;
    }
    dvals[3] = nan_val;  /* Insert NaN */
    
    /* Main test loop */
    for (i = 0; i < iterations; i++) {
        int idx1 = i % 8;
        int idx2 = (i + 1) % 8;
        int cond_select = i % 10;  /* 0-9, where 9 might be invalid */
        
        /* Test all specific condition code functions */
        sum += test_unordered(dvals[idx1], dvals[idx2]);
        sum += test_ordered(dvals[idx1], dvals[idx2]);
        sum += test_uneq(dvals[idx1], dvals[idx2]);
        sum += test_unge(dvals[idx1], dvals[idx2]);
        sum += test_ungt(dvals[idx1], dvals[idx2]);
        sum += test_unle(dvals[idx1], dvals[idx2]);
        sum += test_unlt(dvals[idx1], dvals[idx2]);
        sum += test_ltgt(dvals[idx1], dvals[idx2]);
        
        /* Test with long doubles (x87) */
        sum += test_longdouble_cond(UNORDERED, ldvals[idx1], ldvals[idx2]);
        sum += test_longdouble_cond(ORDERED, ldvals[idx1], ldvals[idx2]);
        sum += test_longdouble_cond(UNEQ, ldvals[idx1], ldvals[idx2]);
        
        /* Test generic condition function - may trigger default case */
        sum += test_condition(cond_select, dvals[idx1], dvals[idx2]);
        
        /* Mix with regular C comparisons to provide context */
        if (dvals[idx1] != dvals[idx2]) {
            sum += test_unordered(dvals[idx2], dvals[idx1]);
        }
        if (dvals[idx1] >= dvals[idx2]) {
            sum += test_unge(dvals[idx2], dvals[idx1]);
        }
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_sum = sum;
    
    printf("Result: %d\n", final_sum);
    
    /* Additional test that might trigger the default case directly */
    if (argc > 2 && strcmp(argv[2], "test-default") == 0) {
        /* Try to pass an invalid condition code */
        int invalid_cond = 255;  /* Clearly invalid */
        asm volatile (
            "comisd %1, %0\n\t"
            "set%c2 %0"
            : "+x"(g_d1)
            : "x"(g_d2), "u"(invalid_cond)
            : "cc"
        );
    }
    
    return 0;
}
