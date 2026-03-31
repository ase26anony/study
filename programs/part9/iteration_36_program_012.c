#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector type for SSE/AVX comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function that performs all possible floating-point comparisons */
int compare_doubles_full(double a, double b) {
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
    if (islessgreater(a, b)) result |= 2048;  /* LTGT */
    
    return result;
}

/* Function with switch based on comparison results */
const char* classify_comparison(double a, double b) {
    /* Force actual comparison by using volatile */
    volatile double va = a;
    volatile double vb = b;
    
    if (isunordered(va, vb)) {
        return "UNORDERED";
    }
    
    /* Ordered comparisons */
    if (va < vb) return "LT";
    if (va > vb) return "GT";
    if (va == vb) return "EQ";
    
    /* Special cases for unordered comparisons */
    if (!isless(va, vb) && !isgreater(va, vb) && !isunordered(va, vb)) 
        return "UNEQ";
    if (!isless(va, vb)) 
        return "UNLT";  /* nlt */
    if (!isgreater(va, vb)) 
        return "UNGT";  /* nle */
    
    return "UNKNOWN";
}

/* Inline assembly to directly trigger condition code output */
double inline_asm_fpu_compare(double a, double b) {
    double result;
    int unordered_flag;
    
    /* Using x87 FPU comparison - triggers UNORDERED condition code */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "movzbl %%al, %0\n\t"   /* Move result */
        "fstp %%st(0)\n\t"      /* Clean up FPU stack */
        : "=r" (unordered_flag)
        : "m" (a), "m" (b)
        : "al", "cc", "st"
    );
    
    result = unordered_flag ? global_nan : a;
    
    /* Another inline asm with different condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        : /* no output */
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    return result;
}

/* Vector comparisons using SSE/AVX */
void vector_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {global_nan, 3.0};
    v2df vec3 = {global_inf, global_neg_inf};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp_lt = vec1 < vec2;    /* May have UNORDERED elements */
    v2df cmp_gt = vec1 > vec3;
    v2df cmp_eq = vec1 == vec2;
    
    /* Prevent dead code elimination */
    volatile v2df volatile_cmp = cmp_lt;
    (void)volatile_cmp;
}

/* Parse command line argument to double, handling "nan", "inf" */
double parse_fp_arg(const char *arg) {
    if (strcmp(arg, "nan") == 0 || strcmp(arg, "NAN") == 0) {
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
    /* Test cases designed to trigger various condition codes */
    double test_cases[][2] = {
        {NAN, 1.0},           /* UNORDERED */
        {1.0, NAN},           /* UNORDERED */
        {NAN, NAN},           /* UNORDERED */
        {INFINITY, -INFINITY}, /* GT */
        {INFINITY, INFINITY},  /* EQ */
        {-INFINITY, INFINITY}, /* LT */
        {0.0, -0.0},          /* EQ (but special) */
        {DBL_MAX, DBL_MAX * 0.5}, /* GT */
        {DBL_MIN, DBL_MIN},   /* EQ */
        {1.0, 2.0},           /* LT */
        {2.0, 1.0},           /* GT */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_fp_arg(argv[1]);
        double b = parse_fp_arg(argv[2]);
        
        printf("Testing with command line values: %g, %g\n", a, b);
        
        /* Perform all comparison types */
        int result = compare_doubles_full(a, b);
        printf("Comparison result mask: 0x%x\n", result);
        
        const char *classification = classify_comparison(a, b);
        printf("Classification: %s\n", classification);
        
        /* Use inline assembly */
        double asm_result = inline_asm_fpu_compare(a, b);
        printf("Inline ASM result: %g\n", asm_result);
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Volatile to prevent optimization */
        volatile double va = a;
        volatile double vb = b;
        
        /* Force multiple comparison types */
        int cmp_results = compare_doubles_full(va, vb);
        
        /* Use results to prevent dead code elimination */
        if (cmp_results & 64) {  /* isunordered */
            printf("Case %d: UNORDERED detected\n", i);
        }
        
        /* Call classification function */
        const char *cls = classify_comparison(va, vb);
        printf("  Classification: %s\n", cls);
    }
    
    /* Vector comparisons */
    printf("\nPerforming vector comparisons...\n");
    vector_comparisons();
    
    /* Additional complex control flow with switch */
    printf("\nComplex control flow test:\n");
    for (int i = 0; i < num_cases; i++) {
        volatile double a = test_cases[i][0];
        volatile double b = test_cases[i][1];
        
        /* Complex if-else chain that may generate various condition codes */
        if (isunordered(a, b)) {
            printf("  Case %d: Unordered\n", i);
        } else if (a < b) {
            printf("  Case %d: Less than\n", i);
        } else if (a > b) {
            printf("  Case %d: Greater than\n", i);
        } else if (a == b) {
            printf("  Case %d: Equal\n", i);
        } else {
            /* This branch should be unreachable but forces compiler
               to consider all comparison possibilities */
            printf("  Case %d: Unexpected\n", i);
        }
        
        /* Test for UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT */
        if (!isless(a, b) && !isgreater(a, b) && !isunordered(a, b)) {
            printf("    UNEQ condition true\n");
        }
        if (!isless(a, b)) {
            printf("    UNGE/NLT condition true\n");
        }
        if (!isgreater(a, b)) {
            printf("    UNLE/UNG condition true\n");
        }
        if (islessgreater(a, b)) {
            printf("    LTGT/UNE condition true\n");
        }
    }
    
    /* Final inline assembly with explicit condition code usage */
    printf("\nFinal inline assembly tests:\n");
    for (int i = 0; i < 3; i++) {
        volatile double a = test_cases[i][0];
        volatile double b = test_cases[i][1];
        
        int is_unordered;
        int is_less;
        int is_greater;
        
        /* Multiple inline asm blocks to trigger different condition codes */
        asm volatile (
            "fldl %2\n\t"
            "fldl %1\n\t"
            "fucomip %%st(1), %%st\n\t"
            "setp %%al\n\t"
            "setb %%cl\n\t"
            "seta %%dl\n\t"
            "movzbl %%al, %0\n\t"
            "movzbl %%cl, %3\n\t"
            "movzbl %%dl, %4\n\t"
            "fstp %%st(0)"
            : "=r" (is_unordered), "=r" (is_less), "=r" (is_greater)
            : "m" (a), "m" (b), "0" (is_unordered), "1" (is_less), "2" (is_greater)
            : "al", "cl", "dl", "cc", "st"
        );
        
        printf("  Test %d: Unordered=%d, Less=%d, Greater=%d\n", 
               i, is_unordered, is_less, is_greater);
    }
    
    return 0;
}
