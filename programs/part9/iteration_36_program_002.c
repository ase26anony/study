/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector type for SSE comparisons */
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
    /* Force actual comparison by using volatile */
    volatile double va = a;
    volatile double vb = b;
    
    if (isunordered(va, vb)) {
        /* Further classify unordered cases */
        if (isless(va, vb)) return "UNORDERED_LESS";  /* Shouldn't happen */
        if (isgreater(va, vb)) return "UNORDERED_GREATER";  /* Shouldn't happen */
        return "UNORDERED";
    }
    
    if (va < vb) return "LESS";
    if (va > vb) return "GREATER";
    if (va == vb) return "EQUAL";
    
    /* Special cases that might generate UNEQ, UNGE, UNGT, UNLE, UNLT */
    if (!(va < vb) && !(va > vb) && !isunordered(va, vb)) return "UNEQ?";
    if (!(va < vb) && !isunordered(va, vb)) return "UNGE?";
    if (!(va <= vb) && !isunordered(va, vb)) return "UNGT?";
    if (!(va > vb) && !isunordered(va, vb)) return "UNLE?";
    if (!(va >= vb) && !isunordered(va, vb)) return "UNLT?";
    
    return "UNKNOWN";
}

/* Inline assembly that forces condition code output */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto x87 stack */
        "fldl %1\n\t"           /* Load a onto x87 stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clear stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %%eax\n\t"
        "shl $8, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "or %%edx, %%eax\n\t"
        : "=a" (result)
        : "m" (a), "m" (b)
        : "cc", "st"
    );
    
    return result;
}

/* Vector comparisons */
static v2df vector_compare(v2df a, v2df b) {
    /* These may generate multiple comparison instructions */
    v2df cmp_lt = a < b;
    v2df cmp_gt = a > b;
    v2df cmp_eq = a == b;
    v2df cmp_ne = a != b;
    v2df cmp_le = a <= b;
    v2df cmp_ge = a >= b;
    
    /* Combine results */
    return cmp_lt + cmp_gt * 2 + cmp_eq * 4 + cmp_ne * 8 + cmp_le * 16 + cmp_ge * 32;
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
        {DBL_MIN, DBL_MAX, "MIN vs MAX"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {1.0, 1.0, "1.0 vs 1.0"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Testing with command line values: %g, %g\n", a, b);
        
        /* Force all comparison types */
        int cmp_result = compare_all_conditions(a, b);
        const char *classification = classify_comparison(a, b);
        int asm_result = fp_compare_asm(a, b);
        
        printf("Comparison result: 0x%x\n", cmp_result);
        printf("Classification: %s\n", classification);
        printf("Assembly result: 0x%x\n", asm_result);
        
        /* Vector comparison */
        v2df va = {a, a};
        v2df vb = {b, b};
        v2df vresult = vector_compare(va, vb);
        printf("Vector result: [%g, %g]\n", vresult[0], vresult[1]);
    } else {
        /* Run all test cases */
        printf("Running comprehensive test suite...\n");
        
        for (int i = 0; i < num_cases; i++) {
            double a = test_cases[i].a;
            double b = test_cases[i].b;
            
            /* Prevent optimization */
            volatile double va = a;
            volatile double vb = b;
            
            printf("\nTest %d: %s\n", i + 1, test_cases[i].desc);
            
            /* Perform comparisons that should generate various condition codes */
            int cmp_result = compare_all_conditions(va, vb);
            const char *classification = classify_comparison(va, vb);
            
            printf("  Comparison mask: 0x%03x\n", cmp_result);
            printf("  Classification: %s\n", classification);
            
            /* Use inline assembly for some cases to force specific codegen */
            if (i % 3 == 0) {
                int asm_result = fp_compare_asm(va, vb);
                printf("  Assembly compare: 0x%x\n", asm_result);
            }
        }
        
        /* Test vector operations */
        printf("\nTesting vector comparisons...\n");
        v2df vec1 = {1.0, NAN};
        v2df vec2 = {NAN, 2.0};
        v2df vec_result = vector_compare(vec1, vec2);
        
        /* Use result to prevent dead code elimination */
        volatile double dummy = vec_result[0] + vec_result[1];
        printf("Vector dummy result: %g\n", dummy);
    }
    
    /* Complex control flow with mixed comparisons */
    printf("\nComplex control flow test...\n");
    double test_vals[] = {NAN, INFINITY, -INFINITY, 0.0, 1.0, -1.0};
    int num_vals = sizeof(test_vals) / sizeof(test_vals[0]);
    
    int total = 0;
    for (int i = 0; i < num_vals; i++) {
        for (int j = 0; j < num_vals; j++) {
            volatile double a = test_vals[i];
            volatile double b = test_vals[j];
            
            /* Switch-like behavior using comparisons */
            if (isunordered(a, b)) {
                total += 1;  /* UNORDERED */
            } else if (a < b) {
                total += 2;  /* LESS */
            } else if (a > b) {
                total += 3;  /* GREATER */
            } else if (a == b) {
                total += 4;  /* EQUAL */
            } else {
                /* Should catch UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT */
                total += 5;
            }
            
            /* Additional comparisons that might generate rare conditions */
            if (!(a < b) && !isunordered(a, b)) total += 10;  /* UNGE */
            if (!(a <= b) && !isunordered(a, b)) total += 20; /* UNGT */
            if (!(a > b) && !isunordered(a, b)) total += 30;  /* UNLE */
            if (!(a >= b) && !isunordered(a, b)) total += 40; /* UNLT */
            if (islessgreater(a, b)) total += 50;  /* LTGT */
        }
    }
    
    printf("Total from complex flow: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
