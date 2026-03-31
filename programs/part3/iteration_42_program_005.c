/* test_i386_condcodes.c - Target coverage for i386.cc condition code printing */
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
enum cc_enum {
    UNORDERED = 0,
    ORDERED = 1,
    UNEQ = 2,
    UNGE = 3,
    UNGT = 4,
    UNLE = 5,
    UNLT = 6,
    LTGT = 7
};

/* Function 1: Test UNORDERED condition */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* Using x87 instruction with UNORDERED condition code */
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

/* Function 2: Test ORDERED condition */
__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
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

/* Function 3: Test UNEQ condition */
__attribute__((noinline))
static int test_uneq(double a, double b) {
    int result;
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

/* Function 4: Test UNGE condition (nlt) */
__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNGE)
        : "cc", "st"
    );
    return result;
}

/* Function 5: Test UNGT condition (nle) */
__attribute__((noinline))
static int test_ungt(double a, double b) {
    int result;
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

/* Function 6: Test UNLE condition */
__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
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

/* Function 7: Test UNLT condition */
__attribute__((noinline))
static int test_unlt(double a, double b) {
    int result;
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

/* Function 8: Test LTGT condition (une) */
__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(LTGT)
        : "cc", "st"
    );
    return result;
}

/* Function 9: Mixed SSE and x87 operations */
__attribute__((noinline))
static int test_mixed_operations(double a, double b, long double c, long double d) {
    int result1, result2;
    
    /* SSE comparison for double */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result1)
        : "x"(a), "x"(b), "u"(UNORDERED)
        : "cc"
    );
    
    /* x87 comparison for long double */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result2)
        : "m"(c), "m"(d), "u"(ORDERED)
        : "cc", "st"
    );
    
    return result1 + result2;
}

/* Function 10: Dynamic condition code selection - may trigger default case */
__attribute__((noinline))
static int test_dynamic_cc(double a, double b, int cc_code) {
    int result;
    
    /* Switch to create control flow that might confuse the compiler */
    switch (cc_code & 0x7) {
        case UNORDERED:
        case ORDERED:
        case UNEQ:
        case UNGE:
        case UNGT:
        case UNLE:
        case UNLT:
        case LTGT:
            asm volatile (
                "fldl %2\n\t"
                "fldl %1\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(cc_code & 0x7)
                : "cc", "st"
            );
            break;
        default:
            /* This might trigger output_operand_lossage if cc_code is invalid */
            result = -1;
            break;
    }
    
    return result;
}

/* Function 11: Complex floating-point operations with condition codes */
__attribute__((noinline))
static int test_complex_fp(double a, double b) {
    double temp;
    int result = 0;
    
    /* Create NaN to trigger unordered comparisons */
    temp = (a != a) ? NAN : b;
    
    /* Test multiple condition codes in sequence */
    result |= test_unordered(a, temp);
    result |= test_ordered(temp, b);
    result |= test_uneq(a, b);
    
    /* Mix with regular C comparisons */
    if (!(a >= b)) {
        result |= test_unge(a, b);
    }
    
    if (a != b) {
        result |= test_ltgt(a, b);
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int accumulator = 0;
    int loop_count = 100;
    int i;
    
    /* Parse loop count from command line */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    /* Initialize arrays with volatile values */
    double d_array[8];
    long double ld_array[8];
    
    for (i = 0; i < 8; i++) {
        d_array[i] = g_d1 + i * 0.5;
        ld_array[i] = g_ld1 + i * 0.5L;
    }
    
    /* Main test loop */
    for (i = 0; i < loop_count; i++) {
        int idx = i % 8;
        int idx2 = (i + 1) % 8;
        
        /* Call all condition code test functions */
        accumulator += test_unordered(d_array[idx], d_array[idx2]);
        accumulator += test_ordered(d_array[idx2], d_array[idx]);
        accumulator += test_uneq(d_array[idx], d_array[idx2]);
        accumulator += test_unge(d_array[idx], d_array[idx2]);
        accumulator += test_ungt(d_array[idx], d_array[idx2]);
        accumulator += test_unle(d_array[idx], d_array[idx2]);
        accumulator += test_unlt(d_array[idx], d_array[idx2]);
        accumulator += test_ltgt(d_array[idx], d_array[idx2]);
        
        /* Test mixed operations */
        accumulator += test_mixed_operations(
            d_array[idx], d_array[idx2],
            ld_array[idx], ld_array[idx2]
        );
        
        /* Test complex FP operations */
        accumulator += test_complex_fp(d_array[idx], d_array[idx2]);
        
        /* Test dynamic condition code selection */
        g_selector = (g_selector + 1) & 0xF; /* May produce values > 7 */
        accumulator += test_dynamic_cc(
            d_array[idx], 
            d_array[idx2], 
            g_selector
        );
        
        /* Create NaN values to trigger unordered comparisons */
        if (i % 3 == 0) {
            double nan_val = NAN;
            accumulator += test_unordered(d_array[idx], nan_val);
            accumulator += test_ordered(nan_val, d_array[idx2]);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulated result: %d\n", accumulator);
    
    /* Additional test: try to trigger the default case directly */
    {
        int invalid_cc = 255; /* Invalid condition code */
        int dummy_result;
        
        /* This might trigger output_operand_lossage */
        asm volatile (
            "# Attempt with invalid condition code\n\t"
            "fldl %1\n\t"
            "fldl %2\n\t"
            "fucomip %%st(1), %%st(0)\n\t"
            "set%c0 %0\n\t"
            "fstp %%st(0)"
            : "=r"(dummy_result)
            : "m"(g_d1), "m"(g_d2), "u"(invalid_cc)
            : "cc", "st"
        );
        
        printf("Dummy result with invalid CC: %d\n", dummy_result);
    }
    
    return accumulator != 0 ? 0 : 1;
}
