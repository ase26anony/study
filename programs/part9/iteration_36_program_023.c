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

/* Function to classify comparison results */
int classify_comparison(double a, double b) {
    int result = 0;
    
    /* Standard comparisons that can produce unordered results */
    if (a < b) result |= 1;      /* Less than */
    if (a > b) result |= 2;      /* Greater than */
    if (a == b) result |= 4;     /* Equal */
    if (a != b) result |= 8;     /* Not equal */
    if (a <= b) result |= 16;    /* Less than or equal */
    if (a >= b) result |= 32;    /* Greater than or equal */
    
    /* <math.h> macros for unordered comparisons */
    if (isunordered(a, b)) result |= 64;     /* UNORDERED */
    if (isless(a, b)) result |= 128;         /* Ordered less than */
    if (isgreater(a, b)) result |= 256;      /* Ordered greater than */
    if (islessequal(a, b)) result |= 512;    /* Ordered less or equal */
    if (isgreaterequal(a, b)) result |= 1024; /* Ordered greater or equal */
    if (islessgreater(a, b)) result |= 2048; /* LTGT */
    
    return result;
}

/* Function with mixed ordered/unordered comparisons in control flow */
const char* compare_with_switch(double a, double b) {
    /* Use fpclassify to get detailed comparison results */
    int a_class = fpclassify(a);
    int b_class = fpclassify(b);
    
    if (isunordered(a, b)) {
        /* Unordered comparisons */
        if (isless(a, b)) return "less (unordered)";
        if (isgreater(a, b)) return "greater (unordered)";
        if (a == b) return "ueq";  /* UNEQ */
        if (a != b) return "une";  /* LTGT */
        if (islessequal(a, b)) return "ule";  /* UNLE */
        if (isgreaterequal(a, b)) return "uge";  /* UNGE */
        return "unordered";
    } else {
        /* Ordered comparisons */
        if (a < b) return "lt";
        if (a > b) return "gt";
        if (a == b) return "eq";
        if (a <= b) return "le";
        if (a >= b) return "ge";
        return "unknown";
    }
}

/* Inline assembly to force condition code output */
double inline_asm_fp_compare(double a, double b) {
    double result;
    int unordered_flag, greater_flag, equal_flag;
    
    /* Force x87 FPU comparison with unordered check */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack, now st(0)=a, st(1)=b */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clear FPU stack */
        "setp %3\n\t"           /* Set if unordered (parity flag) */
        "seta %4\n\t"           /* Set if above (greater) */
        "sete %5\n\t"           /* Set if equal */
        : "=m"(result), "=r"(unordered_flag), "=r"(greater_flag), "=r"(equal_flag)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* Another inline assembly with different condition codes */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        : /* No outputs */
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    return result;
}

/* Vector comparison function */
void vector_comparisons(void) {
    v2df vec_a = {1.0, 2.0};
    v2df vec_b = {NAN, 3.0};
    v2df vec_c = {INFINITY, -INFINITY};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp1 = vec_a < vec_b;   /* May generate UNORDERED/ORDERED conditions */
    v2df cmp2 = vec_a > vec_c;   /* May generate UNGT/UNLT conditions */
    v2df cmp3 = vec_a == vec_b;  /* May generate UNEQ conditions */
    
    /* Prevent dead code elimination */
    volatile v2df* volatile_ptr = &cmp1;
    (void)volatile_ptr;
}

/* Parse string to double, handling special values */
double parse_fp_value(const char* str) {
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
    /* Test cases covering various floating-point scenarios */
    double test_cases[][2] = {
        {NAN, 1.0},           /* UNORDERED comparisons */
        {1.0, NAN},           /* UNORDERED comparisons */
        {NAN, NAN},           /* Both NaN */
        {INFINITY, -INFINITY},/* Ordered extreme values */
        {0.0, -0.0},          /* Equal but signed zeros */
        {1.0, 2.0},           /* Normal ordered less */
        {2.0, 1.0},           /* Normal ordered greater */
        {1.0, 1.0},           /* Normal equal */
        {INFINITY, 1.0},      /* Infinity comparisons */
        {-INFINITY, 1.0},     /* Negative infinity */
        {NAN, INFINITY},      /* NaN with infinity */
        {INFINITY, NAN}       /* Infinity with NaN */
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command-line arguments if provided */
    if (argc >= 3) {
        double a = parse_fp_value(argv[1]);
        double b = parse_fp_value(argv[2]);
        
        printf("Testing with command-line values: %g, %g\n", a, b);
        
        /* Perform all types of comparisons */
        int classification = classify_comparison(a, b);
        const char* switch_result = compare_with_switch(a, b);
        
        printf("Classification: 0x%x\n", classification);
        printf("Switch result: %s\n", switch_result);
        
        /* Force inline assembly generation */
        double asm_result = inline_asm_fp_compare(a, b);
        (void)asm_result;
    }
    
    /* Run through all test cases */
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Mix of comparison types to trigger different condition codes */
        volatile int cmp_results[12];
        
        /* Standard comparisons */
        cmp_results[0] = (a < b);    /* May generate UNLT/ULT */
        cmp_results[1] = (a > b);    /* May generate UNGT/UGT */
        cmp_results[2] = (a == b);   /* May generate UNEQ/EQ */
        cmp_results[3] = (a != b);   /* May generate LTGT/NE */
        cmp_results[4] = (a <= b);   /* May generate UNLE/ULE */
        cmp_results[5] = (a >= b);   /* May generate UNGE/UGE */
        
        /* <math.h> macro comparisons */
        cmp_results[6] = isunordered(a, b);   /* UNORDERED */
        cmp_results[7] = isless(a, b);        /* Ordered less */
        cmp_results[8] = isgreater(a, b);     /* Ordered greater */
        cmp_results[9] = islessequal(a, b);   /* Ordered less or equal */
        cmp_results[10] = isgreaterequal(a, b); /* Ordered greater or equal */
        cmp_results[11] = islessgreater(a, b);  /* LTGT */
        
        /* Call classification function */
        int class_result = classify_comparison(a, b);
        (void)class_result;
        
        /* Call switch-based comparison */
        const char* switch_res = compare_with_switch(a, b);
        (void)switch_res;
    }
    
    /* Use vector extensions */
    vector_comparisons();
    
    /* More inline assembly variations */
    double x = global_nan;
    double y = 3.14159;
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "mov $0, %0\n\t"
        "mov $1, %%eax\n\t"
        "cmovp %%eax, %0\n\t"    /* Conditional move if unordered (parity) */
        : "=r"(cmov_result)
        : "m"(x), "m"(y)
        : "eax", "cc", "st"
    );
    
    /* Another with different condition */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "mov $0, %0\n\t"
        "mov $1, %%eax\n\t"
        "cmovb %%eax, %0\n\t"    /* Conditional move if below (less than) */
        : "=r"(cmov_result)
        : "m"(y), "m"(x)
        : "eax", "cc", "st"
    );
    
    printf("Test program completed. Compile with:\n");
    printf("  gcc -O2 -mfpmath=387 -march=i686 -S this_file.c\n");
    printf("  gcc -O3 -march=native -msse2 -mno-sse4 -c this_file.c\n");
    
    return 0;
}
