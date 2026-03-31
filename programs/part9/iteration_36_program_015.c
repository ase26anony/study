#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector type for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Function that performs all possible floating-point comparisons */
int compare_floats_comprehensive(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;      /* Less than */
    if (a > b) result |= 2;      /* Greater than */
    if (a <= b) result |= 4;     /* Less than or equal */
    if (a >= b) result |= 8;     /* Greater than or equal */
    if (a == b) result |= 16;    /* Equal */
    if (a != b) result |= 32;    /* Not equal */
    
    /* <math.h> macros that map to x86 condition codes */
    if (isunordered(a, b)) result |= 64;      /* UNORDERED */
    if (isless(a, b)) result |= 128;          /* Ordered less than */
    if (isgreater(a, b)) result |= 256;       /* Ordered greater than */
    if (islessequal(a, b)) result |= 512;     /* Ordered less or equal */
    if (isgreaterequal(a, b)) result |= 1024; /* Ordered greater or equal */
    if (islessgreater(a, b)) result |= 2048;  /* LTGT - less or greater (ordered, not equal) */
    
    return result;
}

/* Function with switch based on comparison results */
const char* classify_comparison(double a, double b) {
    /* Force actual comparison by using volatile */
    volatile double va = a;
    volatile double vb = b;
    
    if (isunordered(va, vb)) {
        return "unordered";
    } else if (isless(va, vb)) {
        return "less";
    } else if (isgreater(va, vb)) {
        return "greater";
    } else if (va == vb) {
        /* Distinguish +0.0 and -0.0 */
        if (signbit(va) != signbit(vb)) {
            return "equal_but_opposite_sign";
        }
        return "equal";
    } else {
        return "other";
    }
}

/* Inline assembly that directly uses floating-point condition codes */
double inline_asm_fp_compare(double a, double b) {
    double result;
    int unordered_flag, greater_flag, equal_flag;
    
    /* Use fucomip which sets condition codes including parity for unordered */
    asm volatile (
        "fucomip %%st(1), %%st\n\t"
        "setp %[unordered]\n\t"
        "seta %[greater]\n\t"
        "sete %[equal]"
        : [unordered] "=r" (unordered_flag),
          [greater] "=r" (greater_flag),
          [equal] "=r" (equal_flag)
        : "t" (a), "u" (b)
        : "cc", "st"
    );
    
    /* Use conditional move based on comparison results */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "jp 1f\n\t"          /* Jump if unordered */
        "ja 2f\n\t"          /* Jump if above (greater) */
        "jb 3f\n\t"          /* Jump if below (less) */
        "mov $0x3FF0000000000000, %%rax\n\t"  /* 1.0 for equal */
        "jmp 4f\n"
        "1:\n\t"
        "mov $0x7FF8000000000000, %%rax\n\t"  /* QNaN for unordered */
        "jmp 4f\n"
        "2:\n\t"
        "mov $0x4000000000000000, %%rax\n\t"  /* 2.0 for greater */
        "jmp 4f\n"
        "3:\n\t"
        "mov $0xBFE0000000000000, %%rax\n\t"  /* -0.5 for less */
        "4:\n\t"
        "movq %%rax, %0"
        : "=m" (result)
        : "m" (a), "m" (b)
        : "rax", "cc"
    );
    
    return result;
}

