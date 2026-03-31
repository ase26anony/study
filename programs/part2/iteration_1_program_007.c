/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_cond_test_vec fp_cond_test.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_cond_test_32 fp_cond_test.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function to stress FP comparison condition code generation */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    volatile double v_neg_inf = neg_inf_val;
    
    int result = 0;
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* Normal number comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Comparisons with NaN (unordered cases) */
    if (v1 == v_nan) result ^= 64;      /* Should be false, unordered */
    if (v1 != v_nan) result ^= 128;     /* Should be true if v1 is not NaN */
    if (v1 < v_nan)  result ^= 256;     /* Unordered */
    if (v1 <= v_nan) result ^= 512;     /* Unordered */
    if (v1 > v_nan)  result ^= 1024;    /* Unordered */
    if (v1 >= v_nan) result ^= 2048;    /* Unordered */
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 4096; /* False, unordered */
    if (v_nan != v_nan) result ^= 8192; /* True, unordered */
    
    /* Comparisons with infinity */
    if (v1 == v_inf) result ^= 16384;
    if (v_inf == v_inf) result ^= 32768;
    if (v_neg_inf < v_inf) result ^= 65536;
    
    /* Complex conditional expressions using ?: operator */
    double cond_result = (v1 < v2) ? 1.0 : (v1 > v2) ? -1.0 : 0.0;
    result ^= (int)(cond_result * 1000);
    
    /* More complex unordered checks */
    if (!(v1 < v_nan) && !(v1 > v_nan) && (v1 == v_nan)) {
        /* This path should never be taken for non-NaN v1 */
        result = -1;
    }
    
    /* Goto-based control flow to prevent optimization */
    if (v1 != v1) { /* Check if v1 is NaN */
        goto unordered_path;
    }
    
    ordered_path:
    result += 1000000;
    goto continue_main;
    
    unordered_path:
    result += 2000000;
    
    continue_main:
    
    /* Conditional moves based on FP comparisons */
    int cmov_result;
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cmov_result)
        : "x"(v1), "x"(v_nan)
        : "al", "cc"
    );
    result ^= cmov_result * 4;
    
    /* Another inline asm with different condition */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cmov_result)
        : "x"(v1), "x"(v2)
        : "al", "cc"
    );
    result ^= cmov_result * 8;
    
    return result;
}

/* Function using vector extensions for SIMD comparisons */
static int vector_fp_comparisons(double *arr1, double *arr2, int n) {
    v2df vresult_mask = {0, 0};
    int mask_result = 0;
    
    for (int i = 0; i < n - 1; i += 2) {
        v2df va = {arr1[i], arr1[i+1]};
        v2df vb = {arr2[i], arr2[i+1]};
        
        /* Various vector comparisons */
        v2di cmp_eq = (v2di)(va == vb);
        v2di cmp_neq = (v2di)(va != vb);
        v2di cmp_lt = (v2di)(va < vb);
        v2di cmp_le = (v2di)(va <= vb);
        v2di cmp_gt = (v2di)(va > vb);
        v2di cmp_ge = (v2di)(va >= vb);
        
        /* Combine masks */
        vresult_mask += (v2df)cmp_eq + (v2df)cmp_neq + 
                       (v2df)cmp_lt + (v2df)cmp_le +
                       (v2df)cmp_gt + (v2df)cmp_ge;
        
        /* Extract mask bits */
        long long *mask_ptr = (long long*)&cmp_eq;
        mask_result ^= mask_ptr[0] ^ mask_ptr[1];
    }
    
    /* Reduce vector result */
    double *res_ptr = (double*)&vresult_mask;
    return mask_result + (int)(res_ptr[0] + res_ptr[1]);
}

