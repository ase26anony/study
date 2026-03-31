/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function to classify comparison results */
static const char* fp_compare_classify(double a, double b) {
    /* Use all possible comparison operators */
    if (a < b) return "LT";
    if (a > b) return "GT";
    if (a == b) return "EQ";
    if (a != b) return "NE";
    
    /* Unordered comparisons */
    if (isunordered(a, b)) return "UNORDERED";
    if (isless(a, b)) return "LESS";
    if (isgreater(a, b)) return "GREATER";
    if (islessequal(a, b)) return "LESSEQUAL";
    if (isgreaterequal(a, b)) return "GREATEREQUAL";
    if (islessgreater(a, b)) return "LESSGREATER";
    
    return "UNKNOWN";
}

/* Function with mixed ordered/unordered comparisons in control flow */
static int fp_compare_switch(double a, double b) {
    int result = 0;
    
    /* Complex switch-like logic using if-else chains */
    if (isunordered(a, b)) {
        result = 1; /* UNORDERED */
    } else if (a == b) {
        result = 2; /* EQ or UNEQ depending on context */
    } else if (a != b) {
        if (isless(a, b)) {
            result = 3; /* LT */
        } else if (isgreater(a, b)) {
            result = 4; /* GT */
        } else if (islessequal(a, b)) {
            result = 5; /* LE */
        } else if (isgreaterequal(a, b)) {
            result = 6; /* GE */
        } else if (islessgreater(a, b)) {
            result = 7; /* LTGT */
        }
    }
    
    /* Additional comparisons using math.h macros */
    if (isless(a, b)) result |= 0x10;
    if (isgreater(a, b)) result |= 0x20;
    if (islessequal(a, b)) result |= 0x40;
    if (isgreaterequal(a, b)) result |= 0x80;
    if (islessgreater(a, b)) result |= 0x100;
    if (isunordered(a, b)) result |= 0x200;
    
    return result;
}

/* Inline assembly to directly generate condition code output */
static int fp_compare_asm(double a, double b) {
    int result = 0;
    
    /* Force x87 floating-point comparison with unordered check */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clean up FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %0\n\t"   /* Move result */
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)", "eax"
    );
    
    return result;
}

/* Another inline assembly variant with different condition codes */
static int fp_compare_asm2(double a, double b) {
    int result;
    
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "sete %%al\n\t"         /* Set if equal (ZF=1) */
        "setc %%ah\n\t"         /* Set if below (CF=1) */
        "setp %%dl\n\t"         /* Set if unordered */
        "movzbl %%al, %0\n\t"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)", "eax", "edx"
    );
    
    return result;
}

/* Vector extensions for SSE comparisons */
#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));

static void vector_compare(v2df a, v2df b) {
    v2df cmp_lt = a < b;
    v2df cmp_gt = a > b;
    v2df cmp_eq = a == b;
    v2df cmp_ne = a != b;
    v2df cmp_le = a <= b;
    v2df cmp_ge = a >= b;
    
    /* Use results to prevent optimization */
    volatile v2df result = cmp_lt + cmp_gt + cmp_eq + cmp_ne + cmp_le + cmp_ge;
    (void)result;
}
#endif

/* Parse double from string, handling NaN */
static double parse_double(const char* str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NaN") == 0) {
        return NAN;
    } else if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0) {
        return INFINITY;
    } else if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0) {
        return -INFINITY;
    }
    return atof(str);
}

int main(int argc, char *argv[]) {
    /* Test cases with various floating-point values */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, -INFINITY},
        {INFINITY, 1.0},
        {-INFINITY, 1.0},
        {0.0, -0.0},
        {1.0, 2.0},
        {2.0, 1.0},
        {1.0, 1.0},
        {1.0, -1.0}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int total_results = 0;
    
    printf("Testing floating-point comparisons...\n");
    
    /* Test 1: Standard comparisons with NaN */
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Force compiler to generate comparison code */
        volatile int cmp_lt = (a < b);
        volatile int cmp_gt = (a > b);
        volatile int cmp_eq = (a == b);
        volatile int cmp_ne = (a != b);
        volatile int cmp_le = (a <= b);
        volatile int cmp_ge = (a >= b);
        
        total_results += cmp_lt + cmp_gt + cmp_eq + cmp_ne + cmp_le + cmp_ge;
        
        /* Use math.h comparison macros */
        volatile int is_unordered = isunordered(a, b);
        volatile int is_less = isless(a, b);
        volatile int is_greater = isgreater(a, b);
        volatile int is_lessequal = islessequal(a, b);
        volatile int is_greaterequal = isgreaterequal(a, b);
        volatile int is_lessgreater = islessgreater(a, b);
        
        total_results += is_unordered + is_less + is_greater + 
                        is_lessequal + is_greaterequal + is_lessgreater;
        
        /* Call classification function */
        const char* classification = fp_compare_classify(a, b);
        printf("Case %d: %g vs %g -> %s\n", i, a, b, classification);
    }
    
    /* Test 2: Inline assembly comparisons */
    printf("\nTesting inline assembly comparisons...\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        int asm_result1 = fp_compare_asm(a, b);
        int asm_result2 = fp_compare_asm2(a, b);
        int switch_result = fp_compare_switch(a, b);
        
        total_results += asm_result1 + asm_result2 + switch_result;
        
        printf("ASM results: %d, %d, switch: %d\n", 
               asm_result1, asm_result2, switch_result);
    }
    
#ifdef __SSE2__
    /* Test 3: Vector comparisons */
    printf("\nTesting vector comparisons...\n");
    v2df vec_a = {1.0, NAN};
    v2df vec_b = {NAN, 2.0};
    v2df vec_c = {INFINITY, -INFINITY};
    v2df vec_d = {0.0, -0.0};
    
    vector_compare(vec_a, vec_b);
    vector_compare(vec_c, vec_d);
    vector_compare(vec_a, vec_c);
#endif
    
    /* Test 4: Command-line argument comparisons */
    if (argc >= 3) {
        printf("\nTesting command-line arguments...\n");
        double arg1 = parse_double(argv[1]);
        double arg2 = parse_double(argv[2]);
        
        /* Complex conditional logic to force multiple comparisons */
        if (isunordered(arg1, arg2)) {
            printf("Arguments are unordered\n");
            if (arg1 < arg2) { /* This branch may be unreachable but forces codegen */
                printf("Unexpected: arg1 < arg2 when unordered\n");
            }
        } else if (arg1 < arg2) {
            printf("arg1 < arg2\n");
        } else if (arg1 > arg2) {
            printf("arg1 > arg2\n");
        } else if (arg1 == arg2) {
            printf("arg1 == arg2\n");
        } else {
            printf("arg1 != arg2 (but not < or >)\n");
        }
        
        /* More comparisons in different forms */
        volatile double temp = arg1;
        while (temp < arg2) { /* Loop condition forces comparison */
            temp += 1.0;
            if (temp >= arg2) break;
        }
    }
    
    printf("\nTotal results accumulated: %d\n", total_results);
    return total_results != 0 ? 0 : 1;
}
