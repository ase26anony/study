/* fp_condition_stress.c - Exhaustively test FP comparison condition code generation */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent constant folding and optimization */
static volatile double sink;

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function with complex control flow to prevent optimization */
static int __attribute__((noinline)) 
stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf) 
{
    int result = 0;
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double vnan = nan_val;
    volatile double vinf = inf_val;
    volatile double vneginf = neg_inf;
    
    /* Matrix of comparisons to generate various condition codes */
    
    /* UNORDERED cases - comparisons with NaN */
    if (vnan == v1) {
        result ^= 1;
        goto label_unordered;
    }
    
    if (v1 < vnan) {
        result ^= 2;
    }
    
    if (vnan <= v2) {
        result ^= 4;
    }
    
    if (vnan > vinf) {
        result ^= 8;
    }
    
    if (vneginf >= vnan) {
        result ^= 16;
    }
    
    if (vnan != vnan) {  /* Always true for NaN != NaN */
        result ^= 32;
    }
    
label_unordered:
    /* ORDERED cases - normal comparisons */
    if (v1 == v2) {
        result ^= 64;
        goto label_ordered;
    }
    
    if (v1 < v2) {
        result ^= 128;
    }
    
    if (v1 <= v2) {
        result ^= 256;
    }
    
    if (v1 > v2) {
        result ^= 512;
    }
    
    if (v1 >= v2) {
        result ^= 1024;
    }
    
    if (v1 != v2) {
        result ^= 2048;
    }
    
label_ordered:
    /* UNEQ cases - unordered or equal */
    if (!(vnan < v1) && !(vnan > v1)) {  /* UNEQ: unordered or equal */
        result ^= 4096;
    }
    
    /* UNGE cases - unordered or greater-or-equal */
    if (!(v1 < vnan)) {  /* UNGE: not less (unordered or greater-or-equal) */
        result ^= 8192;
    }
    
    /* UNGT cases - unordered or greater */
    if (!(v1 <= vnan)) {  /* UNGT: not less-or-equal (unordered or greater) */
        result ^= 16384;
    }
    
    /* UNLE cases - unordered or less-or-equal */
    if (vnan <= v1 || vnan != vnan) {  /* UNLE: unordered or less-or-equal */
        result ^= 32768;
    }
    
    /* UNLT cases - unordered or less */
    if (vnan < v1 || vnan != vnan) {  /* UNLT: unordered or less */
        result ^= 65536;
    }
    
    /* LTGT cases - less or greater (ordered and not equal) */
    if ((v1 < v2) || (v1 > v2)) {  /* LTGT: less or greater (ordered) */
        result ^= 131072;
    }
    
    /* Conditional moves using FP comparison results */
    double cmov_result = (v1 < v2) ? v1 : v2;
    cmov_result = (vnan == v1) ? vnan : cmov_result;
    cmov_result = (v1 != v1) ? vinf : cmov_result;
    
    sink = cmov_result;
    
    return result;
}

/* Function with vectorized comparisons */
static int __attribute__((noinline))
stress_vector_comparisons(v2df vec1, v2df vec2, v2df vec_nan)
{
    int result = 0;
    
    /* Various vector comparisons generating different condition codes */
    v2di cmp_eq = (vec1 == vec2);
    v2di cmp_neq = (vec1 != vec2);
    v2di cmp_lt = (vec1 < vec2);
    v2di cmp_le = (vec1 <= vec2);
    v2di cmp_gt = (vec1 > vec2);
    v2di cmp_ge = (vec1 >= vec2);
    
    /* Comparisons with NaN */
    v2di cmp_nan_eq = (vec1 == vec_nan);
    v2di cmp_nan_neq = (vec1 != vec_nan);
    v2di cmp_nan_lt = (vec1 < vec_nan);
    v2di cmp_nan_le = (vec1 <= vec_nan);
    
    /* Extract results to prevent optimization */
    long long* eq_ptr = (long long*)&cmp_eq;
    long long* neq_ptr = (long long*)&cmp_neq;
    
    result ^= (int)(eq_ptr[0] ^ eq_ptr[1]);
    result ^= (int)(neq_ptr[0] ^ neq_ptr[1]);
    
    /* Use __builtin_ia32_cmpsd for explicit condition code control */
    v2df cmp_result;
    
    /* Generate various condition codes via builtin */
    cmp_result = __builtin_ia32_cmpeqsd(vec1, vec2);
    result ^= ((int*)&cmp_result)[0];
    
    cmp_result = __builtin_ia32_cmpltsd(vec1, vec2);
    result ^= ((int*)&cmp_result)[0];
    
    cmp_result = __builtin_ia32_cmplesd(vec1, vec2);
    result ^= ((int*)&cmp_result)[0];
    
    cmp_result = __builtin_ia32_cmpunordsd(vec1, vec_nan);
    result ^= ((int*)&cmp_result)[0];
    
    cmp_result = __builtin_ia32_cmpneqsd(vec1, vec2);
    result ^= ((int*)&cmp_result)[0];
    
    cmp_result = __builtin_ia32_cmpnltsd(vec1, vec2);
    result ^= ((int*)&cmp_result)[0];
    
    cmp_result = __builtin_ia32_cmpnlesd(vec1, vec2);
    result ^= ((int*)&cmp_result)[0];
    
    cmp_result = __builtin_ia32_cmpordsd(vec1, vec2);
    result ^= ((int*)&cmp_result)[0];
    
    return result;
}

