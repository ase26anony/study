/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_zero = 0.0;

/* Vector type for SSE/AVX comparisons */
typedef double v2df __attribute__((vector_size(16)));

/* Function that performs all possible floating-point comparisons */
int compare_all_results(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;      /* LT - may generate 'nge' or 'nlt' */
    if (a > b) result |= 2;      /* GT - may generate 'ngt' or 'nle' */
    if (a <= b) result |= 4;     /* LE - may generate 'ngt' or 'nle' */
    if (a >= b) result |= 8;     /* GE - may generate 'nge' or 'nlt' */
    if (a == b) result |= 16;    /* EQ - may generate 'ueq' when unordered */
    if (a != b) result |= 32;    /* NEQ - may generate 'une' or 'ltgt' */
    
    /* <math.h> macros that map directly to x86 condition codes */
    if (isunordered(a, b)) result |= 64;      /* UNORDERED - should generate 'unord' */
    if (isless(a, b)) result |= 128;          /* LT with ordered check */
    if (isgreater(a, b)) result |= 256;       /* GT with ordered check */
    if (islessequal(a, b)) result |= 512;     /* LE with ordered check */
    if (isgreaterequal(a, b)) result |= 1024; /* GE with ordered check */
    if (islessgreater(a, b)) result |= 2048;  /* LTGT - should generate 'une' */
    
    return result;
}

/* Function with switch based on comparison results */
const char* classify_comparison(double a, double b) {
    /* Use fpclassify to get detailed classification */
    int a_class = fpclassify(a);
    int b_class = fpclassify(b);
    
    if (isunordered(a, b)) {
        return "UNORDERED";
    } else if (isless(a, b)) {
        return "LESS";
    } else if (isgreater(a, b)) {
        return "GREATER";
    } else if (a == b) {
        /* Handle +0.0 vs -0.0 */
        if (signbit(a) != signbit(b) && a == 0.0 && b == 0.0) {
            return "ZERO_SIGN_DIFF";
        }
        return "EQUAL";
    } else if (islessgreater(a, b)) {
        return "LESS_GREATER";
    }
    
    return "UNKNOWN";
}

/* Inline assembly to force condition code output */
double inline_asm_fp_compare(double a, double b) {
    double result;
    int unordered_flag, greater_flag, equal_flag;
    
    /* Force x87 FPU comparison with inline assembly */
    /* This should generate the condition code mnemonics */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clear FPU stack */
        "setp %b0\n\t"          /* Set if unordered (parity) - may use 'unord' */
        "seta %b1\n\t"          /* Set if greater - may use 'nbe' or 'a' */
        "sete %b2\n\t"          /* Set if equal - may use 'e' or 'ueq' */
        : "=r"(unordered_flag), "=r"(greater_flag), "=r"(equal_flag)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* Another inline assembly with different condition codes */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "jp 1f\n\t"             /* Jump if unordered */
        "ja 2f\n\t"             /* Jump if above (greater) */
        "je 3f\n\t"             /* Jump if equal */
        "jb 4f\n\t"             /* Jump if below (less) */
        "1:\n\t"
        "movl $1, %0\n\t"
        "jmp 5f\n\t"
        "2:\n\t"
        "movl $2, %0\n\t"
        "jmp 5f\n\t"
        "3:\n\t"
        "movl $3, %0\n\t"
        "jmp 5f\n\t"
        "4:\n\t"
        "movl $4, %0\n\t"
        "5:\n\t"
        : "=r"(result)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    return result;
}

