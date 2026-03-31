/* test_i386_condcodes.c
 * Target: i386.cc lines 13992-14017
 * Compile with: gcc -O2 -mfpmath=387 -march=i686 -masm=intel -S test_i386_condcodes.c
 * Or: gcc -O3 -mfpmath=both -march=core2 -ffast-math -fverbose-asm test_i386_condcodes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Define condition code constants matching i386.h */
#define UNORDERED 0
#define ORDERED   1
#define UNEQ      2
#define UNGE      3
#define UNGT      4
#define UNLE      5
#define UNLT      6
#define LTGT      7

/* Volatile variables to prevent constant folding */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile long double vld1 = 3.0L;
volatile long double vld2 = 4.0L;
volatile int cond_selector = 0;

/* ========== Functions for specific condition codes ========== */

/* Test UNORDERED condition with x87 */
__attribute__((noinline))
static int test_unordered_x87(long double a, long double b) {
    int result;
    /* Use fucomip to compare and set condition codes */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNORDERED), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

/* Test ORDERED condition with SSE */
__attribute__((noinline))
static int test_ordered_sse(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    return result;
}

/* Test UNEQ condition with mixed operations */
__attribute__((noinline))
static int test_uneq_mixed(double a, long double b) {
    int result1, result2;
    /* First with SSE */
    asm volatile (
        "comisd %3, %2\n\t"
        "set%c0 %0"
        : "=r"(result1)
        : "x"((double)a), "x"((double)b), "u"(UNEQ)
        : "cc"
    );
    /* Then with x87 */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNEQ), "=r"(result2)
        : "t"((long double)a), "u"((long double)b)
        : "cc", "st"
    );
    return result1 & result2;
}

/* Test UNGE condition */
__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNGE)
        : "cc"
    );
    return result;
}

/* Test UNGT condition */
__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNGT), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

/* Test UNLE condition */
__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNLE)
        : "cc"
    );
    return result;
}

/* Test UNLT condition */
__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNLT), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

/* Test LTGT condition */
__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(LTGT)
        : "cc"
    );
    return result;
}

/* ========== Function that uses switch to select condition code ========== */

/* This function may trigger the default case in output_operand_lossage */
__attribute__((noinline))
static int test_cond_switch(int cond_code, double a, double b) {
    int result = 0;
    
    switch (cond_code & 0x7) {  /* Mask to 3 bits */
        case UNORDERED:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNORDERED)
                : "cc"
            );
            break;
        case ORDERED:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(ORDERED)
                : "cc"
            );
            break;
        case UNEQ:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNEQ)
                : "cc"
            );
            break;
        case UNGE:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGE)
                : "cc"
            );
            break;
        case UNGT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGT)
                : "cc"
            );
            break;
        case UNLE:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLE)
                : "cc"
            );
            break;
        case UNLT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLT)
                : "cc"
            );
            break;
        case LTGT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(LTGT)
                : "cc"
            );
            break;
        default:
            /* This might trigger output_operand_lossage if cond_code is out of range */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cond_code)  /* Potentially invalid condition code */
                : "cc"
            );
            break;
    }
    return result;
}

/* ========== Helper with volatile control flow ========== */

__attribute__((noinline))
static int test_with_nan_control(double a, double b, int use_nan) {
    volatile int local_cond = cond_selector;
    double x = a;
    double y = b;
    
    if (use_nan) {
        x = 0.0 / 0.0;  /* Generate NaN */
    }
    
    /* Force compiler to consider all condition codes */
    int results[8];
    results[0] = test_unordered_x87(x, y);
    results[1] = test_ordered_sse(x, y);
    results[2] = test_uneq_mixed(x, y);
    results[3] = test_unge(x, y);
    results[4] = test_ungt(x, y);
    results[5] = test_unle(x, y);
    results[6] = test_unlt(x, y);
    results[7] = test_ltgt(x, y);
    
    /* Also test the switch-based function */
    int switch_result = test_cond_switch(local_cond, x, y);
    
    /* Combine all results */
    int sum = switch_result;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    
    return sum;
}

/* ========== Main function ========== */

int main(int argc, char *argv[]) {
    int loop_count = 100;
    volatile int total_sum = 0;
    
    /* Parse loop count from command line if provided */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    /* Initialize arrays with volatile values to prevent constant folding */
    double darray[10];
    long double ldarray[10];
    
    for (int i = 0; i < 10; i++) {
        darray[i] = vd1 * i + vd2;
        ldarray[i] = vld1 * i + vld2;
    }
    
    /* Main test loop */
    for (int iter = 0; iter < loop_count; iter++) {
        /* Vary the condition selector */
        cond_selector = iter & 0xF;  /* Could be > 7 to trigger default case */
        
        /* Test with normal values */
        for (int i = 0; i < 9; i++) {
            int idx1 = i % 10;
            int idx2 = (i + 1) % 10;
            
            /* Mix double and long double tests */
            total_sum += test_unordered_x87(ldarray[idx1], ldarray[idx2]);
            total_sum += test_ordered_sse(darray[idx1], darray[idx2]);
            total_sum += test_uneq_mixed(darray[idx1], ldarray[idx2]);
            total_sum += test_unge(darray[idx1], darray[idx2]);
            total_sum += test_ungt(ldarray[idx1], ldarray[idx2]);
            total_sum += test_unle(darray[idx1], darray[idx2]);
            total_sum += test_unlt(ldarray[idx1], ldarray[idx2]);
            total_sum += test_ltgt(darray[idx1], darray[idx2]);
            
            /* Test the switch-based function - may trigger default case */
            total_sum += test_cond_switch(cond_selector, darray[idx1], darray[idx2]);
        }
        
        /* Test with NaN values every few iterations */
        if (iter % 7 == 0) {
            total_sum += test_with_nan_control(darray[0], darray[1], 1);
        }
        
        /* Also test with infinities */
        if (iter % 11 == 0) {
            double inf = 1.0 / 0.0;  /* Generate infinity */
            total_sum += test_unordered_x87(inf, ldarray[0]);
            total_sum += test_ordered_sse(inf, darray[0]);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    /* Additional test that might trigger the printing logic more directly */
    if (argc > 2 && strcmp(argv[2], "verbose") == 0) {
        /* Force assembly output of condition codes */
        for (int cc = 0; cc < 8; cc++) {
            printf("Testing condition code %d: ", cc);
            int r = test_cond_switch(cc, 1.0, 2.0);
            printf("result = %d\n", r);
        }
        
        /* Test with potentially invalid condition code */
        printf("Testing with invalid condition code 15: ");
        int r = test_cond_switch(15, 1.0, 2.0);
        printf("result = %d\n", r);
    }
    
    return total_sum != 0 ? 0 : 1;
}
