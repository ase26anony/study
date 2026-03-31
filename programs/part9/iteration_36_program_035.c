#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector type for SSE/AVX comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function to perform all possible floating-point comparisons */
int compare_doubles_full(double a, double b) {
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
const char* classify_comparison(double a, double b) {
    if (isunordered(a, b)) {
        return "UNORDERED";
    }
    
    int cmp = compare_doubles_full(a, b);
    
    switch (cmp & 0x3F) {  /* Look at basic comparison bits */
        case 1:  return "LESS";
        case 2:  return "GREATER";
        case 16: return "EQUAL";
        case 32: return "NOT_EQUAL";
        case 4:  return "LESS_EQUAL";
        case 8:  return "GREATER_EQUAL";
        default: return "COMPLEX";
    }
}

/* Inline assembly to force condition code output */
double fp_compare_asm(double a, double b) {
    double result;
    int unordered_flag, greater_flag, less_flag;
    
    /* Using x87 floating-point compare */
    __asm__ volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack, a is at st(0), b at st(1) */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop st(0) */
        "fstp %%st(0)\n\t"      /* Pop b from stack */
        "setp %3\n\t"           /* Set if unordered (parity flag) */
        "seta %4\n\t"           /* Set if greater (above) */
        "setb %5\n\t"           /* Set if less (below) */
        : "=m"(result), "=r"(unordered_flag), "=r"(greater_flag), "=r"(less_flag)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* Use the flags to compute a result */
    if (unordered_flag) {
        result = global_nan;
    } else if (greater_flag) {
        result = a;
    } else if (less_flag) {
        result = b;
    } else {
        result = (a + b) / 2.0;
    }
    
    return result;
}

/* Vector comparison function */
void vector_comparisons(void) {
    v2df vec_a, vec_b, vec_cmp;
    v4sf vec_fa, vec_fb, vec_fcmp;
    
    /* Initialize vectors with mixed values */
    vec_a = (v2df){1.0, global_nan};
    vec_b = (v2df){global_nan, 2.0};
    
    /* Perform vector comparisons - may generate multiple condition codes */
    vec_cmp = vec_a < vec_b;
    vec_cmp = vec_a > vec_b;
    vec_cmp = vec_a == vec_b;
    
    /* Float vector comparisons */
    vec_fa = (v4sf){1.0f, -1.0f, 0.0f, global_nan};
    vec_fb = (v4sf){global_nan, 1.0f, -0.0f, 1.0f};
    
    vec_fcmp = vec_fa < vec_fb;
    vec_fcmp = vec_fa > vec_fb;
    vec_fcmp = vec_fa == vec_fb;
}

/* Parse double from string, handling "nan", "inf", "-inf" */
double parse_fp_arg(const char *arg) {
    if (strcmp(arg, "nan") == 0) return NAN;
    if (strcmp(arg, "inf") == 0) return INFINITY;
    if (strcmp(arg, "-inf") == 0) return -INFINITY;
    return atof(arg);
}

int main(int argc, char *argv[]) {
    /* Test cases including NaN values */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, -INFINITY},
        {0.0, -0.0},
        {DBL_MAX, DBL_MIN},
        {1.0, 2.0},
        {2.0, 1.0},
        {1.0, 1.0}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_fp_arg(argv[1]);
        double b = parse_fp_arg(argv[2]);
        
        printf("Testing with command line values: %g, %g\n", a, b);
        
        /* Force all comparison types */
        int cmp_result = compare_doubles_full(a, b);
        const char *classification = classify_comparison(a, b);
        
        printf("Comparison result: 0x%x\n", cmp_result);
        printf("Classification: %s\n", classification);
        
        /* Use inline assembly comparison */
        double asm_result = fp_compare_asm(a, b);
        printf("Assembly comparison result: %g\n", asm_result);
        
        /* Check for NaN result */
        if (isnan(asm_result)) {
            printf("Result is NaN (unordered comparison detected)\n");
        }
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Prevent optimization */
        volatile double va = a;
        volatile double vb = b;
        
        /* Perform comparisons that may generate various condition codes */
        int result = compare_doubles_full(va, vb);
        
        /* Use results to prevent dead code elimination */
        if (result != 0) {
            printf("Test %d: a=%g, b=%g, result=0x%x\n", 
                   i, va, vb, result);
        }
        
        /* Force generation of condition code output via inline asm */
        double asm_res = fp_compare_asm(va, vb);
        (void)asm_res;  /* Use result to prevent elimination */
    }
    
    /* Vector comparisons */
    printf("\nPerforming vector comparisons...\n");
    vector_comparisons();
    
    /* Additional unordered comparison tests using math.h macros */
    printf("\nTesting specific unordered conditions:\n");
    
    /* UNORDERED: isunordered(NAN, 1.0) */
    if (isunordered(NAN, 1.0)) {
        printf("UNORDERED condition triggered\n");
    }
    
    /* UNEQ: !isunordered(a,b) && !(a != b) for NaN? */
    /* Actually UNEQ is "unordered or equal" */
    
    /* UNGE: "not less than" (unordered or greater or equal) */
    double unge_test = fp_compare_asm(global_nan, 1.0);
    (void)unge_test;
    
    /* UNGT: "not less than or equal" */
    double ungt_test = fp_compare_asm(1.0, global_nan);
    (void)ungt_test;
    
    /* UNLE: "unordered or less or equal" */
    /* UNLT: "unordered or less than" */
    /* LTGT: "less or greater" (ordered and not equal) */
    
    /* Complex switch to potentially generate all condition codes */
    printf("\nFinal summary - testing condition code generation complete.\n");
    
    return 0;
}
