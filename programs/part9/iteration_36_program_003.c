/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector type for SSE comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function that performs all possible unordered comparisons */
static int __attribute__((noinline)) 
perform_unordered_comparisons(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map to x86 unordered predicates */
    if (isunordered(a, b)) result |= 64;
    if (isless(a, b)) result |= 128;
    if (isgreater(a, b)) result |= 256;
    if (islessequal(a, b)) result |= 512;
    if (isgreaterequal(a, b)) result |= 1024;
    if (islessgreater(a, b)) result |= 2048;
    
    return result;
}

/* Function with switch based on comparison results */
static const char* __attribute__((noinline))
classify_comparison(double a, double b) {
    /* This complex logic forces generation of multiple condition codes */
    if (isunordered(a, b)) {
        return "UNORDERED";
    }
    
    if (a == b) {
        if (signbit(a) != signbit(b)) {
            return "EQ but different signs";
        }
        return "ORDERED_EQ";
    }
    
    if (a < b) {
        if (isless(a, b)) {
            return "ORDERED_LT";
        }
        return "LT_UNORDERED?";
    }
    
    if (a > b) {
        if (isgreater(a, b)) {
            return "ORDERED_GT";
        }
        return "GT_UNORDERED?";
    }
    
    /* Special cases that might generate UNEQ, LTGT, etc. */
    if (!isless(a, b) && !isgreater(a, b) && !isunordered(a, b)) {
        return "UNEQ_CANDIDATE";
    }
    
    return "UNKNOWN";
}

/* Inline assembly to directly trigger condition code output */
static int __attribute__((noinline))
inline_asm_fpu_comparison(double a, double b) {
    int result;
    
    /* Force use of x87 FPU with unordered comparisons */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Clean up stack */
        
        /* Test for UNORDERED (parity flag) */
        "setp %%al\n\t"
        "movzbl %%al, %0\n\t"
        : "=r" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)", "al"
    );
    
    return result;
}

/* More complex inline assembly with multiple condition codes */
static void __attribute__((noinline))
complex_asm_conditions(double a, double b, int* results) {
    /* This should generate various condition code outputs */
    asm volatile (
        "fldl %[b]\n\t"
        "fldl %[a]\n\t"
        "fucomip %%st(1), %%st\n\t"
        /* Generate multiple conditional moves based on flags */
        "movl $0, %[r0]\n\t"
        "movl $0, %[r1]\n\t"
        "movl $0, %[r2]\n\t"
        "movl $0, %[r3]\n\t"
        
        /* UNORDERED case */
        "jp 1f\n\t"
        "movl $1, %[r0]\n\t"
        "1:\n\t"
        
        /* ORDERED cases */
        "ja 2f\n\t"      /* above: st(0) > st(1) */
        "movl $1, %[r1]\n\t"
        "2:\n\t"
        
        "jb 3f\n\t"      /* below: st(0) < st(1) */
        "movl $1, %[r2]\n\t"
        "3:\n\t"
        
        "je 4f\n\t"      /* equal: st(0) == st(1) */
        "movl $1, %[r3]\n\t"
        "4:\n\t"
        
        "fstp %%st(0)\n\t"  /* Clean up */
        : [r0] "=m" (results[0]),
          [r1] "=m" (results[1]),
          [r2] "=m" (results[2]),
          [r3] "=m" (results[3])
        : [a] "m" (a),
          [b] "m" (b)
        : "cc", "st", "st(1)", "eax"
    );
}

/* Vector comparison function */
static void __attribute__((noinline))
vector_comparisons(void) {
    v2df vec1, vec2, cmp_result;
    v4sf vecf1, vecf2, cmp_resultf;
    
    /* Initialize with mixed values including NaN */
    vec1 = (v2df){1.0, NAN};
    vec2 = (v2df){NAN, 2.0};
    
    /* These comparisons may generate unordered conditions */
    cmp_result = vec1 < vec2;
    cmp_result = vec1 > vec2;
    cmp_result = vec1 <= vec2;
    cmp_result = vec1 >= vec2;
    
    /* Float vector comparisons */
    vecf1 = (v4sf){1.0f, NAN, -INFINITY, 0.0f};
    vecf2 = (v4sf){NAN, 2.0f, INFINITY, -0.0f};
    
    cmp_resultf = vecf1 < vecf2;
    cmp_resultf = vecf1 == vecf2;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "x"(cmp_result), "x"(cmp_resultf) : "memory");
}

/* Parse double with NaN support */
static double parse_double(const char* str) {
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

int main(int argc, char** argv) {
    double test_values[10];
    int i, j;
    
    /* Initialize test values from command line or defaults */
    if (argc > 1) {
        for (i = 0; i < argc - 1 && i < 10; i++) {
            test_values[i] = parse_double(argv[i + 1]);
        }
    } else {
        /* Default test cases covering various scenarios */
        test_values[0] = NAN;
        test_values[1] = 1.0;
        test_values[2] = -1.0;
        test_values[3] = INFINITY;
        test_values[4] = -INFINITY;
        test_values[5] = 0.0;
        test_values[6] = -0.0;
        test_values[7] = DBL_MAX;
        test_values[8] = DBL_MIN;
        test_values[9] = NAN;
    }
    
    printf("Testing floating-point comparisons for x86 condition code coverage\n");
    
    /* Perform comparisons between all pairs */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            volatile double a = test_values[i];
            volatile double b = test_values[j];
            
            /* Force compiler to generate comparison code */
            int cmp_result = perform_unordered_comparisons(a, b);
            const char* classification = classify_comparison(a, b);
            
            /* Use inline assembly comparisons */
            int asm_result = inline_asm_fpu_comparison(a, b);
            
            int complex_results[4];
            complex_asm_conditions(a, b, complex_results);
            
            /* Prevent dead code elimination */
            if (cmp_result != 0 || asm_result != 0) {
                printf("(%d,%d): cmp=%d, class=%s, asm=%d\n", 
                       i, j, cmp_result, classification, asm_result);
            }
        }
    }
    
    /* Test vector comparisons */
    vector_comparisons();
    
    /* Additional tests with special bit patterns */
    {
        /* Create signaling NaN if possible */
        volatile double signaling_nan = strtod("NAN(s)", NULL);
        
        /* Test with different NaN representations */
        union {
            double d;
            unsigned long long ull;
        } u;
        
        u.ull = 0x7FF0000000000001ULL; /* quiet NaN */
        volatile double qnan = u.d;
        
        u.ull = 0x7FF0000000000001ULL | (1ULL << 51); /* different NaN */
        volatile double qnan2 = u.d;
        
        /* These should generate various condition codes */
        int r1 = perform_unordered_comparisons(qnan, qnan2);
        int r2 = perform_unordered_comparisons(signaling_nan, 1.0);
        
        printf("NaN comparisons: %d %d\n", r1, r2);
    }
    
    /* Loop with volatile to prevent optimization */
    volatile int final_result = 0;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            final_result += perform_unordered_comparisons(test_values[i], test_values[j]);
        }
    }
    
    printf("Final accumulated result: %d\n", final_result);
    
    return 0;
}
