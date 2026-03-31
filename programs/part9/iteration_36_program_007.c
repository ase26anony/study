/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector type for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Function that performs all possible comparisons */
static int compare_all(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* math.h macros that map to x86 condition codes */
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
    /* Force compiler to generate comparison instructions */
    volatile int cmp_eq = (a == b);
    volatile int cmp_lt = (a < b);
    volatile int cmp_gt = (a > b);
    volatile int cmp_un = isunordered(a, b);
    
    /* This switch may cause compiler to generate various condition codes */
    if (cmp_un) {
        return "unordered";
    } else if (cmp_lt) {
        return "less";
    } else if (cmp_gt) {
        return "greater";
    } else if (cmp_eq) {
        return "equal";
    } else {
        /* Should never reach here for valid FP numbers */
        return "unknown";
    }
}

/* Inline assembly to directly trigger condition code output */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* Using x87 FPU comparison - may generate 'fucomip' with condition codes */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clean up FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%ax, %0"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)", "ax"
    );
    
    return result;
}

/* SSE vector comparisons */
static void vector_comparisons(void) {
    v2df vec_a, vec_b, vec_cmp;
    
    /* Initialize with mixed values including NaN */
    vec_a = (v2df){1.0, global_nan};
    vec_b = (v2df){global_nan, 2.0};
    
    /* Vector comparison - may generate multiple condition codes */
    vec_cmp = vec_a < vec_b;
    
    /* Prevent dead code elimination */
    volatile v2df keep = vec_cmp;
    (void)keep;
}

/* Main test function */
int main(int argc, char *argv[]) {
    double test_values[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, -INFINITY},
        {INFINITY, INFINITY},
        {-INFINITY, -INFINITY},
        {0.0, -0.0},           /* +0.0 equals -0.0 but sign differs */
        {DBL_MAX, DBL_MIN},
        {1.0, 2.0},
        {2.0, 1.0},
        {1.0, 1.0}
    };
    
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    int total_results = 0;
    
    printf("Testing floating-point comparisons to trigger x86 condition code output...\n");
    
    /* Test 1: Standard comparisons */
    for (int i = 0; i < num_tests; i++) {
        double a = test_values[i][0];
        double b = test_values[i][1];
        
        /* Force runtime evaluation */
        volatile double va = a;
        volatile double vb = b;
        
        int result = compare_all(va, vb);
        total_results += result;
        
        const char *cls = classify_comparison(va, vb);
        printf("Test %d: compare_all(%.2f, %.2f) = 0x%x, class = %s\n", 
               i, va, vb, result, cls);
    }
    
    /* Test 2: Inline assembly comparisons */
    printf("\nInline assembly comparisons:\n");
    for (int i = 0; i < num_tests; i++) {
        double a = test_values[i][0];
        double b = test_values[i][1];
        
        int asm_result = fp_compare_asm(a, b);
        printf("  asm_compare(%.2f, %.2f) = 0x%02x\n", a, b, asm_result);
        total_results += asm_result;
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
        
        printf("\nCommand-line test: compare_all(%g, %g) = 0x%x\n",
               arg1, arg2, compare_all(arg1, arg2));
        
        /* Complex expression to force multiple condition codes */
        volatile double x = arg1;
        volatile double y = arg2;
        
        /* This complex if-else chain should generate various condition codes */
        if (isunordered(x, y)) {
            printf("  Result: unordered\n");
        } else if (x < y) {
            printf("  Result: less\n");
        } else if (x > y) {
            printf("  Result: greater\n");
        } else if (x == y) {
            printf("  Result: equal\n");
        } else {
            printf("  Result: impossible\n");
        }
        
        /* More comparisons using different operators */
        volatile int cmp1 = (x <= y);
        volatile int cmp2 = (x >= y);
        volatile int cmp3 = (x != y);
        volatile int cmp4 = (x == y);
        
        total_results += cmp1 + cmp2 + cmp3 + cmp4;
    }
    
    /* Test 5: Additional unordered comparisons */
    printf("\nTesting specific unordered conditions:\n");
    
    /* UNEQ: unordered or equal */
    volatile double un = NAN;
    volatile double n = 1.0;
    
    /* These should generate various condition code mnemonics */
    if (isunordered(un, n) || un == n) {
        printf("  UNEQ condition triggered\n");
    }
    
    /* UNGE: unordered or greater or equal (not less than) */
    if (isunordered(un, n) || un >= n) {
        printf("  UNGE condition triggered\n");
    }
    
    /* UNGT: unordered or greater (not less or equal) */
    if (isunordered(un, n) || un > n) {
        printf("  UNGT condition triggered\n");
    }
    
    /* UNLE: unordered or less or equal */
    if (isunordered(un, n) || un <= n) {
        printf("  UNLE condition triggered\n");
    }
    
    /* UNLT: unordered or less than */
    if (isunordered(un, n) || un < n) {
        printf("  UNLT condition triggered\n");
    }
    
    /* LTGT: less or greater (not equal and not unordered) */
    if (islessgreater(n, 2.0)) {
        printf("  LTGT condition triggered\n");
    }
    
    /* Prevent dead code elimination */
    printf("\nTotal results checksum: %d\n", total_results);
    
    return total_results != 0 ? 0 : 1;
}
