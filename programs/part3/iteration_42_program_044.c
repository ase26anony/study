/* Compile with: gcc -O2 -mfpmath=387 -march=i686 -masm=intel -S -o output.s this_file.c */
/* Also try: gcc -O3 -mfpmath=both -march=core2 -ffast-math -fverbose-asm */
/* And: gcc -O1 -m32 -fno-omit-frame-pointer -da */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Condition code constants matching i386.h */
#define UNORDERED 0
#define ORDERED   1
#define UNEQ      2
#define UNGE      3
#define UNGT      4
#define UNLE      5
#define UNLT      6
#define LTGT      7

/* Volatile variables to prevent constant folding */
volatile double vd1 = 1.0, vd2 = 2.0, vd3 = 0.0, vd4 = -1.0;
volatile long double vld1 = 3.14159L, vld2 = 2.71828L;
volatile int condition_selector = 0;

/* Function prototypes */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static int test_mixed_fpu(long double a, double b) __attribute__((noinline));
static void use_condition_in_switch(int cc) __attribute__((noinline));

/* Test UNORDERED condition code */
static int test_unordered(double a, double b) {
    int result;
    /* Use x87 floating point compare with unordered condition */
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

/* Test ORDERED condition code */
static int test_ordered(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b), "u" (ORDERED)
        : "cc", "st"
    );
    return result;
}

/* Test UNEQ condition code */
static int test_uneq(double a, double b) {
    int result;
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

/* Test UNGE condition code */
static int test_unge(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b), "u" (UNGE)
        : "cc", "st"
    );
    return result;
}

/* Test UNGT condition code */
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

/* Test UNLE condition code */
static int test_unle(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b), "u" (UNLE)
        : "cc", "st"
    );
    return result;
}

/* Test UNLT condition code */
static int test_unlt(double a, double b) {
    int result;
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

/* Test LTGT condition code */
static int test_ltgt(double a, double b) {
    int result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b), "u" (LTGT)
        : "cc", "st"
    );
    return result;
}

/* Mixed x87 and SSE operations */
static int test_mixed_fpu(long double a, double b) {
    int result1, result2;
    
    /* x87 operation */
    asm volatile (
        "fldt %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r" (result1)
        : "m" (b), "m" (&a), "u" (UNORDERED)
        : "cc", "st"
    );
    
    /* SSE operation for comparison */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r" (result2)
        : "x" (b), "x" (vd1), "u" (ORDERED)
        : "cc"
    );
    
    return result1 | result2;
}

/* Function that uses condition code in a switch to force printing logic */
static void use_condition_in_switch(int cc) {
    int result;
    double a = vd1, b = vd2;
    
    switch (cc) {
        case UNORDERED:
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
            break;
        case ORDERED:
            asm volatile (
                "fldl %2\n\t"
                "fldl %1\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r" (result)
                : "m" (a), "m" (b), "u" (ORDERED)
                : "cc", "st"
            );
            break;
        case UNEQ:
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
            break;
        case UNGE:
            asm volatile (
                "fldl %2\n\t"
                "fldl %1\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r" (result)
                : "m" (a), "m" (b), "u" (UNGE)
                : "cc", "st"
            );
            break;
        case UNGT:
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
            break;
        case UNLE:
            asm volatile (
                "fldl %2\n\t"
                "fldl %1\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r" (result)
                : "m" (a), "m" (b), "u" (UNLE)
                : "cc", "st"
            );
            break;
        case UNLT:
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
            break;
        case LTGT:
            asm volatile (
                "fldl %2\n\t"
                "fldl %1\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r" (result)
                : "m" (a), "m" (b), "u" (LTGT)
                : "cc", "st"
            );
            break;
        default:
            /* This should trigger output_operand_lossage for invalid condition code */
            asm volatile (
                "fldl %2\n\t"
                "fldl %1\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r" (result)
                : "m" (a), "m" (b), "u" (cc)  /* Potentially invalid condition code */
                : "cc", "st"
            );
            break;
    }
    
    /* Use result to prevent dead code elimination */
    vd3 = result;
}

int main(int argc, char *argv[]) {
    volatile int sum = 0;
    int i, loop_count;
    
    /* Parse loop count from command line or use default */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    } else {
        loop_count = 100;
    }
    
    /* Initialize array of test values */
    double test_values[] = {0.0, 1.0, -1.0, 2.0, -2.0, 1.0/0.0, 0.0/0.0};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Main test loop */
    for (i = 0; i < loop_count; i++) {
        int idx1 = i % num_values;
        int idx2 = (i + 1) % num_values;
        double a = test_values[idx1];
        double b = test_values[idx2];
        
        /* Test all condition codes */
        sum += test_unordered(a, b);
        sum += test_ordered(a, b);
        sum += test_uneq(a, b);
        sum += test_unge(a, b);
        sum += test_ungt(a, b);
        sum += test_unle(a, b);
        sum += test_unlt(a, b);
        sum += test_ltgt(a, b);
        
        /* Test mixed FPU operations */
        sum += test_mixed_fpu(vld1, a);
        
        /* Regular C comparisons to provide context */
        if (a != b) sum += 1;
        if (a >= b) sum += 2;
        
        /* Use switch with volatile selector to force condition code printing */
        condition_selector = i % 9;  /* 8 valid codes + 1 potentially invalid */
        use_condition_in_switch(condition_selector);
    }
    
    /* Print result to prevent optimization */
    printf("Accumulated sum: %d\n", sum);
    
    /* Additional test with potentially invalid condition code */
    if (argc > 2 && strcmp(argv[2], "test-invalid") == 0) {
        /* Force an invalid condition code through volatile */
        volatile int invalid_cc = 99;
        use_condition_in_switch(invalid_cc);
    }
    
    return 0;
}
