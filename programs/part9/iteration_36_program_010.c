#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector type for SSE/AVX comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function that performs all possible unordered comparisons */
static int perform_unordered_comparisons(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;      /* LT - may generate 'nge' or similar */
    if (a > b) result |= 2;      /* GT - may generate 'ngt' or similar */
    if (a <= b) result |= 4;     /* LE - may generate 'nle' or similar */
    if (a >= b) result |= 8;     /* GE - may generate 'nge' or similar */
    if (a == b) result |= 16;    /* EQ - may generate 'neq' or similar */
    if (a != b) result |= 32;    /* NEQ - may generate 'ne' or similar */
    
    /* <math.h> macros that map directly to x86 unordered predicates */
    if (isunordered(a, b)) result |= 64;      /* UNORDERED - should generate 'unord' */
    if (isless(a, b)) result |= 128;          /* LT with ordered check */
    if (isgreater(a, b)) result |= 256;       /* GT with ordered check */
    if (islessequal(a, b)) result |= 512;     /* LE with ordered check */
    if (isgreaterequal(a, b)) result |= 1024; /* GE with ordered check */
    if (islessgreater(a, b)) result |= 2048;  /* LTGT - should generate 'une' */
    
    /* Additional unordered comparisons */
    if (!isunordered(a, b)) result |= 4096;   /* ORDERED - should generate 'ord' */
    
    return result;
}

/* Function with switch based on comparison results */
static const char* classify_comparison(double a, double b) {
    if (isunordered(a, b)) {
        return "UNORDERED";
    }
    
    /* This switch structure may force compiler to generate 
       multiple different condition code outputs */
    if (a < b) return "LT";
    if (a > b) return "GT";
    if (a == b) return "EQ";
    
    /* Handle special cases that might generate UNEQ, UNGE, etc. */
    if (!(a >= b)) return "NOT GE (possibly NLT)";
    if (!(a <= b)) return "NOT LE (possibly NLE)";
    if (!(a != b)) return "NOT NE (possibly EQ)";
    
    return "UNKNOWN";
}

/* Inline assembly to directly trigger condition code output */
static int inline_asm_fp_comparison(double a, double b) {
    int result;
    
    /* Using x87 FPU comparison - may generate 'fucomip' with condition codes */
    asm volatile (
        "fucomip %%st(1), %%st\n\t"
        "setp %%al\n\t"
        "setb %%ah\n\t"
        "sete %%dl\n\t"
        "movzbl %%al, %%eax\n\t"
        "shl $8, %%eax\n\t"
        "movzbl %%ah, %%ecx\n\t"
        "or %%ecx, %%eax\n\t"
        "shl $8, %%eax\n\t"
        "movzbl %%dl, %%ecx\n\t"
        "or %%ecx, %%eax"
        : "=a"(result)
        : "t"(a), "u"(b)
        : "cc", "st", "dl", "ecx"
    );
    
    return result;
}

/* Vector comparison function */
static void vector_comparisons(void) {
    v2df vec_a, vec_b, vec_cmp;
    volatile v2df vec_result;
    
    /* Initialize vectors with mixed values including NaN */
    vec_a = (v2df){1.0, NAN};
    vec_b = (v2df){NAN, 2.0};
    
    /* Perform vector comparisons - may generate multiple condition codes */
    vec_cmp = vec_a < vec_b;   /* LT comparison */
    vec_result = vec_cmp;
    
    vec_cmp = vec_a > vec_b;   /* GT comparison */
    vec_result = vec_cmp;
    
    vec_cmp = vec_a == vec_b;  /* EQ comparison */
    vec_result = vec_cmp;
    
    vec_cmp = vec_a != vec_b;  /* NEQ comparison */
    vec_result = vec_cmp;
    
    (void)vec_result; /* Prevent unused variable warning */
}

/* Parse double from string, handling "nan", "inf", "-inf" */
static double parse_fp_arg(const char *arg) {
    if (strcmp(arg, "nan") == 0) return NAN;
    if (strcmp(arg, "inf") == 0) return INFINITY;
    if (strcmp(arg, "-inf") == 0) return -INFINITY;
    return atof(arg);
}

int main(int argc, char *argv[]) {
    /* Test cases designed to trigger various unordered conditions */
    struct {
        double a, b;
        const char *desc;
    } test_cases[] = {
        {NAN, 1.0, "NAN vs 1.0"},
        {1.0, NAN, "1.0 vs NAN"},
        {NAN, NAN, "NAN vs NAN"},
        {INFINITY, -INFINITY, "INF vs -INF"},
        {0.0, -0.0, "0.0 vs -0.0"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
        {INFINITY, INFINITY, "INF vs INF"},
        {-INFINITY, -INFINITY, "-INF vs -INF"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_fp_arg(argv[1]);
        double b = parse_fp_arg(argv[2]);
        
        printf("Testing with command line values: %g, %g\n", a, b);
        
        /* Force all comparison types */
        int result = perform_unordered_comparisons(a, b);
        const char *classification = classify_comparison(a, b);
        int asm_result = inline_asm_fp_comparison(a, b);
        
        printf("Result: 0x%x\n", result);
        printf("Classification: %s\n", classification);
        printf("Assembly result: 0x%x\n", asm_result);
        
        /* Prevent dead code elimination */
        volatile int dummy = result + asm_result;
        (void)dummy;
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i].a;
        double b = test_cases[i].b;
        
        printf("\nTest %d: %s\n", i + 1, test_cases[i].desc);
        
        /* Perform comparisons that may generate various condition codes */
        int cmp_result = perform_unordered_comparisons(a, b);
        const char *cls = classify_comparison(a, b);
        
        printf("  Comparison result: 0x%04x\n", cmp_result);
        printf("  Classification: %s\n", cls);
        
        /* Use inline assembly for some cases to force specific codegen */
        if (i % 3 == 0) {
            int asm_res = inline_asm_fp_comparison(a, b);
            printf("  Inline ASM result: 0x%x\n", asm_res);
        }
    }
    
    /* Vector comparisons */
    printf("\nPerforming vector comparisons:\n");
    vector_comparisons();
    
    /* Additional complex control flow to force multiple condition code uses */
    printf("\nComplex control flow test:\n");
    for (int i = 0; i < 5; i++) {
        double x = test_cases[i].a;
        double y = test_cases[i].b;
        
        /* Nested if-else with various comparisons */
        if (isunordered(x, y)) {
            printf("  Case %d: Unordered\n", i);
        } else if (x < y) {
            printf("  Case %d: Less than\n", i);
        } else if (x > y) {
            printf("  Case %d: Greater than\n", i);
        } else if (x == y) {
            printf("  Case %d: Equal\n", i);
        } else {
            printf("  Case %d: Other (should not happen)\n", i);
        }
        
        /* Switch-like behavior using comparison results */
        int cmp_val = 0;
        if (x < y) cmp_val = 1;
        else if (x > y) cmp_val = 2;
        else if (x == y) cmp_val = 3;
        else if (x != y) cmp_val = 4;
        else if (x <= y) cmp_val = 5;
        else if (x >= y) cmp_val = 6;
        
        (void)cmp_val; /* Prevent unused warning */
    }
    
    /* Final summary to prevent optimization */
    volatile double final_check = global_nan + global_inf + global_neg_inf;
    printf("\nFinal check: %g (should be nan)\n", final_check);
    
    return 0;
}
