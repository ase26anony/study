/* fp_condition_stress.c - Exhaustively test FP comparison condition code generation */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent constant folding and optimization */
static volatile double sink;

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Stress function with exhaustive FP comparisons */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf) {
    int result = 0;
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double vnan = nan_val;
    volatile double vinf = inf_val;
    volatile double vneginf = neg_inf;
    
    /* Complex control flow with goto to prevent simplification */
    int state = 0;
    
    /* Block 1: Direct comparisons with NaN (unordered cases) */
    if (vnan == v1) {
        state |= 1;
        goto label_unordered;
    }
    
    if (v1 < vnan) {
        state |= 2;
        goto label_unordered;
    }
    
    if (vnan != vnan) {  /* Always true for NaN */
        state |= 4;
        goto label_unordered;
    }
    
    if (vnan >= v2) {
        state |= 8;
        goto label_unordered;
    }
    
label_unordered:
    /* Use results in conditional expressions */
    result += (vnan == v1) ? 0 : 1;
    result += (v1 < vnan) ? 0 : 2;
    result += (vnan != vnan) ? 4 : 0;
    result += (vnan >= v2) ? 8 : 0;
    
    /* Block 2: Ordered comparisons */
    if (v1 == v2) {
        state |= 16;
        goto label_ordered;
    }
    
    if (v1 < v2) {
        state |= 32;
        goto label_ordered;
    }
    
    if (v1 <= v2) {
        state |= 64;
        goto label_ordered;
    }
    
    if (v1 > v2) {
        state |= 128;
        goto label_ordered;
    }
    
    if (v1 >= v2) {
        state |= 256;
        goto label_ordered;
    }
    
label_ordered:
    /* More conditional expressions */
    result += (v1 == v2) ? 16 : 0;
    result += (v1 < v2) ? 32 : 0;
    result += (v1 <= v2) ? 64 : 0;
    result += (v1 > v2) ? 128 : 0;
    result += (v1 >= v2) ? 256 : 0;
    
    /* Block 3: Comparisons with infinity */
    if (vinf == v1) {
        state |= 512;
        goto label_inf;
    }
    
    if (v1 < vinf) {
        state |= 1024;
        goto label_inf;
    }
    
    if (vneginf > v1) {
        state |= 2048;
        goto label_inf;
    }
    
label_inf:
    result += (vinf == v1) ? 512 : 0;
    result += (v1 < vinf) ? 1024 : 0;
    result += (vneginf > v1) ? 2048 : 0;
    
    /* Block 4: Mixed NaN and normal comparisons */
    double temp = (v1 != vnan) ? v1 : v2;
    temp = (vnan < vinf) ? temp : vinf;  /* This is false but compiler doesn't know */
    temp = (vnan > vneginf) ? temp : vneginf;
    
    result += (int)(temp * 1000);
    
    return result + state;
}

/* Function with vectorized comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), __builtin_nan("")};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    /* Various vector comparisons that generate condition codes */
    v2di mask1 = (v2di)(vec1 == vec2);
    v2di mask2 = (v2di)(vec1 < vec2);
    v2di mask3 = (v2di)(vec1 <= vec2);
    v2di mask4 = (v2di)(vec1 > vec2);
    v2di mask5 = (v2di)(vec1 >= vec2);
    
    /* Unordered vector comparisons */
    v2di mask6 = (v2di)(vec_nan == vec1);
    v2di mask7 = (v2di)(vec1 < vec_nan);
    v2di mask8 = (v2di)(vec_nan != vec_nan);
    v2di mask9 = (v2di)(vec_nan >= vec2);
    
    /* Comparisons with infinity */
    v2di mask10 = (v2di)(vec_inf == vec1);
    v2di mask11 = (v2di)(vec1 < vec_inf);
    
    /* Sum all masks to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        sum += ((long long*)&mask1)[i];
        sum += ((long long*)&mask2)[i];
        sum += ((long long*)&mask3)[i];
        sum += ((long long*)&mask4)[i];
        sum += ((long long*)&mask5)[i];
        sum += ((long long*)&mask6)[i];
        sum += ((long long*)&mask7)[i];
        sum += ((long long*)&mask8)[i];
        sum += ((long long*)&mask9)[i];
        sum += ((long long*)&mask10)[i];
        sum += ((long long*)&mask11)[i];
    }
    
    return sum;
}

/* Function with inline assembly using condition codes */
static int asm_fp_conditions(double a, double b, double nan_val) {
    int result = 0;
    uint8_t cc_result;
    
    /* Test UNORDERED (setp) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(nan_val)
        : "cc"
    );
    result += cc_result;
    
    /* Test ORDERED (setnp) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 2;
    
    /* Test UNEQ (sete after unordered check) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 4;
    
    /* Test UNLT (setb) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 8;
    
    /* Test UNLE (setbe) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 16;
    
    /* Test UNGT (seta) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 32;
    
    /* Test UNGE (setae) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 64;
    
    /* Test LTGT (setne) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 128;
    
    return result;
}

/* Loop-based comparisons to force code generation */
static int loop_fp_comparisons(double *values, int count) {
    int sum = 0;
    
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count; j++) {
            volatile double a = values[i];
            volatile double b = values[j];
            
            /* Exhaustive comparison matrix */
            if (a == b) sum += 1;
            if (a != b) sum += 2;
            if (a < b) sum += 4;
            if (a <= b) sum += 8;
            if (a > b) sum += 16;
            if (a >= b) sum += 32;
            
            /* Conditional move based on FP comparison */
            double cmov_result = (a < b) ? a : b;
            sum += (int)cmov_result;
            
            /* Complex conditional expression */
            sum += (a == b) ? 64 : (a < b) ? 128 : 256;
        }
    }
    
    return sum;
}

int main(void) {
    /* Initialize FP values */
    double normal1 = 1.5;
    double normal2 = 2.5;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf = -__builtin_inf();
    double zero = 0.0;
    double neg_zero = -0.0;
    
    /* Array for loop comparisons */
    double values[6] = {normal1, normal2, nan_val, inf_val, neg_inf, zero};
    
    int total = 0;
    
    /* Test 1: Stress function with all comparison types */
    total += stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf);
    total += stress_fp_comparisons(nan_val, normal1, nan_val, inf_val, neg_inf);
    total += stress_fp_comparisons(inf_val, neg_inf, nan_val, inf_val, neg_inf);
    total += stress_fp_comparisons(zero, neg_zero, nan_val, inf_val, neg_inf);
    
    /* Test 2: Vectorized comparisons */
    total += vector_fp_comparisons();
    
    /* Test 3: Inline assembly with condition codes */
    total += asm_fp_conditions(normal1, normal2, nan_val);
    total += asm_fp_conditions(nan_val, normal1, nan_val);
    total += asm_fp_conditions(inf_val, normal1, nan_val);
    
    /* Test 4: Loop-based comparisons */
    total += loop_fp_comparisons(values, 6);
    
    /* Use sink to prevent optimization */
    sink = total;
    
    printf("Result: %d (checksum to prevent dead code elimination)\n", total);
    return total != 0 ? 0 : 1;
}
