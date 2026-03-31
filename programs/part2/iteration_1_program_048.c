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
    
    /* Matrix of comparisons to generate various condition codes */
    
    /* Normal vs Normal comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Normal vs NaN (unordered comparisons) */
    if (v1 == v_nan) result ^= 64;      /* Should be false, may generate UNORDERED */
    if (v1 != v_nan) result ^= 128;     /* Should be true, may generate UNORDERED */
    if (v1 < v_nan)  result ^= 256;     /* UNORDERED path */
    if (v1 <= v_nan) result ^= 512;     /* UNORDERED path */
    if (v1 > v_nan)  result ^= 1024;    /* UNORDERED path */
    if (v1 >= v_nan) result ^= 2048;    /* UNORDERED path */
    
    /* NaN vs Normal */
    if (v_nan == v2) result ^= 4096;    /* UNORDERED */
    if (v_nan != v2) result ^= 8192;    /* UNORDERED */
    
    /* NaN vs NaN */
    if (v_nan == v_nan) result ^= 16384; /* false, UNORDERED */
    if (v_nan != v_nan) result ^= 32768; /* true, UNORDERED */
    
    /* Infinity comparisons */
    if (v_inf == v_inf) result ^= 65536;
    if (v_neg_inf < v_inf) result ^= 131072;
    if (v_inf > v1) result ^= 262144;
    
    /* Complex conditional expressions using ?: operator */
    double cond_result = (v1 < v_nan) ? 1.0 : 2.0;  /* UNORDERED path */
    cond_result += (v_nan == v2) ? 3.0 : 4.0;       /* UNORDERED path */
    cond_result += (v1 != v_nan) ? 5.0 : 6.0;       /* UNORDERED path */
    
    /* Use result to prevent optimization */
    result += (int)cond_result;
    
    return result;
}

/* Function with goto-based complex control flow */
static int stress_fp_goto(double a, double b, double nan_val) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    int checksum = 0;
    
    /* Complex control flow with goto to prevent optimization */
    if (x < y) goto label_lt;
    if (x == y) goto label_eq;
    if (x != nan) goto label_unordered;
    
label_lt:
    checksum += 1;
    if (y > nan) goto label_end;  /* UNORDERED */
    
label_eq:
    checksum += 2;
    if (nan == x) goto label_unordered;  /* UNORDERED */
    
label_unordered:
    checksum += 4;
    if (x >= nan) checksum += 8;  /* UNORDERED */
    
label_end:
    /* More unordered comparisons */
    if (!(nan < x)) checksum += 16;  /* UNGE -> "nlt" */
    if (!(nan <= x)) checksum += 32; /* UNGT -> "nle" */
    if (x <= nan) checksum += 64;    /* UNLE -> "ule" */
    if (x < nan) checksum += 128;    /* UNLT -> "ult" */
    
    /* LTGT -> "une" */
    if (x != y && !(x < y) && !(x > y)) checksum += 256; /* Actually impossible for FP */
    /* Use actual LTGT: not equal and ordered */
    if (x == x && y == y && x != y) checksum += 512; /* Ordered and not equal */
    
    return checksum;
}

/* Vectorized FP comparisons */
static int stress_vector_comparisons(const double* arr1, const double* arr2, int n, double nan_val) {
    v2df v_nan = {nan_val, nan_val};
    int checksum = 0;
    
    for (int i = 0; i < n - 1; i += 2) {
        v2df v1 = {arr1[i], arr1[i + 1]};
        v2df v2 = {arr2[i], arr2[i + 1]};
        
        /* Various vector comparisons */
        v2di cmp_eq = (v2di)(v1 == v2);
        v2di cmp_neq = (v2di)(v1 != v2);
        v2di cmp_lt = (v2di)(v1 < v2);
        v2di cmp_le = (v2di)(v1 <= v2);
        v2di cmp_gt = (v2di)(v1 > v2);
        v2di cmp_ge = (v2di)(v1 >= v2);
        
        /* Comparisons with NaN */
        v2di cmp_nan_eq = (v2di)(v1 == v_nan);  /* UNORDERED */
        v2di cmp_nan_neq = (v2di)(v1 != v_nan); /* UNORDERED */
        
        /* Accumulate results */
        checksum += ((int64_t*)&cmp_eq)[0] + ((int64_t*)&cmp_eq)[1];
        checksum += ((int64_t*)&cmp_neq)[0] + ((int64_t*)&cmp_neq)[1];
        checksum += ((int64_t*)&cmp_lt)[0] + ((int64_t*)&cmp_lt)[1];
        checksum += ((int64_t*)&cmp_nan_eq)[0] + ((int64_t*)&cmp_nan_eq)[1];
        checksum += ((int64_t*)&cmp_nan_neq)[0] + ((int64_t*)&cmp_nan_neq)[1];
    }
    
    return checksum;
}

