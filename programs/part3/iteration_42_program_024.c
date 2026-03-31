/* test_i386_condcodes.c - Target x86 condition code printing in i386.cc */
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

/* Function prototypes for condition code tests */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static int test_mixed_fp(long double a, double b) __attribute__((noinline));
static void use_condcode_in_switch(int cc) __attribute__((noinline));

/* Test UNORDERED condition code */
static int test_unordered(double a, double b) {
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

/* Test ORDERED condition code */
static int test_ordered(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(ORDERED), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

/* Test UNEQ condition code */
static int test_uneq(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNEQ), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

/* Test UNGE condition code */
static int test_unge(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNGE), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

/* Test UNGT condition code */
static int test_ungt(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNGT), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

/* Test UNLE condition code */
static int test_unle(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNLE), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

/* Test UNLT condition code */
static int test_unlt(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNLT), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

/* Test LTGT condition code */
static int test_ltgt(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(LTGT), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

/* Mixed x87 and SSE floating-point operations */
static int test_mixed_fp(long double a, double b) {
    int result1, result2;
    
    /* x87 operation with long double */
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result1)
        : "u"(UNORDERED), "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* SSE operation with double */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0"
        : "=r"(result2)
        : "u"(ORDERED), "x"(a), "x"(b)
        : "cc"
    );
    
    return result1 | result2;
}

/* Function that uses a switch to select condition codes */
/* This may help trigger the default case in the uncovered code */
static void use_condcode_in_switch(int cc) {
    int result = 0;
    double a = g_d1, b = g_d2;
    
    switch (cc) {
        case 0:  /* UNORDERED */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNORDERED), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 1:  /* ORDERED */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(ORDERED), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 2:  /* UNEQ */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNEQ), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 3:  /* UNGE */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNGE), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 4:  /* UNGT */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNGT), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 5:  /* UNLE */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNLE), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 6:  /* UNLT */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNLT), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 7:  /* LTGT */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(LTGT), "x"(a), "x"(b)
                : "cc"
            );
            break;
        default:
            /* This might trigger the default case in i386.cc output_operand_lossage */
            /* Using an invalid condition code value */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(cc), "x"(a), "x"(b)  /* cc might be out of valid range */
                : "cc"
            );
            break;
    }
    
    /* Use result to prevent dead code elimination */
    g_selector = result;
}

int main(int argc, char *argv[]) {
    volatile int sum = 0;
    int i, iterations;
    
    /* Parse iterations from command line or use default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    /* Initialize array of test values */
    double d_vals[8];
    long double ld_vals[8];
    
    for (i = 0; i < 8; i++) {
        d_vals[i] = (double)(i * 1.5);
        ld_vals[i] = (long double)(i * 2.5L);
    }
    
    /* Create some NaN values for unordered comparisons */
    double nan_val = 0.0 / 0.0;
    long double nan_ld = 0.0L / 0.0L;
    
    printf("Testing x86 condition code printing (targeting i386.cc lines 13992-14017)\n");
    
    /* Main test loop with varied inputs */
    for (i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Test all condition codes with different value pairs */
        sum += test_unordered(d_vals[idx], nan_val);
        sum += test_ordered(d_vals[idx], d_vals[(idx + 1) % 8]);
        sum += test_uneq(d_vals[idx], d_vals[(idx + 2) % 8]);
        sum += test_unge(d_vals[idx], d_vals[(idx + 3) % 8]);
        sum += test_ungt(d_vals[idx], d_vals[(idx + 4) % 8]);
        sum += test_unle(d_vals[idx], d_vals[(idx + 5) % 8]);
        sum += test_unlt(d_vals[idx], d_vals[(idx + 6) % 8]);
        sum += test_ltgt(d_vals[idx], d_vals[(idx + 7) % 8]);
        
        /* Test mixed floating-point types */
        sum += test_mixed_fp(ld_vals[idx], d_vals[idx]);
        
        /* Test with long double NaN */
        sum += test_unordered((double)ld_vals[idx], (double)nan_ld);
        
        /* Use switch-based condition code selection */
        /* Vary the selector to potentially hit different cases */
        g_selector = (i * 17) % 12;  /* 12 > 8 to potentially hit default */
        use_condcode_in_switch(g_selector);
        
        /* Mix with regular C comparisons to provide context */
        if (d_vals[idx] != d_vals[(idx + 1) % 8]) {
            sum += test_ordered(d_vals[idx], d_vals[(idx + 1) % 8]);
        }
        
        if (d_vals[idx] >= d_vals[(idx + 2) % 8]) {
            sum += test_unge(d_vals[idx], d_vals[(idx + 2) % 8]);
        }
    }
    
    printf("Accumulated sum: %d\n", sum);
    
    /* Additional test: try to trigger the default case directly */
    /* by using what might be an invalid condition code */
    {
        int invalid_cc = 99;  /* Clearly invalid condition code */
        int dummy_result;
        double a = 1.0, b = 2.0;
        
        printf("Attempting to trigger default case with invalid condition code...\n");
        
        /* This might cause output_operand_lossage in i386.cc */
        asm volatile (
            "# Potential invalid condition code usage\n"
            "comisd %2, %3\n\t"
            : "=@ccc"(dummy_result)
            : "u"(invalid_cc), "x"(a), "x"(b)
            : "cc"
        );
    }
    
    return sum == 0 ? 0 : 1;
}
