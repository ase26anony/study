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
    
    /* Complex control flow with goto to prevent optimization */
    if (v1 == v2) goto label_eq;
    if (v1 != v2) goto label_neq;
    
label_eq:
    /* Ordered comparisons */
    if (v1 < v2) result |= 1;
    if (v1 <= v2) result |= 2;
    if (v1 > v2) result |= 4;
    if (v1 >= v2) result |= 8;
    
    /* Unordered comparisons with NaN */
    if (v_nan == v1) result |= 16;      /* UNORDERED/UNEQ paths */
    if (v1 < v_nan) result |= 32;       /* UNORDERED/UNLT paths */
    if (v_nan < v1) result |= 64;       /* UNORDERED/UNGT paths */
    if (v_nan != v_nan) result |= 128;  /* UNORDERED path */
    
    /* More unordered comparisons */
    if (!(v_nan >= v1)) result |= 256;  /* UNORDERED/UNGE path (nlt) */
    if (!(v_nan <= v1)) result |= 512;  /* UNORDERED/UNLE path (ule) */
    if (!(v_nan > v1)) result |= 1024;  /* UNORDERED/UNGT path (nle) */
    
    /* LTGT comparison (une) */
    if (v1 != v2 && !(v1 < v2) && !(v1 > v2)) {
        /* This shouldn't happen for normal numbers, but handle NaN */
        result |= 2048;
    }
    
    goto label_done;
    
label_neq:
    /* Different set of comparisons */
    if (v_inf > v_neg_inf) result |= 4096;
    if (v_neg_inf < v_inf) result |= 8192;
    
    /* NaN comparisons that should be unordered */
    if (v_nan == v_nan) {
        /* This is false for NaN */
        result |= 16384;
    }
    
    /* Ordered check */
    if (v1 == v1 && v2 == v2) {  /* Both are numbers (not NaN) */
        result |= 32768;
    }
    
label_done:
    
    /* Conditional moves based on FP comparisons */
    double cmov_result = (v1 < v2) ? 1.0 : 2.0;
    cmov_result = (v1 <= v2) ? cmov_result * 2.0 : cmov_result / 2.0;
    cmov_result = (v1 > v2) ? cmov_result + 1.0 : cmov_result - 1.0;
    cmov_result = (v1 >= v2) ? cmov_result * 3.0 : cmov_result / 3.0;
    
    /* NaN-based conditional moves */
    cmov_result = (v_nan == v1) ? cmov_result + 10.0 : cmov_result;
    cmov_result = (v1 != v1) ? cmov_result - 10.0 : cmov_result;  /* v1 is NaN check */
    
    /* Convert to int to use in result */
    result += (int)cmov_result;
    
    return result;
}

