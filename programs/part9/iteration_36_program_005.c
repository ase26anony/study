/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double g_volatile_double = 0.0;

/* Helper to parse NaN from command line */
static double parse_double(const char *str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NAN") == 0) {
        return NAN;
    }
    if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0) {
        return INFINITY;
    }
    if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0) {
        return -INFINITY;
    }
    return atof(str);
}

/* Function that performs all possible floating-point comparisons */
static int compare_all(double a, double b) {
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
static const char* classify_comparison(double a, double b) {
    /* This switch should generate various condition code outputs */
    if (isunordered(a, b)) {
        return "unordered";
    } else if (isless(a, b)) {
        return "less";
    } else if (isgreater(a, b)) {
        return "greater";
    } else if (a == b) {
        /* Distinguish +0 and -0 */
        if (signbit(a) != signbit(b)) {
            return "equal_opposite_sign";
        }
        return "equal";
    } else if (islessgreater(a, b)) {
        return "less_or_greater";
    }
    return "unknown";
}

/* Inline assembly to directly trigger condition code output */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto x87 stack */
        "fldl %1\n\t"           /* Load a onto x87 stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clear stack */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $8, %%edx\n\t"
        "or %%edx, %%eax\n\t"
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
        "movl $2, %0\n\t"       /* Below */
        "jmp 5f\n"
        "3:\n\t"
        "movl $3, %0\n\t"       /* Above */
        "jmp 5f\n"
        "4:\n\t"
        "movl $4, %0\n\t"       /* Equal */
        "5:\n\t"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    return result;
}

/* Vector comparisons using GCC extensions */
#ifdef __SSE2__
static void vector_comparisons(void) {
    typedef double v2df __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    volatile v2df a = {NAN, 1.0};
    volatile v2df b = {2.0, NAN};
    volatile v2df c = {3.0, 4.0};
    volatile v2df d = {5.0, 6.0};
    
    /* These vector comparisons may generate condition code output */
    v2df cmp1 = a < b;  /* Should have NaN comparisons */
    v2df cmp2 = c <= d;
    v2df cmp3 = a == b;
    
    /* Use results to prevent optimization */
    v2di *p1 = (v2di*)&cmp1;
    v2di *p2 = (v2di*)&cmp2;
    v2di *p3 = (v2di*)&cmp3;
    
    g_volatile_double = (*p1)[0] + (*p2)[0] + (*p3)[0];
}
#endif

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Test cases designed to trigger all condition codes */
    struct {
        double a, b;
        const char *desc;
    } test_cases[] = {
        {NAN, 1.0, "NaN vs 1.0"},
        {1.0, NAN, "1.0 vs NaN"},
        {NAN, NAN, "NaN vs NaN"},
        {INFINITY, -INFINITY, "Inf vs -Inf"},
        {INFINITY, 1.0, "Inf vs 1.0"},
        {-INFINITY, 1.0, "-Inf vs 1.0"},
        {0.0, -0.0, "+0 vs -0"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {DBL_MAX, DBL_MAX, "DBL_MAX vs DBL_MAX"},
        {DBL_MIN, DBL_MIN, "DBL_MIN vs DBL_MIN"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Testing command line values: %g, %g\n", a, b);
        
        int cmp_result = compare_all(a, b);
        const char *cls = classify_comparison(a, b);
        int asm_result1 = fp_compare_asm(a, b);
        int asm_result2 = fp_compare_asm2(a, b);
        
        printf("  compare_all: 0x%x\n", cmp_result);
        printf("  classification: %s\n", cls);
        printf("  asm_result1: 0x%x\n", asm_result1);
        printf("  asm_result2: 0x%x\n", asm_result2);
        
        g_volatile_double = a + b + cmp_result + asm_result1 + asm_result2;
    }
    
    /* Run all test cases */
    printf("\nRunning comprehensive test cases:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i].a;
        double b = test_cases[i].b;
        
        /* Force compiler to generate actual comparisons */
        volatile int cmp_result = compare_all(a, b);
        volatile const char *cls = classify_comparison(a, b);
        volatile int asm_result1 = fp_compare_asm(a, b);
        volatile int asm_result2 = fp_compare_asm2(a, b);
        
        printf("Test %d: %s\n", i, test_cases[i].desc);
        printf("  a=%g, b=%g\n", a, b);
        printf("  cmp_result=0x%x, class=%s\n", cmp_result, cls);
        
        /* Accumulate to prevent dead code elimination */
        g_volatile_double += a + b + cmp_result + asm_result1 + asm_result2;
    }
    
#ifdef __SSE2__
    printf("\nRunning vector comparisons...\n");
    vector_comparisons();
#endif
    
    /* Final output to use all results */
    printf("\nFinal volatile value: %g\n", g_volatile_double);
    
    return (int)g_volatile_double;
}
