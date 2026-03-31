/* fp_condition_stress.c
 * Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off fp_condition_stress.c -o fp_stress
 * Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math fp_condition_stress.c -o fp_stress
 * And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline fp_condition_stress.c -o fp_stress_32
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function to stress FP comparison condition code generation */
static int stress_fp_comparisons(double a, double b, double c, double d) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v3 = c;
    volatile double v4 = d;
    
    int result = 0;
    
    /* Create NaN and infinity constants */
    const double nan_val = __builtin_nan("");
    const double inf_pos = __builtin_inf();
    const double inf_neg = -__builtin_inf();
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_pos;
    
    /* Exhaustive comparison matrix to trigger all condition codes */
    
    /* 1. UNORDERED cases (comparisons involving NaN) */
    if (v1 != v1) {  /* v1 is NaN check */
        result |= 1;
    }
    
    if (v_nan == v2) {
        result |= 2;
        goto unordered_block;
    }
    
    if (v2 < v_nan) {
        result |= 4;
    }
    
unordered_block:
    if (v_nan != v_nan) {  /* Always true for NaN != NaN */
        result |= 8;
    }
    
    /* 2. ORDERED cases (normal comparisons) */
    if (v1 == v2 && v1 == v2) {  /* Double check to prevent optimization */
        result |= 16;
        goto ordered_block;
    }
    
    if (v2 < v3) {
        result |= 32;
    }
    
ordered_block:
    if (v3 <= v4) {
        result |= 64;
    }
    
    if (v4 > v1) {
        result |= 128;
    }
    
    if (v1 >= v3) {
        result |= 256;
    }
    
    /* 3. UNEQ (unordered or equal) */
    double temp = v_nan;
    if (!(temp == v2) || (v2 != v2)) {
        result |= 512;
        goto uneq_block;
    }
    
uneq_block:
    /* 4. UNGE (unordered or greater than or equal) */
    if (!(v1 < v_nan) || (v1 != v1)) {
        result |= 1024;
    }
    
    /* 5. UNGT (unordered or greater than) */
    if (!(v_nan <= v2) || (v2 != v2)) {
        result |= 2048;
    }
    
    /* 6. UNLE (unordered or less than or equal) */
    if (!(v3 > v_nan) || (v3 != v3)) {
        result |= 4096;
    }
    
    /* 7. UNLT (unordered or less than) */
    if (!(v4 >= v_nan) || (v4 != v4)) {
        result |= 8192;
    }
    
    /* 8. LTGT (less than or greater than, but not equal and not unordered) */
    if ((v1 < v2) || (v1 > v2)) {
        if (v1 == v1 && v2 == v2) {  /* Both ordered */
            result |= 16384;
        }
    }
    
    /* Complex conditional expressions using ?: operator */
    double cond_val = (v1 < v_nan) ? 1.0 : 
                     (v_nan == v2) ? 2.0 :
                     (v3 >= v_inf) ? 3.0 :
                     (v_inf <= v4) ? 4.0 : 5.0;
    
    result += (int)(cond_val * 100);
    
    /* Inline assembly with condition codes */
    int cc_result = 0;
    
    /* ucomisd with setp (parity/UNORDERED) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cc_result)
        : "x"(v_nan), "x"(v1)
        : "cc", "eax"
    );
    result += cc_result * 1000;
    
    /* ucomisd with setb (UNLT when unordered?) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cc_result)
        : "x"(v1), "x"(v2)
        : "cc", "eax"
    );
    result += cc_result * 2000;
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %3\n\t"
        "fcmovbe %1, %0"
        : "=t"(cmov_result)
        : "u"(v3), "x"(v1), "x"(v2)
        : "cc"
    );
    result += (int)(cmov_result * 10000);
    
    return result;
}

/* Function with vectorized FP comparisons */
static int vector_fp_comparisons(const double* arr1, const double* arr2, int n) {
    v2df sum_mask = {0.0, 0.0};
    
    for (int i = 0; i < n - 1; i += 2) {
        /* Load vectors */
        v2df v1, v2;
        memcpy(&v1, &arr1[i], sizeof(v2df));
        memcpy(&v2, &arr2[i], sizeof(v2df));
        
        /* Various vector comparisons generating different condition codes */
        v2df cmp_eq = v1 == v2;      /* EQ */
        v2df cmp_neq = v1 != v2;     /* NEQ/UNEQ */
        v2df cmp_lt = v1 < v2;       /* LT/UNLT */
        v2df cmp_le = v1 <= v2;      /* LE/UNLE */
        v2df cmp_gt = v1 > v2;       /* GT/UNGT */
        v2df cmp_ge = v1 >= v2;      /* GE/UNGE */
        
        /* Create NaN vector */
        v2df nan_vec = {__builtin_nan(""), __builtin_nan("")};
        v2df cmp_unord = v1 != v1;   /* UNORDERED check */
        v2df cmp_nan = v1 == nan_vec; /* Comparison with NaN */
        
        /* Mix comparisons to prevent optimization */
        sum_mask += cmp_eq + cmp_neq * 2.0 + cmp_lt * 3.0 + 
                   cmp_le * 4.0 + cmp_gt * 5.0 + cmp_ge * 6.0 +
                   cmp_unord * 7.0 + cmp_nan * 8.0;
    }
    
    /* Extract result */
    double result_arr[2];
    memcpy(result_arr, &sum_mask, sizeof(sum_mask));
    return (int)(result_arr[0] + result_arr[1]);
}

/* Main function with complex control flow */
int main(void) {
    /* Initialize test values */
    double normal_vals[] = {1.0, 2.0, 3.0, 4.0, -1.0, -2.0, 0.0, -0.0};
    const double nan_val = __builtin_nan("");
    const double inf_pos = __builtin_inf();
    const double inf_neg = -__builtin_inf();
    
    int total_result = 0;
    
    /* Test with various combinations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Mix normal values with specials */
            double a = normal_vals[i];
            double b = normal_vals[j];
            double c = (i % 3 == 0) ? nan_val : normal_vals[(i + j) % 8];
            double d = (j % 4 == 0) ? inf_pos : normal_vals[(i * j) % 8];
            
            /* Complex control flow with goto */
            if (a < b) {
                goto label_a;
            } else if (a > b) {
                goto label_b;
            } else {
                goto label_c;
            }
            
label_a:
            total_result += stress_fp_comparisons(a, b, c, d);
            goto next_iter;
            
label_b:
            total_result += stress_fp_comparisons(b, a, d, c);
            goto next_iter;
            
label_c:
            total_result += stress_fp_comparisons(c, d, a, b);
            /* Fall through */
            
next_iter:
            /* Add NaN and infinity to the mix */
            if (i % 2 == 0) {
                total_result += stress_fp_comparisons(nan_val, b, inf_pos, d);
            }
            if (j % 2 == 1) {
                total_result += stress_fp_comparisons(a, nan_val, c, inf_neg);
            }
        }
    }
    
    /* Test vectorized comparisons */
    double arr1[100], arr2[100];
    for (int i = 0; i < 100; i++) {
        arr1[i] = i * 0.1;
        arr2[i] = (i % 10 == 0) ? nan_val : (100 - i) * 0.1;
    }
    
    total_result += vector_fp_comparisons(arr1, arr2, 100);
    
    /* Final checksum to prevent dead code elimination */
    volatile int checksum = total_result;
    
    printf("FP comparison stress test complete. Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