/* Function with inline assembly using condition codes */
static int __attribute__((noinline))
stress_asm_condition_codes(double a, double b, double nan_val)
{
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that reads FP condition codes */
    
    /* UNORDERED/ORDERED test (parity flag) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(nan_val)
        : "cc"
    );
    result ^= (cc_result << 0);
    
    /* Less than test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 1);
    
    /* Equal test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 2);
    
    /* Less or equal test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 3);
    
    /* Not less than (UNGE) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 4);
    
    /* Not equal (LTGT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 5);
    
    /* Not less or equal (UNGT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 6);
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "cmova %3, %0"
        : "=r"(cmov_result)
        : "x"(a), "x"(b), "r"(a)
        : "cc"
    );
    sink = cmov_result;
    
    return result;
}

/* Complex control flow with goto to prevent optimization */
static int __attribute__((noinline))
stress_complex_flow(double a, double b, double nan_val)
{
    int result = 0;
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    
    /* Jump table style with FP comparisons */
    if (x < y) goto case_lt;
    if (x == y) goto case_eq;
    if (x != x) goto case_unordered;
    if (nan < x) goto case_nan_lt;
    
    goto default_case;
    
case_lt:
    result = 1;
    if (y < x) goto case_gt;  /* Never taken, but creates control flow */
    goto end;
    
case_eq:
    result = 2;
    if (nan == nan) goto case_unordered;  /* Never true */
    goto end;
    
case_gt:
    result = 3;
    goto end;
    
case_unordered:
    result = 4;
    /* More unordered comparisons */
    if (nan == x) result ^= 8;
    if (x < nan) result ^= 16;
    if (nan <= y) result ^= 32;
    goto end;
    
case_nan_lt:
    result = 5;
    goto end;
    
default_case:
    result = 6;
    /* All remaining comparisons */
    if (!(x < y) && !(x > y) && (x == x) && (y == y)) {
        result ^= 64;  /* EQ and ORDERED */
    }
    goto end;
    
end:
    return result;
}

int main(void)
{
    int total_result = 0;
    
    /* Initialize FP special values */
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf = -__builtin_inf();
    double zero = 0.0;
    double one = 1.0;
    double neg_one = -1.0;
    
    /* Test various value combinations */
    double test_values[] = {zero, one, neg_one, inf_val, neg_inf, nan_val};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Exhaustive pairwise comparisons */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            total_result ^= stress_fp_comparisons(
                test_values[i], 
                test_values[j],
                nan_val,
                inf_val,
                neg_inf
            );
        }
    }
    
    /* Vector comparisons */
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {3.0, 4.0};
    v2df vec_nan = {nan_val, nan_val};
    
    total_result ^= stress_vector_comparisons(vec1, vec2, vec_nan);
    
    /* More vector combinations */
    v2df vec_inf = {inf_val, neg_inf};
    total_result ^= stress_vector_comparisons(vec1, vec_inf, vec_nan);
    
    /* Inline assembly condition code tests */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            total_result ^= stress_asm_condition_codes(
                test_values[i],
                test_values[j],
                nan_val
            );
        }
    }
    
    /* Complex control flow tests */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            total_result ^= stress_complex_flow(
                test_values[i],
                test_values[j],
                nan_val
            );
        }
    }
    
    /* Loop with array comparisons to generate more code */
    double arr1[10], arr2[10];
    for (int i = 0; i < 10; i++) {
        arr1[i] = (double)i;
        arr2[i] = (double)(i * 2);
    }
    
    arr1[5] = nan_val;
    arr2[7] = nan_val;
    
    for (int i = 0; i < 10; i++) {
        volatile double tmp1 = arr1[i];
        volatile double tmp2 = arr2[i];
        
        if (tmp1 < tmp2) total_result += i;
        if (tmp1 == tmp2) total_result -= i;
        if (tmp1 != tmp1) total_result ^= i;  /* NaN check */
        if (tmp2 != tmp2) total_result ^= (i << 4);
        
        /* Generate UNGE, UNGT, etc. */
        if (!(tmp1 < tmp2)) total_result += (i << 8);
        if (!(tmp1 <= tmp2)) total_result += (i << 12);
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result checksum: %d\n", total_result);
    
    return total_result != 0 ? 0 : 1;
}
