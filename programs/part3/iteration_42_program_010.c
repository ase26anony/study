/* Condition code test program for i386.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent constant folding */
volatile int g_volatile_index = 0;
volatile double g_volatile_double = 3.14159;
volatile long double g_volatile_ldouble = 2.71828L;

/* Condition code enumeration matching i386.cc */
enum fp_condition_codes {
    UNORDERED = 0,
    ORDERED = 1,
    UNEQ = 2,
    UNGE = 3,
    UNGT = 4,
    UNLE = 5,
    UNLT = 6,
    LTGT = 7,
    MAX_CONDITIONS = 8
};

/* Function prototypes */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static int test_mixed_conditions(long double a, long double b, int cond) __attribute__((noinline));
static void print_condition_code(int code) __attribute__((noinline));

/* Individual condition code test functions */
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
    /* Using UNGE condition code */
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
    /* Using UNGT condition code with x87 */
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

static int test_unle(double a, double b) {
    int result;
    /* Using UNLE condition code */
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
    /* Using UNLT condition code with x87 */
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

static int test_ltgt(double a, double b) {
    int result;
    /* Using LTGT condition code */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(LTGT)
        : "cc"
    );
    return result;
}

/* Function that uses long double (x87) with dynamic condition code */
static int test_mixed_conditions(long double a, long double b, int cond) {
    int result;
    
    /* Force compiler to handle condition code as parameter */
    switch (cond & 0x7) {
        case UNORDERED:
            asm volatile (
                "fldt %2\n\t"
                "fldt %1\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c3 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNORDERED)
                : "cc", "st"
            );
            break;
        case ORDERED:
            asm volatile (
                "fldt %2\n\t"
                "fldt %1\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c3 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(ORDERED)
                : "cc", "st"
            );
            break;
        case UNEQ:
            asm volatile (
                "fldt %2\n\t"
                "fldt %1\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c3 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNEQ)
                : "cc", "st"
            );
            break;
        default:
            /* This might trigger the default case in output_operand_lossage
               if cond is out of expected range */
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
    }
    return result;
}

/* Function that could trigger condition code printing */
static void print_condition_code(int code) {
    /* This function's assembly might cause the compiler to output
       condition code strings during RTL generation */
    const char* names[] = {
        "unord", "ord", "ueq", "nlt", "nle", "ule", "ult", "une"
    };
    
    if (code >= 0 && code < MAX_CONDITIONS) {
        /* Use inline asm with condition code to force printing */
        int dummy;
        asm volatile (
            "mov $1, %0\n\t"
            "test %0, %0\n\t"
            "set%c1 %0"
            : "=r"(dummy)
            : "u"(code)
            : "cc"
        );
    } else {
        /* Potentially trigger default case with invalid code */
        int dummy;
        asm volatile (
            "mov $1, %0\n\t"
            "test %0, %0\n\t"
            "set%c1 %0"
            : "=r"(dummy)
            : "u"(code)
            : "cc"
        );
    }
}

int main(int argc, char *argv[]) {
    int i, j;
    volatile int accumulator = 0;
    int loop_count = 100;
    
    /* Parse loop count from command line */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    /* Create arrays with special floating-point values */
    double dvalues[] = {
        0.0, 1.0, -1.0, INFINITY, -INFINITY, NAN,
        g_volatile_double, 1.0/g_volatile_double
    };
    int dcount = sizeof(dvalues)/sizeof(dvalues[0]);
    
    long double ldvalues[] = {
        0.0L, 1.0L, -1.0L, INFINITY, -INFINITY, NAN,
        g_volatile_ldouble, 1.0L/g_volatile_ldouble
    };
    int ldcount = sizeof(ldvalues)/sizeof(ldvalues[0]);
    
    /* Main test loop */
    for (i = 0; i < loop_count; i++) {
        /* Update volatile index to prevent optimization */
        g_volatile_index = i % MAX_CONDITIONS;
        
        /* Test all condition code functions */
        for (j = 0; j < dcount - 1; j++) {
            accumulator += test_unordered(dvalues[j], dvalues[j+1]);
            accumulator += test_ordered(dvalues[j], dvalues[j+1]);
            accumulator += test_uneq(dvalues[j], dvalues[j+1]);
            accumulator += test_unge(dvalues[j], dvalues[j+1]);
            accumulator += test_ungt(dvalues[j], dvalues[j+1]);
            accumulator += test_unle(dvalues[j], dvalues[j+1]);
            accumulator += test_unlt(dvalues[j], dvalues[j+1]);
            accumulator += test_ltgt(dvalues[j], dvalues[j+1]);
        }
        
        /* Test mixed conditions with long double */
        for (j = 0; j < ldcount - 1; j++) {
            accumulator += test_mixed_conditions(ldvalues[j], ldvalues[j+1], 
                                                g_volatile_index);
            
            /* Also test with potentially invalid condition codes */
            accumulator += test_mixed_conditions(ldvalues[j], ldvalues[j+1],
                                                g_volatile_index + MAX_CONDITIONS);
        }
        
        /* Call print function with various codes */
        print_condition_code(g_volatile_index);
        print_condition_code(g_volatile_index + 1);  /* Might be out of range */
        print_condition_code(g_volatile_index - 1);  /* Might be negative */
    }
    
    /* Use switch with volatile to force condition code handling */
    switch (g_volatile_index) {
        case 0:
            accumulator += test_unordered(g_volatile_double, g_volatile_double + 1.0);
            break;
        case 1:
            accumulator += test_ordered(g_volatile_double, g_volatile_double - 1.0);
            break;
        case 2:
            accumulator += test_uneq(g_volatile_double, g_volatile_double * 2.0);
            break;
        case 3:
            accumulator += test_unge(g_volatile_double, g_volatile_double / 2.0);
            break;
        case 4:
            accumulator += test_ungt(g_volatile_double, -g_volatile_double);
            break;
        case 5:
            accumulator += test_unle(g_volatile_double, g_volatile_double);
            break;
        case 6:
            accumulator += test_unlt(g_volatile_double, 0.0);
            break;
        case 7:
            accumulator += test_ltgt(g_volatile_double, 1.0);
            break;
        default:
            /* This should trigger the default case in i386.cc if we
               somehow get an invalid condition code to print */
            {
                int invalid_code = g_volatile_index;
                int dummy;
                asm volatile (
                    "mov $1, %0\n\t"
                    "test %0, %0\n\t"
                    "set%c1 %0"
                    : "=r"(dummy)
                    : "u"(invalid_code)
                    : "cc"
                );
                accumulator += dummy;
            }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulator result: %d\n", accumulator);
    
    return 0;
}
