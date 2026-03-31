/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_conds fp_conds.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_conds_vec fp_conds.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_conds_32 fp_conds.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Stress function with exhaustive FP comparisons */
static int fp_comparison_stress(double d1, double d2, double d3, double d4) {
    volatile double vd1 = d1;
    volatile double vd2 = d2;
    volatile double vd3 = d3;
    volatile double vd4 = d4;
    
    int result = 0;
    double nan = __builtin_nan("");
    double inf = __builtin_inf();
    double neg_inf = -__builtin_inf();
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* Normal vs Normal comparisons */
    if (vd1 == vd2) result ^= 1;
    if (vd1 != vd2) result ^= 2;
    if (vd1 < vd2)  result ^= 4;
    if (vd1 <= vd2) result ^= 8;
    if (vd1 > vd2)  result ^= 16;
    if (vd1 >= vd2) result ^= 32;
    
    /* Normal vs NaN (unordered comparisons) */
    if (vd1 == nan) result ^= 64;      /* UNORDERED/UNEQ path */
    if (vd1 != nan) result ^= 128;     /* UNORDERED/LTGT path */
    if (vd1 < nan)  result ^= 256;     /* UNORDERED/UNLT path */
    if (vd1 <= nan) result ^= 512;     /* UNORDERED/UNLE path */
    if (vd1 > nan)  result ^= 1024;    /* UNORDERED/UNGT path */
    if (vd1 >= nan) result ^= 2048;    /* UNORDERED/UNGE path */
    
    /* NaN vs NaN */
    if (nan == nan) result ^= 4096;    /* UNORDERED path */
    if (nan != nan) result ^= 8192;    /* ORDERED/LTGT path */
    
    /* Infinity comparisons */
    if (vd1 == inf) result ^= 16384;
    if (vd1 < inf)  result ^= 32768;
    if (neg_inf < vd1) result ^= 65536;
    
    /* Complex conditional expressions with ?: operator */
    double cond_result = (vd1 < vd2) ? vd3 : vd4;
    result ^= *(int*)&cond_result;
    
    /* Nested comparisons to prevent optimization */
    if ((vd1 < vd2) && (vd3 > vd4)) {
        result ^= 131072;
    }
    
    if ((vd1 != vd2) || (vd3 == vd4)) {
        result ^= 262144;
    }
    
    /* Goto-based control flow with FP conditions */
    if (vd1 < vd2) goto label1;
    if (vd1 > vd2) goto label2;
    if (vd1 == vd2) goto label3;
    
    /* Unreachable but forces condition code generation */
    result ^= 524288;
    
label1:
    result ^= 1048576;
    goto label4;
    
label2:
    result ^= 2097152;
    goto label4;
    
label3:
    result ^= 4194304;
    
label4:
    /* More unordered comparisons with volatile */
    volatile double vnan = nan;
    if (vnan < vd1) result ^= 8388608;    /* UNORDERED */
    if (vd1 < vnan) result ^= 16777216;   /* UNORDERED */
    
    return result;
}

/* Function with vectorized comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), 3.0};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    int result = 0;
    
    /* Vector comparisons generate cmppd/ucomisd with condition codes */
    v2di cmp_eq = (v2di)(vec1 == vec2);
    v2di cmp_lt = (v2di)(vec1 < vec2);
    v2di cmp_le = (v2di)(vec1 <= vec2);
    v2di cmp_gt = (v2di)(vec1 > vec2);
    v2di cmp_ge = (v2di)(vec1 >= vec2);
    v2di cmp_neq = (v2di)(vec1 != vec2);
    
    /* Vector comparisons with NaN (unordered) */
    v2di cmp_nan_eq = (v2di)(vec1 == vec_nan);
    v2di cmp_nan_lt = (v2di)(vec1 < vec_nan);
    v2di cmp_nan_le = (v2di)(vec1 <= vec_nan);
    
    /* Extract results to prevent optimization */
    result ^= cmp_eq[0] ^ cmp_eq[1];
    result ^= cmp_lt[0] ^ cmp_lt[1];
    result ^= cmp_nan_eq[0] ^ cmp_nan_eq[1];
    
    /* Loop with vector comparisons */
    double arr1[4] = {1.0, 2.0, __builtin_nan(""), 4.0};
    double arr2[4] = {2.0, 1.0, 3.0, __builtin_nan("")};
    
    for (int i = 0; i < 4; i++) {
        v2df v1 = {arr1[i], arr1[(i+1)%4]};
        v2df v2 = {arr2[i], arr2[(i+1)%4]};
        v2di cmp = (v2di)(v1 < v2);
        result ^= cmp[0];
        result ^= cmp[1];
    }
    
    return result;
}

/* Function with inline assembly using condition codes */
static int asm_fp_conditions(double a, double b) {
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that reads FP condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "sete %%ah\n\t"
        "setb %%cl\n\t"
        "seta %%dl"
        : "=a"(cc_result), "=c"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    result ^= cc_result;
    
    /* More assembly with different condition codes */
    int cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "movl $0, %0\n\t"
        "cmovbel %3, %0"
        : "=r"(cmov_result)
        : "x"(a), "x"(b), "r"(42)
        : "cc"
    );
    
    result ^= cmov_result;
    
    return result;
}

/* Main function that exercises all comparison paths */
int main(void) {
    double normal1 = 1.5;
    double normal2 = 2.5;
    double normal3 = -3.5;
    double normal4 = 0.0;
    
    int checksum = 0;
    
    /* Exercise scalar comparisons */
    checksum ^= fp_comparison_stress(normal1, normal2, normal3, normal4);
    checksum ^= fp_comparison_stress(__builtin_nan(""), normal1, normal2, normal3);
    checksum ^= fp_comparison_stress(__builtin_inf(), -__builtin_inf(), normal1, normal2);
    
    /* Exercise vector comparisons */
    checksum ^= vector_fp_comparisons();
    
    /* Exercise inline assembly paths */
    checksum ^= asm_fp_conditions(normal1, normal2);
    checksum ^= asm_fp_conditions(normal1, __builtin_nan(""));
    checksum ^= asm_fp_conditions(__builtin_inf(), normal1);
    
    /* Additional unordered comparison patterns */
    volatile double vnan = __builtin_nan("");
    volatile double vinf = __builtin_inf();
    
    /* These should trigger UNORDERED condition codes */
    if (!(vnan < normal1)) checksum ^= 1;
    if (!(normal1 < vnan)) checksum ^= 2;
    if (!(vnan <= normal1)) checksum ^= 4;
    if (!(normal1 <= vnan)) checksum ^= 8;
    if (!(vnan > normal1)) checksum ^= 16;
    if (!(normal1 > vnan)) checksum ^= 32;
    
    /* ORDERED comparisons */
    if (normal1 == normal1) checksum ^= 64;
    if (normal1 <= normal1) checksum ^= 128;
    if (normal1 >= normal1) checksum ^= 256;
    
    /* UNEQ comparisons (equal or unordered) */
    double un_eq_test = (vnan == normal1) ? 1.0 : 0.0;
    checksum ^= *(int*)&un_eq_test;
    
    /* LTGT comparisons (less or greater, but not equal/unordered) */
    double ltgt_test = (normal1 != normal2) ? 1.0 : 0.0;
    checksum ^= *(int*)&ltgt_test;
    
    /* Complex expression mixing ordered and unordered */
    checksum ^= ((normal1 < normal2) && !(vnan < normal1)) ? 512 : 1024;
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
