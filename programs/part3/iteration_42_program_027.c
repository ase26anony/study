/* test_condition_codes.c - Target x86 condition code printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent constant folding */
volatile double g_dbl1 = 1.0;
volatile double g_dbl2 = 2.0;
volatile long double g_ldbl1 = 3.0L;
volatile long double g_ldbl2 = 4.0L;
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

/* Function 1: Test UNORDERED condition with x87 */
__attribute__((noinline))
static int test_unordered_x87(long double a, long double b) {
    int result;
    /* Use x87 fucomip instruction with unordered condition */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNORDERED), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

/* Function 2: Test ORDERED condition with SSE */
__attribute__((noinline))
static int test_ordered_sse(double a, double b) {
    int result;
    /* Use SSE comisd instruction with ordered condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result), "=x"(a)
        : "x"(b), "0"(ORDERED)
        : "cc"
    );
    return result;
}

/* Function 3: Test UNEQ condition with mixed operations */
__attribute__((noinline))
static int test_uneq_mixed(double a, long double b) {
    int result1, result2;
    /* First do x87 comparison */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "fstp %%st(0)"
        : "=u"(UNEQ)
        : "t"((long double)a), "u"(b)
        : "cc", "st"
    );
    
    /* Then SSE comparison for same values */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result1), "=x"(a)
        : "x"((double)b), "0"(UNEQ)
        : "cc"
    );
    
    /* Use the condition code in another way */
    asm volatile (
        "mov $0, %0\n\t"
        "j%c0 1f\n\t"
        "mov $1, %0\n\t"
        "1:"
        : "=r"(result2)
        : "i"(UNEQ)
        : "cc"
    );
    
    return result1 | result2;
}

/* Function 4: Test UNGE condition (nlt) */
__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result), "=x"(a)
        : "x"(b), "0"(UNGE)
        : "cc"
    );
    return result;
}

/* Function 5: Test UNGT condition (nle) */
__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "t"(a), "u"(b), "i"(UNGT)
        : "cc", "st"
    );
    return result;
}

/* Function 6: Test UNLE condition */
__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result), "=x"(a)
        : "x"(b), "0"(UNLE)
        : "cc"
    );
    return result;
}

/* Function 7: Test UNLT condition */
__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "t"(a), "u"(b), "i"(UNLT)
        : "cc", "st"
    );
    return result;
}

/* Function 8: Test LTGT condition (une) */
__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result), "=x"(a)
        : "x"(b), "0"(LTGT)
        : "cc"
    );
    return result;
}

/* Helper function that uses condition code parameter - may trigger printing */
__attribute__((noinline))
static int use_condition_code(int cc, double a, double b) {
    int result;
    
    /* Switch to potentially confuse optimizer */
    switch (cc) {
        case UNORDERED: cc = UNORDERED; break;
        case ORDERED:   cc = ORDERED;   break;
        case UNEQ:      cc = UNEQ;      break;
        case UNGE:      cc = UNGE;      break;
        case UNGT:      cc = UNGT;      break;
        case UNLE:      cc = UNLE;      break;
        case UNLT:      cc = UNLT;      break;
        case LTGT:      cc = LTGT;      break;
        default:        cc = UNORDERED; break;
    }
    
    /* Use the condition code in assembly */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result), "=x"(a)
        : "x"(b), "0"(cc)
        : "cc"
    );
    
    return result;
}

/* Function that might trigger the default case */
__attribute__((noinline))
static int test_invalid_condition(double a, double b) {
    int result = 0;
    volatile int invalid_cc = 100;  /* Definitely invalid */
    
    /* Try to use invalid condition code - might trigger output_operand_lossage */
    asm volatile (
        "# Attempt with potentially invalid condition\n\t"
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result), "=x"(a)
        : "x"(b), "0"(invalid_cc)
        : "cc"
    );
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int i, iterations;
    
    /* Get iterations from command line or use default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    /* Initialize test arrays with volatile values */
    double dbl_array[8];
    long double ldbl_array[8];
    
    for (i = 0; i < 8; i++) {
        dbl_array[i] = g_dbl1 + i * 0.5;
        ldbl_array[i] = g_ldbl1 + i * 0.5L;
    }
    
    printf("Testing x86 condition codes for %d iterations...\n", iterations);
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs to prevent optimization */
        int idx1 = iter % 8;
        int idx2 = (iter + 1) % 8;
        
        /* Test all condition codes */
        total += test_unordered_x87(ldbl_array[idx1], ldbl_array[idx2]);
        total += test_ordered_sse(dbl_array[idx1], dbl_array[idx2]);
        total += test_uneq_mixed(dbl_array[idx1], ldbl_array[idx2]);
        total += test_unge(dbl_array[idx1], dbl_array[idx2]);
        total += test_ungt(ldbl_array[idx1], ldbl_array[idx2]);
        total += test_unle(dbl_array[idx1], dbl_array[idx2]);
        total += test_unlt(ldbl_array[idx1], ldbl_array[idx2]);
        total += test_ltgt(dbl_array[idx1], dbl_array[idx2]);
        
        /* Use condition code via helper with varying selector */
        g_selector = iter % 9;  /* 8 valid + 1 extra */
        int cc = g_selector;
        if (cc > 7) cc = 0;  /* Wrap to valid */
        
        total += use_condition_code(cc, dbl_array[idx1], dbl_array[idx2]);
        
        /* Occasionally test with potentially invalid condition */
        if (iter % 13 == 0) {
            total += test_invalid_condition(dbl_array[idx1], dbl_array[idx2]);
        }
        
        /* Mix with regular C comparisons for context */
        if (dbl_array[idx1] != dbl_array[idx2]) {
            total += 1;
        }
        if (dbl_array[idx1] >= dbl_array[idx2]) {
            total += 2;
        }
    }
    
    printf("Final accumulated result: %d\n", total);
    
    /* Force assembly output of condition codes */
    asm volatile (
        "# Final assembly block with multiple conditions\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "comisd %2, %1\n\t"
        "set%c3 %0"
        : "=r"(total), "=x"(g_dbl1)
        : "x"(g_dbl2), "i"(UNORDERED), "i"(ORDERED), "i"(UNEQ), 
          "i"(UNGE), "i"(UNGT), "i"(UNLE), "i"(UNLT), "i"(LTGT)
        : "cc", "st"
    );
    
    return total != 0;
}
