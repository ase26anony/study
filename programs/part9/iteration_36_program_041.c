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
static int compare_doubles(double a, double b) {
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

/* Helper function with switch based on comparison results */
static const char* classify_comparison(double a, double b) {
    if (isunordered(a, b)) return "unordered";
    if (a < b) return "less";
    if (a > b) return "greater";
    if (a == b) return "equal";
    return "unknown";
}

/* Inline assembly to directly trigger condition code output */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare with unordered handling */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto x87 stack */
        "fldl %1\n\t"           /* Load a onto x87 stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %%eax\n"
        : "=a"(result)
        : "m"(a), "m"(b)
        : "cc"
    );
    
    return result;
}

/* Vector extensions for packed floating-point comparisons */
#ifdef USE_VECTOR
typedef double v2df __attribute__((vector_size(16)));

static void vector_comparisons(void) {
    volatile v2df a = {1.0, NAN};
    volatile v2df b = {NAN, 2.0};
    volatile v2df cmp_result;
    
    /* These may generate multiple comparison instructions */
    cmp_result = a < b;
    cmp_result = a > b;
    cmp_result = a <= b;
    cmp_result = a >= b;
    cmp_result = a == b;
    cmp_result = a != b;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(cmp_result) : "memory");
}
#endif

/* Parse command line argument to double, handling special values */
static double parse_fp_arg(const char *arg) {
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

int main(int argc, char *argv[]) {
    /* Array of test cases with various combinations */
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
    int total_results = 0;
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_fp_arg(argv[1]);
        double b = parse_fp_arg(argv[2]);
        
        printf("Testing with command line values: %g, %g\n", a, b);
        
        /* Perform all types of comparisons */
        int result = compare_doubles(a, b);
        printf("Comparison result bits: %d\n", result);
        
        const char *classification = classify_comparison(a, b);
        printf("Classification: %s\n", classification);
        
        /* Use inline assembly */
        int asm_result = fp_compare_asm(a, b);
        printf("Assembly comparison result: %d\n", asm_result);
        
        total_results = result + asm_result;
    } else {
        /* Run through predefined test cases */
        printf("Running predefined test cases:\n");
        
        for (int i = 0; i < num_cases; i++) {
            double a = test_cases[i][0];
            double b = test_cases[i][1];
            
            printf("\nTest case %d: %g, %g\n", i, a, b);
            
            /* Mix of ordered and unordered comparisons in control flow */
            int result = compare_doubles(a, b);
            printf("  Comparison bits: %d\n", result);
            
            /* Switch-like behavior to encourage multiple condition codes */
            const char *cls = classify_comparison(a, b);
            printf("  Classified as: %s\n", cls);
            
            /* Use inline assembly for each test case */
            int asm_result = fp_compare_asm(a, b);
            printf("  ASM result: %d\n", asm_result);
            
            total_results += result + asm_result;
        }
    }
    
#ifdef USE_VECTOR
    /* Vector comparisons if enabled */
    vector_comparisons();
#endif
    
    /* Additional complex control flow with floating-point comparisons */
    {
        volatile double x = global_nan;
        volatile double y = 3.14159;
        volatile double z = global_inf;
        
        /* Complex if-else chain with various comparisons */
        if (isunordered(x, y)) {
            total_results += 1000;
        } else if (x < y) {
            total_results += 2000;
        } else if (x > y) {
            total_results += 3000;
        } else if (x == y) {
            total_results += 4000;
        }
        
        /* Nested comparisons */
        if (!isunordered(z, y)) {
            if (z > y) {
                total_results += 5000;
            } else if (z < y) {
                total_results += 6000;
            }
        }
        
        /* Multiple comparisons in single expression */
        if ((x != y) && !isunordered(x, y)) {
            total_results += 7000;
        }
    }
    
    /* Prevent dead code elimination of all results */
    asm volatile("" : : "r"(total_results) : "memory");
    
    printf("\nTotal results accumulator: %d\n", total_results);
    
    return total_results != 0 ? 0 : 1;
}
