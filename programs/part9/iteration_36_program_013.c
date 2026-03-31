/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector type for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function that performs all possible unordered comparisons */
static int compare_all_results(double a, double b) {
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
    /* This switch should generate various condition code outputs */
    if (isunordered(a, b)) {
        return "unordered";
    } else if (isless(a, b)) {
        return "less";
    } else if (isgreater(a, b)) {
        return "greater";
    } else if (a == b) {
        /* Distinguish +0.0 from -0.0 */
        if (signbit(a) != signbit(b) && a == 0.0 && b == 0.0) {
            return "zero_different_sign";
        }
        return "equal";
    } else if (islessgreater(a, b)) {
        return "less_or_greater";
    }
    return "unknown";
}

/* Inline assembly to directly trigger condition code output */
static int inline_asm_fp_compare(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto x87 stack */
        "fldl %1\n\t"           /* Load a onto x87 stack, now st(0)=a, st(1)=b */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $8, %%edx\n\t"
        "orl %%edx, %%eax\n\t"
        "fstp %%st(0)\n\t"      /* Clean up x87 stack */
        : "=a" (result)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    return result;
}

/* Another inline assembly variant with different condition codes */
static int inline_asm_fp_compare2(double a, double b) {
    int result = 0;
    
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        /* Conditional move based on various conditions */
        "movl $0, %0\n\t"
        "jp 1f\n\t"             /* Jump if unordered */
        "ja 2f\n\t"             /* Jump if above (CF=0 && ZF=0) */
        "jb 3f\n\t"             /* Jump if below (CF=1) */
        "je 4f\n\t"             /* Jump if equal (ZF=1) */
        "1:\n\t"
        "movl $1, %0\n\t"       /* Unordered */
        "jmp 5f\n\t"
        "2:\n\t"
        "movl $2, %0\n\t"       /* Greater */
        "jmp 5f\n\t"
        "3:\n\t"
        "movl $3, %0\n\t"       /* Less */
        "jmp 5f\n\t"
        "4:\n\t"
        "movl $4, %0\n\t"       /* Equal */
        "5:\n\t"
        "fstp %%st(0)"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    return result;
}

/* Vector comparison function */
static void vector_comparisons(void) {
    v2df vec_a, vec_b, vec_cmp;
    v4sf vec_fa, vec_fb, vec_fcmp;
    
    /* Initialize vectors with mixed values */
    vec_a = (v2df){1.0, NAN};
    vec_b = (v2df){NAN, 2.0};
    
    /* These comparisons may generate unordered condition codes */
    vec_cmp = vec_a < vec_b;
    vec_cmp = vec_a > vec_b;
    vec_cmp = vec_a <= vec_b;
    vec_cmp = vec_a >= vec_b;
    
    /* Float vector comparisons */
    vec_fa = (v4sf){1.0f, NAN, INFINITY, -0.0f};
    vec_fb = (v4sf){NAN, 2.0f, -INFINITY, 0.0f};
    
    vec_fcmp = vec_fa < vec_fb;
    vec_fcmp = vec_fa > vec_fb;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "x"(vec_cmp), "x"(vec_fcmp) : "memory");
}

/* Parse double from string, handling "nan", "inf", "-inf" */
static double parse_fp_arg(const char *arg) {
    if (strcmp(arg, "nan") == 0) return NAN;
    if (strcmp(arg, "inf") == 0) return INFINITY;
    if (strcmp(arg, "-inf") == 0) return -INFINITY;
    return atof(arg);
}

int main(int argc, char *argv[]) {
    double test_values[10];
    int i, j;
    
    /* Initialize test values from command line or defaults */
    if (argc > 1) {
        for (i = 0; i < argc - 1 && i < 10; i++) {
            test_values[i] = parse_fp_arg(argv[i + 1]);
        }
    } else {
        /* Default test cases covering various scenarios */
        test_values[0] = NAN;
        test_values[1] = 1.0;
        test_values[2] = -1.0;
        test_values[3] = INFINITY;
        test_values[4] = -INFINITY;
        test_values[5] = 0.0;
        test_values[6] = -0.0;
        test_values[7] = DBL_MAX;
        test_values[8] = DBL_MIN;
        test_values[9] = 3.14159;
    }
    
    printf("Testing floating-point comparisons...\n");
    
    /* Perform comparisons between all pairs */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            double a = test_values[i];
            double b = test_values[j];
            volatile int result;
            
            /* Force compiler to generate actual comparisons */
            result = compare_all_results(a, b);
            
            /* Use classification function */
            const char *classification = classify_comparison(a, b);
            
            /* Use inline assembly comparisons */
            int asm_result1 = inline_asm_fp_compare(a, b);
            int asm_result2 = inline_asm_fp_compare2(a, b);
            
            /* Print minimal output to prevent optimization */
            if ((i + j) % 7 == 0) {
                printf("a=%g b=%g: class=%s asm1=%d asm2=%d\n", 
                       a, b, classification, asm_result1, asm_result2);
            }
        }
    }
    
    /* Test vector comparisons */
    vector_comparisons();
    
    /* Complex control flow with many comparisons */
    double x = global_nan;
    double y = 0.0;
    int branch_taken = 0;
    
    /* This should generate various condition code mnemonics */
    if (isunordered(x, y)) branch_taken |= 1;
    if (x < y) branch_taken |= 2;
    if (x > y) branch_taken |= 4;
    if (x <= y) branch_taken |= 8;
    if (x >= y) branch_taken |= 16;
    if (x == y) branch_taken |= 32;
    if (x != y) branch_taken |= 64;
    
    /* More comparisons with different predicates */
    if (!isunordered(x, y) && x == y) branch_taken |= 128;  /* UNEQ */
    if (!isunordered(x, y) && x >= y) branch_taken |= 256;  /* UNGE */
    if (!isunordered(x, y) && x > y) branch_taken |= 512;   /* UNGT */
    if (!isunordered(x, y) && x <= y) branch_taken |= 1024; /* UNLE */
    if (!isunordered(x, y) && x < y) branch_taken |= 2048;  /* UNLT */
    if (islessgreater(x, y)) branch_taken |= 4096;          /* LTGT */
    
    printf("Branch pattern: %d\n", branch_taken);
    
    /* Final summary to prevent dead code elimination */
    volatile int final_result = 0;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            final_result += compare_all_results(test_values[i], test_values[j]);
        }
    }
    
    printf("Final checksum: %d\n", final_result);
    
    return 0;
}
