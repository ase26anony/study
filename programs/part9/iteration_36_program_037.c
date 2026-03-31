#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function to parse NaN from command line */
static double parse_double_or_nan(const char *str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NAN") == 0) {
        return NAN;
    }
    if (strcmp(str, "inf") == 0) {
        return INFINITY;
    }
    if (strcmp(str, "-inf") == 0) {
        return -INFINITY;
    }
    return atof(str);
}

/* Helper function that performs all possible comparisons */
static int compare_floats_full(double a, double b) {
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
    /* Force actual comparison by using volatile */
    volatile double va = a;
    volatile double vb = b;
    
    if (isunordered(va, vb)) {
        return "unordered";
    } else if (isless(va, vb)) {
        return "less";
    } else if (isgreater(va, vb)) {
        return "greater";
    } else if (va == vb) {
        /* Distinguish +0 and -0 */
        if (signbit(va) != signbit(vb)) {
            return "equal_opposite_sign";
        }
        return "equal";
    } else if (islessgreater(va, vb)) {
        return "less_or_greater";
    }
    return "unknown";
}

/* Inline assembly to force condition code output */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Pop remaining b */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $8, %%edx\n\t"
        "orl %%edx, %%eax"
        : "=a" (result)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    return result;
}

/* Another inline assembly variant with different condition codes */
static int fp_compare_asm2(double a, double b) {
    int result = 0;
    
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        /* Test various conditions */
        "jp 1f\n\t"             /* Jump if unordered */
        "jb 2f\n\t"             /* Jump if below */
        "ja 3f\n\t"             /* Jump if above */
        "je 4f\n\t"             /* Jump if equal */
        "movl $0, %0\n\t"
        "jmp 5f\n"
        "1:\n\t"
        "movl $1, %0\n\t"       /* Unordered */
        "jmp 5f\n"
        "2:\n\t"
        "movl $2, %0\n\t"       /* Less than */
        "jmp 5f\n"
        "3:\n\t"
        "movl $3, %0\n\t"       /* Greater than */
        "jmp 5f\n"
        "4:\n\t"
        "movl $4, %0\n\t"       /* Equal */
        "5:\n"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st"
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
    
    /* These vector comparisons may generate condition code output */
    v2df cmp1 = a < b;  /* May involve unordered comparisons */
    v2df cmp2 = c > d;
    v2df cmp3 = a == b;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "x" (cmp1), "x" (cmp2), "x" (cmp3));
}
#endif

/* Main test function */
static void run_comparisons(double a, double b, int test_num) {
    printf("Test %d: a=%g, b=%g\n", test_num, a, b);
    
    /* Force multiple comparison types */
    int res1 = compare_floats_full(a, b);
    const char *cls = classify_comparison(a, b);
    int res2 = fp_compare_asm(a, b);
    int res3 = fp_compare_asm2(a, b);
    
    /* Use results to prevent optimization */
    volatile int dummy = res1;
    (void)dummy;
    
    printf("  Classification: %s\n", cls);
    printf("  Compare result: 0x%x\n", res1);
    printf("  ASM1 result: %d\n", res2);
    printf("  ASM2 result: %d\n", res3);
}

int main(int argc, char *argv[]) {
    /* Test cases designed to trigger various condition codes */
    double test_cases[][2] = {
        {NAN, 1.0},           /* Unordered comparisons */
        {1.0, NAN},           /* Unordered comparisons */
        {NAN, NAN},           /* Both NaN */
        {INFINITY, -INFINITY},/* Ordered comparison */
        {0.0, -0.0},          /* Equal with different signs */
        {1.0, 2.0},           /* Less than */
        {2.0, 1.0},           /* Greater than */
        {3.14159, 3.14159},   /* Equal */
        {INFINITY, INFINITY}, /* Equal infinities */
        {-INFINITY, -INFINITY},/* Equal negative infinities */
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Add command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double_or_nan(argv[1]);
        double b = parse_double_or_nan(argv[2]);
        run_comparisons(a, b, 0);
    }
    
    /* Run all test cases */
    for (int i = 0; i < num_tests; i++) {
        run_comparisons(test_cases[i][0], test_cases[i][1], i + 1);
    }
    
#ifdef __SSE2__
    /* Vector comparisons */
    vector_comparisons();
    printf("Vector comparisons completed\n");
#endif
    
    /* Complex switch statement to force multiple condition code outputs */
    volatile double x = global_nan;
    volatile double y = 42.0;
    
    int switch_result = 0;
    if (isunordered(x, y)) switch_result = 1;
    else if (x < y) switch_result = 2;
    else if (x > y) switch_result = 3;
    else if (x == y) switch_result = 4;
    else if (x != y) switch_result = 5;
    else if (x <= y) switch_result = 6;
    else if (x >= y) switch_result = 7;
    
    printf("Final switch result: %d\n", switch_result);
    
    return 0;
}