/* Inline assembly to directly exercise condition code output */
static int stress_asm_condition_codes(double a, double b, double nan_val) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    
    int result = 0;
    uint8_t cc_result;
    
    /* Test UNORDERED (setp) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(nan)
        : "cc"
    );
    result += cc_result;
    
    /* Test ORDERED (setnp) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result += cc_result * 2;
    
    /* Test less than (setb) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result += cc_result * 4;
    
    /* Test not less than (setnb) - UNGE -> "nlt" */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnb %0"
        : "=r"(cc_result)
        : "x"(nan), "x"(x)
        : "cc"
    );
    result += cc_result * 8;
    
    /* Test less or equal (setbe) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result += cc_result * 16;
    
    /* Test not less or equal (setnbe) - UNGT -> "nle" */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %0"
        : "=r"(cc_result)
        : "x"(nan), "x"(x)
        : "cc"
    );
    result += cc_result * 32;
    
    return result;
}

/* Main test function */
int main(void) {
    /* Initialize FP values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    
    int total_result = 0;
    
    /* Stress scalar comparisons */
    total_result += stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    total_result += stress_fp_comparisons(normal2, normal1, nan_val, inf_val, neg_inf_val);
    total_result += stress_fp_comparisons(inf_val, neg_inf_val, nan_val, inf_val, neg_inf_val);
    
    /* Stress with goto control flow */
    total_result += stress_fp_goto(normal1, normal2, nan_val);
    total_result += stress_fp_goto(nan_val, normal1, nan_val);
    
    /* Stress vectorized comparisons */
    double arr1[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    double arr2[8] = {8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    arr1[3] = nan_val;  /* Insert NaN to trigger unordered comparisons */
    
    total_result += stress_vector_comparisons(arr1, arr2, 8, nan_val);
    
    /* Stress inline assembly condition codes */
    total_result += stress_asm_condition_codes(normal1, normal2, nan_val);
    total_result += stress_asm_condition_codes(nan_val, normal1, nan_val);
    
    /* Additional explicit tests for each condition code */
    
    /* UNEQ: unordered or equal */
    volatile double ueq_test1 = nan_val;
    volatile double ueq_test2 = 5.0;
    if (!(ueq_test1 < ueq_test2) && !(ueq_test1 > ueq_test2)) {
        total_result += 1000;  /* Should trigger UNEQ */
    }
    
    /* UNGE: not less than (unordered or greater or equal) */
    if (!(nan_val < normal1)) {
        total_result += 2000;  /* Should trigger UNGE -> "nlt" */
    }
    
    /* UNGT: not less or equal (unordered or greater) */
    if (!(nan_val <= normal1)) {
        total_result += 3000;  /* Should trigger UNGT -> "nle" */
    }
    
    /* UNLE: unordered or less or equal */
    if (normal1 <= nan_val) {
        total_result += 4000;  /* Should trigger UNLE -> "ule" */
    }
    
    /* UNLT: unordered or less than */
    if (normal1 < nan_val) {
        total_result += 5000;  /* Should trigger UNLT -> "ult" */
    }
    
    /* LTGT: less or greater (ordered and not equal) */
    if (normal1 == normal1 && normal2 == normal2 && normal1 != normal2) {
        total_result += 6000;  /* Should trigger LTGT -> "une" */
    }
    
    /* ORDERED: both operands are numbers */
    if (normal1 == normal1 && normal2 == normal2) {
        total_result += 7000;  /* Should use ORDERED checks */
    }
    
    /* Print result to prevent optimization */
    printf("Total checksum: %d\n", total_result);
    
    return total_result != 0 ? 0 : 1;
}
