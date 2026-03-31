#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding and optimization */
volatile double volatile_nan = NAN;
volatile double volatile_inf = INFINITY;
volatile double volatile_zero = 0.0;

/* Vector type for SSE/AVX comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Function that performs all possible floating-point comparisons */
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
    if (isunordered(a, b)) {
        return "unordered";
    } else if (a < b) {
        return "less";
    } else if (a > b) {
        return "greater";
    } else if (a == b) {
        return "equal";
    }
    return "unknown";
}

/* Inline assembly to directly generate condition code output */
static int inline_asm_fp_compare(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare with unordered handling */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clear FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%ax, %0"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "ax"
    );
    
    return result;
}

/* Vector comparisons using SSE/AVX */
static v2df vector_compare(v2df a, v2df b) {
    /* These comparisons may generate multiple condition codes */
    v2df cmp_lt = a < b;
    v2df cmp_gt = a > b;
    v2df cmp_eq = a == b;
    v2df cmp_ne = a != b;
    v2df cmp_le = a <= b;
    v2df cmp_ge = a >= b;
    
    /* Combine results */
    return cmp_lt + cmp_gt * 2 + cmp_eq * 4 + cmp_ne * 8 + cmp_le * 16 + cmp_ge * 32;
}

int main(int argc, char *argv[]) {
    /* Test cases including NaN, Infinity, and normal numbers */
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
        {DBL_MAX, DBL_MIN},
        {volatile_nan, volatile_zero}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int total_results = 0;
    
    /* Read values from command line if provided */
    if (argc >= 3) {
        double a = strtod(argv[1], NULL);
        double b = strtod(argv[2], NULL);
        
        /* Handle "nan" string */
        if (argc >= 4 && strcmp(argv[3], "nan") == 0) {
            a = NAN;
        }
        if (argc >= 5 && strcmp(argv[4], "nan") == 0) {
            b = NAN;
        }
        
        test_cases[0][0] = a;
        test_cases[0][1] = b;
        num_cases = 1;
    }
    
    printf("Testing floating-point comparisons...\n");
    
    /* Test each case with various comparison methods */
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        printf("\nTest case %d: a=%g, b=%g\n", i, a, b);
        
        /* Method 1: Standard comparisons */
        int cmp_result = compare_all_conditions(a, b);
        total_results += cmp_result;
        
        /* Method 2: Classification */
        const char *classification = classify_comparison(a, b);
        printf("  Classification: %s\n", classification);
        
        /* Method 3: Inline assembly */
        int asm_result = inline_asm_fp_compare(a, b);
        printf("  Assembly result: 0x%02x\n", asm_result);
        
        /* Method 4: Vector comparisons */
        v2df vec_a = {a, a};
        v2df vec_b = {b, b};
        v2df vec_result = vector_compare(vec_a, vec_b);
        printf("  Vector result: [%g, %g]\n", vec_result[0], vec_result[1]);
        
        /* Force generation of specific condition codes through control flow */
        if (isunordered(a, b)) {
            printf("  UNORDERED condition triggered\n");
        }
        if (!isunordered(a, b) && a == b) {
            printf("  ORDERED EQUAL condition triggered\n");
        }
        if (isless(a, b)) {
            printf("  LESS condition triggered\n");
        }
        if (isgreater(a, b)) {
            printf("  GREATER condition triggered\n");
        }
        if (islessgreater(a, b)) {
            printf("  LESS/GREATER (LTGT) condition triggered\n");
        }
    }
    
    /* Additional test to ensure all code paths are used */
    volatile double v1 = volatile_nan;
    volatile double v2 = 3.14159;
    
    /* Complex conditional that may generate multiple condition codes */
    if ((v1 < v2) || isunordered(v1, v2)) {
        total_results += 1000;
    }
    
    if ((v1 > v2) && !isunordered(v1, v2)) {
        total_results += 2000;
    }
    
    if (v1 == v2) {
        total_results += 3000;
    }
    
    if (v1 != v2) {
        total_results += 4000;
    }
    
    /* Final output to prevent dead code elimination */
    printf("\nTotal results accumulator: %d\n", total_results);
    
    return total_results == 0 ? 1 : 0;
}
