/* Compile with: gcc -O2 -mfpmath=387 -march=i686 -masm=intel -S -o output.s this_file.c */
/* Also try: gcc -O3 -mfpmath=both -march=core2 -ffast-math -fverbose-asm */
/* And: gcc -O1 -m32 -fno-omit-frame-pointer -da */

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

/* Volatile globals to prevent constant propagation */
static volatile int cond_selector = 0;
static volatile double volatile_dbl = 3.14159;
static volatile long double volatile_ldbl = 2.71828L;

/* Function prototypes */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static void use_cond_code(int cc, double a, double b) __attribute__((noinline));

/* Individual test functions for each condition code */
static int test_unordered(double a, double b) {
    int result;
    /* Using x87 fucomip with UNORDERED condition */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNORDERED), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

static int test_ordered(double a, double b) {
    int result;
    /* Mix x87 and regular comparison */
    if (!isnan(a) && !isnan(b)) {
        asm volatile (
            "fcomip %%st(1), %%st(0)\n\t"
            "set%c0 %1"
            : "=u"(ORDERED), "=r"(result)
            : "t"(a), "u"(b)
            : "cc", "st"
        );
    } else {
        result = 0;
    }
    return result;
}

static int test_uneq(double a, double b) {
    int result;
    /* Using UNEQ condition code */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNEQ), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

static int test_unge(double a, double b) {
    int result;
    /* Using UNGE condition code with SSE for variety */
    double cmp_result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result), "=x"(cmp_result)
        : "x"(b), "1"(a), "u"(UNGE)
        : "cc"
    );
    return result;
}

static int test_ungt(double a, double b) {
    int result;
    /* Using UNGT condition code */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNGT), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

static int test_unle(double a, double b) {
    int result;
    /* Using UNLE condition code */
    asm volatile (
        "fcomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNLE), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

static int test_unlt(double a, double b) {
    int result;
    /* Using UNLT condition code with long double */
    long double la = a, lb = b;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNLT), "=r"(result)
        : "t"(la), "u"(lb)
        : "cc", "st"
    );
    return result;
}

static int test_ltgt(double a, double b) {
    int result;
    /* Using LTGT condition code */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(LTGT), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

/* Function that uses condition code from parameter - may trigger printing */
static void use_cond_code(int cc, double a, double b) {
    int result;
    
    /* Switch to potentially confuse optimizer */
    switch (cc & 7) {  /* Mask to 0-7 range */
        case UNORDERED:
            asm volatile (
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %1"
                : "=u"(cc), "=r"(result)
                : "t"(a), "u"(b)
                : "cc", "st"
            );
            break;
        case ORDERED:
            asm volatile (
                "fcomip %%st(1), %%st(0)\n\t"
                "set%c0 %1"
                : "=u"(cc), "=r"(result)
                : "t"(a), "u"(b)
                : "cc", "st"
            );
            break;
        case UNEQ:
            asm volatile (
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %1"
                : "=u"(cc), "=r"(result)
                : "t"(a), "u"(b)
                : "cc", "st"
            );
            break;
        case UNGE:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result), "=x"(a)
                : "x"(b), "1"(a), "u"(cc)
                : "cc"
            );
            break;
        case UNGT:
            asm volatile (
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %1"
                : "=u"(cc), "=r"(result)
                : "t"(a), "u"(b)
                : "cc", "st"
            );
            break;
        case UNLE:
            asm volatile (
                "fcomip %%st(1), %%st(0)\n\t"
                "set%c0 %1"
                : "=u"(cc), "=r"(result)
                : "t"(a), "u"(b)
                : "cc", "st"
            );
            break;
        case UNLT:
            asm volatile (
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %1"
                : "=u"(cc), "=r"(result)
                : "t"(a), "u"(b)
                : "cc", "st"
            );
            break;
        case LTGT:
            asm volatile (
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %1"
                : "=u"(cc), "=r"(result)
                : "t"(a), "u"(b)
                : "cc", "st"
            );
            break;
        default:
            /* This might trigger output_operand_lossage if cc is out of range */
            asm volatile (
                "set%c0 %0"
                : "=r"(result)
                : "u"(cc)  /* Invalid condition code if cc > 7 */
                : "cc"
            );
            break;
    }
    
    /* Use result to prevent elimination */
    cond_selector += result;
}

int main(int argc, char *argv[]) {
    volatile int sum = 0;
    int i, iterations;
    
    /* Get iterations from command line or use default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    /* Create array of test values including special FP values */
    double test_values[] = {
        0.0, 1.0, -1.0, 3.14159, NAN, INFINITY, -INFINITY,
        volatile_dbl, 1.0/0.0, -1.0/0.0, 0.0/0.0
    };
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Also use long double values */
    long double ld_values[] = {0.0L, 1.0L, volatile_ldbl, NAN, INFINITY};
    
    printf("Testing condition code printing with %d iterations\n", iterations);
    
    for (i = 0; i < iterations; i++) {
        int idx1 = i % num_values;
        int idx2 = (i + 1) % num_values;
        double a = test_values[idx1];
        double b = test_values[idx2];
        
        /* Call all test functions */
        sum += test_unordered(a, b);
        sum += test_ordered(a, b);
        sum += test_uneq(a, b);
        sum += test_unge(a, b);
        sum += test_ungt(a, b);
        sum += test_unle(a, b);
        sum += test_unlt(a, b);
        sum += test_ltgt(a, b);
        
        /* Use volatile selector to choose condition code */
        cond_selector = (cond_selector * 1103515245 + 12345) & 0x7fffffff;
        int cc = cond_selector % 9;  /* 0-8, where 8 is invalid */
        
        /* This call may trigger the default case with invalid cc=8 */
        use_cond_code(cc, a, b);
        
        /* Mix in some regular comparisons */
        if (a != b) sum++;
        if (a >= b) sum++;
        if (isunordered(a, b)) sum++;
        
        /* Use long double occasionally */
        if (i % 7 == 0) {
            long double la = ld_values[idx1 % 5];
            long double lb = ld_values[idx2 % 5];
            int temp;
            asm volatile (
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %1"
                : "=u"(UNORDERED), "=r"(temp)
                : "t"(la), "u"(lb)
                : "cc", "st"
            );
            sum += temp;
        }
    }
    
    /* Force use of all condition codes one more time in a predictable way */
    for (int cc = 0; cc <= 8; cc++) {  /* Include invalid 8 */
        double a = (cc * 1.5) + 0.1;
        double b = (cc * 1.5) - 0.1;
        
        /* Direct inline asm with each condition code */
        int result;
        switch (cc) {
            case 0: asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNORDERED) : "cc"); break;
            case 1: asm volatile ("set%c0 %0" : "=r"(result) : "u"(ORDERED) : "cc"); break;
            case 2: asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNEQ) : "cc"); break;
            case 3: asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNGE) : "cc"); break;
            case 4: asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNGT) : "cc"); break;
            case 5: asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNLE) : "cc"); break;
            case 6: asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNLT) : "cc"); break;
            case 7: asm volatile ("set%c0 %0" : "=r"(result) : "u"(LTGT) : "cc"); break;
            case 8: 
                /* Invalid condition code - may trigger output_operand_lossage */
                asm volatile ("set%c0 %0" : "=r"(result) : "u"(cc) : "cc");
                break;
        }
        sum += result;
    }
    
    printf("Final sum: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
