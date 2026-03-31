/* i386_condition_codes.c - Target coverage for x86 condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile globals to prevent constant folding */
volatile double g_dbl1 = 3.14159;
volatile double g_dbl2 = 2.71828;
volatile long double g_ldbl1 = 1.41421356L;
volatile long double g_ldbl2 = 1.61803399L;
volatile int g_cond_selector = 0;

/* Function prototypes for condition code tests */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static int test_mixed_cond(int cond, double a, double b) __attribute__((noinline));

/* Helper to force condition code usage in output */
static void output_cond_code(int cond_code) __attribute__((noinline));

/* Test UNORDERED condition */
static int test_unordered(double a, double b) {
    int result;
    /* Use x87 instruction with UNORDERED condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b), "u" (UNORDERED)
        : "cc", "st"
    );
    return result;
}

/* Test ORDERED condition */
static int test_ordered(double a, double b) {
    int result;
    /* Mix x87 and regular comparison */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r" (result)
        : "x" (a), "x" (b), "u" (ORDERED)
        : "cc"
    );
    return result;
}

/* Test UNEQ condition */
static int test_uneq(double a, double b) {
    int result;
    /* Use long double for x87 */
    long double la = (long double)a;
    long double lb = (long double)b;
    
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (la), "m" (lb), "u" (UNEQ)
        : "cc", "st"
    );
    return result;
}

/* Test UNGE condition */
static int test_unge(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r" (result)
        : "x" (a), "x" (b), "u" (UNGE)
        : "cc"
    );
    return result;
}

/* Test UNGT condition */
static int test_ungt(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b), "u" (UNGT)
        : "cc", "st"
    );
    return result;
}

/* Test UNLE condition */
static int test_unle(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r" (result)
        : "x" (a), "x" (b), "u" (UNLE)
        : "cc"
    );
    return result;
}

/* Test UNLT condition */
static int test_unlt(double a, double b) {
    int result;
    long double la = (long double)a;
    long double lb = (long double)b;
    
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (la), "m" (lb), "u" (UNLT)
        : "cc", "st"
    );
    return result;
}

/* Test LTGT condition */
static int test_ltgt(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r" (result)
        : "x" (a), "x" (b), "u" (LTGT)
        : "cc"
    );
    return result;
}

/* Mixed condition test - uses switch to potentially trigger default case */
static int test_mixed_cond(int cond, double a, double b) {
    int result = 0;
    
    /* Volatile to prevent optimization */
    volatile int vcond = cond;
    
    switch (vcond & 7) {
        case 0:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNORDERED)
                : "cc"
            );
            break;
        case 1:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (ORDERED)
                : "cc"
            );
            break;
        case 2:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNEQ)
                : "cc"
            );
            break;
        case 3:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNGE)
                : "cc"
            );
            break;
        case 4:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNGT)
                : "cc"
            );
            break;
        case 5:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNLE)
                : "cc"
            );
            break;
        case 6:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNLT)
                : "cc"
            );
            break;
        case 7:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (LTGT)
                : "cc"
            );
            break;
        default:
            /* This might trigger output_operand_lossage if compiler
               doesn't recognize the condition code */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (vcond)  /* Potentially invalid */
                : "cc"
            );
            break;
    }
    
    return result;
}

/* Function that forces condition code printing */
static void output_cond_code(int cond_code) {
    /* This function creates a scenario where the compiler might need
       to output the condition code as a string */
    const char* names[] = {
        "UNORDERED", "ORDERED", "UNEQ", "UNGE",
        "UNGT", "UNLE", "UNLT", "LTGT"
    };
    
    /* Use inline asm with condition code operand */
    int dummy;
    if (cond_code >= 0 && cond_code < 8) {
        asm volatile (
            "mov $0, %0\n\t"
            "test %0, %0\n\t"
            "set%c1 %0"
            : "=r" (dummy)
            : "u" (cond_code)
            : "cc"
        );
    }
    
    /* Print to prevent elimination */
    printf("Condition code %d: %s\n", cond_code, 
           (cond_code >= 0 && cond_code < 8) ? names[cond_code] : "UNKNOWN");
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    volatile int accumulator = 0;
    
    /* Parse loop count from command line if provided */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    /* Create array of test values */
    double test_values[] = {
        0.0, 1.0, -1.0, 2.0, 0.5,
        g_dbl1, g_dbl2, 1.0/g_dbl1, -g_dbl2
    };
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    printf("Testing x86 condition codes with %d iterations\n", loop_count);
    
    for (int i = 0; i < loop_count; i++) {
        /* Use volatile index to prevent optimization */
        volatile int idx = i % num_values;
        volatile int idx2 = (i * 7) % num_values;
        
        double a = test_values[idx];
        double b = test_values[idx2];
        
        /* Test all condition codes */
        accumulator += test_unordered(a, b);
        accumulator += test_ordered(a, b);
        accumulator += test_uneq(a, b);
        accumulator += test_unge(a, b);
        accumulator += test_ungt(a, b);
        accumulator += test_unle(a, b);
        accumulator += test_unlt(a, b);
        accumulator += test_ltgt(a, b);
        
        /* Test mixed condition with potentially invalid code */
        int cond = (i * 13) % 12;  /* Range 0-11, includes invalid codes */
        accumulator += test_mixed_cond(cond, a, b);
        
        /* Force condition code output */
        if (i % 23 == 0) {
            output_cond_code(cond % 10);  /* Includes invalid codes */
        }
        
        /* Mix with regular floating-point comparisons */
        if (a != b) accumulator++;
        if (a >= b) accumulator++;
        if (!(a < b)) accumulator++;
        
        /* Use NaN to trigger unordered comparisons */
        double nan_val = 0.0 / 0.0;
        if (a == a) {  /* Check for NaN */
            accumulator += test_unordered(a, nan_val);
            accumulator += test_ordered(nan_val, b);
        }
    }
    
    /* Also test with long doubles */
    for (int i = 0; i < loop_count / 10; i++) {
        long double la = g_ldbl1 + i * 0.1L;
        long double lb = g_ldbl2 - i * 0.05L;
        
        /* Convert to double for the test functions */
        accumulator += test_unordered((double)la, (double)lb);
        accumulator += test_uneq((double)la, (double)lb);
    }
    
    printf("Final accumulator: %d\n", accumulator);
    
    /* Try to trigger the default case with an invalid condition code */
    if (argc > 2 && strcmp(argv[2], "test-invalid") == 0) {
        printf("\nAttempting to trigger default case with invalid condition code...\n");
        
        /* This might cause output_operand_lossage */
        int invalid_result;
        volatile int invalid_cond = 255;  /* Clearly invalid */
        
        asm volatile (
            "mov $0, %0\n\t"
            "test %0, %0\n\t"
            "set%c1 %0"
            : "=r" (invalid_result)
            : "u" (invalid_cond)  /* Potentially triggers default case */
            : "cc"
        );
        
        printf("Invalid condition test result: %d\n", invalid_result);
    }
    
    return accumulator != 0 ? 0 : 1;
}
