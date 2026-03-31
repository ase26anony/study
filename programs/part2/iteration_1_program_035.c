/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_cond_test_vec fp_cond_test.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_cond_test_32 fp_cond_test.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Global volatile variables to prevent constant folding */
volatile double g_nan = __builtin_nan("");
volatile double g_inf = __builtin_inf();
volatile double g_ninf = -__builtin_inf();
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg_one = -1.0;

/* Function with complex control flow using goto */
int fp_comparison_stress(double a, double b, double c, double d) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v3 = c;
    volatile double v4 = d;
    
    int result = 0;
    
    /* Matrix of comparisons between all combinations */
    
    /* Normal vs Normal comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2) result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2) result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Normal vs NaN comparisons (unordered cases) */
    if (v1 == g_nan) result ^= 64;
    if (v1 != g_nan) result ^= 128;
    if (v1 < g_nan) result ^= 256;
    if (v1 <= g_nan) result ^= 512;
    if (v1 > g_nan) result ^= 1024;
    if (v1 >= g_nan) result ^= 2048;
    
    /* NaN vs NaN comparisons */
    if (g_nan == g_nan) result ^= 4096;
    if (g_nan != g_nan) result ^= 8192;
    if (g_nan < g_nan) result ^= 16384;
    if (g_nan <= g_nan) result ^= 32768;
    if (g_nan > g_nan) result ^= 65536;
    if (g_nan >= g_nan) result ^= 131072;
    
    /* Normal vs Infinity comparisons */
    if (v1 == g_inf) result ^= 262144;
    if (v1 != g_inf) result ^= 524288;
    if (v1 < g_inf) result ^= 1048576;
    if (v1 <= g_inf) result ^= 2097152;
    if (v1 > g_inf) result ^= 4194304;
    if (v1 >= g_inf) result ^= 8388608;
    
    /* Infinity vs Negative Infinity */
    if (g_inf == g_ninf) result ^= 16777216;
    if (g_inf != g_ninf) result ^= 33554432;
    if (g_inf < g_ninf) result ^= 67108864;
    if (g_inf <= g_ninf) result ^= 134217728;
    if (g_inf > g_ninf) result ^= 268435456;
    if (g_inf >= g_ninf) result ^= 536870912;
    
    /* Complex conditional expressions (ternary operator) */
    double cond_val = (v1 < v2) ? (v3 * v4) : (v3 / v4);
    result += (int)cond_val;
    
    cond_val = (v1 == g_nan) ? (v2 + v3) : (v2 - v3);
    result += (int)cond_val;
    
    cond_val = (g_nan != g_nan) ? v1 : v2;
    result += (int)cond_val;
    
    /* Goto-based control flow to prevent optimization */
    if (v1 < v2) goto label_lt;
    if (v1 == v2) goto label_eq;
    if (v1 > v2) goto label_gt;
    if (v1 != v2) goto label_ne;
    
label_lt:
    result += 1000;
    goto label_common;
    
label_eq:
    result += 2000;
    goto label_common;
    
label_gt:
    result += 3000;
    goto label_common;
    
label_ne:
    result += 4000;
    goto label_common;
    
label_common:
    /* More comparisons with different ordering */
    if (v2 <= v3) result += 5000;
    if (v2 >= v3) result += 6000;
    if (v2 <= g_nan) result += 7000;
    if (v2 >= g_nan) result += 8000;
    
    return result;
}

/* Function using vectorized FP comparisons */
int vector_fp_comparisons(double *arr1, double *arr2, int n) {
    v2df vresult_mask = {0, 0};
    v2di imask = {0, 0};
    int total = 0;
    
    for (int i = 0; i < n - 1; i += 2) {
        v2df v1 = {arr1[i], arr1[i+1]};
        v2df v2 = {arr2[i], arr2[i+1]};
        
        /* Various vector comparisons */
        v2df cmp_eq = v1 == v2;
        v2df cmp_ne = v1 != v2;
        v2df cmp_lt = v1 < v2;
        v2df cmp_le = v1 <= v2;
        v2df cmp_gt = v1 > v2;
        v2df cmp_ge = v1 >= v2;
        
        /* Convert comparison results to integers */
        v2di mask_eq = (v2di)cmp_eq;
        v2di mask_ne = (v2di)cmp_ne;
        v2di mask_lt = (v2di)cmp_lt;
        v2di mask_le = (v2di)cmp_le;
        v2di mask_gt = (v2di)cmp_gt;
        v2di mask_ge = (v2di)cmp_ge;
        
        /* Accumulate masks */
        imask += mask_eq + mask_ne + mask_lt + mask_le + mask_gt + mask_ge;
    }
    
    /* Extract results from vector */
    long long *ptr = (long long *)&imask;
    total = (int)(ptr[0] + ptr[1]);
    
    return total;
}

