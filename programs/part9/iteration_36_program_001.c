/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double global_a, global_b;

/* Function to parse NaN from command line */
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

/* Helper function that performs all possible floating-point comparisons */
static int compare_all(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* math.h comparison macros that map to x86 condition codes */
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
        if (a == b) return "UNEQ";      /* Both NaN or unordered equal */
        if (!(a < b) && !(a > b)) return "UNORDERED";
        if (!(a < b)) return "UNGE";    /* Not less than (unordered) */
        if (!(a > b)) return "UNLE";    /* Not greater than (unordered) */
        if (a != b) return "LTGT";      /* Less or greater, but not equal */
        return "UNKNOWN_UNORDERED";
    } else {
        if (a < b) return "LT";
        if (a > b) return "GT";
        if (a == b) return "EQ";
        return "ORDERED";
    }
}

/* Vector extensions for SSE/AVX comparisons */
#ifdef USE_VECTOR
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

static void vector_comparisons(void) {
    volatile v2df va = {NAN, 1.0};
    volatile v2df vb = {2.0, NAN};
    volatile v2df vc;
    
    /* These vector comparisons may generate multiple condition codes */
    vc = va < vb;
    vc = va > vb;
    vc = va <= vb;
    vc = va >= vb;
    vc = va == vb;
    vc = va != vb;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "m"(vc));
}
#endif

/* Inline assembly to directly trigger condition code output */
static void inline_asm_fpu_comparisons(double a, double b) {
    int result_unordered, result_ordered, result_ltgt;
    
    /* Force use of x87 FPU with unordered comparison */
    asm volatile(
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "setp %0\n\t"           /* Set if unordered (parity flag) */
        : "=r"(result_unordered)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* Ordered comparison with different condition codes */
    asm volatile(
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "setb %0\n\t"           /* Set if below (CF=1) */
        : "=r"(result_ordered)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* LTGT condition (less or greater, but not equal/unordered) */
    asm volatile(
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "setne %0\n\t"          /* Set if not equal (ZF=0) */
        : "=r"(result_ltgt)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* Prevent optimization */
    asm volatile("" : : "r"(result_unordered), "r"(result_ordered), "r"(result_ltgt));
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Test cases designed to trigger various condition codes */
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
        {0.0, -0.0, "0.0 vs -0.0"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        test_cases[0].a = parse_double(argv[1]);
        test_cases[0].b = parse_double(argv[2]);
        num_cases = 1;
    }
    
    printf("Testing floating-point condition code generation...\n");
    
    /* Perform all comparisons */
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i].a;
        double b = test_cases[i].b;
        
        printf("\nTest %d: %s\n", i, test_cases[i].desc);
        printf("  a = %g, b = %g\n", a, b);
        
        /* Force compiler to generate comparison code */
        volatile int cmp_result = compare_all(a, b);
        const char *classification = classify_comparison(a, b);
        
        printf("  Classification: %s\n", classification);
        printf("  Compare result: 0x%x\n", cmp_result);
        
        /* Use inline assembly to trigger condition code output */
        inline_asm_fpu_comparisons(a, b);
        
        /* Complex conditional to force multiple branch generation */
        volatile double temp = a;
        if (isunordered(a, b)) {
            if (a == b) {
                temp = 1.0;  /* UNEQ case */
            } else if (!(a < b) && !(a > b)) {
                temp = 2.0;  /* UNORDERED case */
            } else if (!(a < b)) {
                temp = 3.0;  /* UNGE case (nlt) */
            } else if (!(a > b)) {
                temp = 4.0;  /* UNLE case (ule) */
            } else if (a != b) {
                temp = 5.0;  /* LTGT case (une) */
            }
        } else {
            if (a < b) {
                temp = 6.0;  /* LT case */
            } else if (a > b) {
                temp = 7.0;  /* GT case */
            } else {
                temp = 8.0;  /* EQ case */
            }
        }
        
        /* Prevent dead code elimination */
        global_a = temp;
    }
    
#ifdef USE_VECTOR
    /* Vector comparisons if enabled */
    vector_comparisons();
#endif
    
    /* Final summary to prevent optimization */
    printf("\nCondition code test completed.\n");
    printf("Final values: global_a = %g, global_b = %g\n", global_a, global_b);
    
    return 0;
}
