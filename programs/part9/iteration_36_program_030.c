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

/* Vector type for SSE/AVX comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function that performs all possible unordered comparisons */
static int compare_all_conditions(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map directly to x86 condition codes */
    if (isunordered(a, b)) result |= 64;
    if (isless(a, b)) result |= 128;
    if (isgreater(a, b)) result |= 256;
    if (islessequal(a, b)) result |= 512;
    if (isgreaterequal(a, b)) result |= 1024;
    if (islessgreater(a, b)) result |= 2048;  /* LTGT */
    
    return result;
}

/* Function with switch based on comparison results */
static const char* classify_comparison(double a, double b) {
    /* Force compiler to generate actual comparison instructions */
    volatile double va = a;
    volatile double vb = b;
    
    if (isunordered(va, vb)) {
        return "unordered";
    } else if (isless(va, vb)) {
        return "less";
    } else if (isgreater(va, vb)) {
        return "greater";
    } else if (va == vb) {
        /* Handle +0.0 vs -0.0 */
        if (signbit(va) != signbit(vb)) {
            return "equal_but_opposite_sign";
        }
        return "equal";
    } else if (islessgreater(va, vb)) {
        return "less_or_greater";  /* LTGT */
    }
    
    return "unknown";
}

/* Inline assembly to directly trigger condition code output */
static int inline_asm_fp_compare(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare with unordered handling */
    __asm__ volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack, a is now st(0), b is st(1) */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop st(0) */
        "fstp %%st(0)\n\t"      /* Pop remaining b from stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $8, %%edx\n\t"
        "orl %%edx, %%eax"
        : "=a" (result)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    return result;
}

/* Vector comparison function */
static void vector_comparisons(void) {
    v2df vec_a, vec_b, vec_cmp;
    volatile v2df *volatile_ptr;
    
    /* Initialize vectors with mixed values including NaN */
    vec_a = (v2df){1.0, NAN};
    vec_b = (v2df){NAN, 2.0};
    
    /* Perform vector comparisons - may generate multiple condition codes */
    vec_cmp = vec_a < vec_b;   /* UNORDERED comparisons */
    vec_cmp = vec_a > vec_b;   /* More unordered comparisons */
    vec_cmp = vec_a == vec_b;  /* UNEQ comparisons */
    
    /* Prevent optimization */
    volatile_ptr = &vec_cmp;
    (void)volatile_ptr;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    double test_values[10];
    int num_tests = 0;
    
    /* Parse command line arguments for test values */
    for (int i = 1; i < argc && num_tests < 10; i++) {
        if (strcmp(argv[i], "nan") == 0 || strcmp(argv[i], "NaN") == 0) {
            test_values[num_tests++] = NAN;
        } else if (strcmp(argv[i], "inf") == 0) {
            test_values[num_tests++] = INFINITY;
        } else if (strcmp(argv[i], "-inf") == 0) {
            test_values[num_tests++] = -INFINITY;
        } else {
            test_values[num_tests++] = atof(argv[i]);
        }
    }
    
    /* Default test cases if no arguments provided */
    if (num_tests == 0) {
        test_values[0] = NAN;
        test_values[1] = 1.0;
        test_values[2] = -1.0;
        test_values[3] = INFINITY;
        test_values[4] = -INFINITY;
        test_values[5] = 0.0;
        test_values[6] = -0.0;
        test_values[7] = DBL_MAX;
        test_values[8] = DBL_MIN;
        num_tests = 9;
    }
    
    printf("Testing x86 floating-point condition code generation...\n");
    
    /* Test all combinations of values */
    int total_results = 0;
    for (int i = 0; i < num_tests; i++) {
        for (int j = 0; j < num_tests; j++) {
            double a = test_values[i];
            double b = test_values[j];
            
            /* Force compiler to generate actual comparison code */
            volatile double va = a;
            volatile double vb = b;
            
            /* Test 1: All condition comparisons */
            int cmp_result = compare_all_conditions(va, vb);
            total_results += cmp_result;
            
            /* Test 2: Classification */
            const char *classification = classify_comparison(va, vb);
            (void)classification;  /* Prevent unused variable warning */
            
            /* Test 3: Inline assembly */
            int asm_result = inline_asm_fp_compare(va, vb);
            total_results += asm_result;
            
            /* Test specific unordered conditions */
            if (isunordered(va, vb)) {
                /* These should trigger UNORDERED output */
                int unord_test = (va != vb) ? 1 : 0;
                total_results += unord_test;
            }
            
            /* Test for UNEQ (unordered or equal) */
            if (!isless(va, vb) && !isgreater(va, vb)) {
                /* Could be equal or unordered */
                int uneq_test = (va == vb || isunordered(va, vb)) ? 1 : 0;
                total_results += uneq_test;
            }
            
            /* Test for LTGT (less or greater, but not equal and not unordered) */
            if (islessgreater(va, vb) && !isunordered(va, vb)) {
                total_results += 1000;  /* Mark LTGT case */
            }
        }
    }
    
    /* Test vector comparisons */
    vector_comparisons();
    
    /* Additional inline assembly tests for specific condition codes */
    {
        double a = global_nan;
        double b = 1.0;
        int cc_result;
        
        /* Test UNGE (not less than) */
        __asm__ volatile (
            "fldl %2\n\t"
            "fldl %1\n\t"
            "fucomip %%st(1), %%st\n\t"
            "fstp %%st(0)\n\t"
            "setae %%al\n\t"    /* Set if above or equal (CF=0) - includes unordered */
            "movzbl %%al, %0"
            : "=r" (cc_result)
            : "m" (a), "m" (b)
            : "cc", "st"
        );
        total_results += cc_result;
        
        /* Test UNLE (unordered or less or equal) */
        __asm__ volatile (
            "fldl %2\n\t"
            "fldl %1\n\t"
            "fucomip %%st(1), %%st\n\t"
            "fstp %%st(0)\n\t"
            "setbe %%al\n\t"    /* Set if below or equal (CF=1 or ZF=1) */
            "movzbl %%al, %0"
            : "=r" (cc_result)
            : "m" (b), "m" (a)  /* Swapped to test different condition */
            : "cc", "st"
        );
        total_results += cc_result;
    }
    
    /* Prevent dead code elimination */
    printf("Total results checksum: %d\n", total_results);
    
    return (total_results != 0) ? 0 : 1;
}
