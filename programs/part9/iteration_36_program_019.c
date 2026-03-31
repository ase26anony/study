#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Force generation of x87 FPU instructions */
#pragma GCC optimize ("no-fast-math")
#pragma GCC target ("fpmath=387")

/* Vector type for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Helper function to perform all possible comparisons */
static int classify_comparison(double a, double b) {
    int result = 0;
    
    /* Standard comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map to x86 condition codes */
    if (isunordered(a, b)) result |= 64;      /* UNORDERED */
    if (isless(a, b)) result |= 128;          /* generates "nge" or similar */
    if (isgreater(a, b)) result |= 256;       /* generates "ngle" or similar */
    if (islessequal(a, b)) result |= 512;     /* UNLE? */
    if (isgreaterequal(a, b)) result |= 1024; /* UNGE? */
    if (islessgreater(a, b)) result |= 2048;  /* LTGT -> "une" */
    
    return result;
}

/* Function with inline assembly to force condition code output */
static double fp_compare_with_asm(double a, double b) {
    volatile double result = 0.0;
    int unordered_flag, equal_flag, less_flag;
    
    /* Inline assembly using x87 FPU comparison */
    asm volatile (
        "fldl %[b]\n\t"           /* load b to st(0) */
        "fldl %[a]\n\t"           /* load a to st(0), b moves to st(1) */
        "fucomip %%st(1), %%st\n\t"  /* compare and pop st(0) */
        "fstp %%st(0)\n\t"        /* pop st(0) which was b */
        "setp %[unordered]\n\t"   /* set if unordered (parity flag) */
        "sete %[equal]\n\t"       /* set if equal */
        "setb %[less]\n\t"        /* set if below (CF=1) */
        : [unordered] "=r" (unordered_flag),
          [equal] "=r" (equal_flag),
          [less] "=r" (less_flag)
        : [a] "m" (a),
          [b] "m" (b)
        : "cc", "st", "st(1)"
    );
    
    /* Use the flags to compute a result */
    if (unordered_flag) {
        result = NAN;
    } else if (equal_flag) {
        result = 0.0;
    } else if (less_flag) {
        result = -1.0;
    } else {
        result = 1.0;
    }
    
    return result;
}

/* Another inline assembly variant with different condition codes */
static int fp_compare_ueq(double a, double b) {
    int result;
    
    /* Try to generate "ueq" (unordered or equal) condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setnp %%al\n\t"          /* not unordered */
        "sete %%cl\n\t"           /* equal */
        "andb %%cl, %%al\n\t"     /* not unordered AND equal */
        "movzbl %%al, %0\n\t"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "eax", "ecx", "st", "st(1)"
    );
    
    return result;
}

/* Vector comparison function */
static void vector_comparisons(void) {
    volatile v2df vec1 = {1.0, NAN};
    volatile v2df vec2 = {NAN, 2.0};
    volatile v2df cmp_result;
    
    /* These vector comparisons may generate multiple condition codes */
    cmp_result = vec1 < vec2;   /* Should generate UNORDERED for NaN elements */
    cmp_result = vec1 > vec2;
    cmp_result = vec1 == vec2;
    cmp_result = vec1 != vec2;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "m" (cmp_result));
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Array of test cases with various NaN combinations */
    struct test_case {
        double a;
        double b;
        const char *desc;
    } tests[] = {
        {NAN, 1.0, "NaN vs 1.0"},
        {1.0, NAN, "1.0 vs NaN"},
        {NAN, NAN, "NaN vs NaN"},
        {INFINITY, -INFINITY, "INF vs -INF"},
        {INFINITY, 1.0, "INF vs 1.0"},
        {-INFINITY, 1.0, "-INF vs 1.0"},
        {0.0, -0.0, "0.0 vs -0.0"},
        {DBL_MAX, DBL_MIN, "MAX vs MIN"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    volatile int total_results = 0;
    volatile double asm_results = 0.0;
    
    printf("Testing floating-point comparisons for condition code coverage\n");
    
    /* Test 1: Classify comparisons */
    for (int i = 0; i < num_tests; i++) {
        int result = classify_comparison(tests[i].a, tests[i].b);
        total_results += result;
        printf("Test %d (%s): result = 0x%x\n", 
               i, tests[i].desc, result);
    }
    
    /* Test 2: Inline assembly comparisons */
    printf("\nInline assembly comparisons:\n");
    for (int i = 0; i < num_tests; i++) {
        double asm_result = fp_compare_with_asm(tests[i].a, tests[i].b);
        asm_results += asm_result;
        int ueq_result = fp_compare_ueq(tests[i].a, tests[i].b);
        printf("  %s: asm=%f, ueq=%d\n", 
               tests[i].desc, asm_result, ueq_result);
    }
    
    /* Test 3: Vector comparisons */
    printf("\nPerforming vector comparisons...\n");
    vector_comparisons();
    
    /* Test 4: Command-line arguments if provided */
    if (argc >= 3) {
        double arg1 = atof(argv[1]);
        double arg2 = atof(argv[2]);
        
        /* Handle "nan" string */
        if (argc >= 4 && strcmp(argv[3], "nan") == 0) {
            arg1 = NAN;
        }
        
        printf("\nCommand-line test: %f vs %f\n", arg1, arg2);
        
        /* Force all comparison types */
        volatile int cmp_results[6];
        cmp_results[0] = (arg1 < arg2);
        cmp_results[1] = (arg1 > arg2);
        cmp_results[2] = (arg1 <= arg2);
        cmp_results[3] = (arg1 >= arg2);
        cmp_results[4] = (arg1 == arg2);
        cmp_results[5] = (arg1 != arg2);
        
        /* Use math.h macros */
        cmp_results[0] |= isunordered(arg1, arg2) << 6;
        cmp_results[1] |= isless(arg1, arg2) << 7;
        cmp_results[2] |= isgreater(arg1, arg2) << 8;
        cmp_results[3] |= islessequal(arg1, arg2) << 9;
        cmp_results[4] |= isgreaterequal(arg1, arg2) << 10;
        cmp_results[5] |= islessgreater(arg1, arg2) << 11;
        
        /* Prevent optimization */
        asm volatile ("" : : "m" (cmp_results));
    }
    
    /* Final output to prevent dead code elimination */
    printf("\nTotal results: %d, ASM results sum: %f\n", 
           total_results, asm_results);
    
    return 0;
}
