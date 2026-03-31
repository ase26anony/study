/* i386_condition_codes.c - Target coverage for x86 condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent constant propagation */
volatile double g_dbl1 = 1.0;
volatile double g_dbl2 = 2.0;
volatile long double g_ldbl1 = 3.0L;
volatile long double g_ldbl2 = 4.0L;
volatile int g_cond_selector = 0;

/* Condition code constants matching i386.h */
enum fp_condition_code {
    UNORDERED = 0,
    ORDERED = 1,
    UNEQ = 2,
    UNGE = 3,
    UNGT = 4,
    UNLE = 5,
    UNLT = 6,
    LTGT = 7,
    /* Add invalid code to potentially trigger default case */
    INVALID_CODE = 99
};

/* Force noinline to ensure separate functions */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* Use x87 instruction with UNORDERED condition */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNORDERED)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
    /* SSE comparison with ORDERED condition */
    asm volatile (
        "comisd %1, %2\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    return result;
}

__attribute__((noinline))
static int test_uneq(long double a, long double b) {
    int result;
    /* x87 long double with UNEQ condition */
    asm volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNEQ)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    /* Mixed comparison with UNGE condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNGE)
        : "cc"
    );
    return result;
}

__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    /* x87 with UNGT condition */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNGT)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    /* SSE with UNLE condition */
    asm volatile (
        "comisd %1, %2\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNLE)
        : "cc"
    );
    return result;
}

__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    /* x87 with UNLT condition */
    asm volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNLT)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    /* SSE with LTGT condition */
    asm volatile (
        "comisd %1, %2\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(LTGT)
        : "cc"
    );
    return result;
}

/* Function that uses switch to select condition code */
__attribute__((noinline))
static int dispatch_condition(int cond_code, double a, double b) {
    int result = 0;
    
    /* This switch may cause the compiler to generate 
       condition code printing during RTL expansion */
    switch (cond_code) {
        case UNORDERED:
            asm volatile (
                "comisd %1, %2\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNORDERED)
                : "cc"
            );
            break;
        case ORDERED:
            asm volatile (
                "comisd %1, %2\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(ORDERED)
                : "cc"
            );
            break;
        case UNEQ:
            asm volatile (
                "comisd %1, %2\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNEQ)
                : "cc"
            );
            break;
        case UNGE:
            asm volatile (
                "comisd %1, %2\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGE)
                : "cc"
            );
            break;
        case UNGT:
            asm volatile (
                "comisd %1, %2\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGT)
                : "cc"
            );
            break;
        case UNLE:
            asm volatile (
                "comisd %1, %2\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLE)
                : "cc"
            );
            break;
        case UNLT:
            asm volatile (
                "comisd %1, %2\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLT)
                : "cc"
            );
            break;
        case LTGT:
            asm volatile (
                "comisd %1, %2\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(LTGT)
                : "cc"
            );
            break;
        default:
            /* Potentially trigger output_operand_lossage with invalid code */
            asm volatile (
                "comisd %1, %2\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cond_code)  /* Invalid condition code */
                : "cc"
            );
            break;
    }
    return result;
}

/* Function that mixes x87 and SSE operations */
__attribute__((noinline))
static int mixed_fp_operations(double d1, double d2, 
                               long double ld1, long double ld2) {
    int sum = 0;
    
    /* Regular C comparisons to provide context */
    sum += (d1 != d2);
    sum += (d1 >= d2);
    sum += (ld1 < ld2);
    
    /* Inline assembly with various condition codes */
    sum += test_unordered(d1, d2);
    sum += test_ordered(d1, d2);
    sum += test_uneq(ld1, ld2);
    sum += test_unge(d1, d2);
    sum += test_ungt(ld1, ld2);
    sum += test_unle(d1, d2);
    sum += test_unlt(ld1, ld2);
    sum += test_ltgt(d1, d2);
    
    return sum;
}

int main(int argc, char *argv[]) {
    volatile int accumulator = 0;
    int loop_count = 100;
    
    /* Use command line argument for loop count if provided */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    /* Initialize arrays with volatile values */
    double dbl_array[8];
    long double ldbl_array[8];
    
    for (int i = 0; i < 8; i++) {
        dbl_array[i] = g_dbl1 + i * 0.5;
        ldbl_array[i] = g_ldbl1 + i * 0.5L;
    }
    
    /* Main loop with complex control flow */
    for (int iter = 0; iter < loop_count; iter++) {
        volatile int idx = iter % 8;
        volatile int cond_idx = iter % 9;  /* 8 valid + 1 for potential invalid */
        
        /* Call individual test functions */
        accumulator += test_unordered(dbl_array[idx], dbl_array[(idx + 1) % 8]);
        accumulator += test_ordered(dbl_array[idx], dbl_array[(idx + 2) % 8]);
        accumulator += test_uneq(ldbl_array[idx], ldbl_array[(idx + 3) % 8]);
        accumulator += test_unge(dbl_array[idx], dbl_array[(idx + 4) % 8]);
        accumulator += test_ungt(ldbl_array[idx], ldbl_array[(idx + 5) % 8]);
        accumulator += test_unle(dbl_array[idx], dbl_array[(idx + 6) % 8]);
        accumulator += test_unlt(ldbl_array[idx], ldbl_array[(idx + 7) % 8]);
        accumulator += test_ltgt(dbl_array[idx], dbl_array[idx]);
        
        /* Mixed operations */
        accumulator += mixed_fp_operations(
            dbl_array[idx], dbl_array[(idx + 1) % 8],
            ldbl_array[idx], ldbl_array[(idx + 2) % 8]
        );
        
        /* Dispatch with potentially invalid condition code */
        if (cond_idx == 8) {
            /* Use invalid code to potentially trigger default case */
            accumulator += dispatch_condition(INVALID_CODE, 
                                            dbl_array[idx], 
                                            dbl_array[(idx + 1) % 8]);
        } else {
            accumulator += dispatch_condition(cond_idx, 
                                            dbl_array[idx], 
                                            dbl_array[(idx + 1) % 8]);
        }
        
        /* Modify global volatile to affect next iteration */
        g_cond_selector = (g_cond_selector + 1) % 10;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulated result: %d\n", accumulator);
    
    return 0;
}
