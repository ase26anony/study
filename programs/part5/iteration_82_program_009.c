/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -o x87_test x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld_array[16];

/* Helper to get unpredictable long double values */
long double get_ld_value(int idx) {
    return global_ld_array[idx];
}

/* Complex comparison function using multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison codes */
    if (a < b) {
        if (c == d) return 1;
        if (c != d) return 2;
        if (c <= d) return 3;
        if (c >= d) return 4;
    } else if (a > b) {
        if (c < d) return 5;
        if (c > d) return 6;
    } else if (a == b) {
        if (c != d) return 7;
        if (c <= d) return 8;
        if (c >= d) return 9;
    }
    
    /* Unordered comparisons */
    if (a != a || b != b) {  /* NaN detection */
        if (c == c && d == d) return 10;  /* c and d are ordered */
        if (c != c || d != d) return 11;  /* c or d is NaN */
    }
    
    return 0;
}

/* Function to test unordered comparisons with NaN */
int test_unordered_comparisons(long double nan1, long double nan2, 
                               long double normal, long double inf) {
    int result = 0;
    
    /* UNORDERED comparisons (NaN vs anything) */
    if (nan1 != nan1) result |= 1;      /* UNORDERED check */
    if (nan1 == nan1) result |= 2;      /* Should be false for NaN */
    
    /* UNEQ (unordered or equal) */
    if (!(nan1 < normal) && !(nan1 > normal)) result |= 4;
    
    /* UNGE (not less than) - nlt */
    if (!(nan1 < normal)) result |= 8;
    
    /* UNGT (not less than or equal) - nle */
    if (!(nan1 <= normal)) result |= 16;
    
    /* UNLE (unordered or less than or equal) - ule */
    if (nan1 <= normal || nan1 != nan1) result |= 32;
    
    /* UNLT (unordered or less than) - ult */
    if (nan1 < normal || nan1 != nan1) result |= 64;
    
    /* LTGT (less than or greater than) - une */
    if (nan1 < normal || nan1 > normal) result |= 128;
    
    /* NaN vs NaN comparisons */
    if (nan1 == nan2) result |= 256;    /* Should be false */
    if (nan1 != nan2) result |= 512;    /* Should be true */
    
    /* NaN vs infinity */
    if (nan1 < inf) result |= 1024;
    if (nan1 > inf) result |= 2048;
    if (nan1 == inf) result |= 4096;
    
    return result;
}

/* Function with mixed precision comparisons */
int mixed_precision_comparisons(long double ld, double d, float f, int i) {
    int result = 0;
    
    /* Mixed type comparisons causing promotions */
    if (ld < (long double)d) result |= 1;
    if ((long double)f > ld) result |= 2;
    if (ld == (long double)i) result |= 4;
    if ((long double)d != ld) result |= 8;
    
    /* Complex expression with casts */
    if ((ld < (long double)(d * f)) && ((long double)i >= ld)) {
        result |= 16;
    }
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ld_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition using x87 comparison */
    while (counter < limit && iterations < 100) {
        counter += 1.0L;
        iterations++;
        
        /* Nested comparison inside loop */
        if (counter != counter) {  /* NaN check */
            break;
        }
    }
    
    return iterations;
}

/* Switch based on comparison results */
int switch_on_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* First level of comparisons */
    if (a < b) {
        result = 1;
        if (b > c) result = 2;
    } else if (a > b) {
        result = 3;
        if (b < c) result = 4;
    } else if (a == b) {
        result = 5;
        if (b != c) result = 6;
    } else {
        /* Unordered case */
        result = 7;
    }
    
    /* Additional switch for finer control */
    switch (result) {
        case 1:
            if (c <= a) result = 10;
            break;
        case 2:
            if (c >= b) result = 20;
            break;
        case 3:
            if (a != c) result = 30;
            break;
        default:
            if (!(a < c) && !(a > c)) result = 40;  /* Unordered check */
    }
    
    return result;
}

