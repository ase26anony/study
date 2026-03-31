#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function that performs various floating-point comparisons */
static int compare_floats_verbose(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map to x86 condition codes */
    if (isunordered(a, b)) result |= 64;
    if (isless(a, b)) result |= 128;
    if (isgreater(a, b)) result |= 256;
    if (islessequal(a, b)) result |= 512;
    if (isgreaterequal(a, b)) result |= 1024;
    if (islessgreater(a, b)) result |= 2048;
    
    return result;
}

/* Function with switch based on comparison results */
static const char* classify_comparison(double a, double b) {
    /* Force actual comparisons by using volatile intermediate */
    volatile double va = a;
    volatile double vb = b;
    
    if (isunordered(va, vb)) {
        return "unordered";
    } else if (va < vb) {
        return "less";
    } else if (va > vb) {
        return "greater";
    } else if (va == vb) {
        /* Distinguish +0 and -0 */
        if (signbit(va) != signbit(vb)) {
            return "equal_opposite_sign";
        }
        return "equal";
    }
    return "unknown";
}

/* Inline assembly that directly uses FP condition codes */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* Using x87 FPU comparison - forces condition code output */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FP stack */
        "fldl %1\n\t"           /* Load a onto FP stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%ax, %0\n\t"   /* Move result */
        "fstp %%st(0)\n\t"      /* Clean up FP stack */
        : "=r" (result)
        : "m" (a), "m" (b)
        : "eax", "cc", "st"
    );
    
    return result;
}

/* Another inline assembly variant with different condition */
static int fp_compare_asm_ordered(double a, double b) {
    int result;
    
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "setnp %%al\n\t"        /* Set if ordered (no parity) */
        "sete %%ah\n\t"         /* Set if equal (ZF=1) */
        "movzbl %%ax, %0\n\t"
        "fstp %%st(0)\n\t"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "eax", "cc", "st"
    );
    
    return result;
}

/* Vector comparisons using GCC extensions */
#ifdef USE_VECTOR
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

static void vector_comparisons(v2df a, v2df b) {
    v2df cmp_lt = a < b;    /* May generate UNLT condition */
    v2df cmp_gt = a > b;    /* May generate UNGT condition */
    v2df cmp_eq = a == b;   /* May generate UNEQ condition */
    v2df cmp_ne = a != b;   /* May generate LTGT condition */
    
    /* Use results to prevent optimization */
    volatile v2df v1 = cmp_lt;
    volatile v2df v2 = cmp_gt;
    volatile v2df v3 = cmp_eq;
    volatile v2df v4 = cmp_ne;
    (void)v1; (void)v2; (void)v3; (void)v4;
}
#endif

/* Parse double with support for special values */
static double parse_double(const char *str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NaN") == 0) {
        return NAN;
    } else if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0) {
        return INFINITY;
    } else if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0) {
        return -INFINITY;
    } else {
        return atof(str);
    }
}

int main(int argc, char *argv[]) {
    /* Test cases designed to trigger various condition codes */
    double test_cases[][2] = {
        {NAN, 1.0},           /* UNORDERED comparisons */
        {1.0, NAN},           /* UNORDERED comparisons */
        {NAN, NAN},           /* UNORDERED comparisons */
        {INFINITY, -INFINITY},/* Ordered comparisons */
        {0.0, -0.0},          /* Equal with different signs */
        {1.0, 2.0},           /* Less than */
        {2.0, 1.0},           /* Greater than */
        {1.0, 1.0},           /* Equal */
        {INFINITY, INFINITY}, /* Equal infinities */
        {-INFINITY, -INFINITY},/* Equal negative infinities */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Testing with command line values: %g, %g\n", a, b);
        
        /* Perform all types of comparisons */
        int cmp_result = compare_floats_verbose(a, b);
        const char *classification = classify_comparison(a, b);
        int asm_result1 = fp_compare_asm(a, b);
        int asm_result2 = fp_compare_asm_ordered(a, b);
        
        printf("Comparison result: 0x%x\n", cmp_result);
        printf("Classification: %s\n", classification);
        printf("Assembly result 1: 0x%x\n", asm_result1);
        printf("Assembly result 2: 0x%x\n", asm_result2);
        
        /* Check for NaN explicitly */
        if (isnan(a) || isnan(b)) {
            printf("At least one operand is NaN\n");
        }
    } else {
        /* Run built-in test cases */
        printf("Running built-in test cases...\n");
        
        for (int i = 0; i < num_cases; i++) {
            double a = test_cases[i][0];
            double b = test_cases[i][1];
            
            /* Force compiler to generate actual comparisons */
            volatile double va = a;
            volatile double vb = b;
            
            printf("\nTest case %d: %g vs %g\n", i, va, vb);
            
            /* This complex conditional should generate multiple
               different x86 condition code mnemonics */
            if (isunordered(va, vb)) {
                printf("  Unordered\n");
                if (va != vb) {  /* Always true for NaN != NaN? */
                    printf("  Not equal (unordered)\n");
                }
            } else if (va < vb) {
                printf("  Less than\n");
            } else if (va > vb) {
                printf("  Greater than\n");
            } else {
                /* Equal case */
                if (signbit(va) != signbit(vb)) {
                    printf("  Equal with opposite signs\n");
                } else {
                    printf("  Equal\n");
                }
                
                /* Additional check that may generate UNEQ */
                if (!(va != vb)) {
                    printf("  Definitely equal (not not-equal)\n");
                }
            }
            
            /* Use inline assembly for some cases */
            if (i % 3 == 0) {
                int asm_res = fp_compare_asm(va, vb);
                printf("  Assembly comparison: 0x%x\n", asm_res);
            }
        }
    }
    
#ifdef USE_VECTOR
    /* Vector comparisons if enabled */
    v2df vec_a = {1.0, NAN};
    v2df vec_b = {2.0, 1.0};
    vector_comparisons(vec_a, vec_b);
#endif
    
    /* Final summary to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < num_cases; i++) {
        final_result ^= compare_floats_verbose(test_cases[i][0], test_cases[i][1]);
    }
    
    printf("\nFinal XOR of all comparison results: 0x%x\n", final_result);
    
    return 0;
}
