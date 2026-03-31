#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector type for SSE/AVX comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function that performs all possible floating-point comparisons */
static int compare_all_conditions(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map to x86 condition codes */
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
    /* Check unordered first */
    if (isunordered(a, b)) {
        /* Further classification of unordered cases */
        if (isless(a, b)) return "UNORDERED_LESS";  /* Shouldn't happen */
        if (isgreater(a, b)) return "UNORDERED_GREATER";  /* Shouldn't happen */
        return "UNORDERED";
    }
    
    /* Ordered comparisons */
    if (a < b) return "LESS";
    if (a > b) return "GREATER";
    if (a == b) return "EQUAL";
    
    /* Special cases for NaN propagation */
    return "UNKNOWN";
}

/* Inline assembly to force condition code output */
static int inline_asm_fp_compare(double a, double b) {
    int result = 0;
    
    /* Using x87 floating-point compare */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto x87 stack */
        "fldl %1\n\t"           /* Load a onto x87 stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clean up stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%ax, %0"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "ax"
    );
    
    return result;
}

/* Another inline assembly variant with different condition codes */
static int inline_asm_fcomi(double a, double b) {
    int flags = 0;
    
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fcomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "lahf\n\t"              /* Load flags into AH */
        "movzbl %%ah, %0"
        : "=r" (flags)
        : "m" (a), "m" (b)
        : "cc", "st", "ax"
    );
    
    return flags;
}

/* Vector comparison function */
static void vector_comparisons(void) {
    v2df vec_a, vec_b, vec_cmp;
    volatile v2df *volatile_result = &vec_cmp;
    
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
    
    /* Prevent optimization */
    (void)volatile_result;
}

/* Float version for different instruction generation */
static void float_vector_comparisons(void) {
    v4sf fvec_a, fvec_b, fvec_cmp;
    volatile v4sf *volatile_result = &fvec_cmp;
    
    fvec_a = (v4sf){1.0f, NAN, -INFINITY, 0.0f};
    fvec_b = (v4sf){NAN, 2.0f, INFINITY, -0.0f};
    
    fvec_cmp = fvec_a < fvec_b;
    fvec_cmp = fvec_a > fvec_b;
    fvec_cmp = fvec_a <= fvec_b;
    fvec_cmp = fvec_a >= fvec_b;
    fvec_cmp = fvec_a == fvec_b;
    fvec_cmp = fvec_a != fvec_b;
    
    (void)volatile_result;
}

/* Parse double with NaN support */
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

int main(int argc, char *argv[]) {
    /* Test cases covering various scenarios */
    struct test_case {
        double a;
        double b;
        const char *desc;
    } test_cases[] = {
        {NAN, 1.0, "NaN vs 1.0"},
        {1.0, NAN, "1.0 vs NaN"},
        {NAN, NAN, "NaN vs NaN"},
        {INFINITY, -INFINITY, "Inf vs -Inf"},
        {0.0, -0.0, "0.0 vs -0.0"},
        {DBL_MAX, DBL_MIN, "MAX vs MIN"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int total_results = 0;
    
    printf("Testing floating-point comparisons for x86 condition code coverage\n");
    printf("==================================================================\n\n");
    
    /* Test 1: Standard comparisons with all test cases */
    for (int i = 0; i < num_tests; i++) {
        volatile double a = test_cases[i].a;
        volatile double b = test_cases[i].b;
        
        printf("Test %d: %s\n", i + 1, test_cases[i].desc);
        printf("  a = %g, b = %g\n", a, b);
        
        int cmp_result = compare_all_conditions(a, b);
        total_results += cmp_result;
        
        const char *classification = classify_comparison(a, b);
        printf("  Classification: %s\n", classification);
        
        /* Force use of inline assembly */
        int asm_result = inline_asm_fp_compare(a, b);
        int asm_flags = inline_asm_fcomi(a, b);
        
        printf("  Inline ASM results: %d (flags: 0x%02x)\n\n", 
               asm_result, asm_flags & 0xFF);
    }
    
    /* Test 2: Use command-line arguments if provided */
    if (argc >= 3) {
        double arg_a = parse_double(argv[1]);
        double arg_b = parse_double(argv[2]);
        
        printf("Command-line test: %s vs %s\n", argv[1], argv[2]);
        printf("  Parsed: %g vs %g\n", arg_a, arg_b);
        
        volatile double varg_a = arg_a;
        volatile double varg_b = arg_b;
        
        int arg_result = compare_all_conditions(varg_a, varg_b);
        total_results += arg_result;
        
        /* Complex conditional chain to force multiple code paths */
        if (isunordered(varg_a, varg_b)) {
            if (varg_a < varg_b) {
                printf("  Unexpected: unordered but less\n");
            } else if (varg_a > varg_b) {
                printf("  Unexpected: unordered but greater\n");
            } else if (varg_a == varg_b) {
                printf("  UNEQ condition\n");
            } else if (varg_a != varg_b) {
                printf("  UNORDERED condition\n");
            }
        } else {
            if (varg_a < varg_b) {
                printf("  LESS condition\n");
            } else if (varg_a > varg_b) {
                printf("  GREATER condition\n");
            } else if (varg_a == varg_b) {
                printf("  EQUAL condition\n");
            }
            
            /* Additional comparisons for LTGT (not equal and ordered) */
            if (islessgreater(varg_a, varg_b)) {
                printf("  LTGT condition (ordered and not equal)\n");
            }
            
            /* UNGE, UNGT, UNLE, UNLT equivalents */
            if (!isless(varg_a, varg_b)) {
                printf("  UNGE/NLT condition\n");
            }
            if (!islessequal(varg_a, varg_b)) {
                printf("  UNGT/NLE condition\n");
            }
            if (islessequal(varg_a, varg_b) || isunordered(varg_a, varg_b)) {
                printf("  UNLE condition\n");
            }
            if (isless(varg_a, varg_b) || isunordered(varg_a, varg_b)) {
                printf("  UNLT/ULT condition\n");
            }
        }
        printf("\n");
    }
    
    /* Test 3: Vector comparisons */
    printf("Performing vector comparisons...\n");
    vector_comparisons();
    float_vector_comparisons();
    printf("Vector comparisons completed.\n\n");
    
    /* Test 4: Mixed precision comparisons */
    printf("Mixed precision tests:\n");
    {
        volatile float fa = NAN;
        volatile double db = 1.0;
        volatile long double lda = 1.0;
        volatile long double ldb = NAN;
        
        /* Cross-type comparisons */
        if (fa < db) printf("  float NaN < double 1.0\n");
        if (lda > ldb) printf("  long double 1.0 > long double NaN\n");
        
        /* More complex expression */
        volatile double result = (fa * db) + (lda / ldb);
        (void)result;  /* Suppress unused warning */
    }
    
    /* Final summary to prevent dead code elimination */
    printf("\nTotal accumulated results: %d\n", total_results);
    printf("Test completed. Check generated assembly for condition code output.\n");
    
    return total_results != 0 ? 0 : 1;
}
