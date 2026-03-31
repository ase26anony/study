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

/* Helper macro for condition codes */
#define UNORDERED 16
#define ORDERED 17
#define UNEQ 18
#define UNGE 19
#define UNGT 20
#define UNLE 21
#define UNLT 22
#define LTGT 23

/* Function prototypes */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static int test_mixed_cond(long double a, long double b, int cond) __attribute__((noinline));
static void print_cond_code(int cond, FILE *file);

/* Test functions for each condition code */
static int test_unordered(double a, double b) {
    int result;
    /* Using x87 instruction with UNORDERED condition */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "i"(UNORDERED)
        : "cc", "st"
    );
    return result;
}

static int test_ordered(double a, double b) {
    int result;
    /* Using SSE instruction with ORDERED condition */
    asm volatile (
        "comisd %1, %2\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "i"(ORDERED)
        : "cc"
    );
    return result;
}

static int test_uneq(double a, double b) {
    int result;
    /* Mixed x87/SSE approach */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "i"(UNEQ)
        : "cc", "st"
    );
    return result;
}

static int test_unge(double a, double b) {
    int result;
    /* Using UNGE condition code */
    asm volatile (
        "comisd %1, %2\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "i"(UNGE)
        : "cc"
    );
    return result;
}

static int test_ungt(double a, double b) {
    int result;
    /* Using UNGT condition code */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "i"(UNGT)
        : "cc", "st"
    );
    return result;
}

static int test_unle(double a, double b) {
    int result;
    /* Using UNLE condition code */
    asm volatile (
        "comisd %1, %2\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "i"(UNLE)
        : "cc"
    );
    return result;
}

static int test_unlt(double a, double b) {
    int result;
    /* Using UNLT condition code */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "i"(UNLT)
        : "cc", "st"
    );
    return result;
}

static int test_ltgt(double a, double b) {
    int result;
    /* Using LTGT condition code */
    asm volatile (
        "comisd %1, %2\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "i"(LTGT)
        : "cc"
    );
    return result;
}

/* Function that uses a switch to select condition codes */
static int test_mixed_cond(long double a, long double b, int cond) {
    int result = 0;
    
    /* Complex control flow to obscure constant folding */
    switch (cond) {
        case UNORDERED:
            asm volatile (
                "fldt %1\n\t"
                "fldt %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c3 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "i"(UNORDERED)
                : "cc", "st"
            );
            break;
            
        case ORDERED:
            /* Convert to double for SSE */
            double da = (double)a;
            double db = (double)b;
            asm volatile (
                "comisd %1, %2\n\t"
                "set%c3 %0"
                : "=r"(result)
                : "x"(da), "x"(db), "i"(ORDERED)
                : "cc"
            );
            break;
            
        case UNEQ:
            asm volatile (
                "fldt %1\n\t"
                "fldt %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c3 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "i"(UNEQ)
                : "cc", "st"
            );
            break;
            
        default:
            /* This might trigger the default case in output_operand_lossage */
            asm volatile (
                "fldt %1\n\t"
                "fldt %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c3 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "i"(cond)  /* Dynamic condition code */
                : "cc", "st"
            );
            break;
    }
    
    return result;
}

/* Function that attempts to trigger the printing logic */
static void print_cond_code(int cond, FILE *file) {
    /* This function mimics the structure that would cause 
       the compiler to generate the condition code printing logic */
    const char *code_str = NULL;
    
    /* Switch similar to the one in i386.cc */
    switch (cond) {
        case UNORDERED: code_str = "unord"; break;
        case ORDERED:   code_str = "ord"; break;
        case UNEQ:      code_str = "ueq"; break;
        case UNGE:      code_str = "nlt"; break;
        case UNGT:      code_str = "nle"; break;
        case UNLE:      code_str = "ule"; break;
        case UNLT:      code_str = "ult"; break;
        case LTGT:      code_str = "une"; break;
        default:
            /* This should trigger output_operand_lossage */
            asm volatile ("# Invalid condition code: %c0" : : "i"(cond));
            code_str = "invalid";
            break;
    }
    
    if (code_str && file) {
        fprintf(file, "%s", code_str);
    }
}

int main(int argc, char *argv[]) {
    volatile int accumulator = 0;
    int iterations = 100;
    
    /* Parse iterations from command line if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize arrays with volatile values */
    double d_array[8];
    long double ld_array[8];
    
    for (int i = 0; i < 8; i++) {
        d_array[i] = g_d1 + i * 0.5;
        ld_array[i] = g_ld1 + i * 0.5L;
    }
    
    /* Main test loop */
    for (int i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Test all condition codes */
        accumulator += test_unordered(d_array[idx], d_array[(idx + 1) % 8]);
        accumulator += test_ordered(d_array[idx], d_array[(idx + 2) % 8]);
        accumulator += test_uneq(d_array[idx], d_array[(idx + 3) % 8]);
        accumulator += test_unge(d_array[idx], d_array[(idx + 4) % 8]);
        accumulator += test_ungt(d_array[idx], d_array[(idx + 5) % 8]);
        accumulator += test_unle(d_array[idx], d_array[(idx + 6) % 8]);
        accumulator += test_unlt(d_array[idx], d_array[(idx + 7) % 8]);
        accumulator += test_ltgt(d_array[idx], d_array[idx]);
        
        /* Test with long doubles */
        accumulator += test_mixed_cond(ld_array[idx], ld_array[(idx + 1) % 8], 
                                      UNORDERED + (i % 8));
        
        /* Mix with regular C comparisons to provide context */
        if (d_array[idx] != d_array[(idx + 1) % 8]) {
            accumulator += test_unordered(d_array[idx], d_array[(idx + 1) % 8]);
        }
        
        if (d_array[idx] >= d_array[(idx + 2) % 8]) {
            accumulator += test_ordered(d_array[idx], d_array[(idx + 2) % 8]);
        }
        
        /* Use volatile selector to control condition code */
        g_selector = (g_selector + 1) % 10;
        int cond = UNORDERED + (g_selector % 9);  /* May go out of range */
        accumulator += test_mixed_cond(ld_array[idx], ld_array[(idx + 3) % 8], cond);
    }
    
    /* Force printing of condition codes */
    printf("Testing condition code printing:\n");
    for (int cond = UNORDERED; cond <= LTGT + 1; cond++) {  /* +1 to include invalid */
        printf("Condition %d: ", cond);
        print_cond_code(cond, stdout);
        printf("\n");
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulator result: %d\n", accumulator);
    
    /* Additional test with NaN values */
    double nan_val = 0.0 / 0.0;
    double inf_val = 1.0 / 0.0;
    
    printf("\nTesting with special values:\n");
    printf("NaN unordered test: %d\n", test_unordered(nan_val, 1.0));
    printf("Inf ordered test: %d\n", test_ordered(inf_val, 1.0));
    
    return accumulator != 0 ? 0 : 1;
}
