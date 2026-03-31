/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding and optimization */
static volatile double volatile_double = 0.0;

/* Function to parse NaN from command line */
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

/* Helper function that performs all possible floating-point comparisons */
static int compare_floats_comprehensive(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered condition codes */
    if (a < b) result |= 1;      /* Less than */
    if (a > b) result |= 2;      /* Greater than */
    if (a <= b) result |= 4;     /* Less than or equal */
    if (a >= b) result |= 8;     /* Greater than or equal */
    if (a == b) result |= 16;    /* Equal */
    if (a != b) result |= 32;    /* Not equal */
    
    /* <math.h> macros that map directly to x86 condition codes */
    if (isunordered(a, b)) result |= 64;      /* UNORDERED */
    if (isless(a, b)) result |= 128;          /* Less (ordered) */
    if (isgreater(a, b)) result |= 256;       /* Greater (ordered) */
    if (islessequal(a, b)) result |= 512;     /* Less or equal (ordered) */
    if (isgreaterequal(a, b)) result |= 1024; /* Greater or equal (ordered) */
    if (islessgreater(a, b)) result |= 2048;  /* LTGT - Less or greater (ordered) */
    
    return result;
}

/* Function with mixed ordered/unordered comparisons in control flow */
static const char* classify_comparison(double a, double b) {
    /* This switch-like logic forces generation of multiple condition codes */
    if (isunordered(a, b)) {
        return "UNORDERED";
    }
    else if (isless(a, b)) {
        return "LESS";
    }
    else if (isgreater(a, b)) {
        return "GREATER";
    }
    else if (a == b) {
        /* Distinguish +0.0 from -0.0 */
        if (signbit(a) != signbit(b)) {
            return "EQUAL_BUT_SIGN_DIFF";
        }
        return "EQUAL";
    }
    else if (islessgreater(a, b)) {
        return "LESS_GREATER";
    }
    
    return "UNKNOWN";
}

/* Vector extensions for SSE/AVX comparisons */
#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

static void vector_comparisons(void) {
    volatile v2df a = {1.0, NAN};
    volatile v2df b = {2.0, 2.0};
    volatile v2df c = {NAN, 3.0};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp1 = a < b;   /* Should generate UNORDERED for second element */
    v2df cmp2 = b > c;   /* Should generate UNORDERED for first element */
    v2df cmp3 = a == c;  /* Both elements unordered? */
    
    /* Prevent dead code elimination */
    volatile_double = cmp1[0] + cmp2[0] + cmp3[0];
}
#endif

/* Inline assembly to directly trigger condition code output */
static void inline_asm_fpu_comparisons(double x, double y) {
    int result_unordered, result_ordered, result_uneq, result_unge;
    int result_ungt, result_unle, result_unlt, result_ltgt;
    
    /* Force values into FPU registers */
    asm volatile("" : "+t"(x), "+u"(y));
    
    /* UNORDERED comparison */
    asm volatile(
        "fucomip %%st(1), %%st\n\t"
        "setp %0"
        : "=r"(result_unordered)
        : "t"(x), "u"(y)
        : "cc", "st"
    );
    
    /* ORDERED comparison */
    asm volatile(
        "fucomip %%st(1), %%st\n\t"
        "setnp %0"
        : "=r"(result_ordered)
        : "t"(x), "u"(y)
        : "cc", "st"
    );
    
    /* Various unordered comparisons using different x86 condition codes */
    /* UNEQ (unordered or equal) */
    asm volatile(
        "fucomip %%st(1), %%st\n\t"
        "sete %0\n\t"
        "setp %%al\n\t"
        "orb %%al, %0"
        : "=r"(result_uneq)
        : "t"(x), "u"(y)
        : "cc", "st", "al"
    );
    
    /* UNGE (not less than) */
    asm volatile(
        "fucomip %%st(1), %%st\n\t"
        "setnb %0"
        : "=r"(result_unge)
        : "t"(x), "u"(y)
        : "cc", "st"
    );
    
    /* UNGT (not less than or equal) */
    asm volatile(
        "fucomip %%st(1), %%st\n\t"
        "setnbe %0"
        : "=r"(result_ungt)
        : "t"(x), "u"(y)
        : "cc", "st"
    );
    
    /* UNLE (unordered or less than or equal) */
    asm volatile(
        "fucomip %%st(1), %%st\n\t"
        "setbe %0\n\t"
        "setp %%al\n\t"
        "orb %%al, %0"
        : "=r"(result_unle)
        : "t"(x), "u"(y)
        : "cc", "st", "al"
    );
    
    /* UNLT (unordered or less than) */
    asm volatile(
        "fucomip %%st(1), %%st\n\t"
        "setb %0\n\t"
        "setp %%al\n\t"
        "orb %%al, %0"
        : "=r"(result_unlt)
        : "t"(x), "u"(y)
        : "cc", "st", "al"
    );
    
    /* LTGT (less than or greater than - ordered and not equal) */
    asm volatile(
        "fucomip %%st(1), %%st\n\t"
        "setne %0\n\t"
        "setnp %%al\n\t"
        "andb %%al, %0"
        : "=r"(result_ltgt)
        : "t"(x), "u"(y)
        : "cc", "st", "al"
    );
    
    /* Prevent dead code elimination */
    volatile_double = result_unordered + result_ordered + result_uneq + 
                     result_unge + result_ungt + result_unle + 
                     result_unlt + result_ltgt;
}

int main(int argc, char *argv[]) {
    /* Test cases designed to trigger various condition codes */
    double test_cases[][2] = {
        {NAN, 1.0},           /* UNORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT */
        {1.0, NAN},           /* UNORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT */
        {INFINITY, -INFINITY},/* ORDERED, LTGT */
        {0.0, -0.0},          /* ORDERED, EQ (but sign differs) */
        {1.0, 2.0},           /* ORDERED, LESS */
        {2.0, 1.0},           /* ORDERED, GREATER */
        {NAN, NAN},           /* UNORDERED, UNEQ */
        {INFINITY, INFINITY}, /* ORDERED, EQ */
        {-INFINITY, -INFINITY},/* ORDERED, EQ */
        {DBL_MAX, DBL_MAX * 0.5}, /* ORDERED, GREATER */
        {0.0, DBL_MIN},       /* ORDERED, LESS */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Testing with command line values: %g, %g\n", a, b);
        
        /* Comprehensive comparison */
        int cmp_result = compare_floats_comprehensive(a, b);
        printf("Comparison result bits: 0x%04x\n", cmp_result);
        
        /* Classification */
        const char *cls = classify_comparison(a, b);
        printf("Classification: %s\n", cls);
        
        /* Inline assembly comparisons */
        inline_asm_fpu_comparisons(a, b);
        printf("Inline assembly comparisons performed\n");
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Force compiler to generate actual comparisons */
        volatile double va = a;
        volatile double vb = b;
        
        /* Perform all types of comparisons */
        int result = compare_floats_comprehensive(va, vb);
        
        /* Classification */
        const char *cls = classify_comparison(va, vb);
        
        printf("Test %2d: %12g vs %12g -> result: 0x%04x, class: %s\n",
               i, va, vb, result, cls);
        
        /* Periodically use inline assembly */
        if (i % 3 == 0) {
            inline_asm_fpu_comparisons(va, vb);
        }
    }
    
#ifdef __SSE2__
    printf("\nPerforming vector comparisons (SSE2):\n");
    vector_comparisons();
#endif
    
    /* Final summary to prevent optimization */
    printf("\nTest completed. Volatile double value: %g\n", volatile_double);
    
    return 0;
}
