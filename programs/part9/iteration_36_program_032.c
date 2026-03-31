#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding and optimization */
volatile double volatile_nan = NAN;
volatile double volatile_inf = INFINITY;
volatile double volatile_zero = 0.0;

/* Vector type for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Function that performs all possible floating-point comparisons */
int compare_floats_full(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - these can generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* math.h comparison macros - directly map to x86 condition codes */
    if (isunordered(a, b)) result |= 64;
    if (isless(a, b)) result |= 128;
    if (isgreater(a, b)) result |= 256;
    if (islessequal(a, b)) result |= 512;
    if (isgreaterequal(a, b)) result |= 1024;
    if (islessgreater(a, b)) result |= 2048;
    
    return result;
}

/* Function with switch based on comparison results */
const char* classify_comparison(double a, double b) {
    if (isunordered(a, b)) {
        return "UNORDERED";
    }
    
    /* This switch structure encourages compiler to generate multiple condition codes */
    if (a < b) {
        if (a == -b) return "LT with special";
        return "LT";
    } else if (a > b) {
        if (b == 0.0) return "GT with zero";
        return "GT";
    } else if (a == b) {
        /* Distinguish +0 and -0 */
        if (signbit(a) != signbit(b)) return "EQ with signed zero";
        return "EQ";
    }
    
    return "UNKNOWN";
}

/* Inline assembly that directly uses floating-point condition codes */
double inline_asm_fp_compare(double a, double b) {
    double result;
    int unordered_flag, greater_flag, less_flag;
    
    /* Force x87 FPU comparison with inline assembly */
    asm volatile (
        "fldl %[b]\n\t"           /* Load b onto FPU stack */
        "fldl %[a]\n\t"           /* Load a onto FPU stack (now st(0)=a, st(1)=b) */
        "fucomip %%st(1), %%st\n\t" /* Compare and pop */
        "fstp %%st(0)\n\t"        /* Clear FPU stack */
        "setp %[unordered]\n\t"   /* Set if unordered (parity flag) */
        "seta %[greater]\n\t"     /* Set if above (CF=0 and ZF=0) */
        "setb %[less]\n\t"        /* Set if below (CF=1) */
        : [unordered] "=r" (unordered_flag),
          [greater] "=r" (greater_flag),
          [less] "=r" (less_flag)
        : [a] "m" (a),
          [b] "m" (b)
        : "cc", "st"
    );
    
    /* Another inline assembly that might trigger different condition code output */
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
    
    result = (unordered_flag ? NAN : (greater_flag ? 1.0 : (less_flag ? -1.0 : 0.0)));
    return result;
}

/* Vector comparison function */
void vector_comparisons(void) {
    v2df vec_a, vec_b, vec_cmp;
    
    /* Initialize vectors with mixed values */
    vec_a = (v2df){1.0, NAN};
    vec_b = (v2df){NAN, 2.0};
    
    /* These comparisons may generate unordered condition code output */
    vec_cmp = vec_a < vec_b;
    vec_cmp = vec_a > vec_b;
    vec_cmp = vec_a <= vec_b;
    vec_cmp = vec_a >= vec_b;
    
    /* Prevent dead code elimination */
    volatile v2df volatile_vec = vec_cmp;
}

/* Parse string to double, handling special values */
double parse_fp_arg(const char* arg) {
    if (strcmp(arg, "nan") == 0 || strcmp(arg, "NAN") == 0) {
        return NAN;
    } else if (strcmp(arg, "inf") == 0 || strcmp(arg, "INF") == 0) {
        return INFINITY;
    } else if (strcmp(arg, "-inf") == 0 || strcmp(arg, "-INF") == 0) {
        return -INFINITY;
    } else if (strcmp(arg, "-0") == 0 || strcmp(arg, "-0.0") == 0) {
        return -0.0;
    } else {
        return atof(arg);
    }
}

int main(int argc, char** argv) {
    /* Array of test cases designed to trigger various condition codes */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, -INFINITY},
        {INFINITY, 1.0},
        {-INFINITY, 1.0},
        {0.0, -0.0},
        {-0.0, 0.0},
        {DBL_MIN, DBL_MAX},
        {DBL_MAX, DBL_MIN},
        {1.0, 2.0},
        {2.0, 1.0},
        {1.0, 1.0}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int total_results = 0;
    
    printf("Testing floating-point comparisons to trigger i386.cc condition code output...\n");
    
    /* Test 1: Full comparison function */
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Mix with volatile values to prevent constant folding */
        a += volatile_zero;
        b += volatile_zero;
        
        int result = compare_floats_full(a, b);
        total_results += result;
        
        const char* classification = classify_comparison(a, b);
        printf("Case %d: a=%g, b=%g, result=0x%x, class=%s\n", 
               i, a, b, result, classification);
    }
    
    /* Test 2: Inline assembly comparisons */
    printf("\nTesting inline assembly comparisons:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Use volatile to force actual computation */
        a *= 1.0 + volatile_zero;
        b *= 1.0 + volatile_zero;
        
        double asm_result = inline_asm_fp_compare(a, b);
        printf("  ASM compare(%g, %g) = %g\n", a, b, asm_result);
        total_results += (int)asm_result;
    }
    
    /* Test 3: Vector comparisons */
    printf("\nTesting vector comparisons:\n");
    vector_comparisons();
    
    /* Test 4: Command line arguments if provided */
    if (argc >= 3) {
        printf("\nTesting command line arguments:\n");
        double arg1 = parse_fp_arg(argv[1]);
        double arg2 = parse_fp_arg(argv[2]);
        
        /* Force all types of comparisons with arguments */
        volatile int cmp_results[12];
        cmp_results[0] = arg1 < arg2;
        cmp_results[1] = arg1 > arg2;
        cmp_results[2] = arg1 <= arg2;
        cmp_results[3] = arg1 >= arg2;
        cmp_results[4] = arg1 == arg2;
        cmp_results[5] = arg1 != arg2;
        cmp_results[6] = isunordered(arg1, arg2);
        cmp_results[7] = isless(arg1, arg2);
        cmp_results[8] = isgreater(arg1, arg2);
        cmp_results[9] = islessequal(arg1, arg2);
        cmp_results[10] = isgreaterequal(arg1, arg2);
        cmp_results[11] = islessgreater(arg1, arg2);
        
        for (int i = 0; i < 12; i++) {
            total_results += cmp_results[i];
        }
    }
    
    /* Additional complex control flow to encourage various condition codes */
    printf("\nComplex control flow test:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Complex if-else chain that might generate various condition codes */
        if (isunordered(a, b)) {
            if (a == a) {  /* Always true, but compiler doesn't know */
                printf("  unordered case\n");
            }
        } else if (a < b) {
            if (a > -b) {
                printf("  a < b and a > -b\n");
            }
        } else if (a > b) {
            if (b < -a) {
                printf("  a > b and b < -a\n");
            }
        } else if (a == b) {
            if (signbit(a) != signbit(b)) {
                printf("  equal with opposite signs\n");
            } else {
                printf("  equal with same sign\n");
            }
        } else {
            /* LTGT case: a != b but not less and not greater (impossible for 
               ordered numbers, but compiler might still generate code) */
            printf("  LTGT-like case\n");
        }
    }
    
    /* Final result to prevent dead code elimination */
    printf("\nTotal accumulated result: %d\n", total_results);
    
    return total_results != 0 ? 0 : 1;
}
