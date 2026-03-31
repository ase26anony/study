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

/* Function that performs various floating-point comparisons */
static int compare_floats(double a, double b) {
    int result = 0;
    
    /* Standard C comparison operators - may generate unordered conditions */
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

/* Function with inline assembly to directly generate condition code output */
static int inline_asm_fp_compare(double a, double b) {
    int result = 0;
    
    /* Inline assembly using x87 floating-point comparison */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clear FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%cl\n\t"         /* Set if below (CF=1) */
        "sete %%dl\n\t"         /* Set if equal (ZF=1) */
        "movzbl %%al, %0\n\t"   /* Move unordered result */
        "shll $1, %0\n\t"
        "orl %%ecx, %0\n\t"     /* Combine results */
        "shll $1, %0\n\t"
        "orl %%edx, %0"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "eax", "ecx", "edx", "cc", "st"
    );
    
    return result;
}

/* Function using vector extensions for packed comparisons */
static void vector_fp_comparisons(void) {
    v2df vec_a = {1.0, 2.0};
    v2df vec_b = {NAN, 3.0};
    v2df vec_c = {INFINITY, -INFINITY};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp1 = vec_a < vec_b;  /* Should generate unordered for first element */
    v2df cmp2 = vec_a > vec_c;  /* Should generate ordered comparisons */
    v2df cmp3 = vec_a == vec_b; /* Should generate unordered/equal checks */
    
    /* Prevent dead code elimination */
    volatile v2df* volatile_ptr = &cmp1;
    (void)volatile_ptr;
}

/* Complex switch-based comparison function */
static const char* classify_comparison(double a, double b) {
    /* Check for unordered first */
    if (isunordered(a, b)) {
        /* Further classification within unordered cases */
        if (isless(a, b)) return "UNORDERED_LESS";
        if (isgreater(a, b)) return "UNORDERED_GREATER";
        return "UNORDERED";
    }
    
    /* Ordered comparisons */
    if (a < b) return "LESS";
    if (a > b) return "GREATER";
    if (a == b) {
        /* Distinguish +0.0 from -0.0 */
        if (signbit(a) != signbit(b)) return "EQUAL_BUT_SIGN_DIFF";
        return "EQUAL";
    }
    
    /* Should never reach here for valid floats */
    return "UNKNOWN";
}

int main(int argc, char *argv[]) {
    /* Array of test cases including NaN, Infinity, and normal numbers */
    struct {
        double a, b;
        const char* desc;
    } test_cases[] = {
        {NAN, 1.0, "NaN vs 1.0"},
        {1.0, NAN, "1.0 vs NaN"},
        {NAN, NAN, "NaN vs NaN"},
        {INFINITY, -INFINITY, "Inf vs -Inf"},
        {INFINITY, 1.0, "Inf vs 1.0"},
        {-INFINITY, 1.0, "-Inf vs 1.0"},
        {0.0, -0.0, "+0.0 vs -0.0"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command-line arguments if provided */
    if (argc >= 3) {
        for (int i = 1; i + 1 < argc; i += 2) {
            double a = strtod(argv[i], NULL);
            double b = strtod(argv[i + 1], NULL);
            
            /* Handle "nan" string */
            if (strcasecmp(argv[i], "nan") == 0) a = NAN;
            if (strcasecmp(argv[i + 1], "nan") == 0) b = NAN;
            
            printf("Command-line test: %g vs %g\n", a, b);
            int result = compare_floats(a, b);
            printf("  Comparison result: 0x%x\n", result);
            
            const char* classification = classify_comparison(a, b);
            printf("  Classification: %s\n", classification);
            
            int asm_result = inline_asm_fp_compare(a, b);
            printf("  ASM result: 0x%x\n", asm_result);
        }
    }
    
    /* Run all test cases */
    printf("Running %d test cases:\n", num_cases);
    for (int i = 0; i < num_cases; i++) {
        printf("\nTest %d: %s\n", i + 1, test_cases[i].desc);
        
        /* Force runtime evaluation by using volatile variables */
        volatile double a = test_cases[i].a;
        volatile double b = test_cases[i].b;
        
        /* Perform comparisons */
        int cmp_result = compare_floats(a, b);
        printf("  compare_floats result: 0x%03x\n", cmp_result);
        
        /* Use inline assembly */
        int asm_result = inline_asm_fp_compare(a, b);
        printf("  inline_asm_fp_compare result: 0x%x\n", asm_result);
        
        /* Classify the comparison */
        const char* classification = classify_comparison(a, b);
        printf("  Classification: %s\n", classification);
        
        /* Complex conditional based on comparison results */
        if (isunordered(a, b)) {
            if (isless(a, b)) {
                printf("  Unordered less\n");
            } else if (isgreater(a, b)) {
                printf("  Unordered greater\n");
            } else {
                printf("  Pure unordered\n");
            }
        } else if (a < b) {
            printf("  Ordered less\n");
        } else if (a > b) {
            printf("  Ordered greater\n");
        } else if (a == b) {
            if (signbit(a) != signbit(b)) {
                printf("  Equal with different signs\n");
            } else {
                printf("  Equal\n");
            }
        }
    }
    
    /* Use vector extensions */
    printf("\nPerforming vector comparisons:\n");
    vector_fp_comparisons();
    
    /* Additional tests with global volatile variables */
    printf("\nTesting with global volatile variables:\n");
    int global_result = compare_floats(global_nan, global_inf);
    printf("  NaN vs Inf: 0x%x\n", global_result);
    
    global_result = compare_floats(global_neg_inf, global_nan);
    printf("  -Inf vs NaN: 0x%x\n", global_result);
    
    /* Prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < num_cases; i++) {
        final_result += compare_floats(test_cases[i].a, test_cases[i].b);
    }
    
    printf("\nFinal aggregated result: %d\n", final_result);
    
    return 0;
}
