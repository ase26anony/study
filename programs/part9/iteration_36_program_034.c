/* test_i386_cc.c - Program to trigger x86 unordered floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function to classify comparison results */
static const char* fp_compare_classify(double a, double b) {
    if (isunordered(a, b)) return "UNORDERED";
    if (a < b) return "LT";
    if (a > b) return "GT";
    if (a == b) return "EQ";
    return "UNKNOWN";
}

/* Function performing various unordered comparisons */
static int perform_unordered_comparisons(double x, double y) {
    int result = 0;
    
    /* Use math.h macros that map directly to x86 condition codes */
    if (isunordered(x, y)) result |= 0x01;
    if (isless(x, y)) result |= 0x02;
    if (isgreater(x, y)) result |= 0x04;
    if (islessequal(x, y)) result |= 0x08;
    if (isgreaterequal(x, y)) result |= 0x10;
    if (islessgreater(x, y)) result |= 0x20;  /* LTGT */
    
    /* Direct comparisons that may generate UNEQ, UNGE, UNGT, UNLE, UNLT */
    if (!(x < y)) result |= 0x40;   /* May generate nlt (UNGE) */
    if (!(x <= y)) result |= 0x80;  /* May generate nle (UNGT) */
    if (x != y) result |= 0x100;    /* May generate une (LTGT) */
    
    return result;
}

/* Inline assembly to force condition code output */
static int inline_asm_fp_compare(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare with unordered check */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack, a is now st(0), b is st(1) */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop st(0) */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $8, %%edx\n\t"
        "orl %%edx, %%eax\n\t"
        "fstp %%st(0)\n\t"      /* Clean up FPU stack */
        : "=a" (result)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    return result;
}

/* Vector comparisons using GCC extensions */
#ifdef USE_VECTOR
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

static v2di vector_fp_compare(v2df a, v2df b) {
    /* These comparisons may generate multiple condition codes */
    v2df cmp_lt = a < b;
    v2df cmp_gt = a > b;
    v2df cmp_eq = a == b;
    v2df cmp_ne = a != b;
    
    /* Convert to integer masks */
    v2di mask_lt = (v2di)cmp_lt;
    v2di mask_gt = (v2di)cmp_gt;
    v2di mask_eq = (v2di)cmp_eq;
    v2di mask_ne = (v2di)cmp_ne;
    
    return mask_lt + mask_gt + mask_eq + mask_ne;
}
#endif

/* Complex switch based on floating-point comparisons */
static const char* fp_switch_test(double a, double b) {
    /* Force evaluation of all comparison types */
    int cmp_result = 0;
    
    /* Check all possible conditions */
    if (isunordered(a, b)) {
        cmp_result = 0;
    } else if (a == b) {
        cmp_result = 1;
    } else if (a < b) {
        cmp_result = 2;
    } else if (a > b) {
        cmp_result = 3;
    } else if (a <= b) {
        cmp_result = 4;
    } else if (a >= b) {
        cmp_result = 5;
    } else if (a != b) {
        cmp_result = 6;
    }
    
    /* Switch that might generate various condition code outputs */
    switch (cmp_result) {
        case 0: return "unordered";
        case 1: return "equal";
        case 2: return "less";
        case 3: return "greater";
        case 4: return "less_equal";
        case 5: return "greater_equal";
        case 6: return "not_equal";
        default: return "unknown";
    }
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Array of test cases with various NaN/inf combinations */
    struct test_case {
        double a;
        double b;
        const char *desc;
    } tests[] = {
        {NAN, 1.0, "NAN vs 1.0"},
        {1.0, NAN, "1.0 vs NAN"},
        {NAN, NAN, "NAN vs NAN"},
        {INFINITY, 1.0, "INF vs 1.0"},
        {1.0, INFINITY, "1.0 vs INF"},
        {INFINITY, -INFINITY, "INF vs -INF"},
        {0.0, -0.0, "0.0 vs -0.0"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int total_results = 0;
    
    printf("Testing floating-point comparisons for x86 condition code output\n");
    printf("================================================================\n\n");
    
    /* Test 1: Basic unordered comparisons */
    printf("Test 1: Unordered comparisons using math.h macros\n");
    for (int i = 0; i < num_tests; i++) {
        double a = tests[i].a;
        double b = tests[i].b;
        
        printf("\nTest %d: %s\n", i + 1, tests[i].desc);
        printf("  Classification: %s\n", fp_compare_classify(a, b));
        
        int result = perform_unordered_comparisons(a, b);
        printf("  Comparison result: 0x%03x\n", result);
        
        total_results ^= result; /* Prevent dead code elimination */
    }
    
    /* Test 2: Inline assembly comparisons */
    printf("\n\nTest 2: Inline assembly FP comparisons\n");
    for (int i = 0; i < num_tests; i++) {
        double a = tests[i].a;
        double b = tests[i].b;
        
        int asm_result = inline_asm_fp_compare(a, b);
        printf("  %s: asm result = 0x%04x\n", tests[i].desc, asm_result);
        
        total_results ^= asm_result;
    }
    
    /* Test 3: Switch-based comparisons */
    printf("\n\nTest 3: Switch-based comparison classification\n");
    for (int i = 0; i < num_tests; i++) {
        double a = tests[i].a;
        double b = tests[i].b;
        
        const char *switch_result = fp_switch_test(a, b);
        printf("  %s: switch -> %s\n", tests[i].desc, switch_result);
        
        /* Use result to prevent optimization */
        total_results += (int)switch_result[0];
    }
    
    /* Test 4: Command-line argument tests */
    printf("\n\nTest 4: Command-line argument tests\n");
    if (argc >= 3) {
        for (int i = 1; i < argc - 1; i += 2) {
            double a, b;
            
            /* Parse with NaN support */
            if (strcmp(argv[i], "nan") == 0 || strcmp(argv[i], "NAN") == 0)
                a = NAN;
            else
                a = atof(argv[i]);
                
            if (strcmp(argv[i+1], "nan") == 0 || strcmp(argv[i+1], "NAN") == 0)
                b = NAN;
            else
                b = atof(argv[i+1]);
            
            printf("  Input: %s vs %s\n", argv[i], argv[i+1]);
            printf("    isunordered: %d\n", isunordered(a, b));
            printf("    isless: %d\n", isless(a, b));
            printf("    isgreater: %d\n", isgreater(a, b));
            printf("    islessequal: %d\n", islessequal(a, b));
            printf("    isgreaterequal: %d\n", isgreaterequal(a, b));
            printf("    islessgreater: %d\n", islessgreater(a, b));
            
            total_results += isunordered(a, b) + isless(a, b) + isgreater(a, b);
        }
    }
    
#ifdef USE_VECTOR
    /* Test 5: Vector comparisons */
    printf("\n\nTest 5: Vector floating-point comparisons\n");
    v2df vec_a = {1.0, NAN};
    v2df vec_b = {NAN, 2.0};
    v2di vec_result = vector_fp_compare(vec_a, vec_b);
    
    printf("  Vector comparison result: [0x%016llx, 0x%016llx]\n",
           (long long)vec_result[0], (long long)vec_result[1]);
    
    total_results += (int)vec_result[0] + (int)vec_result[1];
#endif
    
    /* Final result to prevent optimization */
    printf("\n================================================================\n");
    printf("Final accumulated result (prevents dead code elimination): %d\n", total_results);
    
    return total_results != 0 ? 0 : 1;
}
