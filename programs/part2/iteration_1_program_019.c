/* fp_condition_stress.c - Exhaustive test of FP comparison condition codes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent constant folding and optimization */
static volatile double sink;

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function to stress FP condition code generation */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf) {
    int result = 0;
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    volatile double inf = inf_val;
    volatile double neginf = neg_inf;
    
    /* Complex control flow with goto to prevent simplification */
    int state = 0;
    
    /* Block 1: Direct comparisons with normal numbers */
    if (x == y) {
        state |= 1;
        goto label1;
    }
    if (x != y) {
        state |= 2;
        goto label2;
    }
    
label1:
    if (x < y) {
        state |= 4;
        goto label3;
    }
    
label2:
    if (x <= y) {
        state |= 8;
        goto label4;
    }
    
label3:
    if (x > y) {
        state |= 16;
        goto label5;
    }
    
label4:
    if (x >= y) {
        state |= 32;
        goto label6;
    }
    
    /* Block 2: Comparisons involving NaN (triggers unordered conditions) */
label5:
    if (nan == x) {  /* Always false, but compiler must generate UNORDERED check */
        state |= 64;
    }
    
label6:
    if (x < nan) {   /* UNORDERED path */
        state |= 128;
    }
    
    if (nan != nan) { /* UNORDERED: NaN != NaN is true */
        state |= 256;
    }
    
    /* Block 3: Ordered comparisons */
    if (x == x && y == y) { /* Both are ordered numbers */
        state |= 512;
    }
    
    /* Block 4: Infinity comparisons */
    if (x < inf) {
        state |= 1024;
    }
    
    if (x > neginf) {
        state |= 2048;
    }
    
    /* Block 5: Conditional moves based on FP comparisons */
    double cmov_result;
    asm volatile ("# CMOV block start");
    
    /* Generate ucomisd + conditional moves */
    int cmp_result;
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cmp_result)
        : "x"(x), "x"(y)
        : "al", "cc"
    );
    result += cmp_result;
    
    /* More inline assembly with different condition codes */
    int cmp_unordered;
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setnp %%al\n\t"  /* ORDERED */
        "movzbl %%al, %0"
        : "=r"(cmp_unordered)
        : "x"(nan), "x"(x)
        : "al", "cc"
    );
    result += cmp_unordered * 2;
    
    /* Block 6: Vectorized FP comparisons */
    v2df vec1 = {a, b};
    v2df vec2 = {b, a};
    v2df vec_nan = {nan_val, nan_val};
    
    /* Generate cmppd/cmpsd instructions with various predicates */
    v2di mask_eq = (v2di)(vec1 == vec2);
    v2di mask_lt = (v2di)(vec1 < vec2);
    v2di mask_le = (v2di)(vec1 <= vec2);
    v2di mask_gt = (v2di)(vec1 > vec2);
    v2di mask_ge = (v2di)(vec1 >= vec2);
    v2di mask_neq = (v2di)(vec1 != vec2);
    
    /* Comparisons with NaN */
    v2di mask_nan_eq = (v2di)(vec1 == vec_nan);
    v2di mask_nan_lt = (v2di)(vec1 < vec_nan);
    v2di mask_nan_le = (v2di)(vec1 <= vec_nan);
    
    /* Extract results from vector masks */
    long long* m1 = (long long*)&mask_eq;
    long long* m2 = (long long*)&mask_nan_eq;
    
    result += (int)(m1[0] & 1) + (int)(m1[1] & 1);
    result += (int)(m2[0] & 1) + (int)(m2[1] & 1);
    
    /* Block 7: Exhaustive comparison matrix */
    double values[] = {a, b, nan_val, inf_val, neg_inf};
    const char* names[] = {"a", "b", "nan", "inf", "-inf"};
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            volatile double v1 = values[i];
            volatile double v2 = values[j];
            
            /* Use all comparison operators */
            int r = 0;
            r += (v1 == v2) ? 1 : 0;
            r += (v1 != v2) ? 2 : 0;
            r += (v1 < v2) ? 4 : 0;
            r += (v1 <= v2) ? 8 : 0;
            r += (v1 > v2) ? 16 : 0;
            r += (v1 >= v2) ? 32 : 0;
            
            /* Use result to prevent dead code elimination */
            result += r * (i + j + 1);
            
            /* Conditional expression (generates conditional moves) */
            double cond_val = (v1 < v2) ? v1 : v2;
            sink = cond_val;
        }
    }
    
    /* Block 8: More inline assembly with explicit condition codes */
    int cc_result;
    
    /* UNORDERED test */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "jp 1f\n\t"
        "movl $0, %0\n\t"
        "jmp 2f\n\t"
        "1:\n\t"
        "movl $1, %0\n\t"
        "2:"
        : "=r"(cc_result)
        : "x"(nan), "x"(a)
        : "cc"
    );
    result += cc_result * 100;
    
    /* UNLT (unordered or less than) */
    double test_val = a;
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setb %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cc_result)
        : "x"(test_val), "x"(b)
        : "al", "bl", "cc"
    );
    result += cc_result * 200;
    
    /* UNEQ (unordered or equal) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "sete %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cc_result)
        : "x"(test_val), "x"(test_val)
        : "al", "bl", "cc"
    );
    result += cc_result * 300;
    
    /* LTGT (less than or greater than, ordered) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "seta %%al\n\t"
        "setb %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "al", "bl", "cc"
    );
    result += cc_result * 400;
    
    return result + state;
}

/* Main function with various FP values */
int main(void) {
    /* Initialize FP values */
    double normal1 = 3.141592653589793;
    double normal2 = 2.718281828459045;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf = -__builtin_inf();
    
    /* Array of value pairs to test */
    double test_pairs[][2] = {
        {normal1, normal2},
        {normal1, nan_val},
        {nan_val, normal2},
        {nan_val, nan_val},
        {normal1, inf_val},
        {neg_inf, normal2},
        {inf_val, neg_inf},
        {0.0, -0.0}  /* +0 and -0 compare equal */
    };
    
    int total_result = 0;
    
    /* Test each pair */
    for (size_t i = 0; i < sizeof(test_pairs) / sizeof(test_pairs[0]); i++) {
        total_result += stress_fp_comparisons(
            test_pairs[i][0],
            test_pairs[i][1],
            nan_val,
            inf_val,
            neg_inf
        );
    }
    
    /* Additional test with computed values to prevent optimization */
    volatile double v = 1.0;
    for (int i = 0; i < 10; i++) {
        v = v * 1.1;
        total_result += stress_fp_comparisons(v, v * 0.5, nan_val, inf_val, neg_inf);
    }
    
    /* Use result to prevent dead code elimination */
    printf("FP condition stress test result: %d\n", total_result);
    
    return total_result != 0 ? 0 : 1;
}
