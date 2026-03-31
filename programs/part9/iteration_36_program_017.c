#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding and optimization */
volatile double volatile_nan = NAN;
volatile double volatile_inf = INFINITY;
volatile double volatile_zero = 0.0;

/* Vector type for SSE/AVX comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Function to classify comparison results */
int classify_comparison(double a, double b) {
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a == b) result |= 4;
    
    /* Unordered comparisons */
    if (isunordered(a, b)) result |= 8;
    if (!isunordered(a, b)) result |= 16;  /* ORDERED */
    
    /* Specific unordered comparisons from math.h */
    if (isless(a, b)) result |= 32;        /* a < b and ordered */
    if (isgreater(a, b)) result |= 64;     /* a > b and ordered */
    if (islessequal(a, b)) result |= 128;  /* a <= b and ordered */
    if (isgreaterequal(a, b)) result |= 256; /* a >= b and ordered */
    
    /* Direct unordered comparisons */
    if (islessgreater(a, b)) result |= 512; /* a < b or a > b, and ordered (LTGT) */
    
    return result;
}

/* Function that uses all comparison types in control flow */
const char* compare_detailed(double a, double b) {
    /* This function should generate many different condition codes */
    if (isunordered(a, b)) {
        return "UNORDERED";
    }
    
    if (a == b && !isunordered(a, b)) {
        return "ORDERED_EQ";
    }
    
    if (a != b && !isunordered(a, b)) {
        if (a < b) return "ORDERED_LT";
        if (a > b) return "ORDERED_GT";
    }
    
    /* More complex conditions that might generate UNEQ, UNGE, etc. */
    if (!(a < b) && !isunordered(a, b)) {  /* a >= b and ordered (GE) */
        if (a == b) return "EQ";
        return "GE";
    }
    
    if (!(a > b) && !isunordered(a, b)) {  /* a <= b and ordered (LE) */
        if (a == b) return "EQ";
        return "LE";
    }
    
    /* Unordered specific comparisons */
    if (isunordered(a, b) || a == b) {  /* UNEQ: unordered or equal */
        return "UNEQ_COND";
    }
    
    if (isunordered(a, b) || a >= b) {  /* UNGE: unordered or greater/equal */
        return "UNGE_COND";
    }
    
    if (isunordered(a, b) || a > b) {   /* UNGT: unordered or greater */
        return "UNGT_COND";
    }
    
    if (isunordered(a, b) || a <= b) {  /* UNLE: unordered or less/equal */
        return "UNLE_COND";
    }
    
    if (isunordered(a, b) || a < b) {   /* UNLT: unordered or less */
        return "UNLT_COND";
    }
    
    if (a != b && !isunordered(a, b)) {  /* LTGT: less or greater, ordered */
        return "LTGT_COND";
    }
    
    return "UNKNOWN";
}

/* Inline assembly to directly generate condition code output */
double inline_asm_fp_compare(double a, double b) {
    double result;
    int unordered_flag, greater_flag, equal_flag;
    
    /* Using x87 floating-point compare */
    asm volatile (
        "fldl %[b]\n\t"           /* Load b into st(0) */
        "fldl %[a]\n\t"           /* Load a into st(0), b moves to st(1) */
        "fucomip %%st(1), %%st\n\t" /* Compare and pop st(0) */
        "fstp %%st(0)\n\t"        /* Pop st(0) which was b */
        "setp %[unordered]\n\t"   /* Set if unordered (parity flag) */
        "seta %[greater]\n\t"     /* Set if above (CF=0 and ZF=0) */
        "sete %[equal]\n\t"       /* Set if equal (ZF=1) */
        : [unordered] "=r" (unordered_flag),
          [greater] "=r" (greater_flag),
          [equal] "=r" (equal_flag)
        : [a] "m" (a),
          [b] "m" (b)
        : "cc", "st"
    );
    
    /* Another asm to test different condition codes */
    asm volatile (
        "fldl %[b]\n\t"
        "fldl %[a]\n\t"
        "fucomi %%st(1), %%st\n\t"  /* Compare without pop */
        "fstp %%st(0)\n\t"
        "fstp %%st(0)\n\t"
        : /* no outputs */
        : [a] "m" (a),
          [b] "m" (b)
        : "cc", "st"
    );
    
    result = unordered_flag ? volatile_nan : 
             (greater_flag ? a : (equal_flag ? 0.0 : b));
    
    return result;
}

