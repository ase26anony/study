/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_cond_test_vec fp_cond_test.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_cond_test_32 fp_cond_test.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Stress function with exhaustive FP comparisons */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    volatile double volatile_a = a;
    volatile double volatile_b = b;
    volatile double volatile_nan = nan_val;
    volatile double volatile_inf = inf_val;
    volatile double volatile_neg_inf = neg_inf_val;
    
    int result = 0;
    int temp_result = 0;
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* 1. Normal number comparisons */
    if (volatile_a < volatile_b) result ^= 1;
    if (volatile_a <= volatile_b) result ^= 2;
    if (volatile_a > volatile_b) result ^= 4;
    if (volatile_a >= volatile_b) result ^= 8;
    if (volatile_a == volatile_b) result ^= 16;
    if (volatile_a != volatile_b) result ^= 32;
    
    /* 2. Comparisons with NaN (triggers UNORDERED paths) */
    if (volatile_a == volatile_nan) result ^= 64;      /* Always false, but compiler doesn't know */
    if (volatile_a != volatile_nan) result ^= 128;     /* Always true for non-NaN a */
    if (volatile_a < volatile_nan) result ^= 256;      /* Unordered */
    if (volatile_a <= volatile_nan) result ^= 512;     /* Unordered */
    if (volatile_a > volatile_nan) result ^= 1024;     /* Unordered */
    if (volatile_a >= volatile_nan) result ^= 2048;    /* Unordered */
    
    /* 3. NaN vs NaN comparisons */
    if (volatile_nan == volatile_nan) result ^= 4096;  /* Always false */
    if (volatile_nan != volatile_nan) result ^= 8192;  /* Always true */
    
    /* 4. Comparisons with infinity */
    if (volatile_a < volatile_inf) result ^= 16384;
    if (volatile_neg_inf < volatile_a) result ^= 32768;
    if (volatile_inf == volatile_inf) result ^= 65536;
    
    /* 5. Conditional moves based on FP comparisons */
    double cmov_result = (volatile_a < volatile_b) ? 1.0 : 2.0;
    result += (int)cmov_result;
    
    cmov_result = (volatile_a != volatile_nan) ? 3.0 : 4.0;
    result += (int)cmov_result;
    
    /* 6. Complex control flow with goto to prevent optimization */
    if (volatile_a < volatile_b) goto label1;
    if (volatile_a == volatile_nan) goto label2;
    
label1:
    result += 100;
    goto label3;
    
label2:
    result += 200;
    
label3:
    /* 7. More unordered comparisons */
    if (!(volatile_a >= volatile_nan)) result += 300;  /* UNLT: ult */
    if (!(volatile_a > volatile_nan)) result += 400;   /* UNLE: ule */
    if (!(volatile_a < volatile_nan)) result += 500;   /* UNGE: nlt */
    if (!(volatile_a <= volatile_nan)) result += 600;  /* UNGT: nle */
    
    /* 8. Ordered comparisons */
    if (volatile_a == volatile_a && volatile_b == volatile_b) result += 700;  /* Both ordered */
    
    return result;
}

/* Function with vectorized FP comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), __builtin_nan("")};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    int result = 0;
    
    /* Vector comparisons generate cmppd/ucomisd with condition codes */
    v2di cmp_result;
    
    /* Equal */
    cmp_result = (vec1 == vec2);
    result += cmp_result[0] + cmp_result[1];
    
    /* Not equal */
    cmp_result = (vec1 != vec2);
    result += cmp_result[0] + cmp_result[1];
    
    /* Less than */
    cmp_result = (vec1 < vec2);
    result += cmp_result[0] + cmp_result[1];
    
    /* Less than or equal */
    cmp_result = (vec1 <= vec2);
    result += cmp_result[0] + cmp_result[1];
    
    /* Greater than */
    cmp_result = (vec1 > vec2);
    result += cmp_result[0] + cmp_result[1];
    
    /* Greater than or equal */
    cmp_result = (vec1 >= vec2);
    result += cmp_result[0] + cmp_result[1];
    
    /* Unordered comparisons with NaN */
    cmp_result = (vec1 == vec_nan);
    result += cmp_result[0] + cmp_result[1];
    
    cmp_result = (vec1 != vec_nan);
    result += cmp_result[0] + cmp_result[1];
    
    cmp_result = (vec1 < vec_nan);
    result += cmp_result[0] + cmp_result[1];
    
    cmp_result = (vec_nan == vec_nan);
    result += cmp_result[0] + cmp_result[1];
    
    /* Comparisons with infinity */
    cmp_result = (vec1 < vec_inf);
    result += cmp_result[0] + cmp_result[1];
    
    cmp_result = (vec1 > vec_inf);
    result += cmp_result[0] + cmp_result[1];
    
    return result;
}

