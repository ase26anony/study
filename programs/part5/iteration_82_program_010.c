/* x87_comparison_test.c
 * Designed to trigger x87 floating-point comparison output logic in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];
volatile int array_index = 0;

/* Helper to get dynamic long double values */
long double get_ldbl_value(int idx) {
    return global_ldbl_array[idx % 16];
}

/* Complex multi-operand comparison designed to use various x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate multiple x87 comparisons with different condition codes */
    if ((a != b) && (c <= d)) {
        if (a > 0.0L && b < 0.0L) {
            return 1;
        } else if (a == c || b == d) {
            return 2;
        }
    }
    
    if ((a >= b) || (c < d)) {
        if (a != c && b != d) {
            return 3;
        }
    }
    
    return 0;
}

/* Test ordered comparisons (<, >, <=, >=, ==, !=) */
int test_ordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* All standard relational operators */
    if (x < y)  result |= 0x01;
    if (x > y)  result |= 0x02;
    if (x <= y) result |= 0x04;
    if (x >= y) result |= 0x08;
    if (x == y) result |= 0x10;
    if (x != y) result |= 0x20;
    
    return result;
}

/* Test unordered comparisons with NaN */
int test_unordered_comparisons(long double nan_val, long double normal, long double inf) {
    int result = 0;
    
    /* Compare NaN with normal numbers */
    if (nan_val < normal)   result |= 0x0001;  /* Should be false (unordered) */
    if (nan_val > normal)   result |= 0x0002;  /* Should be false (unordered) */
    if (nan_val <= normal)  result |= 0x0004;  /* Should be false (unordered) */
    if (nan_val >= normal)  result |= 0x0008;  /* Should be false (unordered) */
    if (nan_val == normal)  result |= 0x0010;  /* Should be false (unordered) */
    if (nan_val != normal)  result |= 0x0020;  /* Should be true (unordered) */
    
    /* Compare NaN with infinity */
    if (nan_val < inf)      result |= 0x0040;
    if (nan_val > inf)      result |= 0x0080;
    if (nan_val <= inf)     result |= 0x0100;
    if (nan_val >= inf)     result |= 0x0200;
    if (nan_val == inf)     result |= 0x0400;
    if (nan_val != inf)     result |= 0x0800;
    
    /* Compare NaN with another NaN */
    long double nan2 = __builtin_nanl("");
    if (nan_val < nan2)     result |= 0x1000;
    if (nan_val > nan2)     result |= 0x2000;
    if (nan_val <= nan2)    result |= 0x4000;
    if (nan_val >= nan2)    result |= 0x8000;
    if (nan_val == nan2)    result |= 0x10000;
    if (nan_val != nan2)    result |= 0x20000;
    
    return result;
}

/* Test mixed precision comparisons */
int test_mixed_precision(long double ld, double d, float f) {
    int result = 0;
    
    /* Compare long double with double (promotion happens) */
    if (ld < d)   result |= 0x01;
    if (ld > d)   result |= 0x02;
    if (ld <= d)  result |= 0x04;
    if (ld >= d)  result |= 0x08;
    if (ld == d)  result |= 0x10;
    if (ld != d)  result |= 0x20;
    
    /* Compare long double with float */
    if (ld < f)   result |= 0x40;
    if (ld > f)   result |= 0x80;
    if (ld <= f)  result |= 0x100;
    if (ld >= f)  result |= 0x200;
    if (ld == f)  result |= 0x400;
    if (ld != f)  result |= 0x800;
    
    /* Compare with integer constant cast to long double */
    if (ld < (long double)42)   result |= 0x1000;
    if (ld > (long double)-7)   result |= 0x2000;
    if (ld <= (long double)100) result |= 0x4000;
    if (ld >= (long double)0)   result |= 0x8000;
    
    return result;
}

/* Loop with long double termination condition */
int test_loop_comparisons(long double start, long double end, long double step) {
    int count = 0;
    volatile long double x = start;  /* volatile to prevent optimization */
    
    /* Loop condition based on long double comparison */
    while (x < end && !isnan(x)) {
        count++;
        x += step;
        
        /* Additional comparison in loop body */
        if (x > end / 2.0L) {
            count += 10;
        }
    }
    
    return count;
}

/* Switch statement based on comparison results */
int test_switch_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* Nested comparisons to create complex condition */
    if (a < b) {
        result = 1;
        if (c != 0.0L) {
            result = 2;
        }
    } else if (a > b) {
        result = 3;
        if (c == 0.0L) {
            result = 4;
        }
    } else if (a == b) {
        result = 5;
        if (c < 0.0L) {
            result = 6;
        } else if (c > 0.0L) {
            result = 7;
        }
    }
    
    /* Switch on the result of comparisons */
    switch (result) {
        case 1:
            if (a != c) result += 10;
            break;
        case 2:
            if (b <= c) result += 20;
            break;
        case 3:
            if (a >= c) result += 30;
            break;
        case 4:
            if (b != c) result += 40;
            break;
        case 5:
            if (c == 0.0L) result += 50;
            break;
        case 6:
            if (a < c) result += 60;
            break;
        case 7:
            if (b > c) result += 70;
            break;
    }
    
    return result;
}

