/* test_i386_condition_codes.c
 * Target: i386.cc lines 13992-14017
 * Compile with: gcc -O2 -mfpmath=387 -march=i686 -S test_i386_condition_codes.c
 * Or: gcc -O3 -mfpmath=both -march=core2 -ffast-math -fverbose-asm test_i386_condition_codes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent constant folding */
volatile double g_d1 = 1.0;
volatile double g_d2 = 2.0;
volatile long double g_ld1 = 1.0L;
volatile long double g_ld2 = 2.0L;
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

/* Force noinline to ensure separate functions */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* Using x87 instruction with UNORDERED condition code */
    asm volatile (
        "fcomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNORDERED)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
    /* Using ORDERED condition code */
    asm volatile (
        "fcomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(ORDERED)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_uneq(double a, double b) {
    int result;
    /* Using UNEQ condition code */
    asm volatile (
        "fcomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNEQ)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    /* Using UNGE condition code */
    asm volatile (
        "fcomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNGE)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_ungt(double a, double b) {
    int result;
    /* Using UNGT condition code */
    asm volatile (
        "fcomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNGT)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    /* Using UNLE condition code */
    asm volatile (
        "fcomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNLE)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_unlt(double a, double b) {
    int result;
    /* Using UNLT condition code */
    asm volatile (
        "fcomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNLT)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    /* Using LTGT condition code */
    asm volatile (
        "fcomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(LTGT)
        : "cc", "st"
    );
    return result;
}

/* Mixed x87 and SSE operations */
__attribute__((noinline))
static int test_mixed_operations(long double ld1, long double ld2, double d1, double d2) {
    int results[8] = {0};
    
    /* x87 operations */
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fcomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "set%c1 %1"
        : "=r"(results[0]), "=r"(results[1])
        : "m"(ld1), "m"(ld2), "u"(UNORDERED), "u"(ORDERED)
        : "cc", "st"
    );
    
    /* SSE operations */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0\n\t"
        "set%c1 %1"
        : "=r"(results[2]), "=r"(results[3])
        : "x"(d1), "x"(d2), "u"(UNEQ), "u"(UNGE)
        : "cc"
    );
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Function that uses switch to select condition code - may trigger default case */
__attribute__((noinline))
static int test_condition_switch(int cc, double a, double b) {
    int result = 0;
    
    switch (cc) {
        case UNORDERED:
            asm volatile (
                "fcomip %%st(1), %%st(0)\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNORDERED)
                : "cc", "st"
            );
            break;
        case ORDERED:
            asm volatile (
                "fcomip %%st(1), %%st(0)\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(ORDERED)
                : "cc", "st"
            );
            break;
        case UNEQ:
            asm volatile (
                "fcomip %%st(1), %%st(0)\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNEQ)
                : "cc", "st"
            );
            break;
        case UNGE:
            asm volatile (
                "fcomip %%st(1), %%st(0)\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNGE)
                : "cc", "st"
            );
            break;
        case UNGT:
            asm volatile (
                "fcomip %%st(1), %%st(0)\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNGT)
                : "cc", "st"
            );
            break;
        case UNLE:
            asm volatile (
                "fcomip %%st(1), %%st(0)\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNLE)
                : "cc", "st"
            );
            break;
        case UNLT:
            asm volatile (
                "fcomip %%st(1), %%st(0)\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNLT)
                : "cc", "st"
            );
            break;
        case LTGT:
            asm volatile (
                "fcomip %%st(1), %%st(0)\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(LTGT)
                : "cc", "st"
            );
            break;
        default:
            /* This might trigger output_operand_lossage if cc is out of range */
            asm volatile (
                "fcomip %%st(1), %%st(0)\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(cc)  /* Potentially invalid condition code */
                : "cc", "st"
            );
            break;
    }
    
    return result;
}

/* Function with complex control flow to obscure constant folding */
__attribute__((noinline))
static int test_complex_flow(double a, double b, int iterations) {
    volatile int sum = 0;
    double temp = a;
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the values to prevent optimization */
        temp = temp * 1.1 + sin((double)i * 0.1);
        
        /* Use different condition codes based on loop iteration */
        int cc = i % 9;  /* 8 valid codes + 1 potentially invalid */
        
        switch (cc) {
            case 0: sum += test_unordered(temp, b); break;
            case 1: sum += test_ordered(temp, b); break;
            case 2: sum += test_uneq(temp, b); break;
            case 3: sum += test_unge(temp, b); break;
            case 4: sum += test_ungt(temp, b); break;
            case 5: sum += test_unle(temp, b); break;
            case 6: sum += test_unlt(temp, b); break;
            case 7: sum += test_ltgt(temp, b); break;
            case 8: 
                /* Potentially invalid condition code */
                asm volatile (
                    "fcomip %%st(1), %%st(0)\n\t"
                    "set%c0 %0"
                    : "=r"(sum)
                    : "u"(cc)  /* cc=8 is invalid */
                    : "cc", "st"
                );
                break;
        }
        
        /* Mix with regular C comparisons */
        if (temp != b) sum++;
        if (temp >= b) sum--;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int loop_count = 100;
    volatile int total_sum = 0;
    
    /* Parse loop count from command line if provided */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    /* Initialize with volatile values to prevent constant propagation */
    double d1 = g_d1;
    double d2 = g_d2;
    long double ld1 = g_ld1;
    long double ld2 = g_ld2;
    
    printf("Testing i386 condition code printing (targeting lines 13992-14017)\n");
    printf("Using loop count: %d\n", loop_count);
    
    /* Test all condition code functions */
    total_sum += test_unordered(d1, d2);
    total_sum += test_ordered(d1, d2);
    total_sum += test_uneq(d1, d2);
    total_sum += test_unge(d1, d2);
    total_sum += test_ungt(d1, d2);
    total_sum += test_unle(d1, d2);
    total_sum += test_unlt(d1, d2);
    total_sum += test_ltgt(d1, d2);
    
    /* Test mixed x87 and SSE operations */
    total_sum += test_mixed_operations(ld1, ld2, d1, d2);
    
    /* Test with switch statement using volatile selector */
    for (int i = 0; i < 10; i++) {
        g_selector = i;
        total_sum += test_condition_switch(g_selector, d1 + i * 0.1, d2);
    }
    
    /* Test complex flow with loops to prevent optimization */
    total_sum += test_complex_flow(d1, d2, loop_count);
    
    /* Generate NaN values to test UNORDERED cases */
    double nan_val = 0.0 / 0.0;
    double inf_val = 1.0 / 0.0;
    
    total_sum += test_unordered(nan_val, d2);
    total_sum += test_ordered(nan_val, d2);
    total_sum += test_uneq(nan_val, inf_val);
    
    /* Print result to prevent dead code elimination */
    printf("Accumulated sum: %d\n", total_sum);
    
    /* Force assembly output of condition codes */
    asm volatile (
        "# Force condition code output\n"
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fcomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(total_sum)
        : "m"(d1), "m"(d2), "u"(UNORDERED)
        : "cc", "st"
    );
    
    return total_sum != 0 ? 0 : 1;
}
