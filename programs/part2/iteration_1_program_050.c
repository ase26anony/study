/* 
 * FP comparison stress test for GCC x86 backend coverage
 * Compile with: gcc -std=c99 -O2 -march=x86-64 -ffp-contract=off -fno-trapping-math
 * Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math
 * And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Global volatile variables to prevent constant folding */
volatile double g_nan = __builtin_nan("");
volatile double g_inf = __builtin_inf();
volatile double g_neg_inf = -__builtin_inf();
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg_one = -1.0;

/* Function with complex control flow using goto */
static int fp_comparison_stress(double a, double b, double c, double d) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v3 = c;
    volatile double v4 = d;
    
    int result = 0;
    
    /* Matrix of all possible comparisons */
    
    /* UNORDERED cases - comparisons involving NaN */
    if (v1 != v1) {  /* v1 is NaN check */
        result |= 1;
        goto unordered_block;
    }
    
    if (v2 == g_nan) {  /* Always false, but generates UNORDERED */
        result |= 2;
    }
    
unordered_block:
    if (v3 < g_nan) {  /* UNORDERED comparison */
        result |= 4;
    }
    
    if (g_nan >= v4) {  /* Another UNORDERED */
        result |= 8;
    }
    
    /* ORDERED cases - normal comparisons */
    if (v1 == v2) {  /* EQ */
        result |= 16;
        goto ordered_block;
    }
    
    if (v1 != v2) {  /* NEQ/UNEQ */
        result |= 32;
    }
    
ordered_block:
    if (v1 < v2) {  /* LT */
        result |= 64;
        goto lt_block;
    }
    
lt_block:
    if (v2 <= v3) {  /* LE */
        result |= 128;
    }
    
    if (v3 > v4) {  /* GT */
        result |= 256;
    }
    
    if (v4 >= v1) {  /* GE */
        result |= 512;
    }
    
    /* UNEQ - unordered or equal */
    double temp = v1;
    if (temp != temp) {  /* If temp is NaN */
        temp = v2;
    }
    if (temp == v3) {  /* Could be UNEQ */
        result |= 1024;
    }
    
    /* UNGE - unordered or greater than or equal */
    if (!(v1 < v2)) {  /* NOT LT = GE or UNORDERED */
        result |= 2048;
    }
    
    /* UNGT - unordered or greater than */
    if (!(v1 <= v2)) {  /* NOT LE = GT or UNORDERED */
        result |= 4096;
    }
    
    /* UNLE - unordered or less than or equal */
    if (v1 <= v2 || v1 != v1) {  /* Explicit UNLE pattern */
        result |= 8192;
    }
    
    /* UNLT - unordered or less than */
    if (v1 < v2 || v1 != v1) {  /* Explicit UNLT pattern */
        result |= 16384;
    }
    
    /* LTGT - less than or greater than (ordered and not equal) */
    if (v1 < v2 || v1 > v2) {  /* LTGT pattern */
        result |= 32768;
    }
    
    return result;
}

/* Function with vectorized comparisons */
static v2di vector_fp_comparisons(v2df a, v2df b) {
    /* Generate all vector comparison conditions */
    v2di mask1 = (v2di)(a == b);   /* EQ */
    v2di mask2 = (v2di)(a != b);   /* NEQ/UNEQ */
    v2di mask3 = (v2di)(a < b);    /* LT */
    v2di mask4 = (v2di)(a <= b);   /* LE */
    v2di mask5 = (v2di)(a > b);    /* GT */
    v2di mask6 = (v2di)(a >= b);   /* GE */
    
    /* Combine masks */
    return mask1 + mask2 + mask3 + mask4 + mask5 + mask6;
}

