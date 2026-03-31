#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double volatile_nan = NAN;
volatile double volatile_inf = INFINITY;
volatile double volatile_zero = 0.0;

/* Vector type for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Function that performs all possible floating-point comparisons */
int compare_floats_comprehensive(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* math.h comparison macros - directly map to x86 predicates */
    if (isunordered(a, b)) result |= 64;
    if (isless(a, b)) result |= 128;
    if (isgreater(a, b)) result |= 256;
    if (islessequal(a, b)) result |= 512;
    if (isgreaterequal(a, b)) result |= 1024;
    if (islessgreater(a, b)) result |= 2048;
    
    return result;
}

/* Function with inline assembly to force condition code output */
double inline_asm_fp_compare(double a, double b) {
    double result;
    int unordered_flag, equal_flag, less_flag;
    
    /* Force x87 FPU comparison with unordered check */
    asm volatile (
        "fldl %[b]\n\t"
        "fldl %[a]\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setp %[unordered]\n\t"
        "sete %[equal]\n\t"
        "setb %[less]"
        : [unordered] "=r" (unordered_flag),
          [equal] "=r" (equal_flag),
          [less] "=r" (less_flag)
        : [a] "m" (a),
          [b] "m" (b)
        : "cc", "st"
    );
    
    /* Use the flags to compute a result */
    result = unordered_flag ? volatile_nan : 
             (less_flag ? -1.0 : (equal_flag ? 0.0 : 1.0));
    
    return result;
}

/* Another inline assembly variant with different condition codes */
void generate_various_conditions(double a, double b) {
    int res_unord, res_ord, res_ueq, res_nlt, res_nle, res_ule, res_ult, res_une;
    
    /* Generate various condition code outputs through inline asm constraints */
    asm volatile (
        "fucomip %%st(1), %%st\n\t"
        : "=@ccp" (res_unord),   /* parity/unordered */
          "=@cco" (res_ord),     /* ordered */
          "=@ccne" (res_une),    /* not equal (LTGT) */
          "=@cce" (res_ueq)      /* equal (UNEQ when combined with unordered) */
        : "t" (a), "u" (b)
        : "cc", "st"
    );
    
    /* Clear FPU stack */
    asm volatile ("fstp %%st(0)" ::: "st");
    
    /* Prevent dead code elimination */
    volatile int dummy = res_unord + res_ord + res_ueq + res_une;
    (void)dummy;
}

/* Vector comparison function */
void vector_fp_comparisons(void) {
    v2df vec_a = {1.0, NAN};
    v2df vec_b = {NAN, 2.0};
    v2df vec_c = {3.0, 4.0};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp_lt = vec_a < vec_b;
    v2df cmp_gt = vec_a > vec_b;
    v2df cmp_eq = vec_a == vec_b;
    v2df cmp_ne = vec_a != vec_b;
    
    /* Use results to prevent optimization */
    volatile v2df volatile_cmp = cmp_lt;
    volatile_cmp = cmp_gt;
    volatile_cmp = cmp_eq;
    volatile_cmp = cmp_ne;
    
    /* Mixed vector/scalar comparison */
    double* ptr_a = (double*)&vec_a;
    double* ptr_b = (double*)&vec_b;
    
    for (int i = 0; i < 2; i++) {
        if (isunordered(ptr_a[i], ptr_b[i])) {
            volatile int dummy = i;
            (void)dummy;
        }
    }
}

/* Parse string to double, handling "nan", "inf", "-inf" */
double parse_fp_arg(const char* arg) {
    if (strcasecmp(arg, "nan") == 0) return NAN;
    if (strcasecmp(arg, "inf") == 0 || strcasecmp(arg, "+inf") == 0) return INFINITY;
    if (strcasecmp(arg, "-inf") == 0) return -INFINITY;
    return atof(arg);
}

int main(int argc, char* argv[]) {
    /* Test cases designed to trigger various condition codes */
    double test_cases[][2] = {
        {NAN, 1.0},           /* unordered comparisons */
        {1.0, NAN},           /* unordered comparisons */
        {NAN, NAN},           /* both NaN */
        {INFINITY, -INFINITY},/* ordered, less/greater */
        {0.0, -0.0},          /* equal (even though signs differ) */
        {1.0, 2.0},           /* normal less than */
        {2.0, 1.0},           /* normal greater than */
        {1.0, 1.0},           /* equal */
        {INFINITY, INFINITY}, /* equal infinities */
        {-INFINITY, -INFINITY},/* equal negative infinities */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_fp_arg(argv[1]);
        double b = parse_fp_arg(argv[2]);
        
        printf("Testing with command line values: %g, %g\n", a, b);
        
        /* Force all comparison types */
        int result = compare_floats_comprehensive(a, b);
        double asm_result = inline_asm_fp_compare(a, b);
        generate_various_conditions(a, b);
        
        printf("Comparison result bits: %d\n", result);
        printf("Assembly comparison result: %g\n", asm_result);
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Mix of comparison methods to trigger different code paths */
        int cmp_result = compare_floats_comprehensive(a, b);
        
        /* Inline assembly comparisons */
        double asm_cmp = inline_asm_fp_compare(a, b);
        
        /* Generate various condition codes */
        generate_various_conditions(a, b);
        
        /* Print summary (prevents dead code elimination) */
        printf("Test %d: a=%g, b=%g, cmp=0x%x, asm=%g\n", 
               i, a, b, cmp_result, asm_cmp);
    }
    
    /* Vector comparisons */
    printf("\nPerforming vector comparisons:\n");
    vector_fp_comparisons();
    
    /* Additional tests with volatile values to prevent constant folding */
    printf("\nTesting with volatile values:\n");
    for (int i = 0; i < 5; i++) {
        double a = volatile_nan + (double)i;
        double b = volatile_zero * (double)i;
        
        /* These comparisons cannot be optimized away */
        if (isunordered(a, b)) {
            printf("  Volatile test %d: unordered\n", i);
        }
        
        /* Force generation of less common condition codes */
        int res = compare_floats_comprehensive(a, b);
        volatile int keep = res;  /* Prevent optimization */
        (void)keep;
    }
    
    /* Complex switch based on comparison results */
    printf("\nComplex condition code test:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* This switch structure may cause compiler to generate 
           multiple different condition code outputs */
        int classification = fpclassify(a) + fpclassify(b) * 10;
        
        switch (classification) {
            case FP_NAN * 11:
                /* Both NaN */
                printf("  Both NaN: ");
                break;
            case FP_NAN + FP_INFINITY * 10:
            case FP_INFINITY + FP_NAN * 10:
                /* NaN and Infinity */
                printf("  NaN and Infinity: ");
                break;
            default:
                printf("  Other combination: ");
                break;
        }
        
        /* Nested comparisons to force multiple condition codes */
        if (a < b) printf("a < b ");
        else if (a > b) printf("a > b ");
        else if (a == b) printf("a == b ");
        
        if (isunordered(a, b)) printf("(unordered)");
        printf("\n");
    }
    
    return 0;
}
