/* test_i386_cc.c - Program to trigger x86 unordered floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Function to parse NaN from command line */
static double parse_double_special(const char *str) {
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

/* Helper function that performs all possible unordered comparisons */
static int compare_all_unordered(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - these may generate unordered conditions */
    if (a < b) result |= 1;      /* Less than */
    if (a > b) result |= 2;      /* Greater than */
    if (a <= b) result |= 4;     /* Less than or equal */
    if (a >= b) result |= 8;     /* Greater than or equal */
    if (a == b) result |= 16;    /* Equal */
    if (a != b) result |= 32;    /* Not equal */
    
    /* <math.h> macros that map directly to x86 unordered predicates */
    if (isunordered(a, b)) result |= 64;      /* UNORDERED */
    if (isless(a, b)) result |= 128;          /* ORDERED + LT */
    if (isgreater(a, b)) result |= 256;       /* ORDERED + GT */
    if (islessequal(a, b)) result |= 512;     /* ORDERED + LE */
    if (isgreaterequal(a, b)) result |= 1024; /* ORDERED + GE */
    
    /* Additional unordered comparisons */
    if (!isgreater(a, b) && !isless(a, b) && !isunordered(a, b)) 
        result |= 2048;  /* UNEQ - unordered or equal */
    
    return result;
}

/* Function with switch based on comparison results - forces multiple condition codes */
static const char* classify_comparison(double a, double b) {
    /* Use volatile to prevent optimization */
    volatile int cmp_result = 0;
    
    /* Perform comparisons that may generate different condition codes */
    if (isunordered(a, b)) {
        cmp_result = 1;  /* UNORDERED */
    } else if (a < b) {
        cmp_result = 2;  /* LT */
    } else if (a > b) {
        cmp_result = 3;  /* GT */
    } else if (a == b) {
        cmp_result = 4;  /* EQ */
    }
    
    /* Additional comparisons for less common condition codes */
    if (!(a < b) && !isunordered(a, b)) {
        cmp_result |= 8;  /* NLT (UNGE) */
    }
    if (!(a <= b) && !isunordered(a, b)) {
        cmp_result |= 16; /* NLE (UNGT) */
    }
    if ((a <= b) || isunordered(a, b)) {
        cmp_result |= 32; /* ULE */
    }
    if ((a < b) || isunordered(a, b)) {
        cmp_result |= 64; /* ULT */
    }
    if ((a != b) && !isunordered(a, b)) {
        cmp_result |= 128; /* LTGT (UNE) */
    }
    
    switch (cmp_result & 0xF) {
        case 1: return "UNORDERED";
        case 2: return "LT";
        case 3: return "GT";
        case 4: return "EQ";
        case 8: return "UNGE/NLT";
        case 9: return "UNORDERED|NLT";
        default: return "MIXED";
    }
}

/* Inline assembly that directly uses floating-point condition codes */
static int inline_asm_fp_compare(double a, double b) {
    int result;
    
    /* Method 1: Using fucomip which sets condition codes including parity for unordered */
    asm volatile (
        "fucomip %%st(1), %%st\n\t"
        "setp %%al\n\t"
        "setb %%ah\n\t"
        "sete %%dl\n\t"
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $1, %%edx\n\t"
        "orl %%edx, %%eax"
        : "=a"(result)
        : "t"(a), "u"(b)
        : "cc", "st", "dl"
    );
    
    /* Method 2: Conditional move based on floating comparison */
    double cmov_result;
    asm volatile (
        "comisd %2, %1\n\t"
        "movsd %1, %0\n\t"
        "jp 1f\n\t"          /* Jump if unordered (parity set) */
        "ja 2f\n\t"          /* Jump if above (ordered & greater) */
        "jb 3f\n\t"          /* Jump if below (ordered & less) */
        "movsd %3, %0\n\t"   /* Equal case */
        "jmp 4f\n\t"
        "1: movsd %4, %0\n\t" /* Unordered case */
        "jmp 4f\n\t"
        "2: movsd %5, %0\n\t" /* Greater case */
        "jmp 4f\n\t"
        "3: movsd %6, %0\n\t" /* Less case */
        "4:"
        : "=x"(cmov_result)
        : "x"(a), "x"(b), 
          "x"(0.0), "x"(1.0), "x"(2.0), "x"(3.0)
        : "cc"
    );
    
    return result | ((int)cmov_result << 8);
}

/* Vector extensions for packed floating-point comparisons */
#ifdef __SSE2__
static void vector_comparisons(void) {
    typedef double v2df __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    volatile v2df a = { NAN, 1.0 };
    volatile v2df b = { 1.0, NAN };
    volatile v2df c = { 2.0, 3.0 };
    volatile v2df d = { 1.0, 4.0 };
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp1 = a < b;  /* Should have unordered results */
    v2df cmp2 = c > d;
    v2df cmp3 = a == b;
    
    /* Use the results to prevent dead code elimination */
    v2di *as_int = (v2di*)&cmp1;
    printf("Vector cmp1: 0x%016llx 0x%016llx\n", 
           (unsigned long long)(*as_int)[0],
           (unsigned long long)(*as_int)[1]);
}
#endif

int main(int argc, char *argv[]) {
    /* Array of test cases designed to trigger various condition codes */
    struct test_case {
        double a;
        double b;
        const char *desc;
    } test_cases[] = {
        { NAN, 1.0, "NAN vs 1.0" },
        { 1.0, NAN, "1.0 vs NAN" },
        { NAN, NAN, "NAN vs NAN" },
        { INFINITY, -INFINITY, "INF vs -INF" },
        { 0.0, -0.0, "0.0 vs -0.0" },
        { 1.0, 2.0, "1.0 vs 2.0" },
        { 2.0, 1.0, "2.0 vs 1.0" },
        { 3.14, 3.14, "3.14 vs 3.14" },
        { INFINITY, INFINITY, "INF vs INF" },
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double_special(argv[1]);
        double b = parse_double_special(argv[2]);
        
        printf("Testing with command line values: %s vs %s\n", argv[1], argv[2]);
        
        /* Force all comparison types */
        int result = compare_all_unordered(a, b);
        const char *classification = classify_comparison(a, b);
        int asm_result = inline_asm_fp_compare(a, b);
        
        printf("Result: 0x%x, Classification: %s, ASM: 0x%x\n", 
               result, classification, asm_result);
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
    for (int i = 0; i < num_tests; i++) {
        double a = test_cases[i].a;
        double b = test_cases[i].b;
        
        /* Mix of comparison methods to trigger different code paths */
        int cmp_result = compare_all_unordered(a, b);
        const char *class_str = classify_comparison(a, b);
        
        /* Use inline assembly for some test cases */
        if (i % 3 == 0) {
            int asm_res = inline_asm_fp_compare(a, b);
            cmp_result ^= asm_res; /* Use result to prevent elimination */
        }
        
        printf("Test %d (%s): result=0x%04x, class=%s\n", 
               i, test_cases[i].desc, cmp_result, class_str);
    }
    
#ifdef __SSE2__
    printf("\nPerforming vector comparisons:\n");
    vector_comparisons();
#endif
    
    /* Additional complex control flow to force generation of condition codes */
    printf("\nComplex control flow test:\n");
    volatile double x = global_nan;
    volatile double y = 0.0;
    
    for (int i = 0; i < 10; i++) {
        y = i * 0.1;
        
        /* This complex if-else chain should generate multiple condition codes */
        if (isunordered(x, y)) {
            printf("unordered ");
        } else if (x < y) {
            printf("less ");
        } else if (x > y) {
            printf("greater ");
        } else if (x == y) {
            printf("equal ");
        }
        
        /* More conditions for less common codes */
        if (!(x >= y) || isunordered(x, y)) {
            printf("[UNLT/ULT] ");
        }
        if (!(x > y) || isunordered(x, y)) {
            printf("[UNLE/ULE] ");
        }
        if ((x != y) && !isunordered(x, y)) {
            printf("[LTGT/UNE] ");
        }
        printf("\n");
        
        /* Alternate x between NaN and normal values */
        x = (i % 2 == 0) ? global_nan : (double)i;
    }
    
    return 0;
}