/* Main test function */
int main() {
    /* Initialize array with various floating-point values */
    global_ld_array[0] = 1.0L;
    global_ld_array[1] = 2.0L;
    global_ld_array[2] = 3.14159265358979323846L;
    global_ld_array[3] = -1.5L;
    global_ld_array[4] = 0.0L;
    global_ld_array[5] = -0.0L;
    global_ld_array[6] = __builtin_infl();      /* Positive infinity */
    global_ld_array[7] = -__builtin_infl();     /* Negative infinity */
    global_ld_array[8] = __builtin_nanl("");    /* Quiet NaN */
    global_ld_array[9] = __builtin_nanl("0x1"); /* Another NaN */
    global_ld_array[10] = 0.0L / 0.0L;          /* NaN from operation */
    global_ld_array[11] = sqrtl(-1.0L);         /* NaN from sqrt(-1) */
    
    /* Get volatile values to prevent optimization */
    volatile long double v_ld1 = get_ld_value(0);
    volatile long double v_ld2 = get_ld_value(1);
    volatile long double v_nan1 = get_ld_value(8);
    volatile long double v_nan2 = get_ld_value(9);
    volatile long double v_inf = get_ld_value(6);
    volatile long double v_neg_inf = get_ld_value(7);
    
    /* Array to store comparison results */
    int results[50];
    int result_index = 0;
    
    /* Test 1: Basic ordered comparisons */
    results[result_index++] = (v_ld1 < v_ld2) ? 1 : 0;
    results[result_index++] = (v_ld1 > v_ld2) ? 1 : 0;
    results[result_index++] = (v_ld1 <= v_ld2) ? 1 : 0;
    results[result_index++] = (v_ld1 >= v_ld2) ? 1 : 0;
    results[result_index++] = (v_ld1 == v_ld2) ? 1 : 0;
    results[result_index++] = (v_ld1 != v_ld2) ? 1 : 0;
    
    /* Test 2: Complex comparison function */
    results[result_index++] = complex_x87_comparison(v_ld1, v_ld2, v_inf, v_neg_inf);
    results[result_index++] = complex_x87_comparison(v_nan1, v_ld1, v_ld2, v_inf);
    
    /* Test 3: Unordered comparisons with NaN */
    results[result_index++] = test_unordered_comparisons(v_nan1, v_nan2, v_ld1, v_inf);
    results[result_index++] = test_unordered_comparisons(v_nan1, v_ld1, v_inf, v_neg_inf);
    
    /* Test 4: Mixed precision */
    double d_val = 2.5;
    float f_val = 3.14f;
    int i_val = 2;
    results[result_index++] = mixed_precision_comparisons(v_ld1, d_val, f_val, i_val);
    results[result_index++] = mixed_precision_comparisons(v_nan1, d_val, f_val, i_val);
    
    /* Test 5: Loop with long double condition */
    results[result_index++] = loop_with_ld_condition(v_ld1, v_ld2 * 5.0L);
    results[result_index++] = loop_with_ld_condition(v_nan1, v_ld2);
    
    /* Test 6: Switch on comparison results */
    results[result_index++] = switch_on_comparison(v_ld1, v_ld2, v_inf);
    results[result_index++] = switch_on_comparison(v_nan1, v_ld1, v_ld2);
    
    /* Test 7: More explicit unordered scenarios */
    volatile long double nan_from_div = 0.0L / 0.0L;
    volatile long double nan_from_sqrt = sqrtl(-1.0L);
    
    /* Compare different NaN sources */
    results[result_index++] = (nan_from_div == nan_from_sqrt) ? 1 : 0;
    results[result_index++] = (nan_from_div != nan_from_sqrt) ? 1 : 0;
    results[result_index++] = (nan_from_div < v_inf) ? 1 : 0;
    results[result_index++] = (nan_from_div > v_neg_inf) ? 1 : 0;
    results[result_index++] = (nan_from_div <= v_ld1) ? 1 : 0;
    results[result_index++] = (nan_from_div >= v_ld2) ? 1 : 0;
    
    /* Test 8: Infinity comparisons */
    results[result_index++] = (v_inf > v_ld1) ? 1 : 0;
    results[result_index++] = (v_neg_inf < v_ld1) ? 1 : 0;
    results[result_index++] = (v_inf == v_inf) ? 1 : 0;
    results[result_index++] = (v_inf != v_inf) ? 1 : 0;
    
    /* Test 9: Complex boolean expressions */
    results[result_index++] = ((v_ld1 < v_ld2) && (v_ld2 > v_ld1)) ? 1 : 0;
    results[result_index++] = ((v_nan1 != v_nan1) || (v_ld1 == v_ld2)) ? 1 : 0;
    results[result_index++] = (!(v_nan1 < v_ld1) && !(v_nan1 > v_ld1)) ? 1 : 0;
    
    /* Test 10: Nested comparisons in conditional */
    volatile long double x = get_ld_value(2);
    volatile long double y = get_ld_value(3);
    volatile long double z = get_ld_value(4);
    
    if ((x < y) && (y > z) && (x != z)) {
        results[result_index++] = 1;
    } else if ((x > y) || (y < z) || (x == z)) {
        results[result_index++] = 2;
    } else if (!(x == x) || !(y == y)) {  /* NaN checks */
        results[result_index++] = 3;
    } else {
        results[result_index++] = 4;
    }
    
    /* Compute hash of results to prevent dead code elimination */
    int hash = 0;
    for (int i = 0; i < result_index; i++) {
        hash ^= results[i] + i;
    }
    
    printf("Result hash: %d\n", hash);
    printf("Total comparisons performed: %d\n", result_index);
    
    return hash != 0 ? 0 : 1;
}
