/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function to classify comparison results */
int compare_classify(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;      /* Less than */
    if (a > b) result |= 2;      /* Greater than */
    if (a <= b) result |= 4;     /* Less than or equal */
    if (a >= b) result |= 8;     /* Greater than or equal */
    if (a == b) result |= 16;    /* Equal */
    if (a != b) result |= 32;    /* Not equal */
    
    /* <math.h> macros that map to x86 unordered comparisons */
    if (isunordered(a, b)) result |= 64;      /* UNORDERED */
    if (isless(a, b)) result |= 128;          /* Ordered less than */
    if (isgreater(a, b)) result |= 256;       /* Ordered greater than */
    if (islessequal(a, b)) result |= 512;     /* Ordered less than or equal */
    if (isgreaterequal(a, b)) result |= 1024; /* Ordered greater than or equal */
    if (islessgreater(a, b)) result |= 2048;  /* LTGT - less or greater (ordered, unequal) */
    
    return result;
}

/* Function that uses switch based on comparison results */
const char* comparison_description(double a, double b) {
    /* Force generation of multiple condition codes */
    if (isunordered(a, b)) {
        return "unordered";
    } else if (isless(a, b)) {
        return "less";
    } else if (isgreater(a, b)) {
        return "greater";
    } else if (a == b) {
        /* Check for -0.0 vs 0.0 equality */
        if (signbit(a) != signbit(b) && a == 0.0 && b == 0.0) {
            return "equal but opposite signs";
        }
        return "equal";
    } else if (islessgreater(a, b)) {
        return "less or greater (LTGT)";
    }
    
    return "unknown";
}

/* Inline assembly to directly trigger condition code output */
double inline_compare_asm(double a, double b) {
    double result;
    int unordered_flag, greater_flag, less_flag;
    
    /* Using x87 floating-point compare */
    __asm__ volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clean up FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "seta %%cl\n\t"         /* Set if above (greater than, ordered) */
        "setb %%dl\n\t"         /* Set if below (less than, ordered) */
        : "=a"(unordered_flag), "=c"(greater_flag), "=d"(less_flag)
        : "m"(a), "m"(b)
        : "cc"
    );
    
    /* Use the flags to compute result */
    if (unordered_flag) {
        result = global_nan;
    } else if (greater_flag) {
        result = a;
    } else if (less_flag) {
        result = b;
    } else {
        /* Equal or unordered special cases */
        result = (a + b) / 2.0;
    }
    
    return result;
}

/* Vector comparisons using GCC extensions */
#ifdef __SSE2__
void vector_comparisons(void) {
    typedef double v2df __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    volatile v2df a = {1.0, NAN};
    volatile v2df b = {NAN, 2.0};
    volatile v2df c = {3.0, 4.0};
    volatile v2df d = {5.0, 6.0};
    
    /* These comparisons may generate unordered condition codes */
    v2df cmp1 = a < b;   /* May contain unordered results */
    v2df cmp2 = c > d;
    v2df cmp3 = a == b;
    v2df cmp4 = c != d;
    
    /* Use the results to prevent optimization */
    v2di* cmp1_int = (v2di*)&cmp1;
    v2di* cmp2_int = (v2di*)&cmp2;
    
    printf("Vector comparison results: %llx %llx\n", 
           (*cmp1_int)[0], (*cmp1_int)[1]);
    printf("Vector comparison results: %llx %llx\n",
           (*cmp2_int)[0], (*cmp2_int)[1]);
}
#endif

/* Complex comparison function that should generate many condition codes */
double complex_compare_sequence(double x, double y) {
    volatile double result = 0.0;
    
    /* Series of comparisons that should generate different condition codes */
    if (x < y) {
        result += 1.0;
    }
    if (x > y) {
        result += 2.0;
    }
    if (x <= y) {
        result += 4.0;
    }
    if (x >= y) {
        result += 8.0;
    }
    if (x == y) {
        result += 16.0;
    }
    if (x != y) {
        result += 32.0;
    }
    
    /* Check for unordered using isunordered */
    if (isunordered(x, y)) {
        result += 64.0;
    }
    
    /* Use isless/greater which don't raise exception for NaN */
    if (isless(x, y)) {
        result += 128.0;
    }
    if (isgreater(x, y)) {
        result += 256.0;
    }
    
    /* LTGT condition - ordered and not equal */
    if (islessgreater(x, y)) {
        result += 512.0;
    }
    
    return result;
}

