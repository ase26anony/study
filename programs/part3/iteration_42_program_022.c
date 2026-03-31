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
#define UNORDERED 0
#define ORDERED   1
#define UNEQ      2
#define UNGE      3
#define UNGT      4
#define UNLE      5
#define UNLT      6
#define LTGT      7

/* Function 1: Test UNORDERED condition with x87 */
static int __attribute__((noinline)) test_unordered_x87(long double a, long double b)
{
    int result;
    /* Using x87 fucomip instruction with unordered condition */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(result), "=r"(result)
        : "0"(UNORDERED)
        : "cc", "st"
    );
    return result;
}

/* Function 2: Test ORDERED condition with SSE */
static int __attribute__((noinline)) test_ordered_sse(double a, double b)
{
    int result;
    /* Using SSE comisd instruction with ordered condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    return result;
}

/* Function 3: Test UNEQ condition with mixed operations */
static int __attribute__((noinline)) test_uneq_mixed(double a, long double b)
{
    int result1, result2;
    /* First do x87 comparison */
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result1)
        : "u"(UNEQ), "m"(b), "m"(a)
        : "cc", "st"
    );
    
    /* Then SSE comparison */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result2)
        : "x"((double)a), "x"((double)b), "u"(UNEQ)
        : "cc"
    );
    
    return result1 | result2;
}

/* Function 4: Test UNGE condition */
static int __attribute__((noinline)) test_unge(double a, double b)
{
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

/* Function 5: Test UNGT condition */
static int __attribute__((noinline)) test_ungt(long double a, long double b)
{
    int result;
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNGT), "m"(b), "m"(a)
        : "cc", "st"
    );
    return result;
}

/* Function 6: Test UNLE condition */
static int __attribute__((noinline)) test_unle(double a, double b)
{
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

/* Function 7: Test UNLT condition */
static int __attribute__((noinline)) test_unlt(long double a, long double b)
{
    int result;
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNLT), "m"(b), "m"(a)
        : "cc", "st"
    );
    return result;
}

/* Function 8: Test LTGT condition */
static int __attribute__((noinline)) test_ltgt(double a, double b)
{
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

/* Helper function that uses switch to select condition code */
static int __attribute__((noinline)) test_cond_switch(int cond_code, double a, double b)
{
    int result = 0;
    
    /* This switch may help trigger the printing logic */
    switch (cond_code) {
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
            /* This might trigger output_operand_lossage for invalid condition */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cond_code)  /* Potentially invalid */
                : "cc"
            );
            break;
    }
    
    return result;
}

/* Function with complex control flow to obscure optimizations */
static int __attribute__((noinline)) test_complex_flow(double a, double b, long double c, long double d)
{
    volatile int sum = 0;
    volatile int i;
    
    /* Loop to prevent constant folding */
    for (i = 0; i < 3; i++) {
        sum += test_unordered_x87(c + i, d - i);
        sum += test_ordered_sse(a + i, b - i);
        
        if (i % 2 == 0) {
            sum += test_uneq_mixed(a * i, c / (i + 1));
        } else {
            sum += test_unge(a / (i + 1), b * i);
        }
        
        sum += test_ungt(c * (i + 1), d / (i + 1));
        sum += test_unle(a - i, b + i);
        sum += test_unlt(c - i, d + i);
        sum += test_ltgt(a * (i + 0.5), b / (i + 0.5));
    }
    
    return sum;
}

int main(int argc, char *argv[])
{
    volatile int total = 0;
    int loop_count = 100;
    
    /* Parse loop count from command line if provided */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    /* Initialize test values from volatile sources */
    double d1 = g_d1;
    double d2 = g_d2;
    long double ld1 = g_ld1;
    long double ld2 = g_ld2;
    
    /* Also use NaN and infinity for unordered cases */
    double nan_val = 0.0 / 0.0;
    double inf_val = 1.0 / 0.0;
    
    printf("Testing i386 condition code printing logic...\n");
    
    /* Main test loop */
    for (int i = 0; i < loop_count; i++) {
        /* Vary inputs to prevent optimization */
        double a = d1 + sin(i * 0.1);
        double b = d2 + cos(i * 0.1);
        long double c = ld1 + sinl(i * 0.2);
        long double d = ld2 + cosl(i * 0.2);
        
        /* Test all condition code functions */
        total += test_unordered_x87(c, d);
        total += test_ordered_sse(a, b);
        total += test_uneq_mixed(a, c);
        total += test_unge(a, b);
        total += test_ungt(c, d);
        total += test_unle(a, b);
        total += test_unlt(c, d);
        total += test_ltgt(a, b);
        
        /* Test with NaN/Inf to trigger unordered conditions */
        total += test_unordered_x87(nan_val, c);
        total += test_ordered_sse(inf_val, b);
        
        /* Complex flow test */
        total += test_complex_flow(a, b, c, d);
        
        /* Test switch-based condition selection */
        int cond = g_selector % 9;  /* 8 valid + 1 potentially invalid */
        total += test_cond_switch(cond, a, b);
        
        /* Update selector */
        g_selector = (g_selector + 1) % 10;
    }
    
    /* Also test direct inline assembly with all condition codes */
    asm volatile (
        "# Testing UNORDERED\n\t"
        "comisd %1, %0\n\t"
        "set%c2 %%al"
        : 
        : "x"(d1), "x"(d2), "u"(UNORDERED)
        : "cc", "al"
    );
    
    asm volatile (
        "# Testing ORDERED\n\t"
        "comisd %1, %0\n\t"
        "set%c2 %%al"
        : 
        : "x"(d1), "x"(nan_val), "u"(ORDERED)
        : "cc", "al"
    );
    
    asm volatile (
        "# Testing UNEQ\n\t"
        "comisd %1, %0\n\t"
        "set%c2 %%al"
        : 
        : "x"(d1), "x"(d1), "u"(UNEQ)
        : "cc", "al"
    );
    
    asm volatile (
        "# Testing UNGE\n\t"
        "comisd %1, %0\n\t"
        "set%c2 %%al"
        : 
        : "x"(d2), "x"(d1), "u"(UNGE)
        : "cc", "al"
    );
    
    asm volatile (
        "# Testing UNGT\n\t"
        "comisd %1, %0\n\t"
        "set%c2 %%al"
        : 
        : "x"(d2), "x"(d1), "u"(UNGT)
        : "cc", "al"
    );
    
    asm volatile (
        "# Testing UNLE\n\t"
        "comisd %1, %0\n\t"
        "set%c2 %%al"
        : 
        : "x"(d1), "x"(d2), "u"(UNLE)
        : "cc", "al"
    );
    
    asm volatile (
        "# Testing UNLT\n\t"
        "comisd %1, %0\n\t"
        "set%c2 %%al"
        : 
        : "x"(d1), "x"(d2), "u"(UNLT)
        : "cc", "al"
    );
    
    asm volatile (
        "# Testing LTGT\n\t"
        "comisd %1, %0\n\t"
        "set%c2 %%al"
        : 
        : "x"(d1), "x"(d2), "u"(LTGT)
        : "cc", "al"
    );
    
    printf("Total accumulated: %d\n", total);
    printf("Test completed.\n");
    
    return total != 0 ? 0 : 1;
}
