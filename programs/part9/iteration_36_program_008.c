#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_zero = 0.0;

/* Function to perform various floating-point comparisons */
int compare_floats(double a, double b) {
    int result = 0;
    
    /* Standard comparison operators - may generate unordered conditions */
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

/* Function with mixed ordered/unordered comparisons in control flow */
const char* classify_comparison(double a, double b) {
    if (isunordered(a, b)) {
        return "UNORDERED";
    }
    if (a == b) {
        if (signbit(a) != signbit(b)) {
            return "EQ but different signs";
        }
        return "EQ";
    }
    if (a < b) {
        if (isless(a, b)) {
            return "LT (ordered)";
        }
        return "LT (unordered case)";
    }
    if (a > b) {
        if (isgreater(a, b)) {
            return "GT (ordered)";
        }
        return "GT (unordered case)";
    }
    
    /* Handle special cases that might generate UNEQ, UNGE, UNGT, etc. */
    if (!isless(a, b) && !isgreater(a, b)) {
        if (a != b) {
            return "UNEQ or LTGT candidate";
        }
    }
    
    return "UNKNOWN";
}

/* Inline assembly to directly trigger condition code output */
double inline_asm_fp_compare(double a, double b) {
    double result;
    int unordered_result, ordered_result, eq_result;
    
    /* Using fucomip which sets condition codes including parity for unordered */
    asm volatile (
        "fucomip %%st(1), %%st\n\t"
        "setp %[unordered]\n\t"
        "sete %[eq]\n\t"
        "seta %[ordered]"
        : [unordered] "=r" (unordered_result),
          [eq] "=r" (eq_result),
          [ordered] "=r" (ordered_result)
        : "t" (a), "u" (b)
        : "cc", "st"
    );
    
    /* Another inline asm that might use different condition codes */
    asm volatile (
        "comisd %1, %0\n\t"
        : : "x" (a), "x" (b) : "cc"
    );
    
    result = unordered_result ? global_nan : 
             (ordered_result ? a : b);
    
    return result;
}

/* Vector extensions for packed floating-point comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

void vector_comparisons(v2df a, v2df b) {
    v2df cmp_lt = a < b;
    v2df cmp_gt = a > b;
    v2df cmp_eq = a == b;
    v2df cmp_ne = a != b;
    v2df cmp_le = a <= b;
    v2df cmp_ge = a >= b;
    
    /* Use volatile to prevent optimization */
    volatile v2df result = cmp_lt + cmp_gt + cmp_eq + cmp_ne + cmp_le + cmp_ge;
    (void)result;
}

/* Parse double from string, handling "nan" and "inf" */
double parse_double(const char* str) {
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

int main(int argc, char* argv[]) {
    /* Test cases designed to trigger various condition codes */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, -INFINITY},
        {INFINITY, 1.0},
        {-INFINITY, 1.0},
        {0.0, -0.0},
        {1.0, 2.0},
        {2.0, 1.0},
        {1.0, 1.0},
        {__builtin_nan(""), __builtin_nan("0x1234")},
        {__builtin_inf(), -__builtin_inf()}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Testing with user values: %g, %g\n", a, b);
        
        int cmp_result = compare_floats(a, b);
        printf("Comparison result bits: %d\n", cmp_result);
        
        const char* classification = classify_comparison(a, b);
        printf("Classification: %s\n", classification);
        
        double asm_result = inline_asm_fp_compare(a, b);
        printf("Inline ASM result: %g\n", asm_result);
        
        /* Vector comparison */
        v2df va = {a, a};
        v2df vb = {b, b};
        vector_comparisons(va, vb);
    }
    
    /* Run all test cases */
    printf("\nRunning comprehensive test suite:\n");
    printf("===============================\n");
    
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        printf("\nTest case %d: a=%g, b=%g\n", i, a, b);
        
        /* Force compiler to generate comparison code */
        int cmp_result = compare_floats(a, b);
        printf("  compare_floats result: 0x%x\n", cmp_result);
        
        const char* classification = classify_comparison(a, b);
        printf("  classification: %s\n", classification);
        
        /* Use inline assembly for each test case */
        volatile double asm_result = inline_asm_fp_compare(a, b);
        (void)asm_result;
        
        /* Vector comparisons */
        v2df va = {a, b};
        v2df vb = {b, a};
        vector_comparisons(va, vb);
        
        /* Additional comparisons that might trigger specific condition codes */
        volatile int is_unordered = isunordered(a, b);
        volatile int is_uneq = (!isless(a, b) && !isgreater(a, b) && a != b);
        volatile int is_unge = !isless(a, b);
        volatile int is_ungt = !islessequal(a, b);
        volatile int is_unle = !isgreater(a, b);
        volatile int is_unlt = !isgreaterequal(a, b);
        volatile int is_ltgt = islessgreater(a, b);
        
        (void)is_unordered;
        (void)is_uneq;
        (void)is_unge;
        (void)is_ungt;
        (void)is_unle;
        (void)is_unlt;
        (void)is_ltgt;
    }
    
    /* Complex switch statement that might generate various condition codes */
    printf("\nComplex comparison analysis:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        int comparison_code;
        if (isunordered(a, b)) {
            comparison_code = 0;  /* UNORDERED */
        } else if (a == b) {
            comparison_code = 1;  /* EQ */
        } else if (isless(a, b)) {
            comparison_code = 2;  /* LT */
        } else if (isgreater(a, b)) {
            comparison_code = 3;  /* GT */
        } else if (!isless(a, b) && !isgreater(a, b) && a != b) {
            comparison_code = 4;  /* UNEQ */
        } else if (!isless(a, b)) {
            comparison_code = 5;  /* UNGE */
        } else if (!islessequal(a, b)) {
            comparison_code = 6;  /* UNGT */
        } else if (!isgreater(a, b)) {
            comparison_code = 7;  /* UNLE */
        } else if (!isgreaterequal(a, b)) {
            comparison_code = 8;  /* UNLT */
        } else if (islessgreater(a, b)) {
            comparison_code = 9;  /* LTGT */
        } else {
            comparison_code = 10; /* Unknown */
        }
        
        printf("  Case %d: comparison_code = %d\n", i, comparison_code);
    }
    
    /* Prevent dead code elimination */
    volatile double final_result = 0.0;
    for (int i = 0; i < num_cases; i++) {
        final_result += test_cases[i][0] + test_cases[i][1];
    }
    printf("\nFinal accumulated result (prevents optimization): %g\n", final_result);
    
    return 0;
}
