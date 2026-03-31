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
    if (v_nan <= v2) result ^= 512;     /* Unordered */
    if (v_nan > v_inf) result ^= 1024;  /* Unordered */
    if (v_neg_inf >= v_nan) result ^= 2048; /* Unordered */
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 4096;  /* Always false */
    if (v_nan != v_nan) result ^= 8192;  /* Always true */
    
    /* Infinity comparisons */
    if (v_inf == v_inf) result ^= 16384;
    if (v_neg_inf < v_inf) result ^= 32768;
    if (v_inf > v1) result ^= 65536;
    
    /* Complex conditional expressions using ?: operator */
    double cond_result = (v1 < v2) ? 1.0 : 
                        (v1 == v_nan) ? 2.0 :
                        (v_nan != v_nan) ? 3.0 :
                        (v_inf > v_neg_inf) ? 4.0 : 5.0;
    
    result ^= (int)(cond_result * 1000);
    
    return result;
}

/* Function with goto-based complex control flow */
static int complex_fp_control_flow(double x, double y, double nan_val) {
    volatile double a = x;
    volatile double b = y;
    volatile double nan = nan_val;
    
    int checksum = 0;
    
    /* Use goto to create non-linear control flow */
    if (a < b) goto label_lt;
    if (a == b) goto label_eq;
    if (a != a) goto label_unordered;  /* a is NaN */
    if (b != b) goto label_unordered;  /* b is NaN */
    
    /* Normal ordered comparisons */
    checksum += 1;
    goto label_done;
    
label_lt:
    checksum += 2;
    if (a <= b) checksum += 4;
    goto label_done;
    
label_eq:
    checksum += 8;
    if (a >= b) checksum += 16;
    goto label_done;
    
label_unordered:
    checksum += 32;
    /* More unordered comparisons */
    if (nan == a) checksum += 64;      /* UNORDERED path */
    if (nan != b) checksum += 128;     /* ORDERED path (if b not NaN) */
    if (!(nan < a)) checksum += 256;   /* UNGE -> "nlt" */
    if (!(nan <= b)) checksum += 512;  /* UNGT -> "nle" */
    if (nan <= nan) checksum += 1024;  /* UNLE -> "ule" */
    if (nan < nan) checksum += 2048;   /* UNLT -> "ult" */
    if (nan != a && nan != b) checksum += 4096; /* LTGT -> "une" */
    
label_done:
    return checksum;
}

/* Vectorized FP comparisons */
static void vector_fp_comparisons(double *arr1, double *arr2, int *mask, int n) {
    /* Use vector extensions for SIMD comparisons */
    for (int i = 0; i < n; i += 2) {
        v2df v1 = {arr1[i], arr1[i+1]};
        v2df v2 = {arr2[i], arr2[i+1]};
        
        /* Generate various comparison masks */
        v2di cmp_eq = (v2di)(v1 == v2);
        v2di cmp_neq = (v2di)(v1 != v2);
        v2di cmp_lt = (v2di)(v1 < v2);
        v2di cmp_le = (v2di)(v1 <= v2);
        v2di cmp_gt = (v2di)(v1 > v2);
        v2di cmp_ge = (v2di)(v1 >= v2);
        
        /* Store results */
        mask[i] = cmp_eq[0] != 0;
        mask[i+1] = cmp_eq[1] != 0;
    }
}

/* Inline assembly that uses FP condition codes */
static int inline_asm_fp_conditions(double a, double b) {
    int result = 0;
    char setp_result, setnp_result, sete_result, setne_result;
    char seta_result, setae_result, setb_result, setbe_result;
    
    /* Use ucomisd and set condition code based on result */
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setp %[setp]\n\t"
        "setnp %[setnp]\n\t"
        "sete %[sete]\n\t"
        "setne %[setne]\n\t"
        "seta %[seta]\n\t"
        "setae %[setae]\n\t"
        "setb %[setb]\n\t"
        "setbe %[setbe]"
        : [setp] "=r" (setp_result),
          [setnp] "=r" (setnp_result),
          [sete] "=r" (sete_result),
          [setne] "=r" (setne_result),
          [seta] "=r" (seta_result),
          [setae] "=r" (setae_result),
          [setb] "=r" (setb_result),
          [setbe] "=r" (setbe_result)
        : [a] "x" (a), [b] "x" (b)
        : "cc"
    );
    
    result = setp_result + (setnp_result << 1) + (sete_result << 2) +
             (setne_result << 3) + (seta_result << 4) + (setae_result << 5) +
             (setb_result << 6) + (setbe_result << 7);
    
    return result;
}

