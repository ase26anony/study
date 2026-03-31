#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function to parse NaN from command line */
static double parse_double_or_nan(const char *str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NaN") == 0) {
        return NAN;
    }
    if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0) {
        return INFINITY;
    }
    if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0) {
        return -INFINITY;
    }
    return atof(str);
}

/* Helper function that performs all possible comparisons */
static int compare_all_conditions(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map to x86 unordered comparison predicates */
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
    /* This switch structure encourages compiler to generate
       multiple different condition code mnemonics */
    if (isunordered(a, b)) {
        return "unordered";
    } else if (a < b) {
        return "less";
    } else if (a > b) {
        return "greater";
    } else if (a == b) {
        /* Distinguish between +0 and -0 */
        if (signbit(a) != signbit(b)) {
            return "equal_but_opposite_sign";
        }
        return "equal";
    } else {
        /* Should never reach here for valid floats */
        return "unknown";
    }
}

/* Inline assembly to directly trigger condition code output */
static int inline_asm_fp_compare(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare and condition code output */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clear FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%ax, %0\n\t"   /* Move result to output */
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "ax"
    );
    
    return result;
}

/* SSE/AVX vector comparisons */
#ifdef __SSE2__
static void vector_comparisons(void) {
    typedef double v2df __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    volatile v2df a = {NAN, 1.0};
    volatile v2df b = {2.0, NAN};
    volatile v2df c = {3.0, 4.0};
    volatile v2df d = {5.0, 6.0};
    
    /* These vector comparisons may generate condition code output */
    v2df cmp_lt = a < b;
    v2df cmp_gt = c > d;
    v2df cmp_eq = a == b;
    
    /* Use results to prevent optimization */
    v2di *as_int = (v2di*)&cmp_lt;
    printf("Vector comparison results: %llx %llx\n", 
           (long long)(*as_int)[0], (long long)(*as_int)[1]);
}
#endif

int main(int argc, char *argv[]) {
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
    
    /* Parse command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double_or_nan(argv[1]);
        double b = parse_double_or_nan(argv[2]);
        
        printf("Command line test: a=%g, b=%g\n", a, b);
        
        /* Perform all comparisons */
        int cmp_result = compare_all_conditions(a, b);
        printf("Comparison result mask: 0x%x\n", cmp_result);
        
        /* Classify the comparison */
        const char *classification = classify_comparison(a, b);
        printf("Classification: %s\n", classification);
        
        /* Use inline assembly */
        int asm_result = inline_asm_fp_compare(a, b);
        printf("Inline assembly result: 0x%x\n", asm_result);
    }
    
    /* Run through all test cases */
    printf("\nRunning test cases:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Mix of ordered and unordered comparisons in control flow */
        int result = compare_all_conditions(a, b);
        
        /* Switch-like behavior to encourage multiple condition codes */
        const char *cls = classify_comparison(a, b);
        
        /* Use inline assembly for some cases */
        if (i % 3 == 0) {
            int asm_res = inline_asm_fp_compare(a, b);
            result ^= asm_res; /* Combine results */
        }
        
        printf("Test %d: a=%g, b=%g, result=0x%x, class=%s\n",
               i, a, b, result, cls);
    }
    
#ifdef __SSE2__
    /* Vector comparisons */
    printf("\nPerforming vector comparisons:\n");
    vector_comparisons();
#endif
    
    /* Complex conditional with many branches to trigger various
       condition code outputs during compilation */
    printf("\nComplex conditional test:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* This complex if-else chain should generate multiple
           different x86 condition code mnemonics */
        if (isunordered(a, b)) {
            if (a == a) { /* a is not NaN? */
                printf("Case %d: b is NaN\n", i);
            } else if (b == b) { /* b is not NaN? */
                printf("Case %d: a is NaN\n", i);
            } else {
                printf("Case %d: both NaN\n", i);
            }
        } else if (a < b) {
            if (a > 0 && b > 0) {
                printf("Case %d: both positive, a < b\n", i);
            } else if (a < 0 && b < 0) {
                printf("Case %d: both negative, a < b\n", i);
            } else {
                printf("Case %d: mixed signs, a < b\n", i);
            }
        } else if (a > b) {
            printf("Case %d: a > b\n", i);
        } else { /* a == b */
            if (signbit(a) != signbit(b)) {
                printf("Case %d: equal but opposite signs\n", i);
            } else {
                printf("Case %d: exactly equal\n", i);
            }
        }
    }
    
    /* Final summary to prevent dead code elimination */
    volatile double final_check = test_cases[0][0] + test_cases[0][1];
    printf("\nFinal check value: %g\n", (double)final_check);
    
    return 0;
}
