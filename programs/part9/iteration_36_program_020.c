/* test_i386_cc.c - Program to trigger x86 unordered floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function to parse NaN from command line */
static double parse_double(const char *str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NaN") == 0) {
        return NAN;
    } else if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0) {
        return INFINITY;
    } else if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0) {
        return -INFINITY;
    }
    return atof(str);
}

/* Helper function that performs all possible comparisons */
static int compare_all_results(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - these can generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map to x86 condition codes */
    if (isunordered(a, b)) result |= 64;
    if (isless(a, b)) result |= 128;        /* a < b, but no invalid if unordered */
    if (isgreater(a, b)) result |= 256;     /* a > b, but no invalid if unordered */
    if (islessequal(a, b)) result |= 512;   /* a <= b, but no invalid if unordered */
    if (isgreaterequal(a, b)) result |= 1024; /* a >= b, but no invalid if unordered */
    if (islessgreater(a, b)) result |= 2048; /* a < b or a > b, but no invalid if unordered */
    
    return result;
}

/* Function with switch based on comparison results */
static const char* classify_comparison(double a, double b) {
    /* This switch structure encourages compiler to generate multiple condition codes */
    if (isunordered(a, b)) {
        return "UNORDERED";
    } else if (isless(a, b)) {
        return "LESS";
    } else if (isgreater(a, b)) {
        return "GREATER";
    } else if (a == b) {
        /* Distinguish +0.0 from -0.0 */
        if (signbit(a) != signbit(b)) {
            return "EQUAL_BUT_SIGN_DIFF";
        }
        return "EQUAL";
    } else if (islessgreater(a, b)) {
        return "LESS_GREATER";
    }
    return "UNKNOWN";
}

/* Inline assembly that directly uses floating-point condition codes */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* Using x87 FPU comparison - forces generation of condition code mnemonics */
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
        "orl %%edx, %%eax\n\t"
        : "=a" (result)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    return result;
}

/* More inline assembly with different condition codes */
static int fp_compare_asm_ordered(double a, double b) {
    int result;
    
    /* Test for ordered comparison (not unordered) */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setnp %%al\n\t"        /* Set if ordered (no parity) */
        "sete %%ah\n\t"         /* Set if equal (ZF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $8, %%edx\n\t"
        "orl %%edx, %%eax\n\t"
        : "=a" (result)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    return result;
}

/* Vector extensions for SSE comparisons */
#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

static void vector_comparisons(void) {
    volatile v2df a = {1.0, NAN};
    volatile v2df b = {NAN, 2.0};
    volatile v2df c = {3.0, 4.0};
    volatile v2df d = {5.0, 6.0};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp1 = a < b;  /* Should produce unordered results */
    v2df cmp2 = c <= d; /* Ordered comparisons */
    v2df cmp3 = a == b; /* Equality with NaN */
    
    /* Use results to prevent optimization */
    volatile v2di *p1 = (v2di*)&cmp1;
    volatile v2di *p2 = (v2di*)&cmp2;
    volatile v2di *p3 = (v2di*)&cmp3;
    
    printf("Vector cmp1: %016llx %016llx\n", (*p1)[0], (*p1)[1]);
    printf("Vector cmp2: %016llx %016llx\n", (*p2)[0], (*p2)[1]);
    printf("Vector cmp3: %016llx %016llx\n", (*p3)[0], (*p3)[1]);
}
#endif

/* Main test function */
int main(int argc, char *argv[]) {
    double test_values[10];
    int i, j;
    
    /* Parse command line arguments or use defaults */
    if (argc > 1) {
        for (i = 0; i < argc - 1 && i < 10; i++) {
            test_values[i] = parse_double(argv[i + 1]);
        }
    } else {
        /* Default test cases including NaN, Inf, and normal numbers */
        test_values[0] = NAN;
        test_values[1] = INFINITY;
        test_values[2] = -INFINITY;
        test_values[3] = 0.0;
        test_values[4] = -0.0;
        test_values[5] = 1.0;
        test_values[6] = -1.0;
        test_values[7] = DBL_MAX;
        test_values[8] = DBL_MIN;
        test_values[9] = 3.141592653589793;
    }
    
    printf("Testing floating-point comparisons to trigger x86 condition code output...\n");
    
    /* Perform comparisons between all pairs */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            double a = test_values[i];
            double b = test_values[j];
            
            /* Force compiler to generate comparison code */
            volatile int cmp_result = compare_all_results(a, b);
            volatile const char *cls = classify_comparison(a, b);
            
            /* Use inline assembly comparisons */
            volatile int asm_result1 = fp_compare_asm(a, b);
            volatile int asm_result2 = fp_compare_asm_ordered(a, b);
            
            /* Print minimal output to see effects but prevent dead code elimination */
            if (cmp_result != 0 || asm_result1 != 0 || asm_result2 != 0) {
                printf("(%d,%d): cmp=0x%x, asm1=0x%x, asm2=0x%x, class=%s\n",
                       i, j, cmp_result, asm_result1, asm_result2, cls);
            }
        }
    }
    
#ifdef __SSE2__
    printf("\nTesting vector comparisons...\n");
    vector_comparisons();
#endif
    
    /* Additional complex control flow to encourage various condition codes */
    printf("\nComplex comparison chain:\n");
    for (i = 0; i < 5; i++) {
        double a = test_values[i * 2];
        double b = test_values[i * 2 + 1];
        
        /* Complex if-else chain that may generate different jump conditions */
        if (isunordered(a, b)) {
            printf("  Case %d: UNORDERED\n", i);
        } else if (a < b) {
            printf("  Case %d: LESS\n", i);
        } else if (a > b) {
            printf("  Case %d: GREATER\n", i);
        } else if (a == b) {
            if (signbit(a) && !signbit(b)) {
                printf("  Case %d: EQUAL (a=-0, b=+0)\n", i);
            } else if (!signbit(a) && signbit(b)) {
                printf("  Case %d: EQUAL (a=+0, b=-0)\n", i);
            } else {
                printf("  Case %d: EQUAL\n", i);
            }
        } else if (a != b) {
            printf("  Case %d: NOT_EQUAL\n", i);
        }
        
        /* Test for UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT conditions */
        volatile int test_unordered = isunordered(a, b);
        volatile int test_uneq = !isunordered(a, b) && (a == b);
        volatile int test_unge = isunordered(a, b) || (a >= b);
        volatile int test_ungt = isunordered(a, b) || (a > b);
        volatile int test_unle = isunordered(a, b) || (a <= b);
        volatile int test_unlt = isunordered(a, b) || (a < b);
        volatile int test_ltgt = !isunordered(a, b) && (a != b);
        
        /* Use results to prevent optimization */
        if (test_unordered || test_uneq || test_unge || test_ungt || 
            test_unle || test_unlt || test_ltgt) {
            printf("    Unordered flags: unord=%d, uneq=%d, unge=%d, ungt=%d, unle=%d, unlt=%d, ltgt=%d\n",
                   test_unordered, test_uneq, test_unge, test_ungt, 
                   test_unle, test_unlt, test_ltgt);
        }
    }
    
    return 0;
}
