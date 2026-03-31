/* Condition code test program for i386.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent constant folding */
volatile double vd1 = 1.0, vd2 = 2.0, vd3 = NAN, vd4 = INFINITY;
volatile long double vld1 = 1.0L, vld2 = 2.0L, vld3 = NAN, vld4 = INFINITY;
volatile int vi = 0;

/* Enum matching the condition codes in i386.cc */
typedef enum {
    UNORDERED = 0,
    ORDERED,
    UNEQ,
    UNGE,
    UNGT,
    UNLE,
    UNLT,
    LTGT,
    MAX_COND
} x86_cond_code;

/* Function prototypes */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static void use_cond_in_asm(x86_cond_code cond, double a, double b) __attribute__((noinline));
static int dispatch_cond_code(int idx, double a, double b) __attribute__((noinline));

/* Test functions for each condition code using x87 instructions */
static int test_unordered(double a, double b) {
    int result;
    /* Use x87 fucomip with UNORDERED condition */
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
    /* Mix x87 and regular comparison */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(ORDERED)
        : "cc", "st"
    );
    return result;
}

static int test_uneq(double a, double b) {
    int result;
    /* Use UNEQ condition code */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNEQ)
        : "cc"
    );
    return result;
}

static int test_unge(double a, double b) {
    int result;
    /* Use UNGE condition code with long double */
    long double la = a, lb = b;
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(la), "m"(lb), "u"(UNGE)
        : "cc", "st"
    );
    return result;
}

static int test_ungt(double a, double b) {
    int result;
    /* Use UNGT condition code */
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
    /* Use UNLE condition code with mixed operations */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNLE)
        : "cc", "st"
    );
    return result;
}

static int test_unlt(double a, double b) {
    int result;
    /* Use UNLT condition code */
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
    /* Use LTGT condition code with long double */
    long double la = a, lb = b;
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(la), "m"(lb), "u"(LTGT)
        : "cc", "st"
    );
    return result;
}

/* Function that takes condition code as parameter and uses it in asm */
static void use_cond_in_asm(x86_cond_code cond, double a, double b) {
    int result1, result2;
    
    /* Use the condition code in x87 asm */
    asm volatile (
        "fldl %3\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result1)
        : "u"(cond), "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* Also use in SSE asm for variety */
    asm volatile (
        "comisd %3, %2\n\t"
        "set%c1 %1"
        : "=r"(result2)
        : "u"(cond), "x"(a), "x"(b)
        : "cc"
    );
    
    vi += result1 + result2;
}

/* Dispatch function with switch to potentially trigger default case */
static int dispatch_cond_code(int idx, double a, double b) {
    x86_cond_code cond;
    
    /* Use switch to map index to condition code */
    switch (idx & 0x7) {
        case 0: cond = UNORDERED; break;
        case 1: cond = ORDERED; break;
        case 2: cond = UNEQ; break;
        case 3: cond = UNGE; break;
        case 4: cond = UNGT; break;
        case 5: cond = UNLE; break;
        case 6: cond = UNLT; break;
        case 7: cond = LTGT; break;
        default: cond = UNORDERED; break; /* Should never hit with mask */
    }
    
    /* Use the condition code */
    use_cond_in_asm(cond, a, b);
    
    /* Also call individual test functions */
    switch (cond) {
        case UNORDERED: return test_unordered(a, b);
        case ORDERED: return test_ordered(a, b);
        case UNEQ: return test_uneq(a, b);
        case UNGE: return test_unge(a, b);
        case UNGT: return test_ungt(a, b);
        case UNLE: return test_unle(a, b);
        case UNLT: return test_unlt(a, b);
        case LTGT: return test_ltgt(a, b);
        default: 
            /* This might trigger output_operand_lossage if something goes wrong */
            asm volatile ("# Invalid condition code %c0" : : "u"(cond));
            return 0;
    }
}

int main(int argc, char *argv[]) {
    int i, j, iterations = 100;
    volatile int total = 0;
    
    /* Parse iteration count from command line */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create array of test values */
    double test_vals[] = {
        0.0, 1.0, -1.0, 2.5, -2.5, 
        NAN, INFINITY, -INFINITY,
        vd1, vd2, vd3, vd4
    };
    int num_vals = sizeof(test_vals) / sizeof(test_vals[0]);
    
    printf("Testing x86 condition codes for %d iterations...\n", iterations);
    
    /* Main test loop */
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < num_vals - 1; j++) {
            double a = test_vals[j];
            double b = test_vals[j + 1];
            
            /* Call all individual test functions */
            total += test_unordered(a, b);
            total += test_ordered(a, b);
            total += test_uneq(a, b);
            total += test_unge(a, b);
            total += test_ungt(a, b);
            total += test_unle(a, b);
            total += test_unlt(a, b);
            total += test_ltgt(a, b);
            
            /* Also use dispatch function */
            total += dispatch_cond_code((i + j) % 8, a, b);
            
            /* Mix with regular floating-point comparisons */
            if (a != b) total++;
            if (a >= b) total++;
            if (isunordered(a, b)) total++;
            if (islessgreater(a, b)) total++;
        }
        
        /* Occasionally use long double values */
        if (i % 10 == 0) {
            long double la = vld1 + i * 0.1;
            long double lb = vld2 - i * 0.1;
            
            /* Force x87 stack operations */
            asm volatile (
                "fldt %1\n\t"
                "fldt %0\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "setp %%al\n\t"
                "fstp %%st(0)"
                :
                : "m"(la), "m"(lb)
                : "cc", "st", "al"
            );
        }
    }
    
    /* Try to trigger edge case with out-of-range condition code */
    if (argc > 2 && strcmp(argv[2], "edge") == 0) {
        /* This might trigger the default case in printing */
        x86_cond_code bad_cond = (x86_cond_code)MAX_COND;
        asm volatile ("# Bad condition code %c0" : : "u"(bad_cond));
    }
    
    printf("Total: %d\n", total);
    return total != 0 ? 0 : 1;
}
