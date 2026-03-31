/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_zero = 0.0;

/* Helper to parse NaN from command line */
static double parse_double(const char *str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NAN") == 0)
        return NAN;
    if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0)
        return INFINITY;
    if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0)
        return -INFINITY;
    return atof(str);
}

/* Function that performs all possible unordered comparisons */
static int compare_all_results(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons (may generate unordered conditions) */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros for unordered comparisons */
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
    if (isunordered(a, b)) return "unordered";
    if (isless(a, b)) return "less";
    if (isgreater(a, b)) return "greater";
    if (islessequal(a, b)) return "lessequal";
    if (isgreaterequal(a, b)) return "greaterequal";
    if (islessgreater(a, b)) return "lessgreater";
    if (a == b) return "equal";
    return "unknown";
}

/* Inline assembly to force condition code output */
static int fp_compare_asm(double a, double b) {
    int result;
    
    /* Using x87 floating-point compare and condition code */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "setp %%al\n\t"         /* Set if unordered (parity) */
        "setb %%ah\n\t"         /* Set if below (CF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $8, %%edx\n\t"
        "orl %%edx, %%eax\n\t"
        "fstp %%st(0)\n\t"      /* Clean up FPU stack */
        : "=a"(result)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    return result;
}

/* Another inline assembly variant with different condition codes */
static int fp_compare_asm2(double a, double b) {
    int result;
    
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "sete %%al\n\t"         /* Set if equal (ZF=1) */
        "setbe %%ah\n\t"        /* Set if below or equal (CF=1 or ZF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $8, %%edx\n\t"
        "orl %%edx, %%eax\n\t"
        "fstp %%st(0)\n\t"
        : "=a"(result)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    return result;
}

/* Vector extensions for SSE comparisons */
#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));

static v2df vector_compare(v2df a, v2df b) {
    /* These comparisons may generate unordered condition handling */
    v2df cmp_lt = a < b;
    v2df cmp_gt = a > b;
    v2df cmp_eq = a == b;
    v2df cmp_ne = a != b;
    v2df cmp_le = a <= b;
    v2df cmp_ge = a >= b;
    
    /* Combine results */
    return cmp_lt + cmp_gt * 2 + cmp_eq * 4 + cmp_ne * 8 + cmp_le * 16 + cmp_ge * 32;
}
#endif

/* Main test function */
int main(int argc, char *argv[]) {
    double test_values[8];
    int i, j;
    
    /* Initialize test values from command line or defaults */
    if (argc > 1) {
        for (i = 0; i < 8 && i < argc - 1; i++) {
            test_values[i] = parse_double(argv[i + 1]);
        }
    } else {
        /* Default test cases including NaN, INF, and normal numbers */
        test_values[0] = NAN;
        test_values[1] = 1.0;
        test_values[2] = -1.0;
        test_values[3] = INFINITY;
        test_values[4] = -INFINITY;
        test_values[5] = 0.0;
        test_values[6] = -0.0;
        test_values[7] = 2.5;
    }
    
    volatile int total_results = 0;
    
    /* Perform comparisons between all pairs */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            double a = test_values[i];
            double b = test_values[j];
            
            /* Force compiler to generate actual comparisons */
            volatile int cmp_result = compare_all_results(a, b);
            total_results ^= cmp_result;  /* Prevent elimination */
            
            /* Use classification function */
            const char *cls = classify_comparison(a, b);
            if (cls[0] != 'u') {  /* Dummy use to prevent elimination */
                total_results += cls[0];
            }
            
            /* Use inline assembly comparisons */
            volatile int asm_result = fp_compare_asm(a, b);
            total_results ^= asm_result;
            
            volatile int asm_result2 = fp_compare_asm2(a, b);
            total_results ^= asm_result2;
        }
    }
    
#ifdef __SSE2__
    /* Test vector comparisons */
    v2df vec_a = {test_values[0], test_values[1]};
    v2df vec_b = {test_values[2], test_values[3]};
    v2df vec_result = vector_compare(vec_a, vec_b);
    
    /* Use vector results to prevent elimination */
    total_results += (int)vec_result[0] + (int)vec_result[1];
#endif
    
    /* Complex conditional with many branches */
    for (i = 0; i < 8; i++) {
        double a = test_values[i];
        for (j = 0; j < 8; j++) {
            double b = test_values[j];
            
            /* Large if-else chain to force multiple condition code generations */
            if (isunordered(a, b)) {
                total_results += 1;
            } else if (isless(a, b)) {
                total_results += 2;
            } else if (isgreater(a, b)) {
                total_results += 3;
            } else if (islessequal(a, b)) {
                total_results += 4;
            } else if (isgreaterequal(a, b)) {
                total_results += 5;
            } else if (islessgreater(a, b)) {
                total_results += 6;
            } else if (a == b) {
                total_results += 7;
            } else {
                total_results += 8;
            }
            
            /* More comparisons using standard operators */
            if (a < b) total_results += 10;
            if (a > b) total_results += 20;
            if (a <= b) total_results += 30;
            if (a >= b) total_results += 40;
            if (a == b) total_results += 50;
            if (a != b) total_results += 60;
        }
    }
    
    /* Final output to prevent dead code elimination */
    printf("Total results checksum: %d\n", total_results);
    
    return 0;
}
