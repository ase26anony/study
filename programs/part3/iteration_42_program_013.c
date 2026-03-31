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

/* Function prototypes for condition code tests */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static int test_mixed_fpu(double a, long double b) __attribute__((noinline));
static void use_cond_code_in_switch(int cc) __attribute__((noinline));

/* Test UNORDERED condition code */
static int test_unordered(double a, double b) {
    int result;
    /* Using x87 instruction with UNORDERED condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNORDERED), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

/* Test ORDERED condition code */
static int test_ordered(double a, double b) {
    int result;
    /* Using SSE instruction with ORDERED condition */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(ORDERED), "x"(a), "x"(b)
        : "cc"
    );
    return result;
}

/* Test UNEQ condition code */
static int test_uneq(double a, double b) {
    int result;
    /* Mix x87 and regular comparison */
    if (a != b) {
        asm volatile (
            "fldl %2\n\t"
            "fldl %3\n\t"
            "fucomip %%st(1), %%st(0)\n\t"
            "set%c0 %0\n\t"
            "fstp %%st(0)"
            : "=r"(result)
            : "u"(UNEQ), "m"(a), "m"(b)
            : "cc", "st"
        );
    } else {
        result = 0;
    }
    return result;
}

/* Test UNGE condition code */
static int test_unge(double a, double b) {
    int result;
    /* Using UNGE (not less than) */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNGE), "x"(a), "x"(b)
        : "cc"
    );
    return result;
}

/* Test UNGT condition code */
static int test_ungt(double a, double b) {
    int result;
    /* Using UNGT (not less or equal) with long double */
    long double la = (long double)a;
    long double lb = (long double)b;
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNGT), "m"(la), "m"(lb)
        : "cc", "st"
    );
    return result;
}

/* Test UNLE condition code */
static int test_unle(double a, double b) {
    int result;
    /* Using UNLE */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(UNLE), "x"(a), "x"(b)
        : "cc"
    );
    return result;
}

/* Test UNLT condition code */
static int test_unlt(double a, double b) {
    int result;
    /* Using UNLT */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNLT), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

/* Test LTGT condition code */
static int test_ltgt(double a, double b) {
    int result;
    /* Using LTGT (unordered or not equal) */
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "u"(LTGT), "x"(a), "x"(b)
        : "cc"
    );
    return result;
}

/* Test mixed FPU operations */
static int test_mixed_fpu(double a, long double b) {
    int result1, result2;
    
    /* First with x87 UNORDERED */
    asm volatile (
        "fldl %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result1)
        : "u"(UNORDERED), "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* Then with SSE ORDERED */
    double d = (double)b;
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c0 %1"
        : "=r"(result2)
        : "u"(ORDERED), "x"(a), "x"(d)
        : "cc"
    );
    
    return result1 | result2;
}

/* Function that uses condition code in a switch - may trigger printing */
static void use_cond_code_in_switch(int cc) {
    int result = 0;
    double a = g_d1;
    double b = g_d2;
    
    switch (cc) {
        case 0:  /* UNORDERED */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNORDERED), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 1:  /* ORDERED */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(ORDERED), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 2:  /* UNEQ */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNEQ), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 3:  /* UNGE */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNGE), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 4:  /* UNGT */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNGT), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 5:  /* UNLE */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNLE), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 6:  /* UNLT */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(UNLT), "x"(a), "x"(b)
                : "cc"
            );
            break;
        case 7:  /* LTGT */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(LTGT), "x"(a), "x"(b)
                : "cc"
            );
            break;
        default:
            /* This might trigger output_operand_lossage for invalid condition code */
            asm volatile (
                "comisd %2, %3\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "u"(cc), "x"(a), "x"(b)  /* Invalid condition code if cc > 7 */
                : "cc"
            );
            break;
    }
    
    /* Use result to prevent optimization */
    g_selector = result;
}