/* Conditional move based on FP comparisons */
static double conditional_move_fp(double a, double b, double nan_val) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    
    /* Use conditional expressions that may generate conditional moves */
    double r1 = (x < y) ? 1.0 : 2.0;
    double r2 = (x == nan) ? 3.0 : 4.0;
    double r3 = (nan != nan) ? 5.0 : 6.0;
    double r4 = (!(x >= y)) ? 7.0 : 8.0;
    double r5 = (!(nan < x)) ? 9.0 : 10.0;  /* UNGE -> "nlt" */
    double r6 = (!(nan <= y)) ? 11.0 : 12.0; /* UNGT -> "nle" */
    
    return r1 + r2 + r3 + r4 + r5 + r6;
}

int main() {
    /* Initialize FP values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double zero = 0.0;
    double neg_zero = -0.0;
    double inf = __builtin_inf();
    double neg_inf = -__builtin_inf();
    double nan_val = __builtin_nan("");
    
    int final_result = 0;
    
    /* Test 1: Exhaustive pairwise comparisons */
    double test_values[] = {normal1, normal2, zero, neg_zero, inf, neg_inf, nan_val};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            final_result ^= stress_fp_comparisons(
                test_values[i], test_values[j], nan_val, inf, neg_inf);
        }
    }
    
    /* Test 2: Complex control flow with goto */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            final_result ^= complex_fp_control_flow(
                test_values[i], test_values[j], nan_val);
        }
    }
    
    /* Test 3: Vectorized comparisons */
    double arr1[8], arr2[8];
    int mask[8];
    
    for (int i = 0; i < 8; i++) {
        arr1[i] = normal1 * i;
        arr2[i] = normal2 * (i % 3);
        if (i == 3) arr1[i] = nan_val;
        if (i == 5) arr2[i] = nan_val;
    }
    
    vector_fp_comparisons(arr1, arr2, mask, 8);
    for (int i = 0; i < 8; i++) {
        final_result ^= mask[i] << i;
    }
    
    /* Test 4: Inline assembly with condition codes */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            final_result ^= inline_asm_fp_conditions(
                test_values[i], test_values[j]);
        }
    }
    
    /* Test 5: Conditional move patterns */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            double cmov_result = conditional_move_fp(
                test_values[i], test_values[j], nan_val);
            final_result ^= (int)cmov_result;
        }
    }
    
    /* Additional unordered comparison patterns */
    volatile double v_nan = nan_val;
    volatile double v_inf = inf;
    
    /* Direct unordered comparisons that should trigger specific cases */
    int unordered_results = 0;
    
    /* UNORDERED: (x UNORDERED y) */
    if (v_nan != v_nan) unordered_results |= 1;  /* Always true */
    
    /* ORDERED: (x ORDERED y) */
    if (v_inf == v_inf) unordered_results |= 2;  /* Always true */
    
    /* UNEQ: (x UNORDERED OR x == y) */
    if (!(v_nan < v_nan || v_nan > v_nan)) unordered_results |= 4;
    
    /* UNGE: !(x < y) including unordered */
    if (!(v_nan < normal1)) unordered_results |= 8;
    
    /* UNGT: !(x <= y) including unordered */
    if (!(v_nan <= normal1)) unordered_results |= 16;
    
    /* UNLE: (x UNORDERED OR x <= y) */
    if (v_nan <= v_nan) unordered_results |= 32;
    
    /* UNLT: (x UNORDERED OR x < y) */
    if (v_nan < v_nan) unordered_results |= 64;
    
    /* LTGT: (x < y OR x > y) and ordered */
    if (normal1 < normal2 || normal1 > normal2) unordered_results |= 128;
    
    final_result ^= unordered_results;
    
    /* Prevent dead code elimination */
    printf("Final checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
