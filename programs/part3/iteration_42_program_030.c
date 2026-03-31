/* test_condition_codes.c - Target coverage for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile globals to prevent constant folding */
volatile double g_d1 = 1.0, g_d2 = 2.0;
volatile long double g_ld1 = 3.0L, g_ld2 = 4.0L;
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

/* ========== Individual condition code test functions ========== */

/* Test UNORDERED condition with x87 */
__attribute__((noinline))
static int test_unordered_x87(long double a, long double b) {
    int result;
    /* x87 floating compare with unordered check */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNORDERED)
        : "cc", "st"
    );
    return result;
}

/* Test ORDERED condition with SSE */
__attribute__((noinline))
static int test_ordered_sse(double a, double b) {
    int result;
    /* SSE2 compare with ordered check */
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
    /* First do x87 comparison */
    asm volatile (
        "fldt %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result1)
        : "m"(a), "m"(b), "u"(UNEQ)
        : "cc", "st"
    );
    
    /* Then SSE comparison for same condition */
    double a2 = a;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result2)
        : "x"(a2), "x"((double)b), "u"(UNEQ)
        : "cc"
    );
    
    return result1 & result2;
}

/* Test UNGE condition (nlt) */
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

/* Test UNGT condition (nle) */
__attribute__((noinline))
static int test_ungt(long double a, long double b) {
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
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNLT)
        : "cc", "st"
    );
    return result;
}

/* Test LTGT condition (une) */
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

/* This function may trigger the default case if invalid code passed */
__attribute__((noinline))
static int dispatch_condition_code(int cc, double a, double b) {
    int result = 0;
    
    /* Switch to demonstrate different condition code usage */
    switch (cc) {
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
            /* This might trigger output_operand_lossage if cc is out of range */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cc)  /* Potentially invalid condition code */
                : "cc"
            );
            break;
    }
    
    return result;
}

/* ========== Main test driver ========== */

int main(int argc, char *argv[]) {
    volatile int accumulator = 0;
    int iterations = 100;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create arrays with NaN values to trigger unordered comparisons */
    double dvals[] = {1.0, 2.0, 0.0/0.0, -1.0, 3.14};  /* NaN in position 2 */
    long double ldvals[] = {1.0L, 2.0L, 0.0L/0.0L, -1.0L, 3.14159L};
    
    printf("Testing x86 condition code printing logic...\n");
    printf("Iterations: %d\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        /* Use volatile index to prevent optimization */
        volatile int idx = i % 5;
        volatile int idx2 = (i + 1) % 5;
        
        /* Test all individual condition code functions */
        accumulator += test_unordered_x87(ldvals[idx], ldvals[idx2]);
        accumulator += test_ordered_sse(dvals[idx], dvals[idx2]);
        accumulator += test_uneq_mixed(dvals[idx], ldvals[idx2]);
        accumulator += test_unge(dvals[idx], dvals[idx2]);
        accumulator += test_ungt(ldvals[idx], ldvals[idx2]);
        accumulator += test_unle(dvals[idx], dvals[idx2]);
        accumulator += test_unlt(ldvals[idx], ldvals[idx2]);
        accumulator += test_ltgt(dvals[idx], dvals[idx2]);
        
        /* Test dispatch function with various condition codes */
        for (int cc = 0; cc < 8; cc++) {
            accumulator += dispatch_condition_code(cc, dvals[idx], dvals[idx2]);
        }
        
        /* Occasionally test with potentially invalid condition code */
        if (i % 13 == 0) {
            accumulator += dispatch_condition_code(i, dvals[idx], dvals[idx2]);
        }
        
        /* Mix with regular C comparisons to provide context */
        if (dvals[idx] != dvals[idx2]) {
            accumulator += 1;
        }
        if (ldvals[idx] >= ldvals[idx2]) {
            accumulator += 2;
        }
    }
    
    /* Force use of volatile accumulator */
    printf("Final accumulator value: %d\n", accumulator);
    
    /* Additional test with inline assembly that directly outputs condition codes */
    {
        int r1, r2, r3, r4, r5, r6, r7, r8;
        
        /* Direct tests that should generate condition code strings */
        asm volatile (
            "comisd %1, %1\n\t"  /* Compare NaN with itself */
            "set%c0 %0"
            : "=r"(r1)
            : "x"(dvals[2]), "u"(UNORDERED)
            : "cc"
        );
        
        asm volatile (
            "comisd %2, %1\n\t"
            "set%c0 %0"
            : "=r"(r2)
            : "x"(dvals[0]), "x"(dvals[1]), "u"(ORDERED)
            : "cc"
        );
        
        asm volatile (
            "comisd %2, %1\n\t"
            "set%c0 %0"
            : "=r"(r3)
            : "x"(dvals[0]), "x"(dvals[0]), "u"(UNEQ)
            : "cc"
        );
        
        asm volatile (
            "comisd %2, %1\n\t"
            "set%c0 %0"
            : "=r"(r4)
            : "x"(dvals[1]), "x"(dvals[0]), "u"(UNGE)
            : "cc"
        );
        
        asm volatile (
            "comisd %2, %1\n\t"
            "set%c0 %0"
            : "=r"(r5)
            : "x"(dvals[1]), "x"(dvals[0]), "u"(UNGT)
            : "cc"
        );
        
        asm volatile (
            "comisd %2, %1\n\t"
            "set%c0 %0"
            : "=r"(r6)
            : "x"(dvals[0]), "x"(dvals[1]), "u"(UNLE)
            : "cc"
        );
        
        asm volatile (
            "comisd %2, %1\n\t"
            "set%c0 %0"
            : "=r"(r7)
            : "x"(dvals[0]), "x"(dvals[1]), "u"(UNLT)
            : "cc"
        );
        
        asm volatile (
            "comisd %2, %1\n\t"
            "set%c0 %0"
            : "=r"(r8)
            : "x"(dvals[0]), "x"(dvals[1]), "u"(LTGT)
            : "cc"
        );
        
        accumulator += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    }
    
    printf("Final result after direct tests: %d\n", accumulator);
    
    return accumulator != 0 ? 0 : 1;
}
