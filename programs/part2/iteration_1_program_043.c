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
    
    /* UNORDERED cases (comparisons involving NaN) */
    if (v_nan == v1) result |= 1;      /* Always false, but generates unordered check */
    if (v1 == v_nan) result |= 2;      /* Same as above */
    if (v_nan < v1)  result |= 4;      /* Unordered comparison */
    if (v1 > v_nan)  result |= 8;      /* Unordered comparison */
    if (v_nan != v_nan) result |= 16;  /* NaN != NaN is true (unordered) */
    
    /* ORDERED cases (normal comparisons) */
    if (v1 == v2) result |= 32;        /* EQ */
    if (v1 != v2) result |= 64;        /* NEQ/UNEQ */
    if (v1 < v2)  result |= 128;       /* LT */
    if (v1 <= v2) result |= 256;       /* LE */
    if (v1 > v2)  result |= 512;       /* GT */
    if (v1 >= v2) result |= 1024;      /* GE */
    
    /* More complex unordered comparisons */
    if (!(v_nan < v1)) result |= 2048;  /* UNGE (not less than, including unordered) */
    if (!(v1 <= v_nan)) result |= 4096; /* UNGT (not less or equal, including unordered) */
    if (v_nan <= v1 || v_nan != v_nan) result |= 8192; /* UNLE */
    if (v1 < v_nan || v1 != v1) result |= 16384; /* UNLT */
    
    /* LTGT (ordered and not equal) */
    if ((v1 < v2 || v1 > v2) && (v1 == v1 && v2 == v2)) result |= 32768;
    
    /* Conditional moves based on FP comparisons */
    double cmov_result = (v1 < v2) ? 1.0 : 2.0;
    result += (int)cmov_result;
    
    cmov_result = (v_nan == v1) ? 3.0 : 4.0;
    result += (int)cmov_result;
    
    /* Complex conditional expressions */
    result += (v1 <= v2 && v1 == v1) ? 5 : 6;
    result += (v_nan >= v2 || v2 != v2) ? 7 : 8;
    
    return result;
}

/* Function with goto-based control flow to prevent optimization */
static int fp_comparisons_with_goto(double a, double b, double nan_val) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    int r = 0;
    
    /* Use goto to create complex control flow around FP comparisons */
    if (x < y) {
        r |= 1;
        goto label1;
    }
    
    if (x == nan) {
        r |= 2;
        goto label2;
    }
    
label1:
    if (y > nan) {
        r |= 4;
        goto label3;
    }
    
label2:
    if (nan != nan) {
        r |= 8;
        goto label4;
    }
    
label3:
    if (!(x <= y)) {
        r |= 16;
        goto label5;
    }
    
label4:
    if (x >= nan || x != x) {
        r |= 32;
        goto label6;
    }
    
label5:
    if (!(nan < x) && x == x) {
        r |= 64;
    }
    
label6:
    return r;
}

/* Vectorized FP comparisons */
static v2di vector_fp_comparisons(v2df a, v2df b, v2df nan_vec) {
    /* Generate various vector comparison masks */
    v2df cmp1 = a < b;      /* LT */
    v2df cmp2 = a == b;     /* EQ */
    v2df cmp3 = a != b;     /* NEQ/UNEQ */
    v2df cmp4 = a <= b;     /* LE */
    v2df cmp5 = a >= b;     /* GE */
    v2df cmp6 = a > b;      /* GT */
    
    /* Unordered comparisons with NaN */
    v2df cmp7 = a == nan_vec;  /* Unordered check */
    v2df cmp8 = nan_vec < b;   /* Unordered check */
    v2df cmp9 = a != a;        /* Check for NaN (unordered) */
    
    /* Combine results into integer mask */
    v2di mask1 = (v2di)cmp1;
    v2di mask2 = (v2di)cmp2;
    v2di mask3 = (v2di)cmp3;
    v2di mask4 = (v2di)cmp4;
    v2di mask5 = (v2di)cmp5;
    v2di mask6 = (v2di)cmp6;
    v2di mask7 = (v2di)cmp7;
    v2di mask8 = (v2di)cmp8;
    v2di mask9 = (v2di)cmp9;
    
    /* Combine all masks */
    v2di result = mask1 & mask2 | mask3 & mask4 | mask5 & mask6 | mask7 & mask8 & mask9;
    
    return result;
}

