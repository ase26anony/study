/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_zero = 0.0;

/* Function to parse NaN from command line */
static double parse_double(const char *str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NAN") == 0)
        return NAN;
    if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0)
        return INFINITY;
    if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0)
        return -INFINITY;
    return atof(str);
}

/* Function performing various unordered comparisons */
static int compare_unordered(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons that can produce unordered results */
    if (a < b) result |= 1;      /* Less than (ordered) */
    if (a > b) result |= 2;      /* Greater than (ordered) */
    if (a == b) result |= 4;     /* Equal (ordered) */
    if (a != b) result |= 8;     /* Not equal (includes unordered) */
    if (a <= b) result |= 16;    /* Less or equal (ordered) */
    if (a >= b) result |= 32;    /* Greater or equal (ordered) */
    
    /* <math.h> macros that explicitly handle unordered */
    if (isunordered(a, b)) result |= 64;
    if (isless(a, b)) result |= 128;
    if (isgreater(a, b)) result |= 256;
    if (islessequal(a, b)) result |= 512;
    if (isgreaterequal(a, b)) result |= 1024;
    if (islessgreater(a, b)) result |= 2048;  /* LTGT condition */
    
    return result;
}

/* Function with switch based on comparison results */
static const char* classify_comparison(double a, double b) {
    /* Check unordered first */
    if (isunordered(a, b)) {
        /* Further classify unordered comparisons */
        if (isless(a, b)) return "UNORDERED_LESS";  /* Shouldn't happen */
        if (isgreater(a, b)) return "UNORDERED_GREATER"; /* Shouldn't happen */
        return "UNORDERED";
    }
    
    /* Ordered comparisons */
    if (isless(a, b)) return "LESS";
    if (isgreater(a, b)) return "GREATER";
    if (a == b) return "EQUAL";
    
    /* This handles the UNEQ case (unordered or equal) */
    if (!isless(a, b) && !isgreater(a, b)) return "UNEQ";
    
    return "UNKNOWN";
}

/* Inline assembly to force condition code output */
static int inline_asm_fp_compare(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clear FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%ax, %0"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "ax"
    );
    
    return result;
}

/* Vector extensions for SSE comparisons */
#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));

static v2df vector_compare(v2df a, v2df b) {
    /* These comparisons produce masks that may use condition codes */
    v2df lt = a < b;    /* Less than */
    v2df gt = a > b;    /* Greater than */
    v2df eq = a == b;   /* Equal */
    v2df ne = a != b;   /* Not equal (includes unordered) */
    v2df le = a <= b;   /* Less or equal */
    v2df ge = a >= b;   /* Greater or equal */
    
    /* Combine results */
    return lt + gt * 2.0 + eq * 4.0 + ne * 8.0 + le * 16.0 + ge * 32.0;
}
#endif

/* Main test function */
int main(int argc, char *argv[]) {
    /* Test cases with various NaN combinations */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, -INFINITY},
        {0.0, -0.0},
        {1.0, 2.0},
        {2.0, 1.0},
        {3.14, 3.14},
        {INFINITY, INFINITY},
        {-INFINITY, -INFINITY}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int total_results = 0;
    
    printf("Testing floating-point comparisons...\n");
    
    /* Test 1: Standard comparisons with unordered operands */
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Force compiler to generate actual comparisons */
        volatile int cmp_result = compare_unordered(a, b);
        total_results += cmp_result;
        
        /* Use classification function */
        const char *cls = classify_comparison(a, b);
        printf("Case %d: %s (a=%g, b=%g)\n", i, cls, a, b);
    }
    
    /* Test 2: Inline assembly comparisons */
    printf("\nInline assembly tests:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        volatile int asm_result = inline_asm_fp_compare(a, b);
        printf("  asm compare(%g, %g) = 0x%02x\n", a, b, asm_result & 0xFF);
        total_results += asm_result;
    }
    
#ifdef __SSE2__
    /* Test 3: Vector comparisons */
    printf("\nVector comparison tests:\n");
    v2df vec_a = {1.0, NAN};
    v2df vec_b = {NAN, 2.0};
    v2df vec_result = vector_compare(vec_a, vec_b);
    
    volatile double v0 = vec_result[0];
    volatile double v1 = vec_result[1];
    printf("  Vector result: [%g, %g]\n", v0, v1);
    total_results += (int)v0 + (int)v1;
#endif
    
    /* Test 4: Command-line arguments */
    if (argc >= 3) {
        double arg1 = parse_double(argv[1]);
        double arg2 = parse_double(argv[2]);
        
        printf("\nCommand-line test:\n");
        printf("  Comparing %s (%g) and %s (%g)\n", 
               argv[1], arg1, argv[2], arg2);
        
        /* Complex conditional to force multiple comparison types */
        if (isunordered(arg1, arg2)) {
            printf("  Result: Unordered\n");
            if (arg1 < arg2) printf("  (unexpected: < when unordered)\n");
        } else if (arg1 < arg2) {
            printf("  Result: Less than\n");
        } else if (arg1 > arg2) {
            printf("  Result: Greater than\n");
        } else if (arg1 == arg2) {
            printf("  Result: Equal\n");
        } else {
            printf("  Result: Not comparable\n");
        }
        
        /* Force generation of UNEQ, UNGE, UNGT, UNLE, UNLT conditions */
        volatile int complex_cmp = 0;
        if (!(arg1 < arg2) && !(arg1 > arg2)) complex_cmp |= 1;  /* UNEQ */
        if (!(arg1 < arg2)) complex_cmp |= 2;                    /* UNLT/UNLE */
        if (!(arg1 > arg2)) complex_cmp |= 4;                    /* UNGT/UNGE */
        if (arg1 != arg2) complex_cmp |= 8;                      /* LTGT */
        
        total_results += complex_cmp;
    }
    
    /* Prevent dead code elimination */
    printf("\nTotal results checksum: %d\n", total_results);
    
    return total_results != 0 ? 0 : 1;
}
