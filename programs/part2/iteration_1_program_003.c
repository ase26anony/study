/* fp_condition_stress.c - Exhaustively test FP comparison condition code generation */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent constant folding with volatile */
static volatile double vzero = 0.0;
static volatile double vone = 1.0;
static volatile double vneg = -1.0;
static volatile double vinf = __builtin_inf();
static volatile double vninf = -__builtin_inf();
static volatile double vnan = __builtin_nan("");
static volatile double vnan2 = __builtin_nan("0xDEAD");

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex control flow with goto to prevent optimization */
static int stress_comparisons(double a, double b, double c, double d) {
    int result = 0;
    volatile double x = a;
    volatile double y = b;
    volatile double z = c;
    volatile double w = d;
    
    /* Matrix of all possible comparisons */
    if (x == y) result ^= 1;
    if (x != y) result ^= 2;
    if (x < y)  result ^= 4;
    if (x <= y) result ^= 8;
    if (x > y)  result ^= 16;
    if (x >= y) result ^= 32;
    
    /* Unordered comparisons with NaN */
    if (x == vnan) result ^= 64;      /* Should be false, may generate UNORDERED */
    if (x != vnan) result ^= 128;     /* Should be true, may generate ORDERED */
    if (vnan < x)  result ^= 256;     /* UNORDERED path */
    if (vnan <= x) result ^= 512;     /* UNORDERED path */
    if (vnan > x)  result ^= 1024;    /* UNORDERED path */
    if (vnan >= x) result ^= 2048;    /* UNORDERED path */
    
    /* NaN vs NaN comparisons */
    if (vnan == vnan2) result ^= 4096;    /* UNORDERED/UNEQ */
    if (vnan != vnan2) result ^= 8192;    /* UNORDERED/LTGT */
    
    /* Infinity comparisons */
    if (x == vinf)  result ^= 16384;
    if (x < vinf)   result ^= 32768;
    if (vninf < x)  result ^= 65536;
    
    /* Conditional moves based on FP comparisons */
    double cmov_result = (x < y) ? x : y;
    result += (int)cmov_result;
    
    cmov_result = (x != vnan) ? x : y;
    result += (int)cmov_result;
    
    cmov_result = (vnan == vnan) ? x : y;  /* Always false */
    result += (int)cmov_result;
    
    /* Goto-based complex control flow */
    if (x < y) goto label1;
    if (x > y) goto label2;
    if (x == y) goto label3;
    
    /* Unordered check */
    if (x != x) goto label4;  /* Check for NaN */
    
label1:
    result += 1000;
    goto label5;
label2:
    result += 2000;
    goto label5;
label3:
    result += 3000;
    goto label5;
label4:
    result += 4000;  /* NaN path */
label5:
    
    return result;
}

/* Vectorized comparisons */
static v2di vector_comparisons(v2df va, v2df vb) {
    /* Generate various condition codes via vector comparisons */
    v2di mask;
    
    /* Equality/inequality */
    v2df cmp_eq = va == vb;
    v2df cmp_ne = va != vb;
    
    /* Ordered comparisons */
    v2df cmp_lt = va < vb;
    v2df cmp_le = va <= vb;
    v2df cmp_gt = va > vb;
    v2df cmp_ge = va >= vb;
    
    /* Combine results */
    memcpy(&mask, &cmp_eq, sizeof(mask));
    
    /* Use inline assembly to force condition code usage */
    double a = va[0], b = vb[0];
    int cc_result;
    
    /* SETP - tests for UNORDERED (parity) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    
    mask[0] ^= cc_result;
    
    /* Test other condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"      /* UNGT (not less or equal) */
        "movzbl %%al, %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    
    mask[1] ^= cc_result;
    
    return mask;
}

/* Main stress function */
static int fp_condition_stress(void) {
    int total = 0;
    
    /* Test various value combinations */
    double test_values[] = {0.0, 1.0, -1.0, __builtin_inf(), -__builtin_inf(), 
                           __builtin_nan(""), __builtin_nan("0x123")};
    
    /* Scalar comparisons */
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            total += stress_comparisons(test_values[i], test_values[j], 
                                       test_values[(i+1)%7], test_values[(j+1)%7]);
        }
    }
    
    /* Vector comparisons */
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {3.0, 4.0};
    v2df vec_nan = {__builtin_nan(""), __builtin_inf()};
    v2df vec_mixed = {0.0, -__builtin_inf()};
    
    v2di mask1 = vector_comparisons(vec1, vec2);
    v2di mask2 = vector_comparisons(vec_nan, vec_mixed);
    v2di mask3 = vector_comparisons(vec_nan, vec_nan);
    
    total += mask1[0] + mask1[1];
    total += mask2[0] + mask2[1];
    total += mask3[0] + mask3[1];
    
    /* Additional inline assembly with different condition codes */
    double a = 1.5, b = 2.5, c = __builtin_nan("");
    
    /* Test UNORDERED explicitly */
    int unordered_flag;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"          /* UNORDERED */
        "movzbl %%al, %0"
        : "=r"(unordered_flag)
        : "x"(a), "x"(c)
        : "al", "cc"
    );
    total += unordered_flag;
    
    /* Test ORDERED */
    int ordered_flag;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"         /* ORDERED */
        "movzbl %%al, %0"
        : "=r"(ordered_flag)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    total += ordered_flag;
    
    /* Test UNLT (unordered or less than) */
    int unlt_flag;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"          /* UNLT (CF=1) */
        "movzbl %%al, %0"
        : "=r"(unlt_flag)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    total += unlt_flag;
    
    /* Test UNLE (unordered or less or equal) */
    int unle_flag;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"         /* UNLE (CF=1 or ZF=1) */
        "movzbl %%al, %0"
        : "=r"(unle_flag)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    total += unle_flag;
    
    /* Test UNEQ (unordered or equal) */
    int uneq_flag;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"          /* UNEQ (ZF=1) - actually EQ, unordered case handled separately */
        "movzbl %%al, %0"
        : "=r"(uneq_flag)
        : "x"(a), "x"(a)  /* Equal values */
        : "al", "cc"
    );
    total += uneq_flag;
    
    /* Test LTGT (less than or greater than) */
    int ltgt_flag;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"          /* Not less or equal (greater) */
        "movzbl %%al, %0"
        : "=r"(ltgt_flag)
        : "x"(b), "x"(a)  /* b > a */
        : "al", "cc"
    );
    total += ltgt_flag;
    
    return total;
}

int main(void) {
    printf("Starting FP condition code stress test...\n");
    
    int result = fp_condition_stress();
    
    printf("Result checksum: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
