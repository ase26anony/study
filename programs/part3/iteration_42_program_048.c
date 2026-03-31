/* i386_condition_codes.c - Target coverage for x86 condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile globals to prevent constant propagation */
volatile double g_d1 = 1.0, g_d2 = 2.0;
volatile long double g_ld1 = 3.0L, g_ld2 = 4.0L;
volatile int g_selector = 0;

/* Condition code constants matching i386.h */
enum x86_cc {
    UNORDERED = 0,
    ORDERED = 1,
    UNEQ = 2,
    UNGE = 3,
    UNGT = 4,
    UNLE = 5,
    UNLT = 6,
    LTGT = 7,
    /* Add invalid code to potentially trigger default case */
    INVALID_CC = 99
};

/* Prevent inlining to ensure separate RTL generation */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* Using x87 instruction with UNORDERED condition code */
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

__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
    /* SSE comparison with ORDERED condition */
    asm volatile (
        "comisd %2, %1\n\t"
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
    /* x87 long double comparison with UNEQ */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
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
    /* Mixed approach with UNGE */
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
    /* x87 with UNGT */
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
    /* SSE with UNLE */
    asm volatile (
        "comisd %2, %1\n\t"
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
    /* x87 with UNLT */
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

__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    /* SSE with LTGT */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(LTGT)
        : "cc"
    );
    return result;
}

/* Function that uses a switch to select condition codes */
__attribute__((noinline))
static int dispatch_condition_code(int cc, double a, double b) {
    int result = 0;
    
    /* This switch may cause the compiler to generate 
       condition code operands in RTL */
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
            /* This might trigger output_operand_lossage if 
               an invalid code reaches the RTL output phase */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cc)  /* Potentially invalid! */
                : "cc"
            );
            break;
    }
    return result;
}

/* Helper to generate NaN values */
static double make_nan(void) {
    union {
        uint64_t i;
        double d;
    } u;
    u.i = 0x7FF8000000000001ULL; /* Quiet NaN */
    return u.d;
}

int main(int argc, char *argv[]) {
    volatile int accum = 0;
    int iterations = 100;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create some special floating-point values */
    double nan_val = make_nan();
    double inf_val = 1.0 / 0.0;  /* Infinity */
    double normal_vals[] = {0.0, 1.0, -1.0, 2.5, -2.5};
    long double ld_vals[] = {0.0L, 1.0L, -1.0L, 3.14L, -3.14L};
    
    /* Loop to prevent optimization and create dynamic behavior */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile globals to prevent constant folding */
        double a = g_d1 + i * 0.1;
        double b = g_d2 + i * 0.2;
        long double la = g_ld1 + i * 0.1L;
        long double lb = g_ld2 + i * 0.2L;
        
        /* Test all condition codes with various value combinations */
        accum += test_unordered(a, b);
        accum += test_ordered(a, nan_val);  /* Compare with NaN */
        accum += test_uneq(la, lb);
        accum += test_unge(a, inf_val);     /* Compare with infinity */
        accum += test_ungt(la, la + 1.0L);  /* Different values */
        accum += test_unle(b, a);           /* Reversed order */
        accum += test_unlt(la, lb);
        accum += test_ltgt(a, b);
        
        /* Use dispatch function with dynamic selector */
        int selector = g_selector + i;
        accum += dispatch_condition_code(selector % 9, a, b); /* 8 valid + 1 for default */
        
        /* Mix with regular C comparisons to provide context */
        if (a != b) accum++;
        if (a >= b) accum--;
        
        /* Occasionally use special values */
        if (i % 7 == 0) {
            accum += test_unordered(nan_val, normal_vals[i % 5]);
            accum += test_ordered(inf_val, -inf_val);
        }
        
        /* Force compiler to consider all condition codes */
        switch (i % 8) {
            case 0: accum += test_unordered(a, nan_val); break;
            case 1: accum += test_ordered(b, b); break;
            case 2: accum += test_uneq(la, la); break;
            case 3: accum += test_unge(a, b); break;
            case 4: accum += test_ungt(la, lb); break;
            case 5: accum += test_unle(b, a); break;
            case 6: accum += test_unlt(la, la - 1.0L); break;
            case 7: accum += test_ltgt(a, a + 0.5); break;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulated result: %d\n", accum);
    
    /* Try to trigger default case with invalid condition code */
    if (argc > 2 && strcmp(argv[2], "test-default") == 0) {
        /* This might cause output_operand_lossage if the compiler
           doesn't catch the invalid constant at compile time */
        int dummy;
        asm volatile (
            "comisd %1, %1\n\t"
            "set%c0 %0"
            : "=r"(dummy)
            : "x"(g_d1), "u"(INVALID_CC)
            : "cc"
        );
        printf("Dummy with invalid CC: %d\n", dummy);
    }
    
    return accum != 0 ? 0 : 1;
}