/* Function with inline assembly using condition codes */
static int asm_fp_comparisons(double a, double b, double nan_val) {
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that reads FP condition codes */
    
    /* Test for UNORDERED (parity flag) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(nan_val)
        : "cc"
    );
    result += cc_result;
    
    /* Test for ORDERED (not parity) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 2;
    
    /* Test for less than */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 3;
    
    /* Test for less than or equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 4;
    
    /* Test for not equal (unordered or not equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 5;
    
    /* Test for equal (ordered and equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 6;
    
    /* Conditional move based on FP comparison */
    double cmov_input = 10.0;
    double cmov_output;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "cmova %3, %0"
        : "=r"(cmov_output)
        : "x"(a), "x"(b), "r"(cmov_input)
        : "cc"
    );
    result += (int)cmov_output;
    
    return result;
}

/* Main function that orchestrates all tests */
int main(void) {
    double normal1 = 1.5;
    double normal2 = 2.5;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    double zero = 0.0;
    double neg_zero = -0.0;
    
    int final_result = 0;
    
    printf("Starting FP comparison coverage test...\n");
    
    /* Test 1: Exhaustive scalar comparisons */
    final_result ^= stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    final_result ^= stress_fp_comparisons(normal2, normal1, nan_val, inf_val, neg_inf_val);
    final_result ^= stress_fp_comparisons(normal1, normal1, nan_val, inf_val, neg_inf_val);
    final_result ^= stress_fp_comparisons(zero, neg_zero, nan_val, inf_val, neg_inf_val);
    final_result ^= stress_fp_comparisons(inf_val, normal1, nan_val, inf_val, neg_inf_val);
    final_result ^= stress_fp_comparisons(neg_inf_val, normal1, nan_val, inf_val, neg_inf_val);
    
    /* Test 2: Vectorized comparisons */
    final_result ^= vector_fp_comparisons();
    
    /* Test 3: Inline assembly with condition codes */
    final_result ^= asm_fp_comparisons(normal1, normal2, nan_val);
    final_result ^= asm_fp_comparisons(normal2, normal1, nan_val);
    final_result ^= asm_fp_comparisons(normal1, nan_val, nan_val);
    final_result ^= asm_fp_comparisons(inf_val, normal1, nan_val);
    
    /* Additional unordered comparison patterns */
    {
        volatile double v1 = normal1;
        volatile double v2 = nan_val;
        volatile double v3 = inf_val;
        
        /* UNEQ: ueq - unordered or equal */
        if (!(v1 > v2) && !(v1 < v2)) final_result += 1000;
        
        /* LTGT: une - less than or greater than (ordered and not equal) */
        if ((v1 < v3) || (v1 > v3)) final_result += 2000;
        
        /* UNGE: nlt - unordered or greater than or equal */
        if (!(v1 < v2)) final_result += 3000;
        
        /* UNGT: nle - unordered or greater than */
        if (!(v1 <= v2)) final_result += 4000;
        
        /* UNLE: ule - unordered or less than or equal */
        if (!(v1 > v2)) final_result += 5000;
        
        /* UNLT: ult - unordered or less than */
        if (!(v1 >= v2)) final_result += 6000;
    }
    
    /* Loop with array comparisons to generate more code patterns */
    {
        double arr1[4] = {1.0, 2.0, nan_val, inf_val};
        double arr2[4] = {2.0, 1.0, nan_val, neg_inf_val};
        int mask[4] = {0};
        
        for (int i = 0; i < 4; i++) {
            volatile double* volatile_ptr1 = &arr1[i];
            volatile double* volatile_ptr2 = &arr2[i];
            
            /* Various comparison patterns in loop */
            if (*volatile_ptr1 < *volatile_ptr2) mask[i] |= 1;
            if (*volatile_ptr1 <= *volatile_ptr2) mask[i] |= 2;
            if (*volatile_ptr1 > *volatile_ptr2) mask[i] |= 4;
            if (*volatile_ptr1 >= *volatile_ptr2) mask[i] |= 8;
            if (*volatile_ptr1 == *volatile_ptr2) mask[i] |= 16;
            if (*volatile_ptr1 != *volatile_ptr2) mask[i] |= 32;
            
            final_result += mask[i];
        }
    }
    
    printf("Final checksum: %d\n", final_result);
    printf("Test completed. Check assembly output for condition code usage.\n");
    
    return final_result != 0 ? 0 : 1;
}
