#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Force compiler to generate x87 FPU instructions */
#ifdef __GNUC__
__attribute__((optimize("no-fast-math")))
#endif
static int classify_comparison(double a, double b) {
    int result = 0;
    
    /* Use all possible comparison operators to generate different condition codes */
    if (a < b) result |= 1;      /* LT - less than */
    if (a > b) result |= 2;      /* GT - greater than */
    if (a <= b) result |= 4;     /* LE - less or equal */
    if (a >= b) result |= 8;     /* GE - greater or equal */
    if (a == b) result |= 16;    /* EQ - equal */
    if (a != b) result |= 32;    /* NEQ - not equal */
    
    /* Use math.h macros that map directly to x86 condition codes */
    if (isunordered(a, b)) result |= 64;      /* UNORDERED */
    if (isless(a, b)) result |= 128;          /* LT (ordered) */
    if (isgreater(a, b)) result |= 256;       /* GT (ordered) */
    if (islessequal(a, b)) result |= 512;     /* LE (ordered) */
    if (isgreaterequal(a, b)) result |= 1024; /* GE (ordered) */
    if (islessgreater(a, b)) result |= 2048;  /* LTGT */
    
    return result;
}

/* Function with inline assembly to directly trigger condition code output */
#ifdef __GNUC__
__attribute__((noinline))
static int fp_compare_asm(double x, double y) {
    int result;
    
    /* Force x87 FPU comparison with unordered handling */
    asm volatile (
        "fldl %2\n\t"           /* Load y onto FPU stack */
        "fldl %1\n\t"           /* Load x onto FPU stack, x in st(0), y in st(1) */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop st(0) */
        "fstp %%st(0)\n\t"      /* Clear remaining y from stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $1, %%edx\n\t"
        "or %%edx, %%eax\n\t"
        : "=a" (result)
        : "m" (x), "m" (y)
        : "cc", "st"
    );
    
    return result;
}

/* More complex inline assembly with multiple condition codes */
__attribute__((noinline))
static void generate_all_conditions(double a, double b) {
    volatile int res1, res2, res3, res4;
    
    /* Generate UNORDERED condition code */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setp %0\n\t"
        : "=r" (res1)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    /* Generate ORDERED condition code */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setnp %0\n\t"
        : "=r" (res2)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    /* Generate UNEQ condition code (unordered or equal) */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setpe %0\n\t"  /* parity or equal */
        : "=r" (res3)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    /* Generate UNGE condition code (unordered or greater or equal) */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setae %0\n\t"  /* above or equal (CF=0) */
        : "=r" (res4)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    /* Prevent dead code elimination */
    printf("ASM results: %d %d %d %d\n", res1, res2, res3, res4);
}
#endif

/* Use vector extensions to generate packed comparisons */
#ifdef __GNUC__
typedef double v2df __attribute__((vector_size(16)));

__attribute__((noinline))
static v2df vector_compare(v2df a, v2df b) {
    /* These comparisons may generate multiple condition codes */
    v2df lt = a < b;    /* Less than */
    v2df gt = a > b;    /* Greater than */
    v2df eq = a == b;   /* Equal */
    v2df ne = a != b;   /* Not equal */
    v2df le = a <= b;   /* Less or equal */
    v2df ge = a >= b;   /* Greater or equal */
    
    /* Combine results */
    return lt + gt * 2.0 + eq * 4.0 + ne * 8.0 + le * 16.0 + ge * 32.0;
}
#endif

/* Parse double with NaN support */
static double parse_double(const char *str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NaN") == 0) {
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

int main(int argc, char *argv[]) {
    /* Prevent constant folding with volatile */
    volatile double test_values[8];
    int i;
    
    /* Initialize test cases with various combinations including NaN */
    double test_cases[][2] = {
        {NAN, 1.0},           /* UNORDERED comparisons */
        {1.0, NAN},           /* UNORDERED comparisons */
        {INFINITY, -INFINITY},/* Ordered comparisons */
        {0.0, -0.0},          /* EQ (0.0 == -0.0) */
        {1.0, 2.0},           /* LT */
        {2.0, 1.0},           /* GT */
        {1.0, 1.0},           /* EQ */
        {NAN, NAN},           /* UNORDERED */
        {INFINITY, INFINITY}, /* EQ */
        {-INFINITY, -INFINITY},/* EQ */
        {1.0, INFINITY},      /* LT */
        {INFINITY, 1.0},      /* GT */
    };
    
    /* Parse command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Testing with command line values: %g, %g\n", a, b);
        
        /* Force generation of all comparison types */
        int result = classify_comparison(a, b);
        printf("Comparison result: %d\n", result);
        
        #ifdef __GNUC__
        int asm_result = fp_compare_asm(a, b);
        printf("ASM comparison result: %d\n", asm_result);
        
        generate_all_conditions(a, b);
        #endif
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
    for (i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Store in volatile to prevent optimization */
        test_values[0] = a;
        test_values[1] = b;
        
        printf("\nTest %d: %g vs %g\n", i, test_values[0], test_values[1]);
        
        /* Generate comparisons */
        int result = classify_comparison(test_values[0], test_values[1]);
        printf("  Classification: 0x%04x\n", result);
        
        #ifdef __GNUC__
        /* Use inline assembly for some cases */
        if (i % 3 == 0) {
            generate_all_conditions(test_values[0], test_values[1]);
        }
        #endif
        
        /* Check for unordered explicitly */
        if (isunordered(test_values[0], test_values[1])) {
            printf("  Unordered detected\n");
        }
        
        /* Generate LTGT condition (less or greater, ordered) */
        if (islessgreater(test_values[0], test_values[1])) {
            printf("  LTGT (ordered less or greater)\n");
        }
        
        /* Generate UNEQ condition (unordered or equal) */
        if (!islessgreater(test_values[0], test_values[1])) {
            /* This includes both equal and unordered cases */
            printf("  Not LTGT (could be UNEQ)\n");
        }
    }
    
    #ifdef __GNUC__
    /* Test vector comparisons */
    printf("\nTesting vector comparisons:\n");
    v2df vec_a = {1.0, NAN};
    v2df vec_b = {2.0, 2.0};
    v2df vec_result = vector_compare(vec_a, vec_b);
    
    /* Force vector result to be used */
    volatile double *vr = (double*)&vec_result;
    printf("Vector result: %g, %g\n", vr[0], vr[1]);
    #endif
    
    /* Complex switch based on comparison results */
    printf("\nTesting switch-based comparison dispatch:\n");
    for (i = 0; i < 4; i++) {
        double x = test_cases[i][0];
        double y = test_cases[i][1];
        volatile int condition = 0;
        
        /* Determine condition code */
        if (isunordered(x, y)) {
            condition = 0; /* UNORDERED */
        } else if (x == y) {
            condition = 1; /* EQ */
        } else if (x < y) {
            condition = 2; /* LT */
        } else if (x > y) {
            condition = 3; /* GT */
        } else if (x <= y) {
            condition = 4; /* LE */
        } else if (x >= y) {
            condition = 5; /* GE */
        } else if (x != y) {
            condition = 6; /* NEQ */
        }
        
        /* Switch that might generate different jump conditions */
        switch (condition) {
            case 0: printf("  UNORDERED\n"); break;
            case 1: printf("  EQ\n"); break;
            case 2: printf("  LT\n"); break;
            case 3: printf("  GT\n"); break;
            case 4: printf("  LE\n"); break;
            case 5: printf("  GE\n"); break;
            case 6: printf("  NEQ\n"); break;
            default: printf("  UNKNOWN\n"); break;
        }
    }
    
    return 0;
}
