/* test_i386_condition_codes.c
 * Target: i386.cc lines 13992-14017
 * Compile with: gcc -O2 -mfpmath=387 -march=i686 -S test_i386_condition_codes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Define condition codes matching i386.h */
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

/* Force use of x87 FPU */
static __attribute__((noinline)) 
int test_unordered(double a, double b) {
    int result;
    /* Using %c0 to output condition code name */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNORDERED), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

static __attribute__((noinline))
int test_ordered(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(ORDERED), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

static __attribute__((noinline))
int test_uneq(long double a, long double b) {
    int result;
    /* Using x87 long double comparison */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNEQ), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

static __attribute__((noinline))
int test_unge(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2; set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(UNGE)
                  : "cc");
    return result;
}

static __attribute__((noinline))
int test_ungt(long double a, long double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGT), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

static __attribute__((noinline))
int test_unle(double a, double b) {
    int result;
    /* Mix SSE and condition code */
    asm volatile ("comisd %2, %1; set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(UNLE)
                  : "cc");
    return result;
}

static __attribute__((noinline))
int test_unlt(long double a, long double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLT), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

static __attribute__((noinline))
int test_ltgt(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2; set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(LTGT)
                  : "cc");
    return result;
}

/* Function that uses switch to select condition code */
static __attribute__((noinline))
int test_conditional_switch(int cond, double a, double b) {
    int result = 0;
    
    switch (cond & 0x7) {  /* Mask to 0-7 range */
        case UNORDERED:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNORDERED)
                          : "cc");
            break;
        case ORDERED:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(ORDERED)
                          : "cc");
            break;
        case UNEQ:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNEQ)
                          : "cc");
            break;
        case UNGE:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNGE)
                          : "cc");
            break;
        case UNGT:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNGT)
                          : "cc");
            break;
        case UNLE:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNLE)
                          : "cc");
            break;
        case UNLT:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNLT)
                          : "cc");
            break;
        case LTGT:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(LTGT)
                          : "cc");
            break;
        default:
            /* This might trigger output_operand_lossage if compiler
               tries to output an invalid condition code */
            asm volatile ("# Invalid condition code %c0"
                          : 
                          : "u"(cond)
                          : "cc");
            result = -1;
    }
    return result;
}

/* Function that might trigger default case with invalid condition */
static __attribute__((noinline))
void test_invalid_condition(void) {
    /* Force compiler to consider printing an invalid condition code */
    int invalid_cond = 255;  /* Definitely out of range */
    
    /* This assembly might cause the compiler to try to output
       an invalid condition code name */
    asm volatile ("# Testing invalid condition: %c0"
                  :
                  : "u"(invalid_cond));
}

/* Mixed FP operations to ensure various condition codes are used */
static __attribute__((noinline))
void mixed_fp_operations(double *darr, long double *ldarr, int n) {
    volatile int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Use different condition codes based on array values */
        sum += test_unordered(darr[i], darr[(i+1)%n]);
        sum += test_ordered(darr[i], darr[(i+2)%n]);
        sum += test_uneq(ldarr[i], ldarr[(i+1)%n]);
        sum += test_unge(darr[i], darr[(i+3)%n]);
        sum += test_ungt(ldarr[i], ldarr[(i+2)%n]);
        sum += test_unle(darr[i], darr[(i+4)%n]);
        sum += test_unlt(ldarr[i], ldarr[(i+3)%n]);
        sum += test_ltgt(darr[i], darr[(i+5)%n]);
        
        /* Also use the switch-based function */
        sum += test_conditional_switch(i & 0x7, darr[i], darr[(i+1)%n]);
    }
    
    /* Prevent dead code elimination */
    asm volatile ("# Sum = %0" : : "r"(sum));
}

int main(int argc, char *argv[]) {
    int iterations = 10;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    
    /* Create arrays with volatile initialization */
    double darr[20];
    long double ldarr[20];
    
    for (int i = 0; i < 20; i++) {
        darr[i] = vd1 * i + vd2;
        ldarr[i] = vld1 * i + vld2;
        
        /* Introduce some NaN values to affect comparisons */
        if (i % 7 == 0) {
            darr[i] = NAN;
            ldarr[i] = NAN;
        }
    }
    
    /* Call functions with different condition codes */
    volatile int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        total += test_unordered(darr[i % 20], darr[(i+1) % 20]);
        total += test_ordered(darr[i % 20], darr[(i+2) % 20]);
        total += test_uneq(ldarr[i % 20], ldarr[(i+3) % 20]);
        total += test_unge(darr[i % 20], darr[(i+4) % 20]);
        total += test_ungt(ldarr[i % 20], ldarr[(i+5) % 20]);
        total += test_unle(darr[i % 20], darr[(i+6) % 20]);
        total += test_unlt(ldarr[i % 20], ldarr[(i+7) % 20]);
        total += test_ltgt(darr[i % 20], darr[(i+8) % 20]);
        
        /* Use volatile selector to prevent constant propagation */
        cond_selector = i;
        total += test_conditional_switch(cond_selector, 
                                        darr[i % 20], 
                                        darr[(i+9) % 20]);
    }
    
    /* Perform mixed operations */
    mixed_fp_operations(darr, ldarr, iterations % 20);
    
    /* Try to trigger invalid condition code handling */
    if (iterations > 5) {
        test_invalid_condition();
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
