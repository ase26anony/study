#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector type for SSE/AVX comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Function that performs all possible floating-point comparisons */
static int compare_all_conditions(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - these can generate unordered conditions */
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
    /* Check for unordered first */
    if (isunordered(a, b)) {
        /* Further classification of unordered cases */
        if (isless(a, b)) return "UNORDERED_LESS";  /* Shouldn't happen */
        if (isgreater(a, b)) return "UNORDERED_GREATER"; /* Shouldn't happen */
        return "UNORDERED";
    }
    
    /* Ordered comparisons */
    if (a < b) return "LESS";
    if (a > b) return "GREATER";
    if (a == b) return "EQUAL";
    
    /* Special cases for NaN propagation */
    if (isnan(a)) return "A_IS_NAN";
    if (isnan(b)) return "B_IS_NAN";
    
    return "UNKNOWN";
}

/* Inline assembly that directly uses floating-point condition codes */
static int inline_asm_fp_compare(double a, double b) {
    int result = 0;
    
    /* Using x87 FPU comparison with inline assembly */
    /* This should trigger the condition code output routines */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clean up FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%ax, %0\n\t"   /* Move result to output */
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)", "ax"
    );
    
    return result;
}

/* Vector comparisons using GCC extensions */
static void vector_comparisons(void) {
    v2df vec_a = {1.0, NAN};
    v2df vec_b = {NAN, 2.0};
    v2df vec_c = {3.0, 4.0};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp_lt = vec_a < vec_b;
    v2df cmp_gt = vec_a > vec_b;
    v2df cmp_eq = vec_a == vec_b;
    v2df cmp_ne = vec_a != vec_b;
    v2df cmp_le = vec_a <= vec_b;
    v2df cmp_ge = vec_a >= vec_b;
    
    /* Use results to prevent optimization */
    volatile v2df volatile_cmp = cmp_lt;
    (void)volatile_cmp;
}

/* Parse command line argument to double, handling "nan" */
static double parse_fp_arg(const char *arg) {
    if (strcmp(arg, "nan") == 0 || strcmp(arg, "NaN") == 0) {
        return NAN;
    }
    if (strcmp(arg, "inf") == 0 || strcmp(arg, "INF") == 0) {
        return INFINITY;
    }
    if (strcmp(arg, "-inf") == 0 || strcmp(arg, "-INF") == 0) {
        return -INFINITY;
    }
    return atof(arg);
}

int main(int argc, char *argv[]) {
    /* Array of test cases designed to trigger various condition codes */
    struct test_case {
        double a;
        double b;
        const char *desc;
    } test_cases[] = {
        {NAN, 1.0, "NaN vs 1.0"},
        {1.0, NAN, "1.0 vs NaN"},
        {NAN, NAN, "NaN vs NaN"},
        {INFINITY, -INFINITY, "INF vs -INF"},
        {0.0, -0.0, "0.0 vs -0.0"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
        {INFINITY, 1.0, "INF vs 1.0"},
        {-INFINITY, 1.0, "-INF vs 1.0"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_fp_arg(argv[1]);
        double b = parse_fp_arg(argv[2]);
        
        printf("Testing command line values: %s vs %s\n", argv[1], argv[2]);
        
        /* Perform all types of comparisons */
        int cmp_result = compare_all_conditions(a, b);
        const char *classification = classify_comparison(a, b);
        int asm_result = inline_asm_fp_compare(a, b);
        
        printf("Comparison result bitmap: 0x%04x\n", cmp_result);
        printf("Classification: %s\n", classification);
        printf("Inline ASM result: 0x%02x\n", asm_result);
        
        /* Force generation of condition codes in control flow */
        if (isunordered(a, b)) {
            printf("Unordered comparison detected\n");
            if (a < b) printf("a < b (unlikely with NaN)\n");
            if (a > b) printf("a > b (unlikely with NaN)\n");
            if (a == b) printf("a == b (unlikely with NaN)\n");
            if (a != b) printf("a != b (always true with NaN)\n");
        } else {
            if (a < b) printf("a < b (ordered)\n");
            else if (a > b) printf("a > b (ordered)\n");
            else printf("a == b (ordered)\n");
        }
    } else {
        /* Run all test cases */
        printf("Running comprehensive test suite...\n");
        
        for (int i = 0; i < num_cases; i++) {
            double a = test_cases[i].a;
            double b = test_cases[i].b;
            
            printf("\nTest %d: %s\n", i + 1, test_cases[i].desc);
            
            /* Mix of comparison methods to trigger different code paths */
            int cmp_result = compare_all_conditions(a, b);
            const char *classification = classify_comparison(a, b);
            
            printf("  Result: 0x%04x, Class: %s\n", cmp_result, classification);
            
            /* Use inline assembly for some cases */
            if (i % 3 == 0) {
                int asm_result = inline_asm_fp_compare(a, b);
                printf("  ASM result: 0x%02x\n", asm_result);
            }
        }
    }
    
    /* Vector comparisons */
    vector_comparisons();
    
    /* Additional complex control flow to generate various condition codes */
    {
        volatile double v1 = global_nan;
        volatile double v2 = 3.14159;
        volatile double v3 = global_inf;
        
        /* Complex if-else chain with mixed comparisons */
        if (isunordered(v1, v2)) {
            if (v1 == v2) {
                printf("Case UNEQ-like\n");
            }
            if (!(v1 < v2)) {
                printf("Case UNGE-like (nlt)\n");
            }
            if (!(v1 <= v2)) {
                printf("Case UNGT-like (nle)\n");
            }
            if (v1 <= v2) {
                printf("Case UNLE-like\n");
            }
            if (v1 < v2) {
                printf("Case UNLT-like\n");
            }
            if (v1 != v2) {
                printf("Case LTGT-like (une)\n");
            }
        }
        
        /* Switch-like behavior using function pointers to prevent optimization */
        int (*comparators[6])(double, double) = {
            [0] = (int (*)(double, double))isless,
            [1] = (int (*)(double, double))isgreater,
            [2] = (int (*)(double, double))islessequal,
            [3] = (int (*)(double, double))isgreaterequal,
            [4] = (int (*)(double, double))islessgreater,
            [5] = (int (*)(double, double))isunordered,
        };
        
        for (int i = 0; i < 6; i++) {
            if (comparators[i](v1, v3)) {
                printf("Comparator %d true\n", i);
            }
        }
    }
    
    return 0;
}
