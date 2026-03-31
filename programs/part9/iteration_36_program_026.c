#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function that performs all possible unordered comparisons */
static int compare_all_conditions(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map directly to x86 unordered predicates */
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
    /* Force actual comparison by using volatile intermediate */
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
static int inline_asm_fp_compare(double a, double b) {
    int result;
    
    /* Using x87 FPU comparison - forces generation of condition codes */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto x87 stack */
        "fldl %1\n\t"           /* Load a onto x87 stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%ax, %0\n\t"   /* Move result */
        "fstp %%st(0)\n\t"      /* Clean up x87 stack */
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "ax"
    );
    
    return result;
}

/* Vector comparisons using GCC extensions */
#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));

static v2df vector_compare(v2df a, v2df b) {
    /* This may generate multiple comparison instructions */
    v2df cmp_lt = a < b;
    v2df cmp_gt = a > b;
    v2df cmp_eq = a == b;
    
    /* Combine results */
    return cmp_lt + cmp_gt * 2.0 + cmp_eq * 4.0;
}
#endif

/* Parse double with NaN support */
static double parse_double(const char* str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NaN") == 0) {
        return NAN;
    } else if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0) {
        return INFINITY;
    } else if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0) {
        return -INFINITY;
    }
    return atof(str);
}

int main(int argc, char** argv) {
    /* Test cases designed to trigger various condition codes */
    double test_cases[][2] = {
        {NAN, 1.0},           /* Unordered comparisons */
        {1.0, NAN},           /* Unordered comparisons */
        {NAN, NAN},           /* Both NaN */
        {INFINITY, -INFINITY},/* Ordered: greater */
        {INFINITY, INFINITY}, /* Equal */
        {-INFINITY, INFINITY},/* Ordered: less */
        {0.0, -0.0},          /* Equal (even though signs differ) */
        {1.0, 2.0},           /* Ordered: less */
        {2.0, 1.0},           /* Ordered: greater */
        {1.0, 1.0},           /* Equal */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Testing with user values: %g, %g\n", a, b);
        
        /* Force all comparison types */
        int result = compare_all_conditions(a, b);
        const char* classification = classify_comparison(a, b);
        int asm_result = inline_asm_fp_compare(a, b);
        
        printf("Comparison result: 0x%x\n", result);
        printf("Classification: %s\n", classification);
        printf("Assembly result: 0x%x\n", asm_result);
        
        /* Prevent dead code elimination */
        volatile int dummy = result + asm_result;
    }
    
    /* Run through all test cases */
    printf("Running comprehensive test suite...\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Mix of comparison methods to trigger different code paths */
        int cmp_result = compare_all_conditions(a, b);
        const char* cls = classify_comparison(a, b);
        
        /* Use inline assembly for some cases */
        if (i % 3 == 0) {
            int asm_res = inline_asm_fp_compare(a, b);
            cmp_result ^= asm_res; /* Mix results to prevent optimization */
        }
        
        printf("Test %d: %g vs %g -> result=0x%x, class=%s\n", 
               i, a, b, cmp_result, cls);
    }
    
#ifdef __SSE2__
    /* Vector comparisons - may generate different instruction patterns */
    printf("\nTesting vector comparisons...\n");
    v2df vec_a = {1.0, NAN};
    v2df vec_b = {NAN, 2.0};
    v2df vec_result = vector_compare(vec_a, vec_b);
    
    /* Force vector result to be used */
    volatile double dummy_vec = vec_result[0] + vec_result[1];
#endif
    
    /* Complex control flow with many branches */
    printf("\nTesting complex control flow...\n");
    volatile int branch_counter = 0;
    
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* This switch-like structure may cause compiler to generate
           multiple different condition code outputs */
        if (isunordered(a, b)) {
            branch_counter |= 1;
        } else if (a < b) {
            branch_counter |= 2;
        } else if (a > b) {
            branch_counter |= 4;
        } else if (a == b) {
            branch_counter |= 8;
        } else {
            branch_counter |= 16; /* Should never happen */
        }
        
        /* Additional comparisons using different operators */
        volatile int cmp1 = (a <= b) ? 1 : 0;
        volatile int cmp2 = (a >= b) ? 1 : 0;
        volatile int cmp3 = (a != b) ? 1 : 0;
        
        branch_counter += cmp1 + cmp2 + cmp3;
    }
    
    printf("Final branch counter: %d\n", branch_counter);
    
    return 0;
}