/* Vector comparison function */
void vector_comparisons(void) {
    v2df a = {1.0, 2.0};
    v2df b = {NAN, 3.0};
    v2df c = {INFINITY, -INFINITY};
    v2df d = {0.0, -0.0};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp1 = a < b;    /* Contains NAN - unordered */
    v2df cmp2 = c > d;    /* Infinity comparisons */
    v2df cmp3 = a == d;   /* Equality with signed zero */
    
    /* Use volatile to prevent optimization */
    volatile v2df v_result = cmp1 + cmp2 + cmp3;
    (void)v_result;
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Array of test cases with various floating-point values */
    struct test_case {
        double a;
        double b;
        const char *desc;
    } test_cases[] = {
        {NAN, 1.0, "NAN vs 1.0"},
        {1.0, NAN, "1.0 vs NAN"},
        {NAN, NAN, "NAN vs NAN"},
        {INFINITY, -INFINITY, "INF vs -INF"},
        {INFINITY, 1.0, "INF vs 1.0"},
        {0.0, -0.0, "+0.0 vs -0.0"},
        {DBL_MAX, DBL_MIN, "MAX vs MIN"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    int total_results = 0;
    
    printf("Testing floating-point comparisons to trigger x86 condition code output\n");
    printf("======================================================================\n\n");
    
    /* Test 1: Standard comparisons */
    printf("Test 1: Standard comparison operators\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i].a;
        double b = test_cases[i].b;
        
        printf("\nCase %d: %s\n", i, test_cases[i].desc);
        printf("  a = %.20g, b = %.20g\n", a, b);
        
        int cmp_result = compare_all_results(a, b);
        total_results |= cmp_result;
        
        printf("  Comparison result mask: 0x%04x\n", cmp_result);
        printf("  Classification: %s\n", classify_comparison(a, b));
    }
    
    /* Test 2: Inline assembly comparisons */
    printf("\n\nTest 2: Inline assembly FP comparisons\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i].a;
        double b = test_cases[i].b;
        
        printf("\nCase %d: %s\n", i, test_cases[i].desc);
        double asm_result = inline_asm_fp_compare(a, b);
        printf("  Inline ASM result: %.20g\n", asm_result);
    }
    
    /* Test 3: Vector comparisons */
    printf("\n\nTest 3: Vector (SSE/AVX) comparisons\n");
    vector_comparisons();
    printf("  Vector comparisons completed\n");
    
    /* Test 4: Use command-line arguments if provided */
    if (argc >= 3) {
        printf("\n\nTest 4: Command-line argument comparisons\n");
        for (int i = 1; i + 1 < argc; i += 2) {
            double a, b;
            
            /* Parse with NaN support */
            if (strcasecmp(argv[i], "nan") == 0) a = NAN;
            else a = atof(argv[i]);
            
            if (strcasecmp(argv[i+1], "nan") == 0) b = NAN;
            else b = atof(argv[i+1]);
            
            printf("  Input: %s vs %s -> ", argv[i], argv[i+1]);
            printf("Classification: %s\n", classify_comparison(a, b));
            
            /* Force generation of all condition codes */
            volatile int res = compare_all_results(a, b);
            (void)res;
        }
    }
    
    /* Test 5: Mixed ordered/unordered in control flow */
    printf("\n\nTest 5: Mixed ordered/unordered control flow\n");
    {
        double values[] = {NAN, INFINITY, -INFINITY, 0.0, -0.0, 1.0, -1.0};
        int num_values = sizeof(values) / sizeof(values[0]);
        
        for (int i = 0; i < num_values; i++) {
            for (int j = 0; j < num_values; j++) {
                double a = values[i];
                double b = values[j];
                
                /* Complex conditional with multiple branches */
                if (isunordered(a, b)) {
                    if (isnan(a) && isnan(b)) {
                        /* Both NaN */
                        total_results |= 0x1000;
                    } else if (isnan(a)) {
                        /* Only a is NaN */
                        total_results |= 0x2000;
                    } else {
                        /* Only b is NaN */
                        total_results |= 0x4000;
                    }
                } else if (a < b) {
                    if (a == 0.0 && signbit(a)) {
                        /* Negative zero less than something */
                        total_results |= 0x8000;
                    }
                } else if (a > b) {
                    if (b == 0.0 && signbit(b)) {
                        /* Something greater than negative zero */
                        total_results |= 0x10000;
                    }
                } else if (a == b) {
                    if (signbit(a) != signbit(b)) {
                        /* Signed zeros */
                        total_results |= 0x20000;
                    }
                }
            }
        }
    }
    
    /* Final output to prevent dead code elimination */
    printf("\n\nFinal summary result: 0x%08x\n", total_results);
    printf("Program completed. Check generated assembly for condition code output.\n");
    
    return total_results != 0 ? 0 : 1;
}
