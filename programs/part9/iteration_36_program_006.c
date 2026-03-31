#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector extension for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Function that performs all possible comparisons */
static int compare_all_results(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map to x86 unordered predicates */
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
    
    /* Special cases for -0.0 and +0.0 */
    if (a == 0.0 && b == 0.0 && signbit(a) != signbit(b))
        return "ZERO_SIGN_DIFF";
    
    return "UNKNOWN";
}

/* Inline assembly to force condition code output */
static int inline_asm_fp_compare(double a, double b) {
    int result = 0;
    
    /* Using x87 floating-point compare */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clean up FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %0\n\t"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)", "eax"
    );
    
    return result;
}

/* Another inline assembly variant with different condition codes */
static int inline_asm_fp_compare2(double a, double b) {
    int result;
    
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "sete %%al\n\t"         /* Set if equal (ZF=1) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %0\n\t"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)", "eax"
    );
    
    return result;
}

/* Vector comparison function */
static void vector_comparisons(void) {
    v2df vec_a = {1.0, 2.0};
    v2df vec_b = {NAN, 3.0};
    v2df vec_c = {INFINITY, -INFINITY};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp1 = vec_a < vec_b;    /* One element is NaN */
    v2df cmp2 = vec_a > vec_c;
    v2df cmp3 = vec_b == vec_c;
    
    /* Use the results to prevent dead code elimination */
    volatile v2df volatile_cmp = cmp1;
    (void)volatile_cmp;
    volatile_cmp = cmp2;
    (void)volatile_cmp;
    volatile_cmp = cmp3;
    (void)volatile_cmp;
}

/* Parse double with NaN support */
static double parse_double(const char* str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NaN") == 0)
        return NAN;
    if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0)
        return INFINITY;
    if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0)
        return -INFINITY;
    return atof(str);
}

int main(int argc, char** argv) {
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
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Testing with command line values: %g, %g\n", a, b);
        
        /* Force all comparison types */
        int results = compare_all_results(a, b);
        const char* classification = classify_comparison(a, b);
        
        printf("Comparison results: 0x%x\n", results);
        printf("Classification: %s\n", classification);
        
        /* Use inline assembly */
        int asm_result1 = inline_asm_fp_compare(a, b);
        int asm_result2 = inline_asm_fp_compare2(a, b);
        printf("Inline ASM results: %d, %d\n", asm_result1, asm_result2);
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Prevent constant folding */
        volatile double volatile_a = a;
        volatile double volatile_b = b;
        
        /* Perform comparisons that may generate various condition codes */
        int cmp_results = compare_all_results(volatile_a, volatile_b);
        
        /* Use inline assembly for each test case */
        int asm_res = inline_asm_fp_compare(volatile_a, volatile_b);
        
        printf("Test %d: %g vs %g -> results: 0x%x, asm: %d\n",
               i, volatile_a, volatile_b, cmp_results, asm_res);
        
        /* Complex conditional logic to encourage different code paths */
        if (isunordered(volatile_a, volatile_b)) {
            if (volatile_a == volatile_a) { /* Always true except for NaN */
                /* This branch should not be taken for NaN */
            }
        } else if (volatile_a < volatile_b) {
            if (volatile_b > volatile_a) { /* Always true */
                /* Nested comparisons */
            }
        } else if (volatile_a > volatile_b) {
            /* Another branch */
        } else {
            /* Equal case - includes -0.0 == +0.0 */
            if (signbit(volatile_a) != signbit(volatile_b)) {
                printf("  Note: Signed zeros differ\n");
            }
        }
    }
    
    /* Vector comparisons */
    printf("\nPerforming vector comparisons:\n");
    vector_comparisons();
    
    /* Additional unordered comparison tests */
    printf("\nSpecial unordered comparison tests:\n");
    
    /* Test UNEQ (unordered or equal) */
    double un_eq_a = NAN;
    double un_eq_b = NAN;
    volatile int un_eq_result = (isunordered(un_eq_a, un_eq_b) || un_eq_a == un_eq_b);
    printf("UNEQ test (NaN == NaN): %d\n", un_eq_result);
    
    /* Test UNGE (unordered or greater or equal) */
    double un_ge_a = NAN;
    double un_ge_b = 1.0;
    volatile int un_ge_result = (isunordered(un_ge_a, un_ge_b) || un_ge_a >= un_ge_b);
    printf("UNGE test (NaN >= 1.0): %d\n", un_ge_result);
    
    /* Test UNGT (unordered or greater) */
    volatile int un_gt_result = (isunordered(un_ge_a, un_ge_b) || un_ge_a > un_ge_b);
    printf("UNGT test (NaN > 1.0): %d\n", un_gt_result);
    
    /* Test UNLE (unordered or less or equal) */
    volatile int un_le_result = (isunordered(un_ge_a, un_ge_b) || un_ge_a <= un_ge_b);
    printf("UNLE test (NaN <= 1.0): %d\n", un_le_result);
    
    /* Test UNLT (unordered or less) */
    volatile int un_lt_result = (isunordered(un_ge_a, un_ge_b) || un_ge_a < un_ge_b);
    printf("UNLT test (NaN < 1.0): %d\n", un_lt_result);
    
    /* Test LTGT (less or greater, but not equal and not unordered) */
    double ltgt_a = 1.0;
    double ltgt_b = 2.0;
    volatile int ltgt_result = ((ltgt_a < ltgt_b) || (ltgt_a > ltgt_b)) && !isunordered(ltgt_a, ltgt_b);
    printf("LTGT test (1.0 < 2.0): %d\n", ltgt_result);
    
    /* Final summary to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < num_cases; i++) {
        final_result ^= compare_all_results(test_cases[i][0], test_cases[i][1]);
    }
    
    printf("\nFinal XOR of all results: 0x%x\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
