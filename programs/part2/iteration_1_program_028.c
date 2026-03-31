/* fp_condition_stress.c - Exhaustively test FP comparison condition code generation */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent constant folding and optimization */
static volatile int global_counter = 0;

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function with complex control flow to prevent optimization */
__attribute__((noinline))
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    int result = 0;
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* 1. Normal number comparisons */
    if (v1 == v2) {
        result += 1;
        goto label_eq;
    } else {
        result += 2;
    }
    
    if (v1 != v2) {
        result += 4;
        goto label_neq;
    }
    
label_eq:
    if (v1 < v2) {
        result += 8;
        goto label_lt;
    }
    
label_neq:
    if (v1 <= v2) {
        result += 16;
        goto label_le;
    }
    
label_lt:
    if (v1 > v2) {
        result += 32;
        goto label_gt;
    }
    
label_le:
    if (v1 >= v2) {
        result += 64;
        goto label_ge;
    }
    
label_gt:
    /* 2. Comparisons involving NaN (unordered cases) */
    if (v1 == v_nan) {  /* Always false, but compiler must generate code */
        result += 128;
        goto label_nan_eq;
    }
    
label_ge:
    if (v_nan == v2) {
        result += 256;
    }
    
label_nan_eq:
    if (v1 < v_nan) {  /* Unordered comparison */
        result += 512;
        goto label_nan_lt;
    }
    
    if (v_nan < v2) {  /* Unordered comparison */
        result += 1024;
    }
    
label_nan_lt:
    if (v_nan != v_nan) {  /* NaN != NaN is true - triggers UNORDERED */
        result += 2048;
        goto label_nan_neq;
    }
    
    if (v_nan == v_nan) {  /* NaN == NaN is false */
        result += 4096;
    }
    
label_nan_neq:
    /* 3. Comparisons with infinity */
    if (v1 < v_inf) {
        result += 8192;
        goto label_inf_lt;
    }
    
    if (v_inf < v1) {
        result += 16384;
    }
    
label_inf_lt:
    /* 4. Conditional moves based on FP comparisons */
    double cmov_result = (v1 < v2) ? v1 : v2;
    result += (int)(cmov_result * 1000);
    
    cmov_result = (v1 != v_nan) ? v1 : v2;
    result += (int)(cmov_result * 1000);
    
    /* 5. Complex expression with multiple unordered possibilities */
    if ((v1 < v2) || (v1 != v1) || (v2 != v2)) {
        result += 32768;
    }
    
    if ((v1 >= v2) && (v1 == v1) && (v2 == v2)) {
        result += 65536;
    }
    
    return result;
}

/* Function with vectorized comparisons */
__attribute__((noinline))
static int stress_vector_comparisons(v2df vec1, v2df vec2, v2df vec_nan) {
    int result = 0;
    
    /* Vector comparisons generate cmppd/cmpsd instructions */
    v2di cmp_eq = (v2di)(vec1 == vec2);
    v2di cmp_lt = (v2di)(vec1 < vec2);
    v2di cmp_le = (v2di)(vec1 <= vec2);
    v2di cmp_gt = (v2di)(vec1 > vec2);
    v2di cmp_ge = (v2di)(vec1 >= vec2);
    v2di cmp_neq = (v2di)(vec1 != vec2);
    
    /* Unordered vector comparisons */
    v2di cmp_nan_eq = (v2di)(vec1 == vec_nan);
    v2di cmp_nan_neq = (v2di)(vec1 != vec_nan);
    v2di cmp_nan_lt = (v2di)(vec1 < vec_nan);
    v2di cmp_nan_le = (v2di)(vec1 <= vec_nan);
    
    /* Extract results to prevent elimination */
    long long *eq_ptr = (long long*)&cmp_eq;
    long long *lt_ptr = (long long*)&cmp_lt;
    long long *nan_eq_ptr = (long long*)&cmp_nan_eq;
    
    result += (int)(eq_ptr[0] & 1);
    result += (int)(lt_ptr[0] & 2);
    result += (int)(nan_eq_ptr[0] & 4);
    
    return result;
}

