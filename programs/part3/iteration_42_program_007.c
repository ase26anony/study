/* test_condition_codes.c */
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

/* Force noinline to ensure separate functions */
#define NOINLINE __attribute__((noinline))

/* Test functions for each condition code */
NOINLINE static int test_unordered(double a, double b) {
    int result;
    /* Using x87 instruction with UNORDERED condition */
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

NOINLINE static int test_ordered(double a, double b) {
    int result;
    /* Using SSE instruction with ORDERED condition */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(ORDERED), "x"(a), "x"(b)
        : "cc"
    );
    return result;
}

NOINLINE static int test_uneq(long double a, long double b) {
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

NOINLINE static int test_unge(double a, double b) {
    int result;
    /* Mixed approach with UNGE */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNGE), "x"(a), "x"(b)
        : "cc"
    );
    return result;
}

NOINLINE static int test_ungt(long double a, long double b) {
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

NOINLINE static int test_unle(double a, double b) {
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

NOINLINE static int test_unlt(long double a, long double b) {
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

NOINLINE static int test_ltgt(double a, double b) {
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
NOINLINE static int test_conditional_switch(int cc, double a, double b) {
    int result = 0;
    
    /* Volatile to prevent optimization */
    volatile int local_cc = cc;
    
    switch (local_cc & 0x7) {
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
            /* This should trigger output_operand_lossage if cc is invalid */
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

/* Helper to generate NaN values */
NOINLINE static double make_nan(void) {
    volatile double zero = 0.0;
    return zero / zero;
}

NOINLINE static long double make_nanl(void) {
    volatile long double zero = 0.0L;
    return zero / zero;
}

int main(int argc, char *argv[]) {
    int i, iterations;
    volatile int accumulator = 0;
    
    /* Parse iterations from command line */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    /* Create test values including NaN, infinity, normal numbers */
    double dvals[8];
    long double ldvals[8];
    
    dvals[0] = g_d1;
    dvals[1] = g_d2;
    dvals[2] = make_nan();
    dvals[3] = INFINITY;
    dvals[4] = -INFINITY;
    dvals[5] = 0.0;
    dvals[6] = -1.0;
    dvals[7] = 3.14159;
    
    ldvals[0] = g_ld1;
    ldvals[1] = g_ld2;
    ldvals[2] = make_nanl();
    ldvals[3] = INFINITY;
    ldvals[4] = -INFINITY;
    ldvals[5] = 0.0L;
    ldvals[6] = -1.0L;
    ldvals[7] = 3.14159265358979323846L;
    
    /* Main test loop */
    for (i = 0; i < iterations; i++) {
        int idx = i & 7;
        int cc_idx = i & 7;
        
        /* Test all condition code functions */
        accumulator += test_unordered(dvals[idx], dvals[(idx + 1) & 7]);
        accumulator += test_ordered(dvals[(idx + 2) & 7], dvals[(idx + 3) & 7]);
        accumulator += test_uneq(ldvals[idx], ldvals[(idx + 1) & 7]);
        accumulator += test_unge(dvals[(idx + 2) & 7], dvals[(idx + 3) & 7]);
        accumulator += test_ungt(ldvals[(idx + 4) & 7], ldvals[(idx + 5) & 7]);
        accumulator += test_unle(dvals[(idx + 6) & 7], dvals[(idx + 7) & 7]);
        accumulator += test_unlt(ldvals[idx], ldvals[(idx + 2) & 7]);
        accumulator += test_ltgt(dvals[(idx + 4) & 7], dvals[(idx + 6) & 7]);
        
        /* Test switch-based function with valid condition codes */
        accumulator += test_conditional_switch(cc_idx, dvals[idx], dvals[(idx + 1) & 7]);
        
        /* Occasionally test with potentially invalid condition code */
        if ((i % 13) == 0) {
            volatile int invalid_cc = 8 + (i & 7);  /* Values 8-15 are invalid */
            accumulator += test_conditional_switch(invalid_cc, dvals[0], dvals[1]);
        }
        
        /* Mix with regular C comparisons */
        if (dvals[idx] != dvals[(idx + 1) & 7]) accumulator ^= 1;
        if (ldvals[idx] >= ldvals[(idx + 2) & 7]) accumulator ^= 2;
        
        /* Update volatile selector */
        g_selector = (g_selector + 1) & 7;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulator result: %d\n", accumulator);
    
    /* Additional test with inline assembly that directly outputs condition codes */
    {
        int r1, r2, r3, r4, r5, r6, r7, r8;
        
        /* Generate assembly that should print condition code names */
        asm volatile (
            "# Condition code test block\n\t"
            "comisd %1, %2\n\t"
            "set%c0 %0\n\t"
            : "=r"(r1) : "u"(UNORDERED), "x"(dvals[0]), "x"(dvals[1]) : "cc");
        
        asm volatile (
            "comisd %1, %2\n\t"
            "set%c0 %0\n\t"
            : "=r"(r2) : "u"(ORDERED), "x"(dvals[2]), "x"(dvals[3]) : "cc");
        
        asm volatile (
            "comisd %1, %2\n\t"
            "set%c0 %0\n\t"
            : "=r"(r3) : "u"(UNEQ), "x"(dvals[4]), "x"(dvals[5]) : "cc");
        
        asm volatile (
            "comisd %1, %2\n\t"
            "set%c0 %0\n\t"
            : "=r"(r4) : "u"(UNGE), "x"(dvals[6]), "x"(dvals[7]) : "cc");
        
        asm volatile (
            "fldl %1\n\t"
            "fldl %2\n\t"
            "fucomip %%st(1), %%st(0)\n\t"
            "set%c0 %0\n\t"
            "fstp %%st(0)"
            : "=r"(r5) : "u"(UNGT), "m"(dvals[0]), "m"(dvals[2]) : "cc", "st");
        
        asm volatile (
            "fldl %1\n\t"
            "fldl %2\n\t"
            "fucomip %%st(1), %%st(0)\n\t"
            "set%c0 %0\n\t"
            "fstp %%st(0)"
            : "=r"(r6) : "u"(UNLE), "m"(dvals[1]), "m"(dvals[3]) : "cc", "st");
        
        asm volatile (
            "fldl %1\n\t"
            "fldl %2\n\t"
            "fucomip %%st(1), %%st(0)\n\t"
            "set%c0 %0\n\t"
            "fstp %%st(0)"
            : "=r"(r7) : "u"(UNLT), "m"(dvals[4]), "m"(dvals[5]) : "cc", "st");
        
        asm volatile (
            "comisd %1, %2\n\t"
            "set%c0 %0\n\t"
            : "=r"(r8) : "u"(LTGT), "x"(dvals[6]), "x"(dvals[7]) : "cc");
        
        printf("Condition code results: %d %d %d %d %d %d %d %d\n", 
               r1, r2, r3, r4, r5, r6, r7, r8);
    }
    
    return 0;
}
