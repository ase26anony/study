#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function that performs various floating-point comparisons */
static int compare_doubles(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map to x86 unordered comparison predicates */
    if (isunordered(a, b)) result |= 64;
    if (isless(a, b)) result |= 128;
    if (isgreater(a, b)) result |= 256;
    if (islessequal(a, b)) result |= 512;
    if (isgreaterequal(a, b)) result |= 1024;
    if (islessgreater(a, b)) result |= 2048;
    
    return result;
}

/* Function with inline assembly to directly generate condition codes */
static int inline_asm_fp_compare(double a, double b) {
    int result = 0;
    
    /* Using x87 floating-point comparison with inline assembly
       This should trigger the condition code output routines */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack, now st(0)=a, st(1)=b */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clean up FPU stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%bl\n\t"         /* Set if below (CF=1) */
        "sete %%cl\n\t"         /* Set if equal (ZF=1) */
        "movzbl %%al, %0\n\t"
        "shll $6, %0\n\t"       /* Shift to bit 6 for unordered */
        "movzbl %%bl, %%eax\n\t"
        "orl %%eax, %0\n\t"     /* OR in below result */
        "movzbl %%cl, %%eax\n\t"
        "shll $4, %%eax\n\t"
        "orl %%eax, %0"         /* OR in equal result */
        : "=r" (result)
        : "m" (a), "m" (b)
        : "eax", "ebx", "ecx", "cc", "st", "st(1)"
    );
    
    return result;
}

/* Vector extensions for SSE comparisons */
#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));

static void vector_fp_comparisons(void) {
    volatile v2df a = {1.0, NAN};
    volatile v2df b = {NAN, 2.0};
    volatile v2df c = {3.0, 4.0};
    volatile v2df d = {5.0, 6.0};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp1 = a < b;  /* Contains unordered results */
    v2df cmp2 = c <= d; /* Ordered comparisons */
    v2df cmp3 = a == b; /* Equality with NaN */
    
    /* Use the results to prevent optimization */
    volatile v2df* dummy = &cmp1;
    (void)dummy;
}
#endif

/* Complex comparison function that uses switch on comparison results */
static const char* classify_comparison(double a, double b) {
    /* Check for unordered first */
    if (isunordered(a, b)) {
        /* Further classification within unordered */
        if (isless(a, b)) return "UNORDERED_LESS";
        if (isgreater(a, b)) return "UNORDERED_GREATER";
        if (a == b) return "UNORDERED_EQUAL";  /* UNEQ case */
        if (!(a < b)) return "UNORDERED_NOT_LESS";  /* UNGE case */
        if (!(a <= b)) return "UNORDERED_NOT_LESS_EQUAL";  /* UNGT case */
        if (a <= b) return "UNORDERED_LESS_EQUAL";  /* UNLE case */
        if (a < b) return "UNORDERED_LESS_THAN";  /* UNLT case */
        if (islessgreater(a, b)) return "ORDERED_NOT_EQUAL";  /* LTGT case */
        return "UNORDERED";
    }
    
    /* Ordered comparisons */
    if (a < b) return "LESS";
    if (a > b) return "GREATER";
    if (a == b) return "EQUAL";
    
    return "UNKNOWN";
}

int main(int argc, char *argv[]) {
    /* Array of test cases including NaN, Infinity, and normal numbers */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, -INFINITY},
        {INFINITY, 1.0},
        {-INFINITY, 1.0},
        {0.0, -0.0},           /* +0 and -0 are equal but may trigger special cases */
        {1.0, 2.0},
        {2.0, 1.0},
        {1.0, 1.0},
        {__builtin_nan(""), 3.14},
        {3.14, __builtin_nan("")}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Parse command line arguments if provided */
    if (argc >= 3) {
        for (int i = 1; i + 1 < argc; i += 2) {
            double a, b;
            
            /* Handle "nan" string */
            if (strcmp(argv[i], "nan") == 0) a = NAN;
            else a = atof(argv[i]);
            
            if (strcmp(argv[i+1], "nan") == 0) b = NAN;
            else b = atof(argv[i+1]);
            
            test_cases[num_cases][0] = a;
            test_cases[num_cases][1] = b;
            num_cases++;
        }
    }
    
    printf("Testing floating-point comparisons...\n");
    
    /* Perform comparisons using different methods */
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        printf("\nTest case %d: a=%g, b=%g\n", i, a, b);
        
        /* Method 1: Standard comparisons */
        int result1 = compare_doubles(a, b);
        printf("  compare_doubles result: 0x%x\n", result1);
        
        /* Method 2: Inline assembly */
        int result2 = inline_asm_fp_compare(a, b);
        printf("  inline_asm result: 0x%x\n", result2);
        
        /* Method 3: Classification */
        const char* classification = classify_comparison(a, b);
        printf("  classification: %s\n", classification);
        
        /* Force generation of specific condition codes by using them in control flow */
        volatile int dummy = 0;
        if (isunordered(a, b)) dummy |= 1;
        if (!(a < b)) dummy |= 2;      /* May generate UNGE */
        if (!(a <= b)) dummy |= 4;     /* May generate UNGT */
        if (a <= b) dummy |= 8;        /* May generate UNLE */
        if (a < b) dummy |= 16;        /* May generate UNLT */
        if (islessgreater(a, b)) dummy |= 32;  /* May generate LTGT */
        
        /* Use the dummy variable to prevent optimization */
        printf("  control flow result: %d\n", dummy);
    }
    
#ifdef __SSE2__
    /* Vector comparisons */
    printf("\nPerforming vector comparisons...\n");
    vector_fp_comparisons();
#endif
    
    /* Additional tests to ensure all condition codes are generated */
    printf("\nSpecialized tests for each condition code:\n");
    
    /* Generate UNORDERED */
    {
        volatile double nan_val = NAN;
        volatile double normal = 1.0;
        if (isunordered(nan_val, normal)) {
            printf("  UNORDERED triggered\n");
        }
    }
    
    /* Generate UNEQ (unordered or equal) */
    {
        volatile double nan1 = NAN;
        volatile double nan2 = NAN;
        if (isunordered(nan1, nan2) || nan1 == nan2) {
            printf("  UNEQ-like condition triggered\n");
        }
    }
    
    /* Generate UNGE (not less than, includes unordered) */
    {
        volatile double nan_val = NAN;
        volatile double normal = 1.0;
        if (!(nan_val < normal)) {
            printf("  UNGE-like condition triggered\n");
        }
    }
    
    /* Generate UNGT (not less than or equal, includes unordered) */
    {
        volatile double nan_val = NAN;
        volatile double normal = 1.0;
        if (!(nan_val <= normal)) {
            printf("  UNGT-like condition triggered\n");
        }
    }
    
    /* Generate UNLE (less than or equal, includes unordered) */
    {
        volatile double nan_val = NAN;
        volatile double normal = 1.0;
        if (nan_val <= normal) {
            printf("  UNLE-like condition triggered\n");
        }
    }
    
    /* Generate UNLT (less than, includes unordered) */
    {
        volatile double nan_val = NAN;
        volatile double normal = 1.0;
        if (nan_val < normal) {
            printf("  UNLT-like condition triggered\n");
        }
    }
    
    /* Generate LTGT (less than or greater than, ordered not equal) */
    {
        volatile double a = 1.0;
        volatile double b = 2.0;
        if (islessgreater(a, b)) {
            printf("  LTGT-like condition triggered\n");
        }
    }
    
    printf("\nTest completed.\n");
    
    return 0;
}
