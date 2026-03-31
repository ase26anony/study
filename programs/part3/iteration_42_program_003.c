/* Condition code test program for i386.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent constant folding */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile long double vld1 = 3.0L;
volatile long double vld2 = 4.0L;
volatile int vindex = 0;

/* Condition code constants matching i386.h */
#define UNORDERED 0
#define ORDERED   1
#define UNEQ      2
#define UNGE      3
#define UNGT      4
#define UNLE      5
#define UNLT      6
#define LTGT      7

/* Test functions for each condition code */
static int __attribute__((noinline)) test_unordered(double a, double b) {
    int result;
    /* Using x87 instruction with UNORDERED condition */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0" 
                  : "=r"(result) 
                  : "u"(UNORDERED) 
                  : "cc", "st");
    return result;
}

static int __attribute__((noinline)) test_ordered(double a, double b) {
    int result;
    /* Mix with regular comparison for context */
    if (a != b) {
        asm volatile ("comisd %1, %0; set%c1 %2" 
                      : "=r"(result) 
                      : "x"(a), "u"(ORDERED), "x"(b)
                      : "cc");
    } else {
        result = 0;
    }
    return result;
}

static int __attribute__((noinline)) test_uneq(long double a, long double b) {
    int result;
    /* x87 long double comparison with UNEQ */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0" 
                  : "=r"(result) 
                  : "u"(UNEQ) 
                  : "cc", "st");
    return result;
}

static int __attribute__((noinline)) test_unge(double a, double b) {
    int result;
    /* SSE comparison with UNGE */
    asm volatile ("comisd %1, %0; set%c1 %2" 
                  : "=r"(result) 
                  : "x"(a), "u"(UNGE), "x"(b)
                  : "cc");
    return result;
}

static int __attribute__((noinline)) test_ungt(long double a, long double b) {
    int result;
    /* x87 with UNGT */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0" 
                  : "=r"(result) 
                  : "u"(UNGT) 
                  : "cc", "st");
    return result;
}

static int __attribute__((noinline)) test_unle(double a, double b) {
    int result;
    /* SSE with UNLE */
    asm volatile ("comisd %1, %0; set%c1 %2" 
                  : "=r"(result) 
                  : "x"(a), "u"(UNLE), "x"(b)
                  : "cc");
    return result;
}

static int __attribute__((noinline)) test_unlt(long double a, long double b) {
    int result;
    /* x87 with UNLT */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0" 
                  : "=r"(result) 
                  : "u"(UNLT) 
                  : "cc", "st");
    return result;
}

static int __attribute__((noinline)) test_ltgt(double a, double b) {
    int result;
    /* Mixed comparison with LTGT */
    if (isnan(a) || isnan(b)) {
        asm volatile ("comisd %1, %0; set%c1 %2" 
                      : "=r"(result) 
                      : "x"(a), "u"(LTGT), "x"(b)
                      : "cc");
    } else {
        result = (a != b) ? 1 : 0;
    }
    return result;
}

/* Helper function that uses switch to select condition code */
static int __attribute__((noinline)) test_condition_code(int cc, double a, double b) {
    int result = 0;
    
    switch (cc) {
        case UNORDERED:
            asm volatile ("comisd %1, %0; set%c1 %2" 
                          : "=r"(result) 
                          : "x"(a), "u"(UNORDERED), "x"(b)
                          : "cc");
            break;
        case ORDERED:
            asm volatile ("comisd %1, %0; set%c1 %2" 
                          : "=r"(result) 
                          : "x"(a), "u"(ORDERED), "x"(b)
                          : "cc");
            break;
        case UNEQ:
            asm volatile ("comisd %1, %0; set%c1 %2" 
                          : "=r"(result) 
                          : "x"(a), "u"(UNEQ), "x"(b)
                          : "cc");
            break;
        case UNGE:
            asm volatile ("comisd %1, %0; set%c1 %2" 
                          : "=r"(result) 
                          : "x"(a), "u"(UNGE), "x"(b)
                          : "cc");
            break;
        case UNGT:
            asm volatile ("comisd %1, %0; set%c1 %2" 
                          : "=r"(result) 
                          : "x"(a), "u"(UNGT), "x"(b)
                          : "cc");
            break;
        case UNLE:
            asm volatile ("comisd %1, %0; set%c1 %2" 
                          : "=r"(result) 
                          : "x"(a), "u"(UNLE), "x"(b)
                          : "cc");
            break;
        case UNLT:
            asm volatile ("comisd %1, %0; set%c1 %2" 
                          : "=r"(result) 
                          : "x"(a), "u"(UNLT), "x"(b)
                          : "cc");
            break;
        case LTGT:
            asm volatile ("comisd %1, %0; set%c1 %2" 
                          : "=r"(result) 
                          : "x"(a), "u"(LTGT), "x"(b)
                          : "cc");
            break;
        default:
            /* This should trigger output_operand_lossage for invalid code */
            asm volatile ("comisd %1, %0; set%c1 %2" 
                          : "=r"(result) 
                          : "x"(a), "u"(cc), "x"(b)
                          : "cc");
            break;
    }
    
    return result;
}