/* Inline assembly that explicitly uses FP condition codes */
static int inline_asm_fp_conditions(double a, double b) {
    int result = 0;
    char setp_result, setnp_result, sete_result, setne_result;
    char setb_result, setbe_result, seta_result, setae_result;
    
    /* Use ucomisd and check various condition codes */
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setp %[setp]\n\t"
        "setnp %[setnp]\n\t"
        "sete %[sete]\n\t"
        "setne %[setne]\n\t"
        "setb %[setb]\n\t"
        "setbe %[setbe]\n\t"
        "seta %[seta]\n\t"
        "setae %[setae]"
        : [setp] "=r" (setp_result),
          [setnp] "=r" (setnp_result),
          [sete] "=r" (sete_result),
          [setne] "=r" (setne_result),
          [setb] "=r" (setb_result),
          [setbe] "=r" (setbe_result),
          [seta] "=r" (seta_result),
          [setae] "=r" (setae_result)
        : [a] "x" (a), [b] "x" (b)
        : "cc"
    );
    
    result |= setp_result ? 1 : 0;
    result |= setnp_result ? 2 : 0;
    result |= sete_result ? 4 : 0;
    result |= setne_result ? 8 : 0;
    result |= setb_result ? 16 : 0;
    result |= setbe_result ? 32 : 0;
    result |= seta_result ? 64 : 0;
    result |= setae_result ? 128 : 0;
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "fcmovb %[true_val], %[result]"
        : [result] "=t" (cmov_result)
        : [a] "t" (a), [b] "t" (b), [true_val] "0" (1.0), [false_val] "t" (2.0)
        : "cc"
    );
    
    result += (int)cmov_result;
    
    return result;
}

/* Main test function */
int main(void) {
    /* Initialize FP values */
    double normal1 = 1.5;
    double normal2 = 2.7;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    double zero = 0.0;
    double neg_zero = -0.0;
    
    int checksum = 0;
    
    /* Test various combinations of values */
    checksum += stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    checksum += stress_fp_comparisons(normal1, nan_val, nan_val, inf_val, neg_inf_val);
    checksum += stress_fp_comparisons(nan_val, normal2, nan_val, inf_val, neg_inf_val);
    checksum += stress_fp_comparisons(nan_val, nan_val, nan_val, inf_val, neg_inf_val);
    checksum += stress_fp_comparisons(inf_val, normal2, nan_val, inf_val, neg_inf_val);
    checksum += stress_fp_comparisons(neg_inf_val, inf_val, nan_val, inf_val, neg_inf_val);
    checksum += stress_fp_comparisons(zero, neg_zero, nan_val, inf_val, neg_inf_val);
    
    /* Test with goto-based control flow */
    checksum += fp_comparisons_with_goto(normal1, normal2, nan_val);
    checksum += fp_comparisons_with_goto(nan_val, normal1, nan_val);
    
    /* Test vectorized comparisons */
    v2df vec_a = {normal1, normal2};
    v2df vec_b = {normal2, normal1};
    v2df vec_nan = {nan_val, nan_val};
    v2di vec_result = vector_fp_comparisons(vec_a, vec_b, vec_nan);
    
    /* Extract results from vector */
    long long* vec_res_ptr = (long long*)&vec_result;
    checksum += (int)(vec_res_ptr[0] & 0xFFFFFFFF);
    checksum += (int)(vec_res_ptr[1] & 0xFFFFFFFF);
    
    /* Test inline assembly */
    checksum += inline_asm_fp_conditions(normal1, normal2);
    checksum += inline_asm_fp_conditions(normal1, nan_val);
    checksum += inline_asm_fp_conditions(nan_val, normal2);
    
    /* Array-based comparisons in loops */
    double test_values[] = {normal1, normal2, nan_val, inf_val, neg_inf_val, zero, neg_zero};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            volatile double x = test_values[i];
            volatile double y = test_values[j];
            
            /* Perform all comparison types */
            if (x == y) checksum += 1;
            if (x != y) checksum += 2;
            if (x < y) checksum += 3;
            if (x <= y) checksum += 4;
            if (x > y) checksum += 5;
            if (x >= y) checksum += 6;
            
            /* Unordered-aware comparisons */
            if (!(x < y)) checksum += 7;      /* UNGE */
            if (!(x <= y)) checksum += 8;     /* UNGT */
            if (x <= y || x != x) checksum += 9;  /* UNLE */
            if (x < y || x != x) checksum += 10;  /* UNLT */
            if ((x < y || x > y) && (x == x && y == y)) checksum += 11; /* LTGT */
        }
    }
    
    /* Prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("FP comparison condition code test complete. Checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
