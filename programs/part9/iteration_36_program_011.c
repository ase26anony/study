#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function to perform various floating-point comparisons */
static int compare_floats(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map to x86 unordered comparisons */
    if (isunordered(a, b)) result |= 64;
    if (isless(a, b)) result |= 128;
    if (isgreater(a, b)) result |= 256;
    if (islessequal(a, b)) result |= 512;
    if (isgreaterequal(a, b)) result |= 1024;
    if (islessgreater(a, b)) result |= 2048;
    
    return result;
}

/* Function that uses inline assembly to force condition code output */
static double inline_asm_fpu_compare(double a, double b) {
    double result;
    uint8_t unordered_flag, equal_flag, less_flag;
    
    /* x87 floating-point comparison with unordered check */
    __asm__ volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clean up FPU stack */
        "setp %3\n\t"           /* Set if unordered (parity flag) */
        "sete %4\n\t"           /* Set if equal */
        "setb %5\n\t"           /* Set if below (a < b) */
        : "=m"(result), "+t"(a), "+u"(b), 
          "=r"(unordered_flag), "=r"(equal_flag), "=r"(less_flag)
        : 
        : "cc", "st"
    );
    
    /* Use the flags to determine result */
    if (unordered_flag) {
        result = global_nan;
    } else if (equal_flag) {
        result = 0.0;
    } else if (less_flag) {
        result = -1.0;
    } else {
        result = 1.0;
    }
    
    return result;
}

/* Vector extension comparisons */
#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));

static void vector_comparisons(void) {
    v2df a = {1.0, NAN};
    v2df b = {NAN, 1.0};
    v2df c = {2.0, 3.0};
    v2df d = {1.0, 2.0};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp1 = a < b;  /* Contains unordered results */
    v2df cmp2 = c > d;
    v2df cmp3 = a == b;
    
    /* Use volatile to prevent optimization */
    volatile v2df* vptr = &cmp1;
    (void)vptr;
}
#endif

/* Main test function */
int main(int argc, char *argv[]) {
    /* Array of test cases with various combinations */
    struct {
        double a, b;
        const char* desc;
    } test_cases[] = {
        {NAN, 1.0, "NAN vs 1.0"},
        {1.0, NAN, "1.0 vs NAN"},
        {NAN, NAN, "NAN vs NAN"},
        {INFINITY, -INFINITY, "INF vs -INF"},
        {INFINITY, 1.0, "INF vs 1.0"},
        {1.0, INFINITY, "1.0 vs INF"},
        {0.0, -0.0, "0.0 vs -0.0"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Parse command line arguments if provided */
    if (argc > 2) {
        double arg1 = atof(argv[1]);
        double arg2 = atof(argv[2]);
        
        /* Handle "nan" string */
        if (strcmp(argv[1], "nan") == 0) arg1 = NAN;
        if (strcmp(argv[2], "nan") == 0) arg2 = NAN;
        if (strcmp(argv[1], "inf") == 0) arg1 = INFINITY;
        if (strcmp(argv[2], "inf") == 0) arg2 = INFINITY;
        
        test_cases[0].a = arg1;
        test_cases[0].b = arg2;
        num_cases = 1;
    }
    
    int total_results = 0;
    
    /* Test each case */
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i].a;
        double b = test_cases[i].b;
        
        printf("Test %d: %s\n", i, test_cases[i].desc);
        
        /* Perform comparisons using standard operators */
        int cmp_result = compare_floats(a, b);
        total_results += cmp_result;
        
        /* Use inline assembly for direct condition code generation */
        double asm_result = inline_asm_fpu_compare(a, b);
        total_results += (int)asm_result;
        
        /* Additional comparisons in control flow to generate more condition codes */
        if (a < b) {
            printf("  a < b\n");
            total_results += 1000;
        }
        if (a > b) {
            printf("  a > b\n");
            total_results += 2000;
        }
        if (a <= b) {
            printf("  a <= b\n");
            total_results += 3000;
        }
        if (a >= b) {
            printf("  a >= b\n");
            total_results += 4000;
        }
        if (a == b) {
            printf("  a == b\n");
            total_results += 5000;
        }
        if (a != b) {
            printf("  a != b\n");
            total_results += 6000;
        }
        
        /* Switch based on comparison results */
        switch (fpclassify(a)) {
            case FP_NAN:
                total_results += 7000;
                break;
            case FP_INFINITE:
                total_results += 8000;
                break;
            case FP_ZERO:
                total_results += 9000;
                break;
            default:
                total_results += 10000;
        }
    }
    
#ifdef __SSE2__
    /* Vector comparisons */
    vector_comparisons();
    total_results += 11000;
#endif
    
    /* Final output to prevent dead code elimination */
    printf("Total results checksum: %d\n", total_results);
    
    /* Additional complex conditional that might generate LTGT, UNEQ, etc. */
    volatile double x = global_nan;
    volatile double y = 0.0;
    
    /* This complex condition may generate multiple comparison types */
    if ((x < y) || (x > y) || (x == y) || (x != y)) {
        printf("Complex condition triggered\n");
    }
    
    /* Use all the math.h comparison macros */
    double test_a = global_nan;
    double test_b = global_inf;
    
    int all_comparisons = 
        isunordered(test_a, test_b) +
        isless(test_a, test_b) * 2 +
        isgreater(test_a, test_b) * 3 +
        islessequal(test_a, test_b) * 4 +
        isgreaterequal(test_a, test_b) * 5 +
        islessgreater(test_a, test_b) * 6;
    
    printf("All comparisons result: %d\n", all_comparisons);
    
    return total_results != 0 ? 0 : 1;
}
