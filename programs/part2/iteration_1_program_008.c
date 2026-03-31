/* Compile with: gcc -std=c99 -O2 -march=x86-64 -ffp-contract=off -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -march=i686 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Stress function with exhaustive FP comparisons */
static int fp_comparison_stress(double d1, double d2, double d3, double nan_val, double inf_val, double neg_inf_val) {
    volatile double vd1 = d1;
    volatile double vd2 = d2;
    volatile double vnan = nan_val;
    volatile double vinf = inf_val;
    volatile double vneg_inf = neg_inf_val;
    
    int result = 0;
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* 1. Normal number comparisons (ORDERED cases) */
    if (vd1 < vd2) result ^= 1;      /* LT */
    if (vd1 <= vd2) result ^= 2;     /* LE */
    if (vd1 > vd2) result ^= 4;      /* GT */
    if (vd1 >= vd2) result ^= 8;     /* GE */
    if (vd1 == vd2) result ^= 16;    /* EQ */
    if (vd1 != vd2) result ^= 32;    /* NEQ */
    
    /* 2. Comparisons with NaN (UNORDERED cases) */
    if (vd1 == vnan) result ^= 64;   /* Should be false, may be UNORDERED */
    if (vd1 != vnan) result ^= 128;  /* Should be true, may be UNORDERED */
    if (vd1 < vnan) result ^= 256;   /* UNORDERED */
    if (vd1 <= vnan) result ^= 512;  /* UNORDERED */
    if (vd1 > vnan) result ^= 1024;  /* UNORDERED */
    if (vd1 >= vnan) result ^= 2048; /* UNORDERED */
    
    /* 3. NaN vs NaN comparisons */
    if (vnan == vnan) result ^= 4096;    /* Always false, UNORDERED */
    if (vnan != vnan) result ^= 8192;    /* Always true, UNEQ? Actually UNORDERED */
    
    /* 4. Comparisons with infinity */
    if (vd1 < vinf) result ^= 16384;
    if (vd1 > vneg_inf) result ^= 32768;
    if (vinf == vinf) result ^= 65536;
    if (vinf > vneg_inf) result ^= 131072;
    
    /* 5. Conditional moves using FP comparison results */
    double cmov_result = (vd1 < vd2) ? d1 : d2;
    result ^= (int)(cmov_result * 1000);
    
    cmov_result = (vd1 != vnan) ? d1 : d2;
    result ^= (int)(cmov_result * 1000);
    
    /* Complex control flow with goto to prevent optimization */
    if (vd1 < vd2) goto label1;
    if (vd1 == vnan) goto label2;
    
    /* Fall through */
    result += 1000;
    goto label3;
    
label1:
    result += 2000;
    goto label3;
    
label2:
    result += 3000;
    /* Continue to label3 */
    
label3:
    /* More comparisons in switch-like pattern */
    int cmp_val = 0;
    if (vd1 < vd2) cmp_val = 1;
    else if (vd1 <= vd2) cmp_val = 2;
    else if (vd1 > vd2) cmp_val = 3;
    else if (vd1 >= vd2) cmp_val = 4;
    else if (vd1 == vd2) cmp_val = 5;
    else if (vd1 != vd2) cmp_val = 6;
    else if (vd1 < vnan) cmp_val = 7;  /* UNORDERED */
    else if (vd1 == vnan) cmp_val = 8; /* UNORDERED */
    
    result ^= cmp_val;
    
    return result;
}

/* Function with vectorized FP comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), 3.0};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    /* Various vector comparisons that generate cmppd/ucomisd with condition codes */
    v2di mask;
    
    /* EQ comparison */
    mask = (v2di)(vec1 == vec2);
    
    /* NE comparison */
    mask = (v2di)(vec1 != vec2);
    
    /* LT comparison */
    mask = (v2di)(vec1 < vec2);
    
    /* LE comparison */
    mask = (v2di)(vec1 <= vec2);
    
    /* GT comparison */
    mask = (v2di)(vec1 > vec2);
    
    /* GE comparison */
    mask = (v2di)(vec1 >= vec2);
    
    /* Comparisons with NaN (unordered) */
    mask = (v2di)(vec1 == vec_nan);
    mask = (v2di)(vec1 != vec_nan);
    mask = (v2di)(vec1 < vec_nan);
    mask = (v2di)(vec_nan == vec_nan);
    
    /* Comparisons with infinity */
    mask = (v2di)(vec1 < vec_inf);
    mask = (v2di)(vec1 > vec_inf);
    
    /* Loop with vector comparisons */
    double arr1[4] = {1.0, 2.0, 3.0, 4.0};
    double arr2[4] = {4.0, 3.0, 2.0, 1.0};
    int cmp_results[4] = {0};
    
    for (int i = 0; i < 4; i++) {
        v2df a = {arr1[i], arr1[(i+1)%4]};
        v2df b = {arr2[i], arr2[(i+1)%4]};
        v2di m = (v2di)(a < b);
        cmp_results[i] = m[0] | m[1];
    }
    
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum ^= cmp_results[i];
    }
    
    return sum;
}