/* Generate NaN values in various ways */
long double generate_nan(int method) {
    switch (method) {
        case 0: return __builtin_nanl("");          /* Built-in quiet NaN */
        case 1: return 0.0L / 0.0L;                 /* Division by zero */
        case 2: return sqrtl(-1.0L);                /* Square root of negative */
        case 3: return __builtin_nanl("0xdeadbeef"); /* NaN with payload */
        case 4: return __builtin_infl() * 0.0L;     /* Infinity times zero */
        default: return __builtin_nanl("");
    }
}

int main(void) {
    /* Initialize global array with various floating-point values */
    global_ldbl_array[0] = 3.14159265358979323846L;  /* pi */
    global_ldbl_array[1] = 2.71828182845904523536L;  /* e */
    global_ldbl_array[2] = 1.41421356237309504880L;  /* sqrt(2) */
    global_ldbl_array[3] = 0.0L;
    global_ldbl_array[4] = -1.0L;
    global_ldbl_array[5] = 100.0L;
    global_ldbl_array[6] = -100.0L;
    global_ldbl_array[7] = 1.0e-10L;
    global_ldbl_array[8] = 1.0e10L;
    global_ldbl_array[9] = __builtin_infl();        /* positive infinity */
    global_ldbl_array[10] = -__builtin_infl();      /* negative infinity */
    global_ldbl_array[11] = generate_nan(0);        /* quiet NaN */
    global_ldbl_array[12] = generate_nan(1);        /* NaN from 0/0 */
    global_ldbl_array[13] = generate_nan(2);        /* NaN from sqrt(-1) */
    global_ldbl_array[14] = 42.0L;
    global_ldbl_array[15] = -42.0L;
    
    /* Results array to prevent dead code elimination */
    int results[64];
    int result_index = 0;
    
    /* Test 1: Ordered comparisons with normal numbers */
    results[result_index++] = test_ordered_comparisons(
        get_ldbl_value(0), get_ldbl_value(1));
    results[result_index++] = test_ordered_comparisons(
        get_ldbl_value(3), get_ldbl_value(4));
    results[result_index++] = test_ordered_comparisons(
        get_ldbl_value(5), get_ldbl_value(6));
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_index++] = test_unordered_comparisons(
        get_ldbl_value(11), get_ldbl_value(0), get_ldbl_value(9));
    results[result_index++] = test_unordered_comparisons(
        get_ldbl_value(12), get_ldbl_value(14), get_ldbl_value(10));
    
    /* Test 3: Mixed precision */
    results[result_index++] = test_mixed_precision(
        get_ldbl_value(0), 3.14159, 3.14159f);
    results[result_index++] = test_mixed_precision(
        get_ldbl_value(4), -1.0, -1.0f);
    
    /* Test 4: Complex multi-operand comparison */
    results[result_index++] = complex_x87_comparison(
        get_ldbl_value(0), get_ldbl_value(1),
        get_ldbl_value(2), get_ldbl_value(3));
    results[result_index++] = complex_x87_comparison(
        get_ldbl_value(11), get_ldbl_value(12),
        get_ldbl_value(13), get_ldbl_value(14));
    
    /* Test 5: Loop with long double comparisons */
    results[result_index++] = test_loop_comparisons(0.0L, 10.0L, 0.1L);
    results[result_index++] = test_loop_comparisons(-5.0L, 5.0L, 0.5L);
    
    /* Test 6: Switch based on comparison results */
    results[result_index++] = test_switch_comparison(
        get_ldbl_value(0), get_ldbl_value(1), get_ldbl_value(2));
    results[result_index++] = test_switch_comparison(
        get_ldbl_value(11), get_ldbl_value(0), get_ldbl_value(9));
    
    /* Additional direct comparisons to cover more cases */
    volatile long double v1 = get_ldbl_value(0);
    volatile long double v2 = get_ldbl_value(1);
    volatile long double v3 = get_ldbl_value(11);
    
    /* Direct unordered comparisons */
    results[result_index++] = (v3 < v1) ? 1 : 0;      /* UNORDERED/UNLT */
    results[result_index++] = (v3 <= v1) ? 1 : 0;     /* UNORDERED/UNLE */
    results[result_index++] = (v3 > v1) ? 1 : 0;      /* UNORDERED/UNGT */
    results[result_index++] = (v3 >= v1) ? 1 : 0;     /* UNORDERED/UNGE */
    results[result_index++] = (v3 == v1) ? 1 : 0;     /* UNORDERED/UNEQ */
    results[result_index++] = (v3 != v1) ? 1 : 0;     /* UNORDERED/LTGT? */
    
    /* Compare two NaNs */
    results[result_index++] = (v3 < get_ldbl_value(12)) ? 1 : 0;
    results[result_index++] = (v3 == get_ldbl_value(12)) ? 1 : 0;
    results[result_index++] = (v3 != get_ldbl_value(12)) ? 1 : 0;
    
    /* Compute final hash to verify execution and prevent optimization */
    int final_hash = 0;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= results[i];
    }
    
    printf("Test completed. Final hash: %d\n", final_hash);
    printf("Number of comparison tests executed: %d\n", result_index);
    
    return final_hash != 0 ? 0 : 1;
}