int main(int argc, char *argv[]) {
    int i, iterations;
    volatile int total = 0;  /* Prevent optimization */
    
    /* Parse iterations from command line or use default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    /* Initialize test values from volatile globals */
    double dvals[4];
    long double ldvals[4];
    
    dvals[0] = g_d1;
    dvals[1] = g_d2;
    dvals[2] = NAN;
    dvals[3] = INFINITY;
    
    ldvals[0] = g_ld1;
    ldvals[1] = g_ld2;
    ldvals[2] = NAN * 1.0L;
    ldvals[3] = INFINITY * 1.0L;
    
    printf("Testing i386 condition code printing (targeting lines 13992-14017)\n");
    printf("Iterations: %d\n", iterations);
    
    for (i = 0; i < iterations; i++) {
        int idx = i % 4;
        int idx2 = (i + 1) % 4;
        
        /* Call all condition code test functions */
        total += test_unordered(dvals[idx], dvals[idx2]);
        total += test_ordered(dvals[idx], dvals[idx2]);
        total += test_uneq(dvals[idx], dvals[idx2]);
        total += test_unge(dvals[idx], dvals[idx2]);
        total += test_ungt(dvals[idx], dvals[idx2]);
        total += test_unle(dvals[idx], dvals[idx2]);
        total += test_unlt(dvals[idx], dvals[idx2]);
        total += test_ltgt(dvals[idx], dvals[idx2]);
        
        /* Test mixed FPU operations */
        total += test_mixed_fpu(dvals[idx], ldvals[idx2]);
        
        /* Use switch with potentially invalid condition codes */
        use_cond_code_in_switch(i % 9);  /* 8 valid + 1 invalid */
        
        /* Mix with regular C comparisons for context */
        if (dvals[idx] >= dvals[idx2]) {
            total += test_ordered(dvals[idx], dvals[idx2]);
        }
        
        if (dvals[idx] != dvals[idx2]) {
            total += test_uneq(dvals[idx], dvals[idx2]);
        }
    }
    
    printf("Total accumulated: %d\n", total);
    
    /* Force printing of condition codes via inline asm with %c modifier */
    printf("\nForcing condition code printing via inline assembly:\n");
    
    /* Direct inline asm that should trigger the printing routines */
    asm volatile (
        "# Condition code test block\n"
        "mov $0, %%eax\n"
        "comisd %1, %2\n"
        "set%c0 %%al\n"
        : "=a"(total)
        : "u"(UNORDERED), "x"(dvals[0]), "x"(dvals[1])
        : "cc"
    );
    
    asm volatile (
        "mov $0, %%eax\n"
        "comisd %1, %2\n"
        "set%c0 %%al\n"
        : "=a"(total)
        : "u"(ORDERED), "x"(dvals[0]), "x"(dvals[1])
        : "cc"
    );
    
    asm volatile (
        "mov $0, %%eax\n"
        "comisd %1, %2\n"
        "set%c0 %%al\n"
        : "=a"(total)
        : "u"(UNEQ), "x"(dvals[0]), "x"(dvals[1])
        : "cc"
    );
    
    asm volatile (
        "mov $0, %%eax\n"
        "comisd %1, %2\n"
        "set%c0 %%al\n"
        : "=a"(total)
        : "u"(UNGE), "x"(dvals[0]), "x"(dvals[1])
        : "cc"
    );
    
    asm volatile (
        "mov $0, %%eax\n"
        "comisd %1, %2\n"
        "set%c0 %%al\n"
        : "=a"(total)
        : "u"(UNGT), "x"(dvals[0]), "x"(dvals[1])
        : "cc"
    );
    
    asm volatile (
        "mov $0, %%eax\n"
        "comisd %1, %2\n"
        "set%c0 %%al\n"
        : "=a"(total)
        : "u"(UNLE), "x"(dvals[0]), "x"(dvals[1])
        : "cc"
    );
    
    asm volatile (
        "mov $0, %%eax\n"
        "comisd %1, %2\n"
        "set%c0 %%al\n"
        : "=a"(total)
        : "u"(UNLT), "x"(dvals[0]), "x"(dvals[1])
        : "cc"
    );
    
    asm volatile (
        "mov $0, %%eax\n"
        "comisd %1, %2\n"
        "set%c0 %%al\n"
        : "=a"(total)
        : "u"(LTGT), "x"(dvals[0]), "x"(dvals[1])
        : "cc"
    );
    
    printf("Final total: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