/* Function with inline assembly using FP condition codes */
static int asm_fp_condition_codes(double a, double b, double nan_val) {
    int result = 0;
    char cc_result;
    
    /* ucomisd with setp (parity/UNORDERED) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(nan_val)
        : "cc"
    );
    result ^= cc_result;
    
    /* ucomisd with seta (above/UNORDERED or GT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 8);
    
    /* ucomisd with setb (below/UNORDERED or LT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 16);
    
    /* ucomisd with sete (equal/EQ or UNEQ) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 24);
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "fcmovbe %3, %0"
        : "=t"(cmov_result)
        : "u"(a), "u"(b), "t"(nan_val)
        : "cc"
    );
    result ^= (int)(cmov_result * 100);
    
    return result;
}

int main(void) {
    /* Initialize FP values */
    double normal1 = 1.5;
    double normal2 = 2.5;
    double normal3 = 1.5;  /* Same as normal1 for equality test */
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    
    printf("Starting FP comparison stress test...\n");
    
    /* Exhaustive comparison matrix */
    int result = 0;
    
    /* Test all combinations of value pairs */
    double test_values[6] = {normal1, normal2, normal3, nan_val, inf_val, neg_inf_val};
    const char* value_names[6] = {"normal1", "normal2", "normal3", "nan", "inf", "-inf"};
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            printf("Testing %s vs %s: ", value_names[i], value_names[j]);
            int r = fp_comparison_stress(test_values[i], test_values[j], 
                                        test_values[(i+1)%6], nan_val, inf_val, neg_inf_val);
            result ^= r;
            printf("result xor = %d\n", r);
        }
    }
    
    /* Vectorized comparisons */
    printf("\nTesting vectorized FP comparisons...\n");
    int vec_result = vector_fp_comparisons();
    result ^= vec_result;
    printf("Vector result: %d\n", vec_result);
    
    /* Inline assembly with condition codes */
    printf("\nTesting inline assembly with FP condition codes...\n");
    int asm_result = asm_fp_condition_codes(normal1, normal2, nan_val);
    result ^= asm_result;
    printf("Assembly result: %d\n", asm_result);
    
    /* Additional unordered comparison patterns */
    printf("\nTesting specific unordered patterns...\n");
    
    /* Pattern 1: Direct unordered checks */
    volatile double x = normal1;
    volatile double y = nan_val;
    
    /* These should generate UNORDERED condition code usage */
    if (!(x < y) && !(x > y) && !(x == y)) {
        /* Either unordered or equal, but with NaN it's unordered */
        result ^= 0x5555;
    }
    
    /* Pattern 2: Ordered check */
    if (x == x && y == y) {  /* y is NaN, so false */
        result ^= 0xAAAA;
    }
    
    /* Pattern 3: UNEQ (unordered or equal) */
    if (!(x < y) && !(x > y)) {  /* Includes unordered case */
        result ^= 0x3333;
    }
    
    /* Pattern 4: LTGT (less or greater, but not equal and not unordered) */
    if ((x < y) || (x > y)) {  /* With NaN, neither is true */
        result ^= 0xCCCC;
    }
    
    /* Pattern 5: UNLT (unordered or less than) */
    if (x < y || !(x >= y)) {  /* With NaN, !(x >= y) is true */
        result ^= 0x6666;
    }
    
    /* Pattern 6: UNLE (unordered or less than or equal) */
    if (x <= y || !(x > y)) {  /* With NaN, !(x > y) is true */
        result ^= 0x7777;
    }
    
    /* Pattern 7: UNGT (unordered or greater than) */
    if (x > y || !(x <= y)) {  /* With NaN, !(x <= y) is true */
        result ^= 0x8888;
    }
    
    /* Pattern 8: UNGE (unordered or greater than or equal) */
    if (x >= y || !(x < y)) {  /* With NaN, !(x < y) is true */
        result ^= 0x9999;
    }
    
    printf("\nFinal checksum: %d\n", result);
    printf("Test completed.\n");
    
    return result != 0 ? 0 : 1;  /* Return 0 if result is non-zero */
}
