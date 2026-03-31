/* i386_condition_codes.c - Target coverage for x86 condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent constant propagation */
volatile double vd1 = 1.0, vd2 = 2.0, vd3 = 0.0, vd4 = -1.0;
volatile long double vld1 = 3.14159265358979323846L;
volatile long double vld2 = 2.71828182845904523536L;
volatile int vi1 = 0, vi2 = 1, vi3 = -1;
volatile int condition_selector = 0;

/* Condition code constants matching i386.h */
enum cmp_code {
    UNORDERED = 0,
    ORDERED = 1,
    UNEQ = 2,
    UNGE = 3,
    UNGT = 4,
    UNLE = 5,
    UNLT = 6,
    LTGT = 7,
    /* Add some invalid codes to potentially trigger default case */
    INVALID1 = 8,
    INVALID2 = 9
};

/* Function to test UNORDERED condition code */
static int __attribute__((noinline)) test_unordered(double a, double b) {
    int result;
    /* Use x87 floating compare with unordered condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNORDERED)
        : "cc", "st"
    );
    return result;
}

/* Function to test ORDERED condition code */
static int __attribute__((noinline)) test_ordered(long double a, long double b) {
    int result;
    /* Long double uses x87 stack */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(ORDERED)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNEQ condition code */
static int __attribute__((noinline)) test_uneq(double a, double b) {
    int result;
    /* Mix with SSE for variety */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNEQ)
        : "cc"
    );
    return result;
}

/* Function to test UNGE condition code */
static int __attribute__((noinline)) test_unge(double a, double b) {
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

/* Function to test UNGT condition code */
static int __attribute__((noinline)) test_ungt(long double a, long double b) {
    int result;
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

/* Function to test UNLE condition code */
static int __attribute__((noinline)) test_unle(double a, double b) {
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

/* Function to test UNLT condition code */
static int __attribute__((noinline)) test_unlt(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNLT)
        : "cc"
    );
    return result;
}

/* Function to test LTGT condition code */
static int __attribute__((noinline)) test_ltgt(long double a, long double b) {
    int result;
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(LTGT)
        : "cc", "st"
    );
    return result;
}

/* Helper function that uses a switch to select condition code */
static int __attribute__((noinline)) 
test_condition_switch(int code, double a, double b) {
    int result = 0;
    
    /* This switch may cause the compiler to generate different 
       condition code operands in RTL */
    switch (code) {
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
            /* This might trigger output_operand_lossage if code is invalid */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(code)  /* Potentially invalid code */
                : "cc"
            );
            break;
    }
    return result;
}

/* Function that mixes x87 and SSE operations */
static int __attribute__((noinline))
test_mixed_operations(double a, double b, long double c, long double d) {
    int r1, r2, r3;
    
    /* x87 operation */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(r1)
        : "m"(c), "m"(d), "u"(UNORDERED)
        : "cc", "st"
    );
    
    /* SSE operation */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(r2)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    
    /* Another x87 with different condition */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(r3)
        : "m"(c), "m"(d), "u"(UNEQ)
        : "cc", "st"
    );
    
    return r1 + r2 + r3;
}

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
    
    /* Initialize some floating point values */
    double darray[8];
    long double ldarray[8];
    
    for (i = 0; i < 8; i++) {
        darray[i] = vd1 * i + vd2;
        ldarray[i] = vld1 * i - vld2;
    }
    
    /* Main test loop */
    for (i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Test all condition code functions */
        total += test_unordered(darray[idx], darray[(idx + 1) % 8]);
        total += test_ordered(ldarray[idx], ldarray[(idx + 2) % 8]);
        total += test_uneq(darray[(idx + 3) % 8], darray[idx]);
        total += test_unge(darray[idx], darray[(idx + 4) % 8]);
        total += test_ungt(ldarray[(idx + 5) % 8], ldarray[idx]);
        total += test_unle(darray[idx], darray[(idx + 6) % 8]);
        total += test_unlt(darray[(idx + 7) % 8], darray[idx]);
        total += test_ltgt(ldarray[idx], ldarray[(idx + 3) % 8]);
        
        /* Test mixed operations */
        total += test_mixed_operations(
            darray[idx], darray[(idx + 1) % 8],
            ldarray[(idx + 2) % 8], ldarray[(idx + 3) % 8]
        );
        
        /* Test condition switch with volatile selector */
        condition_selector = (condition_selector + 1) % 10;
        total += test_condition_switch(
            condition_selector,
            darray[idx], 
            darray[(idx + 4) % 8]
        );
        
        /* Mix with regular C comparisons to provide context */
        if (darray[idx] != darray[(idx + 1) % 8]) {
            total += test_unordered(darray[idx], darray[(idx + 1) % 8]);
        }
        
        if (ldarray[idx] >= ldarray[(idx + 2) % 8]) {
            total += test_ordered(ldarray[idx], ldarray[(idx + 2) % 8]);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total: %d\n", total);
    
    /* Additional test with NaN to trigger UNORDERED */
    double nan_value = 0.0 / 0.0;
    total += test_unordered(nan_value, darray[0]);
    total += test_ordered(nan_value, darray[0]);
    
    printf("Final total: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
