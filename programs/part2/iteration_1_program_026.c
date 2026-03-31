/* 
 * FP comparison stress test for GCC x86 backend
 * Compile with: gcc -std=c99 -O2 -march=x86-64 -ffp-contract=off -o fp_test fp_test.c
 * Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math
 * And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Global volatile variables to prevent constant folding */
volatile double g_nan = __builtin_nan("");
volatile double g_inf = __builtin_inf();
volatile double g_ninf = -__builtin_inf();
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg = -1.0;

/* Function with complex control flow using goto */
static int fp_comparison_stress(double a, double b, double c, double d) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v3 = c;
    volatile double v4 = d;
    
    int result = 0;
    
    /* Block 1: Direct comparisons with all conditions */
    if (v1 == v2) result |= 1;
    if (v1 != v2) result |= 2;
    if (v1 < v2)  result |= 4;
    if (v1 <= v2) result |= 8;
    if (v1 > v2)  result |= 16;
    if (v1 >= v2) result |= 32;
    
    /* Block 2: Comparisons involving NaN (triggers UNORDERED paths) */
    double nan = g_nan;
    if (v1 == nan) result |= 64;      /* UNORDERED/UNEQ */
    if (v1 != nan) result |= 128;     /* ORDERED/LTGT */
    if (v1 < nan)  result |= 256;     /* UNORDERED/UNLT */
    if (nan <= v2) result |= 512;     /* UNORDERED/UNLE */
    if (nan > v3)  result |= 1024;    /* UNORDERED/UNGT */
    if (nan >= v4) result |= 2048;    /* UNORDERED/UNGE */
    
    /* Block 3: Conditional expressions (generates conditional moves) */
    double cmov_result = (v1 < v2) ? v3 : v4;
    result += (int)cmov_result;
    
    cmov_result = (v1 != nan) ? v2 : v3;
    result += (int)cmov_result;
    
    /* Block 4: Complex if-else chain with goto */
    if (v1 == v2) goto label_eq;
    if (v1 < v2)  goto label_lt;
    if (v1 > v2)  goto label_gt;
    goto label_unordered;
    
label_eq:
    result += 1000;
    goto label_done;
    
label_lt:
    result += 2000;
    goto label_done;
    
label_gt:
    result += 3000;
    goto label_done;
    
label_unordered:
    /* Check for NaN explicitly */
    if (v1 != v1 || v2 != v2) {  /* NaN test */
        result += 4000;
    }
    
label_done:
    return result;
}

/* Function with vectorized comparisons */
static v2di vector_comparisons(v2df a, v2df b) {
    /* Generate all vector comparison conditions */
    v2df cmp_eq  = a == b;   /* EQ */
    v2df cmp_ne  = a != b;   /* NEQ/UNORDERED */
    v2df cmp_lt  = a < b;    /* LT */
    v2df cmp_le  = a <= b;   /* LE */
    v2df cmp_gt  = a > b;    /* GT */
    v2df cmp_ge  = a >= b;   /* GE */
    
    /* Convert to integer masks */
    v2di mask_eq  = (v2di)cmp_eq;
    v2di mask_ne  = (v2di)cmp_ne;
    v2di mask_lt  = (v2di)cmp_lt;
    v2di mask_le  = (v2di)cmp_le;
    v2di mask_gt  = (v2di)cmp_gt;
    v2di mask_ge  = (v2di)cmp_ge;
    
    /* Combine masks */
    v2di result = mask_eq + mask_ne + mask_lt + mask_le + mask_gt + mask_ge;
    return result;
}

/* Function with inline assembly using condition codes */
static int inline_asm_fp_comparisons(double a, double b) {
    int result = 0;
    char cc_result;
    
    /* Test 1: UNORDERED (parity) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 0;
    
    /* Test 2: ORDERED (not parity) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 1;
    
    /* Test 3: UNLT (unordered or less than) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 2;
    
    /* Test 4: UNLE (unordered or less than or equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 3;
    
    /* Test 5: UNGT (unordered or greater than) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 4;
    
    /* Test 6: UNGE (unordered or greater than or equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 5;
    
    /* Test 7: UNEQ (unordered or equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 6;
    
    /* Test 8: LTGT (ordered and not equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 7;
    
    return result;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    /* Test matrix of different value combinations */
    double test_values[] = {
        0.0, 1.0, -1.0, 
        __builtin_inf(), -__builtin_inf(),
        __builtin_nan("")
    };
    
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Test all pairs with scalar comparisons */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            checksum += fp_comparison_stress(
                test_values[i], 
                test_values[j],
                test_values[(i+1) % num_values],
                test_values[(j+1) % num_values]
            );
        }
    }
    
    /* Test vectorized comparisons */
    v2df vec_a = {1.0, 2.0};
    v2df vec_b = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), __builtin_inf()};
    
    v2di vec_result = vector_comparisons(vec_a, vec_b);
    checksum += vec_result[0] + vec_result[1];
    
    vec_result = vector_comparisons(vec_a, vec_nan);
    checksum += vec_result[0] + vec_result[1];
    
    vec_result = vector_comparisons(vec_nan, vec_nan);
    checksum += vec_result[0] + vec_result[1];
    
    /* Test inline assembly condition codes */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            checksum += inline_asm_fp_comparisons(
                test_values[i],
                test_values[j]
            );
        }
    }
    
    /* Additional unordered-specific tests */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("0x1234");
    
    /* These should all be false for ordered comparisons */
    if (!(nan1 == nan2)) checksum += 1;
    if (!(nan1 < 1.0))   checksum += 2;
    if (!(1.0 > nan1))   checksum += 4;
    
    /* These should all be true for unordered comparisons */
    if (nan1 != nan1)    checksum += 8;  /* NaN != NaN */
    if (!(nan1 == 1.0))  checksum += 16;
    if (!(1.0 <= nan1))  checksum += 32;
    
    /* Mixed ordered/unordered comparisons */
    double x = 0.0;
    double y = 0.0;
    double z = -0.0;
    
    /* +0.0 == -0.0 should be true */
    if (x == z) checksum += 64;
    
    /* But they compare equal */
    if (!(x < z) && !(x > z)) checksum += 128;
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