/* Function with complex control flow to obscure optimizations */
static int __attribute__((noinline)) test_mixed_comparisons(int iterations) {
    volatile int sum = 0;
    double darray[8];
    long double ldarray[8];
    
    /* Initialize arrays with volatile values */
    for (int i = 0; i < 8; i++) {
        darray[i] = vd1 * i + vd2;
        ldarray[i] = vld1 * i + vld2;
    }
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Call all test functions */
        sum += test_unordered(darray[idx], darray[(idx + 1) % 8]);
        sum += test_ordered(darray[idx], darray[(idx + 2) % 8]);
        sum += test_uneq(ldarray[idx], ldarray[(idx + 3) % 8]);
        sum += test_unge(darray[idx], darray[(idx + 4) % 8]);
        sum += test_ungt(ldarray[idx], ldarray[(idx + 5) % 8]);
        sum += test_unle(darray[idx], darray[(idx + 6) % 8]);
        sum += test_unlt(ldarray[idx], ldarray[(idx + 7) % 8]);
        sum += test_ltgt(darray[idx], darray[idx]);
        
        /* Use switch-based helper with volatile index */
        sum += test_condition_code(vindex % 9, darray[idx], darray[(idx + 1) % 8]);
        
        /* Modify volatile index to change condition codes */
        vindex++;
    }
    
    return sum;
}

/* Function that might trigger invalid condition code */
static void __attribute__((noinline)) test_edge_cases(void) {
    double special_vals[] = {0.0, -0.0, INFINITY, -INFINITY, NAN};
    int n = sizeof(special_vals) / sizeof(special_vals[0]);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            /* Force generation of various condition codes */
            int r1 = test_unordered(special_vals[i], special_vals[j]);
            int r2 = test_ordered(special_vals[i], special_vals[j]);
            int r3 = test_uneq((long double)special_vals[i], (long double)special_vals[j]);
            
            /* Use results to prevent dead code elimination */
            vindex += r1 + r2 + r3;
        }
    }
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    
    /* Parse iterations from command line if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    printf("Testing condition codes with %d iterations\n", iterations);
    
    /* Test edge cases first */
    test_edge_cases();
    
    /* Main test with mixed comparisons */
    volatile int total_sum = test_mixed_comparisons(iterations);
    
    /* Additional test with direct assembly output pattern */
    for (int i = 0; i < 10; i++) {
        double a = vd1 + i;
        double b = vd2 + i;
        
        /* Direct use of condition codes in inline asm with output */
        int r1, r2, r3, r4;
        asm volatile ("comisd %2, %1; set%c3 %0" 
                      : "=r"(r1) 
                      : "x"(a), "x"(b), "u"(UNORDERED) 
                      : "cc");
        asm volatile ("comisd %2, %1; set%c3 %0" 
                      : "=r"(r2) 
                      : "x"(a), "x"(b), "u"(UNEQ) 
                      : "cc");
        asm volatile ("comisd %2, %1; set%c3 %0" 
                      : "=r"(r3) 
                      : "x"(a), "x"(b), "u"(UNGE) 
                      : "cc");
        asm volatile ("comisd %2, %1; set%c3 %0" 
                      : "=r"(r4) 
                      : "x"(a), "x"(b), "u"(LTGT) 
                      : "cc");
        
        total_sum += r1 + r2 + r3 + r4;
    }
    
    printf("Total sum: %d\n", total_sum);
    
    /* Try to trigger default case with potentially invalid code */
    if (argc > 2 && strcmp(argv[2], "test-invalid") == 0) {
        double a = 1.0, b = 2.0;
        int result;
        /* This might trigger output_operand_lossage if 99 is invalid */
        asm volatile ("comisd %1, %0; set%c1 %2" 
                      : "=r"(result) 
                      : "x"(a), "u"(99), "x"(b)
                      : "cc");
        printf("Invalid test result: %d\n", result);
    }
    
    return total_sum != 0 ? 0 : 1;
}
