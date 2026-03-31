#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function to classify comparison results */
static int classify_comparison(double a, double b) {
    int result = 0;
    
    /* Use all possible comparison operators */
    if (a < b) result |= 0x01;
    if (a > b) result |= 0x02;
    if (a <= b) result |= 0x04;
    if (a >= b) result |= 0x08;
    if (a == b) result |= 0x10;
    if (a != b) result |= 0x20;
    
    /* Use math.h macros for unordered comparisons */
    if (isunordered(a, b)) result |= 0x40;
    if (isless(a, b)) result |= 0x80;
    if (isgreater(a, b)) result |= 0x100;
    if (islessequal(a, b)) result |= 0x200;
    if (isgreaterequal(a, b)) result |= 0x400;
    if (islessgreater(a, b)) result |= 0x800;
    
    return result;
}

/* Function that forces generation of various condition codes */
static void perform_comparisons(double x, double y) {
    volatile int res;
    
    /* Series of if-else statements to generate multiple condition codes */
    if (x < y) {
        res = 1;
        asm volatile("" : "+r"(res) : : "cc");
    } else if (x > y) {
        res = 2;
        asm volatile("" : "+r"(res) : : "cc");
    } else if (x == y) {
        res = 3;
        asm volatile("" : "+r"(res) : : "cc");
    } else if (x != y) {
        res = 4;
        asm volatile("" : "+r"(res) : : "cc");
    } else if (isunordered(x, y)) {
        res = 5;
        asm volatile("" : "+r"(res) : : "cc");
    } else if (isless(x, y)) {
        res = 6;
        asm volatile("" : "+r"(res) : : "cc");
    } else if (isgreater(x, y)) {
        res = 7;
        asm volatile("" : "+r"(res) : : "cc");
    } else if (islessequal(x, y)) {
        res = 8;
        asm volatile("" : "+r"(res) : : "cc");
    } else if (isgreaterequal(x, y)) {
        res = 9;
        asm volatile("" : "+r"(res) : : "cc");
    } else if (islessgreater(x, y)) {
        res = 10;
        asm volatile("" : "+r"(res) : : "cc");
    }
    
    /* Complex switch based on comparison results */
    int cmp_result = classify_comparison(x, y);
    switch (cmp_result & 0x3F) {
        case 0x01: res = 100; break;  /* a < b */
        case 0x02: res = 101; break;  /* a > b */
        case 0x04: res = 102; break;  /* a <= b */
        case 0x08: res = 103; break;  /* a >= b */
        case 0x10: res = 104; break;  /* a == b */
        case 0x20: res = 105; break;  /* a != b */
        case 0x40: res = 106; break;  /* unordered */
        default:   res = 107; break;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(res));
}

/* Inline assembly that directly uses floating-point condition codes */
static int inline_asm_fp_compare(double a, double b) {
    int result;
    
    /* Using x87 FPU comparison with unordered handling */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "sete %%ah\n\t"         /* Set if equal */
        "setb %%dl\n\t"         /* Set if below (a < b) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%ecx\n\t"
        "movzbl %%dl, %%edx\n\t"
        "shl $8, %%ecx\n\t"
        "or %%ecx, %%eax\n\t"
        "shl $16, %%edx\n\t"
        "or %%edx, %%eax\n\t"
        : "=a"(result)
        : "m"(a), "m"(b)
        : "cc", "st", "dx", "cx"
    );
    
    return result;
}

/* Vector extensions for SSE comparisons */
#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));

static void vector_comparisons(void) {
    volatile v2df a = {1.0, NAN};
    volatile v2df b = {NAN, 2.0};
    volatile v2df c = {3.0, 4.0};
    volatile v2df d = {5.0, 6.0};
    
    /* Perform vector comparisons */
    v2df cmp1 = a < b;  /* May generate unordered comparisons */
    v2df cmp2 = c > d;
    v2df cmp3 = a == b;
    v2df cmp4 = c != d;
    
    /* Use results to prevent optimization */
    asm volatile("" : : "x"(cmp1), "x"(cmp2), "x"(cmp3), "x"(cmp4));
}
#endif

/* Parse command line argument to double, handling NaN */
static double parse_double_arg(const char *arg) {
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
        double a = parse_double_arg(argv[1]);
        double b = parse_double_arg(argv[2]);
        
        /* Perform comparisons with user-provided values */
        perform_comparisons(a, b);
        int asm_result = inline_asm_fp_compare(a, b);
        printf("Inline assembly result: 0x%08x\n", asm_result);
        
        /* Classify the comparison */
        int classification = classify_comparison(a, b);
        printf("Classification: 0x%08x\n", classification);
    }
    
    /* Run through all test cases */
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Force compiler to generate comparison code */
        perform_comparisons(a, b);
        
        /* Use inline assembly for direct condition code generation */
        int asm_res = inline_asm_fp_compare(a, b);
        
        /* Prevent optimization */
        asm volatile("" : : "r"(asm_res));
    }
    
#ifdef __SSE2__
    /* Use vector comparisons if SSE2 is available */
    vector_comparisons();
#endif
    
    /* Additional unordered comparison scenarios */
    volatile double x = global_nan;
    volatile double y = 3.14159;
    
    /* Complex conditional with unordered results */
    if (isunordered(x, y) || x < y || x > y || x == y) {
        volatile int marker = 1;
        asm volatile("" : "+r"(marker));
    }
    
    /* Use all math.h comparison macros */
    volatile int checks = 0;
    checks += isunordered(x, y) ? 1 : 0;
    checks += isless(x, y) ? 2 : 0;
    checks += isgreater(x, y) ? 4 : 0;
    checks += islessequal(x, y) ? 8 : 0;
    checks += isgreaterequal(x, y) ? 16 : 0;
    checks += islessgreater(x, y) ? 32 : 0;
    
    /* Final output to prevent dead code elimination */
    printf("Final checks: %d\n", checks);
    
    return 0;
}
