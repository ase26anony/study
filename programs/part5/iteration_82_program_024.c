/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld_array[16];
volatile int global_index = 0;

/* Helper to get unpredictable long double values */
long double get_ld_value(int idx) {
    return global_ld_array[idx % 16];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;
    if (c > d) result |= 2;
    if (a <= b) result |= 4;
    if (c >= d) result |= 8;
    if (a == b) result |= 16;
    if (c != d) result |= 32;
    
    /* Mixed with NaN checks */
    if (a != a) result |= 64;  /* Check for NaN */
    if (b == b) result |= 128; /* Check for non-NaN */
    
    /* Complex nested comparison */
    if ((a < b) && (c > d) && !(a != a)) result |= 256;
    
    return result;
}

/* Function focusing on unordered comparisons with NaN */
int unordered_comparisons(long double nan_val, long double normal, long double inf) {
    int result = 0;
    
    /* Explicit unordered comparisons */
    if (!(nan_val == nan_val)) result |= 1;  /* UNORDERED/UNEQ path */
    if (nan_val != nan_val) result |= 2;     /* UNORDERED path */
    
    /* NaN compared with normal numbers */
    if (nan_val < normal) result |= 4;       /* Should be false, may use UNGE */
    if (nan_val > normal) result |= 8;       /* Should be false, may use UNLE */
    if (nan_val <= normal) result |= 16;     /* Should be false, may use UNGT */
    if (nan_val >= normal) result |= 32;     /* Should be false, may use UNLT */
    if (nan_val == normal) result |= 64;     /* Should be false */
    if (nan_val != normal) result |= 128;    /* Should be true, may use UNEQ */
    
    /* NaN compared with infinity */
    if (nan_val < inf) result |= 256;
    if (nan_val > -inf) result |= 512;
    
    /* Two NaNs compared */
    long double nan2 = nan_val * 2.0L;
    if (nan_val == nan2) result |= 1024;
    if (nan_val != nan2) result |= 2048;
    
    return result;
}

/* Function with switch based on comparison results */
int comparison_switch(long double x, long double y) {
    int result = 0;
    
    /* Force generation of condition codes */
    if (x < y) result = 1;
    else if (x > y) result = 2;
    else if (x == y) result = 3;
    else result = 4;  /* Unordered */
    
    /* Additional comparisons in switch context */
    switch(result) {
        case 1:
            if (x <= y && x != y) result += 10;
            break;
        case 2:
            if (x >= y && x != y) result += 20;
            break;
        case 3:
            if (x == y && x >= y && x <= y) result += 30;
            break;
        case 4:
            if (x != x || y != y) result += 40;
            break;
    }
    
    return result;
}

/* Loop with termination based on long double comparisons */
int loop_with_fp_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition using multiple comparisons */
    while (counter < limit && counter == counter && !(counter != counter)) {
        counter += 1.0L;
        iterations++;
        
        /* Break on NaN detection */
        if (counter != counter) break;
        
        /* Nested condition */
        if (counter > start + 5.0L && counter < limit - 3.0L) {
            iterations += 10;
        }
    }
    
    return iterations;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(long double ld, double d, float f) {
    int result = 0;
    
    /* Compare long double with double (promotion happens) */
    if (ld < (long double)d) result |= 1;
    if (ld > (long double)d) result |= 2;
    
    /* Compare long double with float */
    if (ld == (long double)f) result |= 4;
    if (ld != (long double)f) result |= 8;
    
    /* Compare with integer constant cast to long double */
    if (ld < (long double)100) result |= 16;
    if (ld > (long double)-50) result |= 32;
    
    /* Complex mixed expression */
    if ((ld + (long double)d) < (long double)(f * 2.0f)) result |= 64;
    
    return result;
}