/* Parse string to double, handling special values */
double parse_double(const char* str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NAN") == 0) {
        return NAN;
    } else if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0) {
        return INFINITY;
    } else if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0) {
        return -INFINITY;
    } else {
        return atof(str);
    }
}

int main(int argc, char* argv[]) {
    /* Test cases designed to trigger various condition codes */
    double test_cases[][2] = {
        {NAN, 1.0},           /* UNORDERED */
        {1.0, NAN},           /* UNORDERED */
        {NAN, NAN},           /* UNORDERED */
        {INFINITY, -INFINITY},/* Greater than */
        {-INFINITY, INFINITY},/* Less than */
        {0.0, -0.0},          /* Equal (but different signs) */
        {1.0, 2.0},           /* Less than */
        {2.0, 1.0},           /* Greater than */
        {DBL_MAX, DBL_MAX},   /* Equal */
        {DBL_MIN, DBL_MIN},   /* Equal */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Testing with user values: %g, %g\n", a, b);
        
        int result = compare_classify(a, b);
        printf("Comparison classification: 0x%x\n", result);
        
        const char* desc = comparison_description(a, b);
        printf("Comparison description: %s\n", desc);
        
        double asm_result = inline_compare_asm(a, b);
        printf("Inline assembly result: %g\n", asm_result);
        
        double complex_result = complex_compare_sequence(a, b);
        printf("Complex comparison sequence result: %g\n", complex_result);
    }
    
    /* Run through all test cases */
    printf("\nRunning test cases:\n");
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        printf("\nTest case %d: %g vs %g\n", i, a, b);
        
        /* Force all types of comparisons */
        int classify_result = compare_classify(a, b);
        printf("  Classification: 0x%x\n", classify_result);
        
        /* Use inline assembly */
        double asm_res = inline_compare_asm(a, b);
        printf("  Assembly comparison: %g\n", asm_res);
        
        /* Complex sequence */
        double seq_res = complex_compare_sequence(a, b);
        printf("  Sequence result: %g\n", seq_res);
    }
    
#ifdef __SSE2__
    printf("\nPerforming vector comparisons:\n");
    vector_comparisons();
#endif
    
    /* Additional tests with volatile to prevent optimization */
    volatile double v1 = global_nan;
    volatile double v2 = 3.14159;
    
    /* These should generate unordered condition code output */
    if (v1 < v2) {
        printf("Unexpected: NaN < 3.14159\n");
    }
    if (v1 > v2) {
        printf("Unexpected: NaN > 3.14159\n");
    }
    if (v1 == v2) {
        printf("Unexpected: NaN == 3.14159\n");
    }
    if (v1 != v2) {
        printf("Expected: NaN != 3.14159\n");
    }
    
    /* Test with isunordered */
    if (isunordered(v1, v2)) {
        printf("Expected: isunordered(NaN, 3.14159) is true\n");
    }
    
    /* Test UNEQ (unordered or equal) - not directly in C, but can be synthesized */
    if (isunordered(v1, v2) || v1 == v2) {
        printf("UNEQ condition (unordered or equal)\n");
    }
    
    /* Test UNGE (not less than) = unordered or greater or equal */
    if (!isless(v1, v2)) {
        printf("UNGE condition (not less than)\n");
    }
    
    /* Test UNGT (not less than or equal) = unordered or greater */
    if (!islessequal(v1, v2)) {
        printf("UNGT condition (not less than or equal)\n");
    }
    
    /* Test UNLE (unordered or less or equal) */
    if (isunordered(v1, v2) || islessequal(v1, v2)) {
        printf("UNLE condition (unordered or less or equal)\n");
    }
    
    /* Test UNLT (unordered or less than) */
    if (isunordered(v1, v2) || isless(v1, v2)) {
        printf("UNLT condition (unordered or less than)\n");
    }
    
    /* Test LTGT (less or greater, ordered) */
    if (islessgreater(v1, v2)) {
        printf("LTGT condition (less or greater, ordered)\n");
    }
    
    return 0;
}