/* Function with inline assembly using condition codes */
int asm_fp_conditions(double a, double b) {
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that reads FP condition codes */
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setp %[res]\n\t"
        : [res] "=r" (cc_result)
        : [a] "x" (a), [b] "x" (b)
        : "cc"
    );
    result += cc_result;
    
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "sete %[res]\n\t"
        : [res] "=r" (cc_result)
        : [a] "x" (a), [b] "x" (b)
        : "cc"
    );
    result += cc_result * 2;
    
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setb %[res]\n\t"
        : [res] "=r" (cc_result)
        : [a] "x" (a), [b] "x" (b)
        : "cc"
    );
    result += cc_result * 4;
    
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setbe %[res]\n\t"
        : [res] "=r" (cc_result)
        : [a] "x" (a), [b] "x" (b)
        : "cc"
    );
    result += cc_result * 8;
    
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "seta %[res]\n\t"
        : [res] "=r" (cc_result)
        : [a] "x" (a), [b] "x" (b)
        : "cc"
    );
    result += cc_result * 16;
    
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setae %[res]\n\t"
        : [res] "=r" (cc_result)
        : [a] "x" (a), [b] "x" (b)
        : "cc"
    );
    result += cc_result * 32;
    
    /* Test with NaN */
    double nan = __builtin_nan("");
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setp %[res]\n\t"
        : [res] "=r" (cc_result)
        : [a] "x" (nan), [b] "x" (b)
        : "cc"
    );
    result += cc_result * 64;
    
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setnp %[res]\n\t"
        : [res] "=r" (cc_result)
        : [a] "x" (nan), [b] "x" (nan)
        : "cc"
    );
    result += cc_result * 128;
    
    return result;
}

/* Function using GCC builtins for FP comparisons */
int builtin_fp_comparisons(double a, double b) {
    int result = 0;
    
    /* Use GCC vector comparison builtins */
    v2df v1 = {a, b};
    v2df v2 = {b, a};
    
    /* These builtins may generate cmpsd/pd instructions */
    v2df cmp_result;
    
    /* Compare equal */
    cmp_result = __builtin_ia32_cmpeqsd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    /* Compare less than */
    cmp_result = __builtin_ia32_cmpltsd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    /* Compare less than or equal */
    cmp_result = __builtin_ia32_cmplesd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    /* Compare greater than */
    cmp_result = __builtin_ia32_cmpgtsd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    /* Compare greater than or equal */
    cmp_result = __builtin_ia32_cmpgesd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    /* Compare not equal */
    cmp_result = __builtin_ia32_cmpneqsd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    /* Compare ordered */
    cmp_result = __builtin_ia32_cmpordsd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    /* Compare unordered */
    cmp_result = __builtin_ia32_cmpunordsd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    /* Compare not less than */
    cmp_result = __builtin_ia32_cmpnltsd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    /* Compare not less than or equal */
    cmp_result = __builtin_ia32_cmpnlesd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    /* Compare not greater than */
    cmp_result = __builtin_ia32_cmpngtsd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    /* Compare not greater than or equal */
    cmp_result = __builtin_ia32_cmpngesd(v1, v2);
    result += ((int*)&cmp_result)[0];
    
    return result;
}

int main() {
    double test_values[] = {
        0.0, 1.0, -1.0, 2.5, -2.5,
        __builtin_inf(), -__builtin_inf(),
        __builtin_nan(""), __builtin_nan("0xdead"),
        1e-10, -1e-10, 1e10, -1e10
    };
    
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    int final_result = 0;
    
    /* Test all pairs of values */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            final_result ^= fp_comparison_stress(
                test_values[i], 
                test_values[j],
                test_values[(i+1) % num_values],
                test_values[(j+1) % num_values]
            );
        }
    }
    
    /* Test vectorized comparisons */
    double arr1[100], arr2[100];
    for (int i = 0; i < 100; i++) {
        arr1[i] = i * 0.1;
        arr2[i] = (i % 2 == 0) ? i * 0.1 : __builtin_nan("");
    }
    
    final_result ^= vector_fp_comparisons(arr1, arr2, 100);
    
    /* Test inline assembly condition codes */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            final_result ^= asm_fp_conditions(test_values[i], test_values[j]);
        }
    }
    
    /* Test GCC builtins */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            final_result ^= builtin_fp_comparisons(test_values[i], test_values[j]);
        }
    }
    
    /* Additional unordered comparison tests */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("0xbeef");
    volatile double inf = __builtin_inf();
    volatile double ninf = -__builtin_inf();
    
    /* Direct unordered comparisons that should trigger specific condition codes */
    int unordered_tests = 0;
    
    /* UNORDERED case: (nan op x) or (x op nan) */
    if (nan1 < 1.0) unordered_tests += 1;  /* Always false, unordered */
    if (1.0 > nan1) unordered_tests += 2;  /* Always false, unordered */
    
    /* ORDERED case: both operands are not NaN */
    if (1.0 < 2.0) unordered_tests += 4;   /* Ordered, true */
    if (inf > ninf) unordered_tests += 8;  /* Ordered, true */
    
    /* UNEQ case: unordered or equal */
    /* NaN == NaN is false, but UNEQ might be used for special handling */
    
    /* UNGE case: unordered or greater than or equal */
    /* UNGT case: unordered or greater than */
    /* UNLE case: unordered or less than or equal */
    /* UNLT case: unordered or less than */
    
    /* LTGT case: less than or greater than (ordered and not equal) */
    if (1.0 != 2.0) unordered_tests += 16;  /* LTGT case */
    
    final_result ^= unordered_tests;
    
    /* Complex nested conditionals with FP comparisons */
    double x = 1.0, y = 2.0, z = __builtin_nan("");
    for (int i = 0; i < 10; i++) {
        if (x < y) {
            if (z == z) {  /* Always false with NaN */
                final_result += 1000;
            } else {
                final_result += 2000;
            }
        } else if (x > y) {
            final_result += 3000;
        } else if (x != y) {
            final_result += 4000;
        }
        
        /* Mix with integer comparisons to prevent optimization */
        if (x < y && i % 2 == 0) {
            final_result += i;
        }
        
        /* Use result in FP computation */
        x += 0.1;
        y -= 0.05;
    }
    
    printf("Final checksum: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