/* Vector comparison function */
void vector_comparisons(void) {
    v2df vec_a = {1.0, volatile_nan};
    v2df vec_b = {volatile_nan, 2.0};
    v2df vec_c = {3.0, 4.0};
    
    /* These vector comparisons may generate condition codes */
    v2df cmp1 = vec_a < vec_b;  /* Should have NaN comparisons */
    v2df cmp2 = vec_a > vec_b;
    v2df cmp3 = vec_a == vec_b;
    v2df cmp4 = vec_a != vec_b;
    
    /* Use the results to prevent optimization */
    volatile v2df volatile_cmp = cmp1;
    (void)volatile_cmp;
}

/* Parse double with NaN support */
double parse_double(const char* str) {
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

int main(int argc, char* argv[]) {
    /* Test cases covering various scenarios */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, -INFINITY},
        {INFINITY, 1.0},
        {-INFINITY, 1.0},
        {0.0, -0.0},           /* Should be equal */
        {DBL_MAX, DBL_MIN},
        {volatile_nan, volatile_zero},
        {volatile_inf, volatile_zero}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Testing with command line values: %g, %g\n", a, b);
        
        /* Perform all types of comparisons */
        int classification = classify_comparison(a, b);
        const char* detailed = compare_detailed(a, b);
        double asm_result = inline_asm_fp_compare(a, b);
        
        printf("Classification: 0x%x\n", classification);
        printf("Detailed: %s\n", detailed);
        printf("ASM result: %g\n", asm_result);
        
        /* Test with math.h macros directly */
        printf("isunordered: %d\n", isunordered(a, b));
        printf("isless: %d\n", isless(a, b));
        printf("isgreater: %d\n", isgreater(a, b));
        printf("islessequal: %d\n", islessequal(a, b));
        printf("isgreaterequal: %d\n", isgreaterequal(a, b));
        printf("islessgreater: %d\n", islessgreater(a, b));
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
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
        
        /* Use math.h macros */
        volatile int is_unordered = isunordered(a, b);
        volatile int is_ordered = !isunordered(a, b);
        
        /* Call functions that use comparisons */
        classify_comparison(a, b);
        compare_detailed(a, b);
        inline_asm_fp_compare(a, b);
        
        /* Print minimal output to see effects */
        if (i % 5 == 0) {
            printf("Case %d: %g vs %g\n", i, a, b);
        }
    }
    
    /* Vector comparisons */
    vector_comparisons();
    
    /* Complex switch based on comparison results */
    {
        double x = volatile_nan;
        double y = 1.0;
        int result_code = 0;
        
        /* This switch should generate various condition code outputs */
        if (isunordered(x, y)) {
            result_code = 1;  /* UNORDERED */
        } else if (x == y) {
            result_code = 2;  /* ORDERED EQ */
        } else if (x < y) {
            result_code = 3;  /* ORDERED LT */
        } else if (x > y) {
            result_code = 4;  /* ORDERED GT */
        }
        
        /* More complex conditions */
        if (!(x < y) && !isunordered(x, y)) {
            result_code |= 0x10;  /* GE */
        }
        if (!(x > y) && !isunordered(x, y)) {
            result_code |= 0x20;  /* LE */
        }
        if (isunordered(x, y) || x == y) {
            result_code |= 0x40;  /* UNEQ */
        }
        if (isunordered(x, y) || x >= y) {
            result_code |= 0x80;  /* UNGE */
        }
        if (isunordered(x, y) || x > y) {
            result_code |= 0x100; /* UNGT */
        }
        if (isunordered(x, y) || x <= y) {
            result_code |= 0x200; /* UNLE */
        }
        if (isunordered(x, y) || x < y) {
            result_code |= 0x400; /* UNLT */
        }
        if (x != y && !isunordered(x, y)) {
            result_code |= 0x800; /* LTGT */
        }
        
        printf("Final result code: 0x%x\n", result_code);
    }
    
    return 0;
}
