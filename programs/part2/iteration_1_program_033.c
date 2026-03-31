/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_conds fp_conds.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_conds_vec fp_conds.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_conds_32 fp_conds.c */

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
    
    /* Normal vs Normal comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Normal vs NaN (unordered comparisons) */
    if (v1 == v_nan) result ^= 64;      /* Should be false, unordered */
    if (v1 != v_nan) result ^= 128;     /* Should be true, unordered */
    if (v1 < v_nan)  result ^= 256;     /* Unordered */
    if (v1 <= v_nan) result ^= 512;     /* Unordered */
    if (v1 > v_nan)  result ^= 1024;    /* Unordered */
    if (v1 >= v_nan) result ^= 2048;    /* Unordered */
    
    /* NaN vs NaN */
    if (v_nan == v_nan) result ^= 4096; /* Should be false */
    if (v_nan != v_nan) result ^= 8192; /* Should be true */
    
    /* Infinity comparisons */
    if (v_inf == v_inf) result ^= 16384;
    if (v_neg_inf < v_inf) result ^= 32768;
    if (v_inf > v_neg_inf) result ^= 65536;
    
    /* Complex conditional expressions using ?: operator */
    double cond_result = (v1 < v2) ? 1.0 : (v1 > v2) ? 2.0 : 3.0;
    result += (int)cond_result;
    
    /* Conditional with NaN */
    cond_result = (v1 == v_nan) ? 4.0 : (v1 != v_nan) ? 5.0 : 6.0;
    result += (int)cond_result;
    
    return result;
}

/* Function with goto-based control flow to prevent optimization */
static int complex_fp_control_flow(double x, double y, double nan_val) {
    volatile double a = x;
    volatile double b = y;
    volatile double nan = nan_val;
    
    int checksum = 0;
    
    /* Use goto to create non-linear control flow */
    if (a < b) {
        checksum += 1;
        goto label1;
    } else if (a > b) {
        checksum += 2;
        goto label2;
    } else {
        checksum += 3;
        goto label3;
    }
    
label1:
    if (a == nan) {
        checksum += 4;
        goto label4;
    }
    if (a != nan) {
        checksum += 8;
        goto label5;
    }
    
label2:
    if (b <= nan) {
        checksum += 16;
        goto label6;
    }
    if (b >= nan) {
        checksum += 32;
        goto label7;
    }
    
label3:
    if (nan < a) {
        checksum += 64;
        goto label8;
    }
    if (nan > a) {
        checksum += 128;
        goto label9;
    }
    
label4:
    if (a <= b) checksum += 256;
    goto label10;
    
label5:
    if (a >= b) checksum += 512;
    goto label10;
    
label6:
    if (b == b) checksum += 1024;
    goto label10;
    
label7:
    if (nan != nan) checksum += 2048;
    goto label10;
    
label8:
    if (a != a) checksum += 4096;
    goto label10;
    
label9:
    if (b != b) checksum += 8192;
    goto label10;
    
label10:
    return checksum;
}

/* Vectorized FP comparisons */
static int vector_fp_comparisons(double *arr1, double *arr2, int n) {
    v2df sum_mask = {0, 0};
    
    for (int i = 0; i < n - 1; i += 2) {
        v2df v1 = {arr1[i], arr1[i + 1]};
        v2df v2 = {arr2[i], arr2[i + 1]};
        
        /* Various vector comparisons that should generate cmpsd/ucomisd */
        v2di cmp_eq = (v2di)(v1 == v2);
        v2di cmp_ne = (v2di)(v1 != v2);
        v2di cmp_lt = (v2di)(v1 < v2);
        v2di cmp_le = (v2di)(v1 <= v2);
        v2di cmp_gt = (v2di)(v1 > v2);
        v2di cmp_ge = (v2di)(v1 >= v2);
        
        /* Accumulate results */
        sum_mask += (v2df)cmp_eq + (v2df)cmp_ne + (v2df)cmp_lt + 
                   (v2df)cmp_le + (v2df)cmp_gt + (v2df)cmp_ge;
    }
    
    return (int)(sum_mask[0] + sum_mask[1]);
}