/* Initialize array with various floating-point values */
void init_fp_array(void) {
    /* Normal numbers */
    global_ld_array[0] = 0.0L;
    global_ld_array[1] = 1.0L;
    global_ld_array[2] = -1.0L;
    global_ld_array[3] = 3.14159265358979323846L;
    global_ld_array[4] = 2.71828182845904523536L;
    global_ld_array[5] = 100.0L;
    global_ld_array[6] = -100.0L;
    global_ld_array[7] = 1.0e-10L;
    
    /* Infinity */
    global_ld_array[8] = __builtin_infl();
    global_ld_array[9] = -__builtin_infl();
    
    /* NaN values */
    global_ld_array[10] = __builtin_nanl("");
    global_ld_array[11] = 0.0L / 0.0L;
    global_ld_array[12] = sqrtl(-1.0L);
    global_ld_array[13] = global_ld_array[10] * 2.0L;
    
    /* More values from arithmetic that might produce NaN */
    global_ld_array[14] = global_ld_array[8] / 0.0L;
    global_ld_array[15] = global_ld_array[9] * 0.0L;
}

int main(void) {
    init_fp_array();
    
    int result_hash = 0;
    int bool_results[100];
    int result_index = 0;
    
    /* Test 1: Complex x87 comparisons */
    for (int i = 0; i < 8; i += 2) {
        long double a = get_ld_value(i);
        long double b = get_ld_value(i + 1);
        long double c = get_ld_value(i + 8);
        long double d = get_ld_value(i + 9);
        
        int res = complex_x87_comparison(a, b, c, d);
        bool_results[result_index++] = res & 1;
        bool_results[result_index++] = (res >> 1) & 1;
        bool_results[result_index++] = (res >> 2) & 1;
        bool_results[result_index++] = (res >> 3) & 1;
        result_hash ^= res;
    }
    
    /* Test 2: Unordered comparisons with NaN */
    long double nan_val = get_ld_value(10);
    long double normal = get_ld_value(3);
    long double inf = get_ld_value(8);
    
    int unordered_res = unordered_comparisons(nan_val, normal, inf);
    for (int i = 0; i < 12; i++) {
        bool_results[result_index++] = (unordered_res >> i) & 1;
    }
    result_hash ^= unordered_res;
    
    /* Test 3: Comparison switch */
    for (int i = 0; i < 6; i += 2) {
        long double x = get_ld_value(i);
        long double y = get_ld_value(i + 7);
        
        int switch_res = comparison_switch(x, y);
        bool_results[result_index++] = switch_res & 1;
        bool_results[result_index++] = (switch_res >> 1) & 1;
        result_hash ^= switch_res;
    }
    
    /* Test 4: Loop with FP condition */
    int loop_res = loop_with_fp_condition(0.0L, 10.0L);
    bool_results[result_index++] = loop_res > 0;
    bool_results[result_index++] = loop_res < 20;
    result_hash ^= loop_res;
    
    /* Test 5: Mixed precision */
    long double ld_val = get_ld_value(3);
    double d_val = (double)get_ld_value(4);
    float f_val = (float)get_ld_value(5);
    
    int mixed_res = mixed_precision_comparisons(ld_val, d_val, f_val);
    for (int i = 0; i < 7; i++) {
        bool_results[result_index++] = (mixed_res >> i) & 1;
    }
    result_hash ^= mixed_res;
    
    /* Additional direct comparisons to hit specific condition codes */
    volatile long double v1 = get_ld_value(10);  /* NaN */
    volatile long double v2 = get_ld_value(0);   /* 0.0 */
    volatile long double v3 = get_ld_value(8);   /* +Inf */
    
    /* These should generate various x87 condition codes */
    if (v1 < v2)  bool_results[result_index++] = 1; else bool_results[result_index++] = 0;
    if (v1 > v2)  bool_results[result_index++] = 1; else bool_results[result_index++] = 0;
    if (v1 <= v2) bool_results[result_index++] = 1; else bool_results[result_index++] = 0;
    if (v1 >= v2) bool_results[result_index++] = 1; else bool_results[result_index++] = 0;
    if (v1 == v2) bool_results[result_index++] = 1; else bool_results[result_index++] = 0;
    if (v1 != v2) bool_results[result_index++] = 1; else bool_results[result_index++] = 0;
    
    if (v2 < v3)  bool_results[result_index++] = 1; else bool_results[result_index++] = 0;
    if (v2 > v3)  bool_results[result_index++] = 1; else bool_results[result_index++] = 0;
    
    /* Compute final verification hash */
    int final_hash = result_hash;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= (bool_results[i] << (i % 16));
    }
    
    printf("Result hash: %d\n", final_hash);
    printf("Number of boolean results: %d\n", result_index);
    
    return final_hash != 0 ? 0 : 1;
}