/* Function with inline assembly using condition codes */
static int inline_asm_fp_comparisons(double a, double b) {
    int result = 0;
    char cc_result;
    
    /* UNORDERED/ORDERED test with setp (parity flag) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 0;
    
    /* UNEQ test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 1;
    
    /* UNLT test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 2;
    
    /* UNLE test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 3;
    
    /* UNGT test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 4;
    
    /* UNGE test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 5;
    
    /* LTGT test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (cc_result & 1) << 6;
    
    return result;
}

/* Complex control flow with conditional moves */
static double conditional_move_fp(double a, double b, double c, double d) {
    volatile double x = a;
    volatile double y = b;
    volatile double z = c;
    volatile double w = d;
    
    double result = 0.0;
    
    /* Use ternary operator for conditional moves */
    result = (x == y) ? a : b;
    result += (x != y) ? c : d;
    result += (x < y) ? a : -b;
    result += (x <= y) ? c : -d;
    result += (x > y) ? a : b;
    result += (x >= y) ? c : d;
    
    /* NaN comparisons for unordered cases */
    result += (x == g_nan) ? 0.0 : 1.0;
    result += (g_nan == y) ? 0.0 : 2.0;
    result += (x < g_nan) ? 0.0 : 3.0;
    result += (g_nan > y) ? 0.0 : 4.0;
    
    return result;
}

/* Main test function */
int main(void) {
    int checksum = 0;
    
    /* Initialize test values */
    double normal_vals[] = {0.0, 1.0, -1.0, 2.5, -3.75};
    double special_vals[] = {
        __builtin_inf(),
        -__builtin_inf(),
        __builtin_nan(""),
        0.0/0.0  /* Another NaN */
    };
    
    /* Test scalar comparisons */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            checksum ^= fp_comparison_stress(
                normal_vals[i],
                normal_vals[j],
                normal_vals[(i+1)%5],
                normal_vals[(j+1)%5]
            );
        }
    }
    
    /* Test with NaN and infinity */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            checksum ^= fp_comparison_stress(
                special_vals[i],
                special_vals[j],
                special_vals[(i+1)%4],
                special_vals[(j+1)%4]
            );
        }
    }
    
    /* Test vectorized comparisons */
    v2df vec_a = {1.0, 2.0};
    v2df vec_b = {2.0, 1.0};
    v2df vec_c = {__builtin_nan(""), 3.0};
    v2df vec_d = {4.0, __builtin_nan("")};
    
    v2di vec_result1 = vector_fp_comparisons(vec_a, vec_b);
    v2di vec_result2 = vector_fp_comparisons(vec_c, vec_d);
    
    checksum ^= vec_result1[0] ^ vec_result1[1];
    checksum ^= vec_result2[0] ^ vec_result2[1];
    
    /* Test inline assembly */
    checksum ^= inline_asm_fp_comparisons(1.0, 2.0);
    checksum ^= inline_asm_fp_comparisons(__builtin_nan(""), 2.0);
    checksum ^= inline_asm_fp_comparisons(1.0, __builtin_nan(""));
    checksum ^= inline_asm_fp_comparisons(__builtin_nan(""), __builtin_nan(""));
    
    /* Test conditional moves */
    double cmov_result = 0.0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            cmov_result += conditional_move_fp(
                normal_vals[i],
                normal_vals[j],
                normal_vals[(i+2)%5],
                normal_vals[(j+2)%5]
            );
        }
    }
    
    /* Mix cmov_result into checksum */
    checksum ^= *(int*)&cmov_result;
    checksum ^= *((int*)&cmov_result + 1);
    
    /* Array operations with loops to prevent optimization */
    double arr1[8], arr2[8];
    int mask_results[8];
    
    for (int i = 0; i < 8; i++) {
        arr1[i] = i * 0.5;
        arr2[i] = (i % 2 == 0) ? i * 0.5 : __builtin_nan("");
    }
    
    /* Loop with various comparisons */
    for (int i = 0; i < 8; i++) {
        int mask = 0;
        mask |= (arr1[i] == arr2[i]) ? 1 : 0;
        mask |= (arr1[i] != arr2[i]) ? 2 : 0;
        mask |= (arr1[i] < arr2[i]) ? 4 : 0;
        mask |= (arr1[i] <= arr2[i]) ? 8 : 0;
        mask |= (arr1[i] > arr2[i]) ? 16 : 0;
        mask |= (arr1[i] >= arr2[i]) ? 32 : 0;
        mask_results[i] = mask;
        checksum ^= mask;
    }
    
    /* Complex control flow with goto based on FP comparisons */
    volatile double x = 1.0;
    volatile double y = __builtin_nan("");
    
    if (x == x) {
        goto label1;
    }
    
    checksum += 1000;
    
label1:
    if (y != y) {
        goto label2;
    }
    
    checksum += 2000;
    
label2:
    if (x < y || y != y) {  /* UNLT pattern */
        goto label3;
    }
    
    checksum += 3000;
    
label3:
    if (!(x > y)) {  /* UNLE pattern */
        goto label4;
    }
    
    checksum += 4000;
    
label4:
    if (!(x <= y)) {  /* UNGT pattern */
        goto label5;
    }
    
    checksum += 5000;
    
label5:
    if (!(x < y)) {  /* UNGE pattern */
        checksum += 6000;
    }
    
    /* Final output to prevent dead code elimination */
    printf("FP comparison stress test complete. Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
