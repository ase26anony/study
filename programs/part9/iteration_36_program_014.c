/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector types for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function to parse NaN from command line */
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
    
    /* <math.h> comparison macros that map to x86 condition codes */
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
    /* Use fpclassify to get detailed information */
    int a_class = fpclassify(a);
    int b_class = fpclassify(b);
    
    if (isunordered(a, b)) {
        if (a == b) return "UNEQ";  /* This should trigger UNEQ output */
        if (a >= b) return "UNGE";  /* This should trigger UNGE/nlt output */
        if (a > b)  return "UNGT";  /* This should trigger UNGT/nle output */
        if (a <= b) return "UNLE";  /* This should trigger UNLE/ule output */
        if (a < b)  return "UNLT";  /* This should trigger UNLT/ult output */
        return "UNORDERED";
    } else {
        if (a < b) return "LT";
        if (a > b) return "GT";
        if (a == b) return "EQ";
        /* LTGT (not equal and ordered) */
        return "LTGT";  /* This should trigger LTGT/une output */
    }
}

/* Inline assembly to directly generate condition code output */
static int inline_asm_fp_compare(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto x87 stack */
        "fldl %1\n\t"           /* Load a onto x87 stack, a is now st(0), b is st(1) */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop st(0) */
        "fstp %%st(0)\n\t"      /* Pop the remaining b value */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %0\n\t"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)", "eax"
    );
    
    return result;
}

/* Another inline assembly variant with different condition codes */
static int inline_asm_fcomi(double a, double b) {
    int result = 0;
    
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fcomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        : "=@ccbe" (result)  /* Use condition code constraints */
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)"
    );
    
    return result;
}

/* Vector comparison function */
static void vector_comparisons(void) {
    v2df vec_a, vec_b, vec_cmp;
    volatile v2df *volatile_ptr;
    
    /* Initialize vectors with mixed values */
    vec_a = (v2df){1.0, NAN};
    vec_b = (v2df){NAN, 2.0};
    
    /* Perform vector comparisons - may generate multiple condition codes */
    vec_cmp = vec_a < vec_b;
    vec_cmp = vec_a > vec_b;
    vec_cmp = vec_a <= vec_b;
    vec_cmp = vec_a >= vec_b;
    vec_cmp = vec_a == vec_b;
    vec_cmp = vec_a != vec_b;
    
    /* Store to volatile to prevent optimization */
    volatile_ptr = &vec_cmp;
    (void)volatile_ptr;
}

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
        /* Default test cases including NaN, Inf, normal numbers */
        test_values[0] = NAN;
        test_values[1] = INFINITY;
        test_values[2] = -INFINITY;
        test_values[3] = 0.0;
        test_values[4] = -0.0;
        test_values[5] = 1.0;
        test_values[6] = -1.0;
        test_values[7] = DBL_MAX;
        test_values[8] = DBL_MIN;
        test_values[9] = global_nan;  /* volatile to prevent constant folding */
    }
    
    printf("Testing floating-point comparisons to trigger x86 condition code output...\n");
    
    /* Test all pairs of values */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            double a = test_values[i];
            double b = test_values[j];
            volatile double volatile_a = a;  /* Prevent optimization */
            volatile double volatile_b = b;
            
            /* Perform comparisons that may generate various condition codes */
            int cmp_result = compare_all(volatile_a, volatile_b);
            const char *classification = classify_comparison(volatile_a, volatile_b);
            
            /* Use inline assembly comparisons */
            int asm_result1 = inline_asm_fp_compare(volatile_a, volatile_b);
            int asm_result2 = inline_asm_fcomi(volatile_a, volatile_b);
            
            /* Print something to prevent dead code elimination */
            if ((i + j) % 7 == 0) {
                printf("a=%g b=%g: cmp=0x%x class=%s asm1=0x%x asm2=0x%x\n",
                       volatile_a, volatile_b, cmp_result, classification, 
                       asm_result1, asm_result2);
            }
        }
    }
    
    /* Test vector comparisons */
    vector_comparisons();
    
    /* Additional test with mixed ordered/unordered in control flow */
    {
        double special_cases[][2] = {
            {NAN, 1.0},
            {1.0, NAN},
            {NAN, NAN},
            {INFINITY, -INFINITY},
            {0.0, -0.0},
            {1.0, 2.0},
            {2.0, 1.0}
        };
        
        for (i = 0; i < sizeof(special_cases)/sizeof(special_cases[0]); i++) {
            double a = special_cases[i][0];
            double b = special_cases[i][1];
            
            /* Complex conditional that may generate multiple jump types */
            if (isunordered(a, b)) {
                if (a == b) {
                    printf("Case %d: UNEQ\n", i);
                } else if (a >= b) {
                    printf("Case %d: UNGE (nlt)\n", i);
                } else if (a > b) {
                    printf("Case %d: UNGT (nle)\n", i);
                } else if (a <= b) {
                    printf("Case %d: UNLE (ule)\n", i);
                } else if (a < b) {
                    printf("Case %d: UNLT (ult)\n", i);
                } else {
                    printf("Case %d: UNORDERED\n", i);
                }
            } else {
                if (a < b) {
                    printf("Case %d: LT\n", i);
                } else if (a > b) {
                    printf("Case %d: GT\n", i);
                } else if (a == b) {
                    printf("Case %d: EQ\n", i);
                } else {
                    printf("Case %d: LTGT (une)\n", i);
                }
            }
        }
    }
    
    /* Final summary to ensure all code is used */
    printf("Test completed. Check generated assembly for condition code mnemonics.\n");
    
    return 0;
}
