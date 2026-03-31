/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Force use of x87 FPU for some operations */
#ifdef __x86_64__
#define USE_X87 __attribute__((target("x87")))
#else
#define USE_X87
#endif

/* Volatile variables to prevent constant folding */
static volatile double vd1 = 1.0;
static volatile double vd2 = 2.0;
static volatile long double vld1 = 3.0L;
static volatile long double vld2 = 4.0L;
static volatile int selector = 0;

/* Condition code constants matching i386.h */
enum fp_condition {
    UNORDERED = 16,
    ORDERED = 17,
    UNEQ = 18,
    UNGE = 19,
    UNGT = 20,
    UNLE = 21,
    UNLT = 22,
    LTGT = 23
};

/* Function to test UNORDERED condition */
USE_X87 __attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* Use x87 comparison with unordered check */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "al"
    );
    return result;
}

/* Function to test ORDERED condition */
USE_X87 __attribute__((noinline))
static int test_ordered(long double a, long double b) {
    int result;
    /* Use x87 comparison with ordered check */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "al"
    );
    return result;
}

/* Function to test UNEQ condition */
__attribute__((noinline))
static int test_uneq(double a, double b) {
    int result;
    /* Use SSE comparison for uneq */
    asm volatile (
        "comisd %2, %1\n\t"
        "setp %%al\n\t"
        "sete %%cl\n\t"
        "or %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "cc", "al", "cl"
    );
    return result;
}

/* Function to test UNGE condition */
__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    /* nlt = not less than = greater or equal or unordered */
    asm volatile (
        "comisd %2, %1\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "cc", "al"
    );
    return result;
}

/* Function to test UNGT condition */
__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    /* nle = not less or equal = greater or unordered */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "al"
    );
    return result;
}

/* Function to test UNLE condition */
__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    /* ule = unordered or less or equal */
    asm volatile (
        "comisd %2, %1\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "cc", "al"
    );
    return result;
}

/* Function to test UNLT condition */
__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    /* ult = unordered or less than */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "al"
    );
    return result;
}

/* Function to test LTGT condition */
__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    /* une = unordered or not equal */
    asm volatile (
        "comisd %2, %1\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "x" (a), "x" (b)
        : "cc", "al"
    );
    return result;
}

/* Function that uses condition code as operand with %c modifier */
__attribute__((noinline))
static int test_cond_code(int cond, double a, double b) {
    int result;
    /* This should trigger the condition code printing logic */
    switch (cond) {
        case UNORDERED:
            asm volatile ("comisd %2, %1; setp %0" 
                         : "=r" (result) : "x" (a), "x" (b) : "cc");
            break;
        case ORDERED:
            asm volatile ("comisd %2, %1; setnp %0" 
                         : "=r" (result) : "x" (a), "x" (b) : "cc");
            break;
        case UNEQ:
            asm volatile ("comisd %2, %1; setp %%al; sete %%cl; or %%cl, %%al; movzbl %%al, %0" 
                         : "=r" (result) : "x" (a), "x" (b) : "cc", "al", "cl");
            break;
        case UNGE:
            asm volatile ("comisd %2, %1; setnb %0" 
                         : "=r" (result) : "x" (a), "x" (b) : "cc");
            break;
        case UNGT:
            asm volatile ("comisd %2, %1; setnbe %0" 
                         : "=r" (result) : "x" (a), "x" (b) : "cc");
            break;
        case UNLE:
            asm volatile ("comisd %2, %1; setna %0" 
                         : "=r" (result) : "x" (a), "x" (b) : "cc");
            break;
        case UNLT:
            asm volatile ("comisd %2, %1; setb %0" 
                         : "=r" (result) : "x" (a), "x" (b) : "cc");
            break;
        case LTGT:
            asm volatile ("comisd %2, %1; setne %0" 
                         : "=r" (result) : "x" (a), "x" (b) : "cc");
            break;
        default:
            /* This might trigger output_operand_lossage */
            asm volatile ("comisd %2, %1; setp %0" 
                         : "=r" (result) : "x" (a), "x" (b) : "cc");
            break;
    }
    return result;
}

/* Function that directly uses condition codes in extended asm */
__attribute__((noinline))
static void test_all_conditions(double a, double b, long double c, long double d) {
    volatile int sum = 0;
    
    /* Test each condition code */
    sum += test_unordered(a, b);
    sum += test_ordered(c, d);
    sum += test_uneq(a, b);
    sum += test_unge(a, b);
    sum += test_ungt(c, d);
    sum += test_unle(a, b);
    sum += test_unlt(c, d);
    sum += test_ltgt(a, b);
    
    /* Use volatile to prevent optimization */
    *(volatile int*)&sum = sum;
}

/* Main function with complex control flow */
int main(int argc, char *argv[]) {
    int iterations = 100;
    volatile int total = 0;
    
    /* Parse iterations from command line if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create arrays with NaN values to trigger unordered conditions */
    double darray[8];
    long double ldarray[8];
    
    /* Initialize with various values including NaN */
    darray[0] = 1.0;
    darray[1] = 2.0;
    darray[2] = 0.0;
    darray[3] = -0.0;
    darray[4] = vd1 / vd2;  /* 0.5 */
    darray[5] = vd1 / 0.0;  /* Infinity */
    darray[6] = - (vd1 / 0.0); /* -Infinity */
    darray[7] = 0.0 / 0.0;  /* NaN */
    
    ldarray[0] = 3.0L;
    ldarray[1] = 4.0L;
    ldarray[2] = 0.0L;
    ldarray[3] = -0.0L;
    ldarray[4] = vld1 / vld2;  /* 0.75L */
    ldarray[5] = vld1 / 0.0L;  /* Infinity */
    ldarray[6] = - (vld1 / 0.0L); /* -Infinity */
    ldarray[7] = 0.0L / 0.0L;  /* NaN */
    
    /* Main test loop */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile index to prevent optimization */
        volatile int idx = i % 8;
        volatile int idx2 = (i + 1) % 8;
        
        /* Test all conditions with different value pairs */
        test_all_conditions(darray[idx], darray[idx2], 
                           ldarray[idx], ldarray[idx2]);
        
        /* Test condition code selection via switch */
        int cond = UNORDERED + (i % 9);  /* 8 valid + 1 invalid */
        total += test_cond_code(cond, darray[idx], darray[idx2]);
        
        /* Mix with regular comparisons */
        if (darray[idx] != darray[idx2]) {
            total += 1;
        }
        if (ldarray[idx] >= ldarray[idx2]) {
            total += 2;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total: %d\n", total);
    
    /* Try to trigger the default case with invalid condition code */
    if (argc > 2 && strcmp(argv[2], "test") == 0) {
        /* This might trigger output_operand_lossage */
        int dummy;
        asm volatile ("# Invalid condition code test\n\t"
                     : "=r" (dummy) : "u" (100) : "cc");
    }
    
    return 0;
}