/* Inline assembly that uses FP condition codes directly */
static int inline_asm_fp_conds(double a, double b) {
    int result = 0;
    char unordered, equal, less, greater;
    
    /* Test UNORDERED/ORDERED conditions */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0\n\t"
        : "=r"(unordered)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (unordered & 1) << 0;
    
    /* Test EQUAL/UNEQUAL conditions */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0\n\t"
        : "=r"(equal)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (equal & 1) << 1;
    
    /* Test LESS/GREATER conditions */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0\n\t"
        : "=r"(greater)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0\n\t"
        : "=r"(less)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (less & 1) << 2;
    result |= (greater & 1) << 3;
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "fcmovbe %3, %0\n\t"
        : "=t"(cmov_result)
        : "u"(a), "u"(b), "t"(3.14159)
        : "cc"
    );
    result += (int)cmov_result;
    
    return result;
}

/* Main function that exercises all comparison patterns */
int main(void) {
    /* Initialize FP values */
    double normal1 = 1.5;
    double normal2 = 2.7;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    double zero = 0.0;
    double neg_zero = -0.0;
    
    int total_checksum = 0;
    
    /* Test various value combinations */
    double test_values[] = {normal1, normal2, nan_val, inf_val, neg_inf_val, zero, neg_zero};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Exhaustive matrix of comparisons */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            total_checksum ^= stress_fp_comparisons(
                test_values[i], 
                test_values[j],
                nan_val,
                inf_val,
                neg_inf_val
            );
            
            total_checksum ^= complex_fp_control_flow(
                test_values[i],
                test_values[j],
                nan_val
            );
            
            total_checksum ^= inline_asm_fp_conds(
                test_values[i],
                test_values[j]
            );
        }
    }
    
    /* Vectorized comparisons */
    double arr1[16], arr2[16];
    for (int i = 0; i < 16; i++) {
        arr1[i] = i * 0.5;
        arr2[i] = (i % 3) * 0.7;
        if (i == 5) arr1[i] = nan_val;
        if (i == 10) arr2[i] = nan_val;
    }
    
    total_checksum ^= vector_fp_comparisons(arr1, arr2, 16);
    
    /* Additional unordered comparison tests */
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    
    /* These should trigger UNORDERED paths */
    int unordered_tests = 0;
    unordered_tests |= (v_nan == v_nan) ? 0 : 1;
    unordered_tests |= (v_nan != v_nan) ? 2 : 0;
    unordered_tests |= (v_nan < v_inf) ? 4 : 0;
    unordered_tests |= (v_inf > v_nan) ? 8 : 0;
    unordered_tests |= (v_nan <= v_nan) ? 16 : 0;
    unordered_tests |= (v_nan >= v_nan) ? 32 : 0;
    
    total_checksum ^= unordered_tests;
    
    /* Mixed ordered/unordered comparisons */
    double mixed_result = 0.0;
    mixed_result += (normal1 < nan_val) ? 1.0 : 0.0;
    mixed_result += (normal1 > nan_val) ? 2.0 : 0.0;
    mixed_result += (normal1 == nan_val) ? 4.0 : 0.0;
    mixed_result += (normal1 != nan_val) ? 8.0 : 0.0;
    mixed_result += (nan_val < normal1) ? 16.0 : 0.0;
    mixed_result += (nan_val > normal1) ? 32.0 : 0.0;
    
    total_checksum += (int)mixed_result;
    
    /* Prevent dead code elimination */
    volatile int *dummy = &total_checksum;
    
    printf("Final checksum: %d\n", total_checksum);
    return total_checksum & 255; /* Return non-zero to indicate execution */
}
