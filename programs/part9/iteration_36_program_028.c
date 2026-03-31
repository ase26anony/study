/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_zero = 0.0;

/* Vector type for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function that performs all possible floating-point comparisons */
static int fp_compare_all(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered condition codes */
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
    /* This switch should generate various condition code outputs */
    if (isunordered(a, b)) {
        return "unordered";
    } else if (a < b) {
        return "less";
    } else if (a > b) {
        return "greater";
    } else if (a == b) {
        /* Distinguish +0 and -0 */
        if (signbit(a) != signbit(b)) {
            return "equal_opposite_sign";
        }
        return "equal";
    } else {
        /* Should never reach here for valid FP numbers */
        return "impossible";
    }
}

/* Inline assembly to directly trigger condition code output */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* x87 floating-point compare and condition code test */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clear FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%ax, %0\n\t"   /* Move result */
        : "=r" (result)
        : "m" (a), "m" (b)
        : "ax", "cc", "st", "st(1)"
    );
    
    return result;
}

/* More complex inline assembly with multiple condition codes */
static void test_condition_codes(double a, double b) {
    volatile int unordered, equal, less, greater;
    
    /* Test UNORDERED condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "setp %0\n\t"
        : "=r" (unordered)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)"
    );
    
    /* Test ORDERED condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "setnp %0\n\t"
        : "=r" (equal)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)"
    );
    
    /* Test for UNEQ (unordered or equal) */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "setpe %0\n\t"          /* parity or equal */
        : "=r" (less)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)"
    );
    
    /* Test for LTGT (less or greater, ordered) */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "setne %0\n\t"          /* not equal and ordered */
        : "=r" (greater)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)"
    );
    
    /* Prevent dead code elimination */
    if (unordered || equal || less || greater) {
        printf("Condition codes: u=%d e=%d l=%d g=%d\n", 
               unordered, equal, less, greater);
    }
}

/* Vector comparison test */
static void test_vector_comparisons(void) {
    v2df vec1 = { global_nan, 1.0 };
    v2df vec2 = { 2.0, global_nan };
    v2df vec3 = { 0.0, -0.0 };
    v2df vec4 = { global_inf, -global_inf };
    
    /* These vector comparisons may generate condition code output */
    v2df cmp1 = vec1 < vec2;  /* Should involve NaN comparisons */
    v2df cmp2 = vec3 == vec4; /* Infinity comparisons */
    v2df cmp3 = vec1 > vec3;  /* Mixed NaN/normal comparisons */
    
    /* Use results to prevent optimization */
    volatile double* p = (double*)&cmp1;
    if (p[0] || p[1]) {
        printf("Vector comparison results: %f %f\n", p[0], p[1]);
    }
}

/* Parse double with NaN support */
static double parse_fp_arg(const char* arg) {
    if (strcmp(arg, "nan") == 0 || strcmp(arg, "NaN") == 0) {
        return NAN;
    } else if (strcmp(arg, "inf") == 0 || strcmp(arg, "INF") == 0) {
        return INFINITY;
    } else if (strcmp(arg, "-inf") == 0 || strcmp(arg, "-INF") == 0) {
        return -INFINITY;
    } else {
        return atof(arg);
    }
}

int main(int argc, char** argv) {
    /* Test cases covering various FP scenarios */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, 1.0},
        {1.0, -INFINITY},
        {0.0, -0.0},
        {INFINITY, -INFINITY},
        {3.14159, 2.71828},
        {-1.0, 1.0},
        {0.0, NAN}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_fp_arg(argv[1]);
        double b = parse_fp_arg(argv[2]);
        
        printf("Testing with command-line values: %f %f\n", a, b);
        
        /* Trigger all comparison methods */
        int result = fp_compare_all(a, b);
        const char* classification = classify_comparison(a, b);
        int asm_result = fp_compare_asm(a, b);
        
        printf("Result: 0x%x, Class: %s, ASM: 0x%x\n", 
               result, classification, asm_result);
        
        test_condition_codes(a, b);
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Mix volatile and non-volatile to force code generation */
        volatile double va = a;
        volatile double vb = b;
        
        /* Perform comparisons that should generate condition codes */
        int cmp_result = fp_compare_all(va, vb);
        
        /* Use inline assembly for direct condition code generation */
        int asm_cmp = fp_compare_asm(va, vb);
        
        /* Complex conditional based on comparison results */
        if (isunordered(va, vb)) {
            printf("Case %d: Unordered (NaN involved)\n", i);
        } else if (isless(va, vb)) {
            printf("Case %d: Less\n", i);
        } else if (isgreater(va, vb)) {
            printf("Case %d: Greater\n", i);
        } else if (va == vb) {
            printf("Case %d: Equal\n", i);
        }
        
        /* Prevent dead code elimination */
        if (cmp_result || asm_cmp) {
            /* Do nothing, just reference variables */
        }
    }
    
    /* Test vector comparisons */
    test_vector_comparisons();
    
    /* Additional test with runtime-generated NaN */
    double dynamic_nan = nan("");
    double normal = 42.0;
    
    /* This should generate UNORDERED/ORDERED condition codes */
    if (dynamic_nan < normal) {
        printf("Unexpected: NaN < normal\n");
    }
    if (normal > dynamic_nan) {
        printf("Unexpected: normal > NaN\n");
    }
    if (isunordered(dynamic_nan, normal)) {
        printf("Expected: NaN and normal are unordered\n");
    }
    
    /* Test UNEQ (unordered or equal) scenario */
    if (!(dynamic_nan == normal) && !(dynamic_nan != normal)) {
        /* Both comparisons false when NaN is involved */
        printf("NaN comparison paradox\n");
    }
    
    /* Final summary to use all results */
    printf("\nTest completed. Check generated assembly for condition code output.\n");
    
    return 0;
}
