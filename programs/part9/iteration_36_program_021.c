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
    
    /* <math.h> macros that map to x86 unordered comparisons */
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
    /* Force actual comparison by using volatile */
    volatile double va = a;
    volatile double vb = b;
    
    if (isunordered(va, vb)) {
        return "unordered";
    } else if (va < vb) {
        return "less";
    } else if (va > vb) {
        return "greater";
    } else if (va == vb) {
        return "equal";
    }
    return "unknown";
}

/* Inline assembly to directly trigger condition code output */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare with unordered handling */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%ax, %0\n\t"   /* Move result to output */
        "fstp %%st(0)\n\t"      /* Clean up FPU stack */
        : "=r" (result)
        : "m" (a), "m" (b)
        : "eax", "cc", "st"
    );
    
    return result;
}

/* Another inline assembly variant with different condition codes */
static int fp_compare_asm2(double a, double b) {
    int result;
    
    /* Using different condition code tests */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "sete %%al\n\t"         /* Set if equal (ZF=1) */
        "setbe %%ah\n\t"        /* Set if below or equal (CF=1 or ZF=1) */
        "movzbl %%ax, %0\n\t"
        "fstp %%st(0)\n\t"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "eax", "cc", "st"
    );
    
    return result;
}

/* Vector extension comparisons */
#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));

static v2df vector_compare(v2df a, v2df b) {
    /* These comparisons may generate multiple condition codes */
    v2df cmp_lt = a < b;
    v2df cmp_gt = a > b;
    v2df cmp_eq = a == b;
    v2df cmp_ne = a != b;
    
    /* Combine results */
    return cmp_lt + cmp_gt * 2 + cmp_eq * 4 + cmp_ne * 8;
}
#endif

/* Parse double with NaN support */
static double parse_double(const char* str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NAN") == 0) {
        return NAN;
    } else if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0) {
        return INFINITY;
    } else if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0) {
        return -INFINITY;
    } else {
        return atof(str);
    }
}

int main(int argc, char** argv) {
    /* Test cases including NaN values */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, -INFINITY},
        {0.0, -0.0},
        {1.0, 2.0},
        {2.0, 1.0},
        {1.0, 1.0},
        {INFINITY, INFINITY},
        {-INFINITY, -INFINITY}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Testing with command-line values: %g, %g\n", a, b);
        
        /* Force all comparison types */
        int result = compare_floats_verbose(a, b);
        const char* classification = classify_comparison(a, b);
        int asm_result = fp_compare_asm(a, b);
        int asm_result2 = fp_compare_asm2(a, b);
        
        printf("Comparison result: 0x%x\n", result);
        printf("Classification: %s\n", classification);
        printf("ASM result 1: 0x%x\n", asm_result);
        printf("ASM result 2: 0x%x\n", asm_result2);
        
        /* Prevent dead code elimination */
        volatile int dummy = result + asm_result + asm_result2;
        (void)dummy;
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Mix of comparison methods */
        int cmp_result = compare_floats_verbose(a, b);
        const char* class_str = classify_comparison(a, b);
        
        printf("Test %d: %g vs %g -> result=0x%03x, class=%s\n",
               i, a, b, cmp_result, class_str);
        
        /* Use inline assembly for some cases to trigger different condition codes */
        if (i % 3 == 0) {
            int asm_res = fp_compare_asm(a, b);
            printf("  ASM1: 0x%x\n", asm_res);
        }
        if (i % 4 == 0) {
            int asm_res = fp_compare_asm2(a, b);
            printf("  ASM2: 0x%x\n", asm_res);
        }
    }
    
#ifdef __SSE2__
    /* Vector comparisons */
    printf("\nTesting vector comparisons:\n");
    v2df vec_a = {1.0, NAN};
    v2df vec_b = {NAN, 2.0};
    v2df vec_result = vector_compare(vec_a, vec_b);
    
    double* res_ptr = (double*)&vec_result;
    printf("Vector result: [%g, %g]\n", res_ptr[0], res_ptr[1]);
#endif
    
    /* Complex control flow with many comparisons */
    printf("\nComplex comparison chain:\n");
    volatile double x = global_nan;
    volatile double y = 3.14;
    
    if (x < y || x > y || x <= y || x >= y || x == y || x != y) {
        printf("Direct comparisons with NaN\n");
    }
    
    if (isunordered(x, y)) {
        printf("isunordered triggered\n");
    }
    if (isless(x, y)) {
        printf("isless triggered\n");
    }
    if (isgreater(x, y)) {
        printf("isgreater triggered\n");
    }
    if (islessequal(x, y)) {
        printf("islessequal triggered\n");
    }
    if (isgreaterequal(x, y)) {
        printf("isgreaterequal triggered\n");
    }
    if (islessgreater(x, y)) {
        printf("islessgreater triggered\n");
    }
    
    /* Switch-like behavior based on fpclassify */
    int a_class = fpclassify(x);
    int b_class = fpclassify(y);
    
    switch (a_class) {
        case FP_NAN:
            printf("x is NaN\n");
            break;
        case FP_INFINITE:
            printf("x is infinite\n");
            break;
        case FP_ZERO:
            printf("x is zero\n");
            break;
        case FP_SUBNORMAL:
            printf("x is subnormal\n");
            break;
        case FP_NORMAL:
            printf("x is normal\n");
            break;
    }
    
    /* Final summary to prevent optimization */
    volatile int final_result = 0;
    for (int i = 0; i < num_cases; i++) {
        final_result += compare_floats_verbose(test_cases[i][0], test_cases[i][1]);
    }
    
    printf("\nFinal accumulated result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
