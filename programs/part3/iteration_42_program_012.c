/* test_i386_condition_codes.c
 * Target: x86 GCC, specifically the condition code printing logic in i386.cc
 * Compile with: gcc -O2 -mfpmath=387 -march=i686 -masm=intel -S test_i386_condition_codes.c
 * Or: gcc -O3 -mfpmath=both -march=core2 -ffast-math -fverbose-asm test_i386_condition_codes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent constant folding */
volatile double g_dbl1 = 1.0;
volatile double g_dbl2 = 2.0;
volatile long double g_ldbl1 = 3.0L;
volatile long double g_ldbl2 = 4.0L;
volatile int g_selector = 0;

/* Condition code constants matching i386.md */
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
static int test_mixed_fpu(double a, long double b) __attribute__((noinline));
static void use_condition_code(int cc, double a, double b) __attribute__((noinline));

/* Individual test functions for each condition code */
static int test_unordered(double a, double b) {
    int result;
    /* Using x87 FPU comparison with UNORDERED condition */
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

static int test_ordered(double a, double b) {
    int result;
    /* Using SSE comparison with ORDERED condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r" (result)
        : "x" (a), "x" (b), "u" (ORDERED)
        : "cc"
    );
    return result;
}

static int test_uneq(double a, double b) {
    int result;
    /* Mixed x87/SSE approach for UNEQ */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b), "u" (UNEQ)
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
        : "=r" (result)
        : "x" (a), "x" (b), "u" (UNGE)
        : "cc"
    );
    return result;
}

static int test_ungt(double a, double b) {
    int result;
    /* Using UNGT condition code */
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

static int test_unle(double a, double b) {
    int result;
    /* Using UNLE condition code */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r" (result)
        : "x" (a), "x" (b), "u" (UNLE)
        : "cc"
    );
    return result;
}

static int test_unlt(double a, double b) {
    int result;
    /* Using UNLT condition code */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b), "u" (UNLT)
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
        : "=r" (result)
        : "x" (a), "x" (b), "u" (LTGT)
        : "cc"
    );
    return result;
}

/* Mixed x87 and SSE operations */
static int test_mixed_fpu(double a, long double b) {
    int result1, result2;
    
    /* First use x87 with UNORDERED */
    asm volatile (
        "fldt %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result1)
        : "m" (a), "m" (b), "u" (UNORDERED)
        : "cc", "st"
    );
    
    /* Then use SSE with ORDERED */
    double dbl_b = (double)b;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r" (result2)
        : "x" (a), "x" (dbl_b), "u" (ORDERED)
        : "cc"
    );
    
    return result1 ^ result2;
}

/* Function that uses a switch to select condition codes */
static void use_condition_code(int cc, double a, double b) {
    int result;
    
    switch (cc) {
        case UNORDERED:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNORDERED)
                : "cc"
            );
            break;
            
        case ORDERED:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (ORDERED)
                : "cc"
            );
            break;
            
        case UNEQ:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNEQ)
                : "cc"
            );
            break;
            
        case UNGE:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNGE)
                : "cc"
            );
            break;
            
        case UNGT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNGT)
                : "cc"
            );
            break;
            
        case UNLE:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNLE)
                : "cc"
            );
            break;
            
        case UNLT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (UNLT)
                : "cc"
            );
            break;
            
        case LTGT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (LTGT)
                : "cc"
            );
            break;
            
        default:
            /* This should trigger output_operand_lossage for invalid condition code */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r" (result)
                : "x" (a), "x" (b), "u" (cc)  /* Invalid cc might trigger default case */
                : "cc"
            );
            break;
    }
    
    /* Use result to prevent optimization */
    g_selector ^= result;
}

/* Helper to generate NaN values */
static double make_nan(void) {
    volatile double zero = 0.0;
    return 0.0 / zero;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    volatile int accumulator = 0;
    
    /* Parse loop count from command line if provided */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    /* Create test values including NaN, infinity, and normal numbers */
    double test_values[] = {
        0.0, 1.0, -1.0, INFINITY, -INFINITY, make_nan(),
        g_dbl1, g_dbl2, 3.14159, 2.71828
    };
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    long double ld_test_values[] = {
        0.0L, 1.0L, -1.0L, INFINITY, -INFINITY, make_nan(),
        g_ldbl1, g_ldbl2, 3.14159265358979323846L, 2.71828182845904523536L
    };
    
    printf("Testing x86 condition code printing logic...\n");
    
    /* Main test loop */
    for (int i = 0; i < loop_count; i++) {
        /* Use volatile index to prevent optimization */
        volatile int idx = i % num_values;
        volatile int idx2 = (i * 7) % num_values;
        
        /* Test all condition codes */
        accumulator += test_unordered(test_values[idx], test_values[idx2]);
        accumulator += test_ordered(test_values[idx], test_values[idx2]);
        accumulator += test_uneq(test_values[idx], test_values[idx2]);
        accumulator += test_unge(test_values[idx], test_values[idx2]);
        accumulator += test_ungt(test_values[idx], test_values[idx2]);
        accumulator += test_unle(test_values[idx], test_values[idx2]);
        accumulator += test_unlt(test_values[idx], test_values[idx2]);
        accumulator += test_ltgt(test_values[idx], test_values[idx2]);
        
        /* Test mixed FPU operations */
        accumulator += test_mixed_fpu(test_values[idx], ld_test_values[idx2]);
        
        /* Test condition code selection via switch */
        use_condition_code(i % 9, test_values[idx], test_values[idx2]);  /* 9 includes invalid */
        
        /* Mix with regular C comparisons to provide context */
        if (test_values[idx] != test_values[idx2]) {
            accumulator++;
        }
        if (test_values[idx] >= test_values[idx2]) {
            accumulator--;
        }
    }
    
    /* Also test with special NaN cases */
    double nan_val = make_nan();
    accumulator += test_unordered(nan_val, 1.0);
    accumulator += test_ordered(nan_val, 1.0);
    accumulator += test_uneq(nan_val, nan_val);
    
    /* Force potential invalid condition code usage */
    if (argc > 2 && strcmp(argv[2], "invalid") == 0) {
        /* This might trigger the default case in output_operand_lossage */
        int invalid_cc = 999;
        asm volatile (
            "comisd %2, %1\n\t"
            "set%c0 %0"
            : "=r" (accumulator)
            : "x" (g_dbl1), "x" (g_dbl2), "u" (invalid_cc)
            : "cc"
        );
    }
    
    printf("Final accumulator: %d\n", accumulator);
    
    return accumulator != 0 ? 0 : 1;
}
