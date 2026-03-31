#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Force compiler to generate x87 FPU instructions */
#ifdef __GNUC__
__attribute__((optimize("no-fast-math")))
#endif
static int classify_comparison(double a, double b) {
    int result = 0;
    
    /* Use all possible comparison operators */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* Use math.h macros that map to x86 condition codes */
    if (isunordered(a, b)) result |= 64;
    if (isless(a, b)) result |= 128;
    if (isgreater(a, b)) result |= 256;
    if (islessequal(a, b)) result |= 512;
    if (isgreaterequal(a, b)) result |= 1024;
    if (islessgreater(a, b)) result |= 2048;
    
    return result;
}

/* Function with inline assembly to force condition code output */
#ifdef __GNUC__
__attribute__((noinline))
static double fp_compare_asm(double a, double b) {
    volatile int unordered_result = 0;
    volatile int ordered_result = 0;
    volatile int eq_result = 0;
    volatile int ltgt_result = 0;
    
    /* Force x87 floating point comparison with unordered check */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "movzbl %%al, %0\n\t"   /* Store unordered result */
        : "=r" (unordered_result)
        : "m" (a), "m" (b)
        : "al", "cc"
    );
    
    /* Another comparison for ordered result */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "setnp %%al\n\t"        /* Set if ordered (no parity) */
        "movzbl %%al, %0\n\t"
        : "=r" (ordered_result)
        : "m" (a), "m" (b)
        : "al", "cc"
    );
    
    /* Comparison for equality/unordered equal */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "sete %%al\n\t"         /* Set if equal */
        "movzbl %%al, %0\n\t"
        : "=r" (eq_result)
        : "m" (a), "m" (b)
        : "al", "cc"
    );
    
    /* Comparison for less/greater (LTGT) */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "seta %%al\n\t"         /* Set if above (greater) */
        "movzbl %%al, %0\n\t"
        : "=r" (ltgt_result)
        : "m" (a), "m" (b)
        : "al", "cc"
    );
    
    /* Prevent dead code elimination */
    return (double)(unordered_result + ordered_result + eq_result + ltgt_result);
}
#endif

/* Use vector extensions to generate packed comparisons */
#ifdef __GNUC__
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

__attribute__((noinline))
static v2di vector_fp_compare(v2df a, v2df b) {
    /* Generate multiple vector comparisons */
    v2df cmp_lt = a < b;
    v2df cmp_gt = a > b;
    v2df cmp_eq = a == b;
    v2df cmp_ne = a != b;
    v2df cmp_le = a <= b;
    v2df cmp_ge = a >= b;
    
    /* Convert to integer masks */
    v2di mask_lt = (v2di)cmp_lt;
    v2di mask_gt = (v2di)cmp_gt;
    v2di mask_eq = (v2di)cmp_eq;
    v2di mask_ne = (v2di)cmp_ne;
    v2di mask_le = (v2di)cmp_le;
    v2di mask_ge = (v2di)cmp_ge;
    
    return mask_lt + mask_gt + mask_eq + mask_ne + mask_le + mask_ge;
}
#endif

/* Complex switch-based comparison function */
#ifdef __GNUC__
__attribute__((noinline))
static const char* fp_condition_name(double a, double b) {
    /* Force actual comparison instructions */
    volatile double va = a;
    volatile double vb = b;
    
    if (isunordered(va, vb)) {
        return "UNORDERED";
    }
    if (!isunordered(va, vb)) {
        if (va < vb) return "LT";
        if (va > vb) return "GT";
        if (va == vb) return "EQ";
    }
    
    /* More complex conditions */
    if (isless(va, vb)) return "LT (ordered)";
    if (isgreater(va, vb)) return "GT (ordered)";
    if (islessequal(va, vb)) return "LE (ordered)";
    if (isgreaterequal(va, vb)) return "GE (ordered)";
    if (islessgreater(va, vb)) return "LTGT";
    
    /* Unordered specific comparisons */
    if (!isless(va, vb) && !isgreater(va, vb) && isunordered(va, vb)) 
        return "UNEQ";
    if (!isless(va, vb) && isunordered(va, vb)) 
        return "UNGE";
    if (!islessequal(va, vb) && isunordered(va, vb)) 
        return "UNGT";
    if (islessequal(va, vb) || isunordered(va, vb)) 
        return "UNLE";
    if (isless(va, vb) || isunordered(va, vb)) 
        return "UNLT";
    
    return "UNKNOWN";
}
#endif

/* Parse NaN from command line */
static double parse_fp_value(const char* str) {
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

int main(int argc, char* argv[]) {
    /* Prevent constant folding */
    volatile double test_values[8];
    volatile int i;
    
    /* Initialize test cases with NaN values */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, -INFINITY},
        {0.0, -0.0},
        {DBL_MAX, DBL_MIN},
        {1.0, 2.0},
        {2.0, 1.0}
    };
    
    /* Parse from command line if provided */
    if (argc >= 3) {
        test_cases[0][0] = parse_fp_value(argv[1]);
        test_cases[0][1] = parse_fp_value(argv[2]);
    }
    
    printf("Testing floating-point comparisons (x87/x86 condition codes):\n");
    
    /* Perform comparisons using different methods */
    for (i = 0; i < 8; i++) {
        volatile double a = test_cases[i][0];
        volatile double b = test_cases[i][1];
        
        printf("\nTest case %d: a=%g, b=%g\n", i, a, b);
        
        /* Method 1: Standard C comparisons */
        int cmp_result = classify_comparison(a, b);
        printf("  Comparison mask: 0x%04x\n", cmp_result);
        
        /* Method 2: Inline assembly (GCC only) */
#ifdef __GNUC__
        double asm_result = fp_compare_asm(a, b);
        printf("  ASM result: %g\n", asm_result);
#endif
        
        /* Method 3: Condition name */
#ifdef __GNUC__
        const char* cond_name = fp_condition_name(a, b);
        printf("  Condition: %s\n", cond_name);
#endif
        
        /* Method 4: Vector comparisons (GCC only) */
#ifdef __GNUC__
        v2df vec_a = {a, b};
        v2df vec_b = {b, a};
        v2di vec_result = vector_fp_compare(vec_a, vec_b);
        long long* res_ptr = (long long*)&vec_result;
        printf("  Vector result: [0x%016llx, 0x%016llx]\n", 
               res_ptr[0], res_ptr[1]);
#endif
        
        /* Force side effects to prevent optimization */
        test_values[i] = a + b + cmp_result;
    }
    
    /* Final summary to prevent dead code elimination */
    volatile double final_sum = 0.0;
    for (i = 0; i < 8; i++) {
        final_sum += test_values[i];
    }
    
    printf("\nFinal sum (anti-optimization): %g\n", final_sum);
    
    return 0;
}