/* Function with inline assembly that uses condition codes */
static int inline_asm_fp_conds(double x, double y) {
    int result = 0;
    uint8_t cc_result;
    
    /* ucomisd with setp (parity/UNORDERED) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 0);
    
    /* ucomisd with setb (below/UNLT or UNLE) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 1);
    
    /* ucomisd with sete (equal/EQ or UNEQ) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 2);
    
    /* ucomisd with setbe (below or equal/UNLE) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 3);
    
    /* ucomisd with seta (above/UNGT) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 4);
    
    return result;
}

/* Vectorized FP comparisons */
static v2di vector_fp_comparisons(v2df a, v2df b, v2df nan_vec) {
    v2df cmp_result;
    v2di mask_result;
    
    /* Various vector comparisons that should generate cmppd/cmpsd */
    cmp_result = a < b;      /* LT */
    mask_result = (v2di)cmp_result;
    
    cmp_result = a <= b;     /* LE */
    mask_result |= (v2di)cmp_result << 2;
    
    cmp_result = a > b;      /* GT */
    mask_result |= (v2di)cmp_result << 4;
    
    cmp_result = a >= b;     /* GE */
    mask_result |= (v2di)cmp_result << 6;
    
    cmp_result = a == b;     /* EQ */
    mask_result |= (v2di)cmp_result << 8;
    
    cmp_result = a != b;     /* NEQ/UNEQ */
    mask_result |= (v2di)cmp_result << 10;
    
    /* Unordered comparisons with NaN */
    cmp_result = nan_vec == a;  /* UNORDERED */
    mask_result |= (v2di)cmp_result << 12;
    
    cmp_result = a == nan_vec;  /* UNORDERED */
    mask_result |= (v2di)cmp_result << 14;
    
    cmp_result = nan_vec != nan_vec; /* UNORDERED */
    mask_result |= (v2di)cmp_result << 16;
    
    return mask_result;
}

int main() {
    /* Initialize FP values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double zero = 0.0;
    double neg_zero = -0.0;
    double inf = __builtin_inf();
    double neg_inf = -__builtin_inf();
    double nan = __builtin_nan("");
    
    int checksum = 0;
    
    /* Test all combinations of values */
    double test_values[] = {normal1, normal2, zero, neg_zero, inf, neg_inf, nan};
    const char* value_names[] = {"normal1", "normal2", "zero", "neg_zero", "inf", "neg_inf", "nan"};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    printf("Testing FP comparisons...\n");
    
    /* Exhaustive matrix of comparisons */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            printf("Comparing %s vs %s: ", value_names[i], value_names[j]);
            
            /* Stress function call */
            int result = stress_fp_comparisons(
                test_values[i], 
                test_values[j],
                nan,
                inf,
                neg_inf
            );
            
            checksum ^= result;
            printf("result = 0x%08x\n", result);
            
            /* Inline assembly tests */
            if (i != j) {  /* Skip some to avoid too much output */
                int asm_result = inline_asm_fp_conds(test_values[i], test_values[j]);
                checksum ^= asm_result;
                printf("  asm result = 0x%08x\n", asm_result);
            }
        }
    }
    
    /* Vectorized tests */
    printf("\nTesting vectorized FP comparisons...\n");
    
    v2df vec_a = {normal1, normal2};
    v2df vec_b = {normal2, normal1};
    v2df vec_nan = {nan, nan};
    
    v2di vec_result = vector_fp_comparisons(vec_a, vec_b, vec_nan);
    
    /* Extract results from vector */
    long long* vec_arr = (long long*)&vec_result;
    checksum ^= (int)(vec_arr[0] ^ vec_arr[1]);
    
    printf("Vector result = [0x%016llx, 0x%016llx]\n", vec_arr[0], vec_arr[1]);
    
    /* Array-based vector comparisons */
    double arr_a[4] = {1.0, 2.0, 3.0, 4.0};
    double arr_b[4] = {4.0, 3.0, 2.0, 1.0};
    double arr_nan[4] = {nan, nan, nan, nan};
    
    int mask_results[4] = {0};
    
    /* Loop that should potentially vectorize */
    for (int i = 0; i < 4; i++) {
        volatile double a = arr_a[i];
        volatile double b = arr_b[i];
        volatile double n = arr_nan[i];
        
        /* Mix of ordered and unordered comparisons */
        mask_results[i] |= (a < b) ? 1 : 0;
        mask_results[i] |= (a <= b) ? 2 : 0;
        mask_results[i] |= (a > b) ? 4 : 0;
        mask_results[i] |= (a >= b) ? 8 : 0;
        mask_results[i] |= (a == b) ? 16 : 0;
        mask_results[i] |= (a != b) ? 32 : 0;
        
        /* Unordered comparisons */
        mask_results[i] |= (n == a) ? 64 : 0;
        mask_results[i] |= (a == n) ? 128 : 0;
        mask_results[i] |= (n != n) ? 256 : 0;
        
        checksum ^= mask_results[i] * (i + 1);
    }
    
    printf("Array mask results: [%d, %d, %d, %d]\n", 
           mask_results[0], mask_results[1], mask_results[2], mask_results[3]);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum: 0x%08x\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate tests ran */
}
