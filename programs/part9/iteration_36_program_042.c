#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Force compiler to generate actual comparison instructions */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;
volatile double global_zero = 0.0;
volatile double global_neg_zero = -0.0;

/* Vector type for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Function to perform all possible comparisons */
int compare_all_results(double a, double b) {
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
    /* Force actual comparisons by using volatile */
    volatile double va = a;
    volatile double vb = b;
    
    if (isunordered(va, vb)) {
        return "unordered";
    } else if (va < vb) {
        return "less";
    } else if (va > vb) {
        return "greater";
    } else if (va == vb) {
        return "equal";
    } else {
        /* This case handles NaN comparisons */
        return "other";
    }
}

/* Inline assembly to directly use floating-point condition codes */
double inline_asm_fp_compare(double a, double b) {
    double result;
    int unordered_flag, greater_flag, less_flag;
    
    /* Using x87 FPU comparison with inline assembly */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clear FPU stack */
        "setp %b0\n\t"          /* Set if unordered (parity flag) */
        "seta %b1\n\t"          /* Set if above (greater) */
        "setb %b2\n\t"          /* Set if below (less) */
        : "=r"(unordered_flag), "=r"(greater_flag), "=r"(less_flag)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* Another inline assembly with different condition code */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "fcmovbe %3, %0\n\t"    /* Conditional move if below or equal */
        : "=t"(result)
        : "m"(a), "m"(b), "r"(3.14159)
        : "cc", "st"
    );
    
    return result;
}

/* Vector comparison function */
void vector_comparisons(void) {
    v2df vec_a = {1.0, NAN};
    v2df vec_b = {NAN, 2.0};
    v2df vec_c = {3.0, 4.0};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp1 = vec_a < vec_b;  /* May generate UNORDERED conditions */
    v2df cmp2 = vec_a > vec_c;
    v2df cmp3 = vec_a == vec_b;
    
    /* Prevent optimization */
    asm volatile("" : : "x"(cmp1), "x"(cmp2), "x"(cmp3));
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Array of test cases including NaN values */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, -INFINITY},
        {0.0, -0.0},           /* +0.0 == -0.0 but bitwise different */
        {1.0, 2.0},
        {2.0, 1.0},
        {1.0, 1.0},
        {INFINITY, 1.0},
        {1.0, INFINITY},
        {-INFINITY, 1.0},
        {1.0, -INFINITY}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int total_results = 0;
    
    /* Parse command line arguments if provided */
    if (argc > 2) {
        double arg1 = atof(argv[1]);
        double arg2 = atof(argv[2]);
        
        /* Check for "nan" string */
        if (strcmp(argv[1], "nan") == 0) arg1 = NAN;
        if (strcmp(argv[2], "nan") == 0) arg2 = NAN;
        if (strcmp(argv[1], "inf") == 0) arg1 = INFINITY;
        if (strcmp(argv[2], "inf") == 0) arg2 = INFINITY;
        if (strcmp(argv[1], "-inf") == 0) arg1 = -INFINITY;
        if (strcmp(argv[2], "-inf") == 0) arg2 = -INFINITY;
        
        test_cases[0][0] = arg1;
        test_cases[0][1] = arg2;
        num_cases = 1;  /* Only test the provided values */
    }
    
    printf("Testing floating-point comparisons...\n");
    
    /* Test each case */
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Prevent constant folding */
        volatile double va = a;
        volatile double vb = b;
        
        printf("\nTest case %d: a=%g, b=%g\n", i, va, vb);
        
        /* Perform all comparisons */
        int results = compare_all_results(va, vb);
        total_results |= results;
        
        /* Classify the comparison */
        const char *classification = classify_comparison(va, vb);
        printf("  Classification: %s\n", classification);
        
        /* Use inline assembly comparisons */
        double asm_result = inline_asm_fp_compare(va, vb);
        printf("  Assembly result: %g\n", asm_result);
        
        /* Additional comparisons to trigger different condition codes */
        if (islessgreater(va, vb)) {
            printf("  islessgreater true\n");
        }
        if (!islessgreater(va, vb) && !isunordered(va, vb)) {
            printf("  Ordered and not lessgreater\n");
        }
    }
    
    /* Test vector comparisons */
    vector_comparisons();
    
    /* Final summary to prevent dead code elimination */
    printf("\nTotal results mask: 0x%x\n", total_results);
    
    /* Additional complex conditional to trigger more code generation */
    double x = global_nan;
    double y = global_inf;
    
    /* This complex conditional may generate multiple condition code checks */
    if ((x < y) || (x > y) || (x == y) || (x != y) || 
        isunordered(x, y) || isless(x, y) || isgreater(x, y) ||
        islessequal(x, y) || isgreaterequal(x, y) || islessgreater(x, y)) {
        printf("Complex condition triggered\n");
    }
    
    /* Switch-like behavior based on fpclassify */
    int class_x = fpclassify(x);
    switch (class_x) {
        case FP_NAN: printf("x is NaN\n"); break;
        case FP_INFINITE: printf("x is infinite\n"); break;
        case FP_ZERO: printf("x is zero\n"); break;
        case FP_SUBNORMAL: printf("x is subnormal\n"); break;
        case FP_NORMAL: printf("x is normal\n"); break;
    }
    
    return total_results != 0 ? 0 : 1;
}
