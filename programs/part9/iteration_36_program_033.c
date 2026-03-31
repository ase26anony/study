/* test_i386_cc.c - Program to trigger x86 unordered floating-point comparison condition code generation */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Force compiler to generate x87 FPU instructions */
#ifdef __GNUC__
__attribute__((optimize("no-fast-math")))
#endif

/* Helper function to classify comparison results */
static int classify_comparison(double a, double b) {
    int result = 0;
    
    /* Test all standard comparison operators with potential NaN */
    if (a < b) result |= 0x01;
    if (a > b) result |= 0x02;
    if (a <= b) result |= 0x04;
    if (a >= b) result |= 0x08;
    if (a == b) result |= 0x10;
    if (a != b) result |= 0x20;
    
    /* Test <math.h> comparison macros */
    if (isunordered(a, b)) result |= 0x40;
    if (isless(a, b)) result |= 0x80;
    if (isgreater(a, b)) result |= 0x100;
    if (islessequal(a, b)) result |= 0x200;
    if (isgreaterequal(a, b)) result |= 0x400;
    if (islessgreater(a, b)) result |= 0x800;
    
    return result;
}

/* Function with mixed ordered/unordered comparisons in control flow */
static const char* compare_detailed(double a, double b) {
    volatile double va = a;  /* Prevent constant folding */
    volatile double vb = b;
    
    /* Complex control flow to force multiple condition code generations */
    if (isunordered(va, vb)) {
        if (va != vb) {  /* Always true for NaN, but compiler might not know */
            return "unordered";
        }
    } else if (isless(va, vb)) {
        return "less";
    } else if (isgreater(va, vb)) {
        return "greater";
    } else if (va == vb) {
        /* Handle -0.0 == 0.0 case */
        if (signbit(va) != signbit(vb)) {
            return "equal_opposite_sign";
        }
        return "equal";
    } else if (islessgreater(va, vb)) {
        return "lessgreater";
    } else if (!isless(va, vb) && !isgreater(va, vb) && va != vb) {
        /* This should catch UNEQ case */
        return "uneq_candidate";
    }
    
    return "unknown";
}

/* Function using inline assembly to force condition code output */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* Prevent optimization of inputs */
    volatile double va = a;
    volatile double vb = b;
    
    /* Inline assembly using x87 FPU comparison */
    __asm__ volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack, a is now st(0), b is st(1) */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop st(0) */
        "fstp %%st(0)\n\t"      /* Pop remaining b from stack */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "sete %%dl\n\t"         /* Set if equal (ZF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $8, %%edx\n\t"
        "orl %%edx, %%eax\n\t"
        "movzbl %%dl, %%edx\n\t"
        "shl $16, %%edx\n\t"
        "orl %%edx, %%eax"
        : "=a" (result)
        : "m" (va), "m" (vb)
        : "cc", "st", "dl"
    );
    
    return result;
}

/* Vector comparison using GCC extensions */
#ifdef __SSE2__
static void vector_comparisons(void) {
    typedef double v2df __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    volatile v2df a = {NAN, 1.0};
    volatile v2df b = {2.0, NAN};
    volatile v2df c = {3.0, 4.0};
    
    /* These vector comparisons may generate condition codes */
    v2df cmp1 = a < b;   /* Should have unordered results */
    v2df cmp2 = a > b;
    v2df cmp3 = a == b;
    v2df cmp4 = a != b;
    
    /* Use results to prevent dead code elimination */
    v2di *p1 = (v2di*)&cmp1;
    v2di *p2 = (v2di*)&cmp2;
    v2di *p3 = (v2di*)&cmp3;
    v2di *p4 = (v2di*)&cmp4;
    
    printf("Vector cmp1: %016llx %016llx\n", (*p1)[0], (*p1)[1]);
    printf("Vector cmp2: %016llx %016llx\n", (*p2)[0], (*p2)[1]);
    printf("Vector cmp3: %016llx %016llx\n", (*p3)[0], (*p3)[1]);
    printf("Vector cmp4: %016llx %016llx\n", (*p4)[0], (*p4)[1]);
}
#endif

/* Parse double with NaN support */
static double parse_double(const char *str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NaN") == 0) {
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
    struct test_case {
        double a;
        double b;
        const char *desc;
    } tests[] = {
        {NAN, 1.0, "NaN vs 1.0"},
        {1.0, NAN, "1.0 vs NaN"},
        {NAN, NAN, "NaN vs NaN"},
        {INFINITY, -INFINITY, "INF vs -INF"},
        {0.0, -0.0, "0.0 vs -0.0"},
        {1.0, 2.0, "1.0 vs 2.0"},
        {2.0, 1.0, "2.0 vs 1.0"},
        {DBL_MAX, DBL_MAX, "DBL_MAX vs DBL_MAX"},
        {-DBL_MAX, DBL_MAX, "-DBL_MAX vs DBL_MAX"},
        {0.0, NAN, "0.0 vs NaN"},
        {INFINITY, NAN, "INF vs NaN"},
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    volatile int total_results = 0;  /* Prevent optimization */
    
    printf("Testing floating-point comparisons for condition code coverage...\n");
    
    /* Test 1: Standard comparisons with control flow */
    for (int i = 0; i < num_tests; i++) {
        const char *result = compare_detailed(tests[i].a, tests[i].b);
        int classification = classify_comparison(tests[i].a, tests[i].b);
        total_results += classification;
        
        printf("Test %d (%s): %s (classification: 0x%x)\n", 
               i, tests[i].desc, result, classification);
    }
    
    /* Test 2: Inline assembly comparisons */
    printf("\nInline assembly comparison results:\n");
    for (int i = 0; i < num_tests; i++) {
        int asm_result = fp_compare_asm(tests[i].a, tests[i].b);
        printf("  %s: 0x%06x\n", tests[i].desc, asm_result);
        total_results += asm_result;
    }
    
    /* Test 3: Use command line arguments if provided */
    if (argc >= 3) {
        double arg1 = parse_double(argv[1]);
        double arg2 = parse_double(argv[2]);
        
        printf("\nCommand line test (%s vs %s):\n", argv[1], argv[2]);
        
        /* Force all possible comparison types */
        volatile int cmp_results = 0;
        if (arg1 < arg2) cmp_results |= 1;
        if (arg1 > arg2) cmp_results |= 2;
        if (arg1 <= arg2) cmp_results |= 4;
        if (arg1 >= arg2) cmp_results |= 8;
        if (arg1 == arg2) cmp_results |= 16;
        if (arg1 != arg2) cmp_results |= 32;
        
        /* Use math.h macros */
        if (isunordered(arg1, arg2)) cmp_results |= 64;
        if (isless(arg1, arg2)) cmp_results |= 128;
        if (isgreater(arg1, arg2)) cmp_results |= 256;
        if (islessequal(arg1, arg2)) cmp_results |= 512;
        if (isgreaterequal(arg1, arg2)) cmp_results |= 1024;
        if (islessgreater(arg1, arg2)) cmp_results |= 2048;
        
        printf("  Combined results: 0x%x\n", cmp_results);
        total_results += cmp_results;
    }
    
#ifdef __SSE2__
    /* Test 4: Vector comparisons */
    printf("\nVector comparisons:\n");
    vector_comparisons();
#endif
    
    /* Final output to prevent dead code elimination */
    printf("\nTotal results accumulator: %d\n", total_results);
    
    return total_results != 0 ? 0 : 1;
}
