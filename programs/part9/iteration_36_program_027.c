#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
static volatile double global_nan = NAN;
static volatile double global_inf = INFINITY;
static volatile double global_zero = 0.0;

/* Vector type for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Function to classify comparison results */
static int classify_comparison(double a, double b) {
    int result = 0;
    
    /* Standard comparisons that can produce unordered results */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a == b) result |= 4;
    if (a != b) result |= 8;
    
    /* <math.h> macros that map to x86 condition codes */
    if (isunordered(a, b)) result |= 16;
    if (isless(a, b)) result |= 32;
    if (isgreater(a, b)) result |= 64;
    if (islessequal(a, b)) result |= 128;
    if (isgreaterequal(a, b)) result |= 256;
    if (islessgreater(a, b)) result |= 512;
    
    return result;
}

/* Function that uses inline assembly to force condition code output */
static int fp_compare_asm(double a, double b) {
    int result = 0;
    
    /* Inline assembly using x87 floating-point compare */
    /* This should generate the condition code mnemonics */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clear FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "setb %%cl\n\t"         /* Set if below (CF=1) */
        "sete %%dl\n\t"         /* Set if equal (ZF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%cl, %%ecx\n\t"
        "movzbl %%dl, %%edx\n\t"
        "shl $1, %%ecx\n\t"
        "shl $2, %%edx\n\t"
        "or %%ecx, %%eax\n\t"
        "or %%edx, %%eax\n\t"
        : "=a" (result)
        : "m" (a), "m" (b)
        : "cc", "memory", "ecx", "edx"
    );
    
    return result;
}

/* Function with mixed ordered/unordered comparisons in control flow */
static const char* compare_description(double a, double b) {
    /* This complex control flow should generate various condition codes */
    if (isunordered(a, b)) {
        return "unordered";
    } else if (a == b) {
        if (signbit(a) != signbit(b) && a == 0.0) {
            return "zero with different signs";
        }
        return "equal";
    } else if (a < b) {
        return "less than";
    } else if (a > b) {
        return "greater than";
    } else if (a != b) {
        /* This branch is reachable with NaN comparisons */
        return "not equal (unordered case)";
    }
    
    return "unknown";
}

/* Vector comparison function */
static void vector_comparisons(void) {
    v2df vec_a, vec_b, vec_cmp;
    
    /* Initialize vectors with mixed values */
    double a_vals[2] = {NAN, 1.0};
    double b_vals[2] = {2.0, NAN};
    
    vec_a = *(v2df*)a_vals;
    vec_b = *(v2df*)b_vals;
    
    /* Perform vector comparisons - may generate multiple condition codes */
    vec_cmp = vec_a < vec_b;
    vec_cmp = vec_a > vec_b;
    vec_cmp = vec_a == vec_b;
    vec_cmp = vec_a != vec_b;
    
    /* Prevent dead code elimination */
    volatile v2df dummy = vec_cmp;
    (void)dummy;
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Array of test cases including NaN, INF, and special values */
    struct {
        double a;
        double b;
        const char *desc;
    } test_cases[] = {
        {NAN, 1.0, "NaN vs 1.0"},
        {1.0, NAN, "1.0 vs NaN"},
        {NAN, NAN, "NaN vs NaN"},
        {INFINITY, -INFINITY, "+INF vs -INF"},
        {INFINITY, 1.0, "+INF vs 1.0"},
        {-INFINITY, 1.0, "-INF vs 1.0"},
        {0.0, -0.0, "+0.0 vs -0.0"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int total_results = 0;
    
    /* Parse command line arguments if provided */
    if (argc > 2) {
        double arg_a = atof(argv[1]);
        double arg_b = atof(argv[2]);
        
        /* Handle "nan" string */
        if (argc > 3 && strcmp(argv[3], "nan") == 0) {
            arg_a = NAN;
        }
        
        printf("Testing command line values: %g vs %g\n", arg_a, arg_b);
        int result = classify_comparison(arg_a, arg_b);
        printf("Classification result: 0x%x\n", result);
        
        const char *desc = compare_description(arg_a, arg_b);
        printf("Description: %s\n", desc);
        
        int asm_result = fp_compare_asm(arg_a, arg_b);
        printf("Assembly comparison result: 0x%x\n", asm_result);
    }
    
    /* Run all test cases */
    printf("\nRunning comprehensive test suite:\n");
    printf("===============================\n");
    
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i].a;
        double b = test_cases[i].b;
        
        printf("\nTest %d: %s\n", i + 1, test_cases[i].desc);
        printf("  Values: a=%g, b=%g\n", a, b);
        
        /* Force compiler to generate comparison code */
        int cmp_result = classify_comparison(a, b);
        total_results += cmp_result;
        
        const char *desc = compare_description(a, b);
        printf("  Description: %s\n", desc);
        printf("  Classification: 0x%x\n", cmp_result);
        
        /* Use inline assembly for some cases to force condition code output */
        if (i % 3 == 0) {
            int asm_result = fp_compare_asm(a, b);
            printf("  ASM result: 0x%x\n", asm_result);
        }
        
        /* Additional comparisons using volatile to prevent optimization */
        volatile int v1 = (a < b);
        volatile int v2 = (a > b);
        volatile int v3 = (a <= b);
        volatile int v4 = (a >= b);
        volatile int v5 = (a == b);
        volatile int v6 = (a != b);
        
        (void)v1; (void)v2; (void)v3; (void)v4; (void)v5; (void)v6;
    }
    
    /* Test vector comparisons */
    printf("\nTesting vector comparisons:\n");
    vector_comparisons();
    
    /* Test with volatile globals */
    printf("\nTesting with volatile globals:\n");
    volatile double v_nan = global_nan;
    volatile double v_inf = global_inf;
    volatile double v_zero = global_zero;
    
    /* Complex expression that should generate multiple condition codes */
    volatile int complex_cmp = 
        (v_nan < v_inf) + 
        (v_inf > v_zero) * 2 + 
        (isunordered(v_nan, v_zero)) * 4 +
        (islessgreater(v_inf, v_zero)) * 8;
    
    printf("Complex comparison result: %d\n", complex_cmp);
    
    /* Switch statement based on comparison results */
    double test_a = NAN;
    double test_b = 0.0;
    int switch_val = 0;
    
    if (isunordered(test_a, test_b)) switch_val = 1;
    else if (test_a < test_b) switch_val = 2;
    else if (test_a > test_b) switch_val = 3;
    else if (test_a == test_b) switch_val = 4;
    else switch_val = 5;
    
    /* This switch may generate different jump conditions */
    switch (switch_val) {
        case 1: printf("Case: unordered\n"); break;
        case 2: printf("Case: less than\n"); break;
        case 3: printf("Case: greater than\n"); break;
        case 4: printf("Case: equal\n"); break;
        default: printf("Case: other\n"); break;
    }
    
    /* Final summary to prevent dead code elimination */
    printf("\nTotal results sum: %d\n", total_results);
    printf("Test completed.\n");
    
    return total_results != 0 ? 0 : 1;
}