/* Another function with mixed comparisons and control flow */
static int exhaustive_comparison_matrix(void) {
    double values[] = {
        1.0, 2.0, -1.0, -2.0,
        0.0, -0.0, __builtin_inf(), -__builtin_inf(),
        __builtin_nan(""), __builtin_nan("0x1234")
    };
    
    int n = sizeof(values) / sizeof(values[0]);
    int total_result = 0;
    
    /* Compare every pair of values */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            volatile double a = values[i];
            volatile double b = values[j];
            
            /* Use all comparison operators */
            if (a == b) total_result += 1;
            if (a != b) total_result += 2;
            if (a < b)  total_result += 4;
            if (a <= b) total_result += 8;
            if (a > b)  total_result += 16;
            if (a >= b) total_result += 32;
            
            /* Conditional expression */
            double tmp = (a < b) ? a : (a > b) ? b : 0.0;
            total_result += (int)tmp;
            
            /* Complex condition with && and || */
            if ((a < b) || (a == b) || (a > b)) {
                total_result += 64;
            }
            
            /* Check for unordered */
            if (a != a || b != b) {
                total_result += 128;  /* At least one is NaN */
            }
        }
    }
    
    /* Vectorized comparisons on arrays */
    double arr1[8], arr2[8];
    for (int i = 0; i < 8; i++) {
        arr1[i] = values[i % n];
        arr2[i] = values[(i + 1) % n];
    }
    
    total_result += vector_fp_comparisons(arr1, arr2, 8);
    
    return total_result;
}

/* Function with inline assembly using various condition codes */
static void inline_asm_condition_codes(double x, double y, int *results) {
    int r[16] = {0};
    
    /* Various FP comparison condition codes via inline asm */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setp %0\n\t"
        : "=r"(r[0]) : "x"(x), "x"(y) : "cc");
    
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setnp %0\n\t"
        : "=r"(r[1]) : "x"(x), "x"(y) : "cc");
    
    asm volatile (
        "ucomisd %1, %2\n\t"
        "sete %0\n\t"
        : "=r"(r[2]) : "x"(x), "x"(y) : "cc");
    
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setne %0\n\t"
        : "=r"(r[3]) : "x"(x), "x"(y) : "cc");
    
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setb %0\n\t"
        : "=r"(r[4]) : "x"(x), "x"(y) : "cc");
    
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setbe %0\n\t"
        : "=r"(r[5]) : "x"(x), "x"(y) : "cc");
    
    asm volatile (
        "ucomisd %1, %2\n\t"
        "seta %0\n\t"
        : "=r"(r[6]) : "x"(x), "x"(y) : "cc");
    
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setae %0\n\t"
        : "=r"(r[7]) : "x"(x), "x"(y) : "cc");
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %3\n\t"
        "fcmovb %1, %0\n\t"
        : "=t"(cmov_result)
        : "u"(x), "x"(x), "x"(y)
        : "cc"
    );
    r[8] = (int)cmov_result;
    
    /* Store results */
    for (int i = 0; i < 9; i++) {
        results[i] = r[i];
    }
}

int main(void) {
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    
    printf("Starting FP comparison condition code test...\n");
    
    /* Test 1: Basic stress function */
    int result1 = stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    printf("Stress test result: %d\n", result1);
    
    /* Test 2: NaN comparisons */
    int result2 = stress_fp_comparisons(nan_val, normal1, nan_val, inf_val, neg_inf_val);
    printf("NaN test result: %d\n", result2);
    
    /* Test 3: Infinity comparisons */
    int result3 = stress_fp_comparisons(inf_val, normal1, nan_val, inf_val, neg_inf_val);
    printf("Infinity test result: %d\n", result3);
    
    /* Test 4: Exhaustive matrix */
    int result4 = exhaustive_comparison_matrix();
    printf("Exhaustive matrix result: %d\n", result4);
    
    /* Test 5: Inline assembly with condition codes */
    int asm_results[16];
    inline_asm_condition_codes(normal1, normal2, asm_results);
    inline_asm_condition_codes(normal1, nan_val, asm_results + 8);
    
    printf("Inline asm results: ");
    for (int i = 0; i < 16; i++) {
        printf("%d ", asm_results[i]);
    }
    printf("\n");
    
    /* Final checksum to prevent dead code elimination */
    int final_result = result1 ^ result2 ^ result3 ^ result4;
    for (int i = 0; i < 16; i++) {
        final_result ^= asm_results[i];
    }
    
    printf("Final checksum: %d\n", final_result);
    printf("Test completed.\n");
    
    return final_result != 0 ? 0 : 1;
}
