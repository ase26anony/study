#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function that performs various floating-point comparisons */
static int compare_floats(double a, double b) {
    int result = 0;
    
    /* Standard comparison operators - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map to x86 unordered comparison predicates */
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
    if (isunordered(a, b)) {
        return "unordered";
    } else if (a < b) {
        return "less";
    } else if (a > b) {
        return "greater";
    } else if (a == b) {
        /* Handle +0.0 vs -0.0 */
        if (signbit(a) != signbit(b)) {
            return "equal_but_opposite_sign";
        }
        return "equal";
    }
    return "unknown";
}

/* Inline assembly to directly generate condition code output */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto x87 stack */
        "fldl %1\n\t"           /* Load a onto x87 stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clean up stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%ax, %0\n\t"   /* Move result to output */
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)", "ax"
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
        "seta %%ah\n\t"         /* Set if above (CF=0 && ZF=0) */
        "movzbl %%ax, %0\n\t"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)", "ax"
    );
    
    return result;
}

/* Vector extensions for packed floating-point comparisons */
#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));

static void vector_comparisons(void) {
    volatile v2df a = {1.0, NAN};
    volatile v2df b = {NAN, 2.0};
    volatile v2df c = {3.0, 4.0};
    volatile v2df d = {5.0, 6.0};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp1 = a < b;  /* May generate unordered comparisons */
    v2df cmp2 = c > d;
    v2df cmp3 = a == b;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "x"(cmp1), "x"(cmp2), "x"(cmp3));
}
#endif

/* Parse command line argument to double, handling "nan", "inf", "-inf" */
static double parse_fp_arg(const char *arg) {
    if (strcmp(arg, "nan") == 0) {
        return NAN;
    } else if (strcmp(arg, "inf") == 0) {
        return INFINITY;
    } else if (strcmp(arg, "-inf") == 0) {
        return -INFINITY;
    } else {
        return atof(arg);
    }
}

int main(int argc, char *argv[]) {
    /* Array of test cases including NaN, infinity, and normal numbers */
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
        {3.14159, 3.14159},
        {global_nan, 42.0},  /* volatile to prevent constant folding */
        {global_inf, global_neg_inf}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int total_results = 0;
    
    printf("Testing floating-point comparisons...\n");
    
    /* Test 1: Standard comparisons and math.h macros */
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        int result = compare_floats(a, b);
        total_results += result;
        
        const char *classification = classify_comparison(a, b);
        printf("Case %d: a=%g, b=%g, result=0x%x, class=%s\n", 
               i, a, b, result, classification);
    }
    
    /* Test 2: Inline assembly comparisons */
    printf("\nTesting inline assembly comparisons...\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        int asm_result1 = fp_compare_asm(a, b);
        int asm_result2 = fp_compare_asm2(a, b);
        total_results += asm_result1 + asm_result2;
        
        printf("ASM Case %d: a=%g, b=%g, result1=0x%x, result2=0x%x\n",
               i, a, b, asm_result1, asm_result2);
    }
    
    /* Test 3: Command line arguments if provided */
    if (argc >= 3) {
        printf("\nTesting command line arguments...\n");
        double arg1 = parse_fp_arg(argv[1]);
        double arg2 = parse_fp_arg(argv[2]);
        
        int result = compare_floats(arg1, arg2);
        total_results += result;
        
        int asm_result = fp_compare_asm(arg1, arg2);
        total_results += asm_result;
        
        printf("Args: %s=%g, %s=%g, compare=0x%x, asm=0x%x\n",
               argv[1], arg1, argv[2], arg2, result, asm_result);
    }
    
#ifdef __SSE2__
    /* Test 4: Vector comparisons if SSE2 is available */
    printf("\nTesting vector comparisons...\n");
    vector_comparisons();
#endif
    
    /* Complex conditional with multiple branches to encourage 
       generation of various condition code mnemonics */
    printf("\nTesting complex conditional logic...\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        int code = 0;
        
        /* This complex if-else chain may generate multiple 
           different x86 condition code mnemonics */
        if (isunordered(a, b)) {
            code = 1;  /* UNORDERED */
        } else if (a == b) {
            if (signbit(a) == signbit(b)) {
                code = 2;  /* EQ (ordered equal) */
            } else {
                code = 3;  /* UNEQ? Actually EQ but different signs */
            }
        } else if (a < b) {
            code = 4;  /* LT */
        } else if (a > b) {
            code = 5;  /* GT */
        } else if (a <= b) {
            code = 6;  /* LE */
        } else if (a >= b) {
            code = 7;  /* GE */
        } else if (a != b) {
            code = 8;  /* NEQ */
        }
        
        /* Additional comparisons that might generate LTGT, UNGE, UNLE, etc. */
        if (!(a >= b) && !isunordered(a, b)) {
            code |= 0x10;  /* Could generate UNGE (nlt) */
        }
        if (!(a <= b) && !isunordered(a, b)) {
            code |= 0x20;  /* Could generate UNLE (nle) */
        }
        if (islessgreater(a, b)) {
            code |= 0x40;  /* LTGT (une) */
        }
        
        total_results += code;
        printf("Complex case %d: code=0x%x\n", i, code);
    }
    
    /* Final result to prevent dead code elimination */
    printf("\nTotal results accumulator: %d\n", total_results & 0xFF);
    
    return (total_results & 0xFF) == 0 ? 0 : 1;
}
