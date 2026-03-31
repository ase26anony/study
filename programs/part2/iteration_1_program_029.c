/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_conditions fp_conditions.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_conditions_vec fp_conditions.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_conditions_32 fp_conditions.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Stress function with exhaustive FP comparisons */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    volatile double v_neg_inf = neg_inf_val;
    
    int result = 0;
    
    /* Complex control flow with goto to prevent optimization */
    if (v1 == v2) goto label_eq;
    if (v1 != v2) goto label_neq;
    
label_eq:
    /* Ordered equality comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2) result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2) result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Comparisons with NaN to trigger unordered conditions */
    if (v_nan == v1) result ^= 64;      /* Should be false (unordered) */
    if (v1 == v_nan) result ^= 128;     /* Should be false (unordered) */
    if (v_nan != v_nan) result ^= 256;  /* Should be true (unordered) */
    if (v_nan < v1) result ^= 512;      /* Should be false (unordered) */
    if (v1 > v_nan) result ^= 1024;     /* Should be false (unordered) */
    
    /* Comparisons with infinity */
    if (v_inf == v_inf) result ^= 2048;         /* Should be true */
    if (v_inf > v_neg_inf) result ^= 4096;      /* Should be true */
    if (v_neg_inf < v_inf) result ^= 8192;      /* Should be true */
    
    goto label_done;
    
label_neq:
    /* More comparisons with mixed operands */
    if (v1 != v2) {
        /* Conditional move based on FP comparison */
        double cmov_result = (v1 < v2) ? v1 : v2;
        result += (int)cmov_result;
        
        /* Another conditional expression */
        int int_result = (v1 <= v2) ? 1 : 0;
        result += int_result;
    }
    
    /* Unordered comparisons with explicit checks */
    int is_unordered = (v1 != v1) || (v2 != v2);
    if (is_unordered) result |= 0x1000;
    
    /* Direct unordered checks */
    if (v_nan < v1 || v1 < v_nan) result |= 0x2000;  /* UNLT or UNGT */
    if (!(v_nan < v1) && !(v1 < v_nan)) result |= 0x4000;  /* UNEQ or UNORDERED */
    
label_done:
    return result;
}

/* Function with vectorized FP comparisons */
static void vector_fp_comparisons(double *arr1, double *arr2, int *mask, int n) {
    /* Use vector extensions for SIMD comparisons */
    for (int i = 0; i < n; i += 2) {
        v2df vec1, vec2;
        v2di cmp_result;
        
        /* Load vectors */
        memcpy(&vec1, &arr1[i], sizeof(v2df));
        memcpy(&vec2, &arr2[i], sizeof(v2df));
        
        /* Perform various vector comparisons */
        v2di cmp_eq = (v2di)(vec1 == vec2);
        v2di cmp_neq = (v2di)(vec1 != vec2);
        v2di cmp_lt = (v2di)(vec1 < vec2);
        v2di cmp_le = (v2di)(vec1 <= vec2);
        v2di cmp_gt = (v2di)(vec1 > vec2);
        v2di cmp_ge = (v2di)(vec1 >= vec2);
        
        /* Combine results */
        cmp_result = cmp_eq | cmp_neq | cmp_lt | cmp_le | cmp_gt | cmp_ge;
        
        /* Store mask */
        memcpy(&mask[i], &cmp_result, sizeof(v2di));
    }
}

/* Function with inline assembly using condition codes */
static int inline_asm_fp_conditions(double a, double b) {
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that reads FP condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "sete %%bl\n\t"
        "setb %%cl\n\t"
        "seta %%dl\n\t"
        "orb %%al, %%bl\n\t"
        "orb %%cl, %%bl\n\t"
        "orb %%dl, %%bl\n\t"
        "movb %%bl, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "bl", "cl", "dl", "cc"
    );
    
    result = cc_result;
    
    /* Another asm with different condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "setne %%al\n\t"
        "setnp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movb %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "bl", "cc"
    );
    
    result ^= (cc_result << 8);
    
    return result;
}

/* Main function that orchestrates all comparisons */
int main() {
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    double zero = 0.0;
    double neg_zero = -0.0;
    
    int final_result = 0;
    
    /* Test all combinations of values */
    double test_values[] = {normal1, normal2, nan_val, inf_val, neg_inf_val, zero, neg_zero};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Exhaustive matrix of comparisons */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            final_result ^= stress_fp_comparisons(
                test_values[i], 
                test_values[j],
                nan_val,
                inf_val,
                neg_inf_val
            );
        }
    }
    
    /* Vectorized comparisons */
    const int ARRAY_SIZE = 64;
    double arr1[ARRAY_SIZE];
    double arr2[ARRAY_SIZE];
    int mask[ARRAY_SIZE];
    
    /* Initialize arrays with mixed values including NaN */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = (i * 0.1) - 3.0;
        arr2[i] = (i % 2 == 0) ? (i * 0.05) : 
                  (i % 3 == 0) ? nan_val : 
                  (i % 5 == 0) ? inf_val : (i * 0.08);
    }
    
    vector_fp_comparisons(arr1, arr2, mask, ARRAY_SIZE);
    
    /* Aggregate vector results */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result ^= mask[i];
    }
    
    /* Inline assembly comparisons */
    final_result ^= inline_asm_fp_conditions(normal1, normal2);
    final_result ^= inline_asm_fp_conditions(normal1, nan_val);
    final_result ^= inline_asm_fp_conditions(nan_val, inf_val);
    final_result ^= inline_asm_fp_conditions(inf_val, neg_inf_val);
    
    /* More complex control flow with goto */
    volatile double x = normal1;
    volatile double y = normal2;
    
    if (x < y) goto less;
    if (x > y) goto greater;
    if (x == y) goto equal;
    
less:
    if (x < y) final_result |= 0x80000000;
    if (!(x >= y)) final_result |= 0x40000000;
    if (x != y) final_result |= 0x20000000;
    goto after_cmp;
    
greater:
    if (x > y) final_result |= 0x10000000;
    if (!(x <= y)) final_result |= 0x08000000;
    if (x != y) final_result |= 0x04000000;
    goto after_cmp;
    
equal:
    if (x == y) final_result |= 0x02000000;
    if (!(x != y)) final_result |= 0x01000000;
    
after_cmp:
    /* Unordered comparisons with explicit control flow */
    volatile double nan1 = nan_val;
    volatile double nan2 = __builtin_nan("0x1234");
    
    if (nan1 == nan1) goto nan_eq_self;  /* Should not be taken */
    if (nan1 != nan1) goto nan_neq_self; /* Should be taken */
    
nan_eq_self:
    final_result |= 0x00800000;
    goto nan_done;
    
nan_neq_self:
    final_result |= 0x00400000;
    
nan_done:
    /* Mixed ordered/unordered comparisons */
    int unordered_check = (x < nan1) || (nan1 < x) || (x != x) || (nan1 != nan1);
    if (unordered_check) final_result |= 0x00200000;
    
    /* Prevent dead code elimination */
    printf("Final checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