/* Function with inline assembly using condition codes */
__attribute__((noinline))
static int stress_asm_condition_codes(double a, double b, double nan_val) {
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that reads FP condition codes */
    
    /* 1. Test for UNORDERED (parity) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(nan_val)
        : "cc"
    );
    result += cc_result * 1;
    
    /* 2. Test for LESS THAN */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 2;
    
    /* 3. Test for EQUAL */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 4;
    
    /* 4. Test for NOT GREATER THAN (LE) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += cc_result * 8;
    
    /* 5. Test for NOT LESS THAN (GE) with NaN */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnb %0"
        : "=r"(cc_result)
        : "x"(nan_val), "x"(a)
        : "cc"
    );
    result += cc_result * 16;
    
    return result;
}

/* Main test driver */
int main(void) {
    int total_result = 0;
    
    /* Initialize FP values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    double zero = 0.0;
    double neg_zero = -0.0;
    
    /* Create vector values */
    v2df vec1 = {normal1, normal2};
    v2df vec2 = {normal2, normal1};
    v2df vec_nan = {nan_val, nan_val};
    v2df vec_inf = {inf_val, neg_inf_val};
    
    /* Test 1: Exhaustive scalar comparisons */
    total_result += stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    total_result += stress_fp_comparisons(normal1, normal1, nan_val, inf_val, neg_inf_val);
    total_result += stress_fp_comparisons(nan_val, normal2, nan_val, inf_val, neg_inf_val);
    total_result += stress_fp_comparisons(inf_val, normal2, nan_val, inf_val, neg_inf_val);
    total_result += stress_fp_comparisons(neg_inf_val, normal2, nan_val, inf_val, neg_inf_val);
    total_result += stress_fp_comparisons(zero, neg_zero, nan_val, inf_val, neg_inf_val);
    
    /* Test 2: Vector comparisons */
    total_result += stress_vector_comparisons(vec1, vec2, vec_nan);
    total_result += stress_vector_comparisons(vec1, vec_inf, vec_nan);
    total_result += stress_vector_comparisons(vec_nan, vec2, vec_nan);
    
    /* Test 3: Inline assembly with condition codes */
    total_result += stress_asm_condition_codes(normal1, normal2, nan_val);
    total_result += stress_asm_condition_codes(nan_val, normal2, nan_val);
    total_result += stress_asm_condition_codes(inf_val, normal2, nan_val);
    
    /* Test 4: Loop with varying comparisons to prevent optimization */
    volatile double loop_var = 1.0;
    for (int i = 0; i < 10; i++) {
        loop_var += 0.1;
        double temp = loop_var * 2.0;
        
        /* Mix of ordered and unordered comparisons in loop */
        if (loop_var < temp) {
            total_result += 1;
        }
        if (loop_var != loop_var) {  /* Check for NaN */
            total_result += 2;
        }
        if (temp > nan_val) {  /* Unordered comparison */
            total_result += 4;
        }
        
        /* Conditional move in loop */
        double cmov = (loop_var < 5.0) ? loop_var : temp;
        total_result += (int)cmov;
    }
    
    /* Test 5: Additional unordered case patterns */
    double values[] = {normal1, normal2, nan_val, inf_val, neg_inf_val, zero};
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            /* Force all comparison types */
            int cmp_result = 0;
            cmp_result += (values[i] == values[j]) ? 1 : 0;
            cmp_result += (values[i] != values[j]) ? 2 : 0;
            cmp_result += (values[i] < values[j]) ? 4 : 0;
            cmp_result += (values[i] <= values[j]) ? 8 : 0;
            cmp_result += (values[i] > values[j]) ? 16 : 0;
            cmp_result += (values[i] >= values[j]) ? 32 : 0;
            
            /* Use goto to create complex control flow */
            if (cmp_result & 1) goto add_result1;
            if (cmp_result & 2) goto add_result2;
            if (cmp_result & 4) goto add_result4;
            
add_result1:
            total_result += cmp_result;
            continue;
            
add_result2:
            total_result += cmp_result * 2;
            continue;
            
add_result4:
            total_result += cmp_result * 3;
            continue;
        }
    }
    
    /* Prevent dead code elimination */
    global_counter = total_result;
    
    printf("Result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