/* Vector comparison function */
void vector_comparisons(void) {
    v2df vec_a, vec_b, cmp_result;
    
    /* Initialize vectors with mixed values */
    double a_data[2] = {NAN, 1.0};
    double b_data[2] = {2.0, NAN};
    
    vec_a = *(v2df*)a_data;
    vec_b = *(v2df*)b_data;
    
    /* Perform vector comparisons - may generate multiple condition codes */
    cmp_result = vec_a < vec_b;   /* Less than comparison */
    cmp_result = vec_a > vec_b;   /* Greater than comparison */
    cmp_result = vec_a <= vec_b;  /* Less or equal */
    cmp_result = vec_a >= vec_b;  /* Greater or equal */
    cmp_result = vec_a == vec_b;  /* Equal */
    cmp_result = vec_a != vec_b;  /* Not equal */
    
    /* Prevent dead code elimination */
    volatile v2df keep = cmp_result;
    (void)keep;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Array of test cases designed to trigger various condition codes */
    struct {
        double a, b;
        const char *desc;
    } test_cases[] = {
        {NAN, 1.0, "NAN vs 1.0"},
        {1.0, NAN, "1.0 vs NAN"},
        {NAN, NAN, "NAN vs NAN"},
        {INFINITY, -INFINITY, "INF vs -INF"},
        {INFINITY, 1.0, "INF vs 1.0"},
        {-INFINITY, 1.0, "-INF vs 1.0"},
        {0.0, -0.0, "+0.0 vs -0.0"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
        {0.0, 0.0, "0.0 vs 0.0"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command-line arguments if provided */
    if (argc >= 3) {
        for (int i = 1; i + 1 < argc; i += 2) {
            double a = strtod(argv[i], NULL);
            double b = strtod(argv[i + 1], NULL);
            
            /* Handle "nan" string */
            if (strcasecmp(argv[i], "nan") == 0) a = NAN;
            if (strcasecmp(argv[i + 1], "nan") == 0) b = NAN;
            if (strcasecmp(argv[i], "inf") == 0) a = INFINITY;
            if (strcasecmp(argv[i + 1], "inf") == 0) b = INFINITY;
            if (strcasecmp(argv[i], "-inf") == 0) a = -INFINITY;
            if (strcasecmp(argv[i + 1], "-inf") == 0) b = -INFINITY;
            
            printf("Testing command-line: %g vs %g\n", a, b);
            int result = compare_floats_comprehensive(a, b);
            const char *cls = classify_comparison(a, b);
            double asm_result = inline_asm_fp_compare(a, b);
            printf("  Result: 0x%04x, Class: %s, ASM: %g\n\n", 
                   result, cls, asm_result);
        }
    }
    
    /* Run predefined test cases */
    printf("Running predefined test cases:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i].a;
        double b = test_cases[i].b;
        
        printf("\nTest %d: %s\n", i + 1, test_cases[i].desc);
        
        /* Perform comprehensive comparisons */
        int cmp_result = compare_floats_comprehensive(a, b);
        printf("  Comparison mask: 0x%04x\n", cmp_result);
        
        /* Use classification function */
        const char *classification = classify_comparison(a, b);
        printf("  Classification: %s\n", classification);
        
        /* Use inline assembly comparison */
        double asm_result = inline_asm_fp_compare(a, b);
        printf("  ASM comparison result: %g\n", asm_result);
        
        /* Test with volatile to prevent optimization */
        volatile double va = a;
        volatile double vb = b;
        volatile int unordered_test = isunordered(va, vb);
        volatile int less_test = isless(va, vb);
        volatile int greater_test = isgreater(va, vb);
        volatile int eq_test = (va == vb);
        volatile int ne_test = (va != vb);
        
        printf("  Unordered: %d, Less: %d, Greater: %d, Equal: %d, NotEqual: %d\n",
               unordered_test, less_test, greater_test, eq_test, ne_test);
    }
    
    /* Perform vector comparisons */
    printf("\nPerforming vector comparisons...\n");
    vector_comparisons();
    
    /* Complex control flow with many comparisons */
    printf("\nComplex control flow test:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i].a;
        double b = test_cases[i].b;
        
        /* Switch-like behavior using if-else chain */
        if (isunordered(a, b)) {
            printf("  Case %d: UNORDERED\n", i);
        } else if (isless(a, b)) {
            printf("  Case %d: ORDERED LESS\n", i);
        } else if (isgreater(a, b)) {
            printf("  Case %d: ORDERED GREATER\n", i);
        } else if (a == b) {
            if (signbit(a) != signbit(b)) {
                printf("  Case %d: EQUAL BUT OPPOSITE SIGN (UNEQ?)\n", i);
            } else {
                printf("  Case %d: EQUAL\n", i);
            }
        } else {
            /* This should rarely happen but covers LTGT */
            printf("  Case %d: ORDERED NOT EQUAL (LTGT)\n", i);
        }
    }
    
    /* Final summary to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < num_cases; i++) {
        final_result += compare_floats_comprehensive(
            test_cases[i].a, test_cases[i].b);
    }
    
    printf("\nFinal checksum: 0x%08x\n", final_result);
    
    return 0;
}
