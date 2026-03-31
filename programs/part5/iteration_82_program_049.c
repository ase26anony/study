/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
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

/* Complex comparison function using multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison patterns */
    if (a != b) {
        if (c <= d) {
            if (a > c) {
                return 1;
            } else if (b < d) {
                return 2;
            }
        }
    }
    
    if (a == b && c >= d) {
        return 3;
    }
    
    /* Unordered comparisons with explicit NaN checks */
    if (a != a) {  /* NaN check */
        if (b == b) {  /* Not NaN */
            return 4;
        }
    }
    
    return 0;
}

/* Test function for ordered comparisons */
int test_ordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* All standard relational operators */
    if (x < y)  result |= 1;
    if (x > y)  result |= 2;
    if (x <= y) result |= 4;
    if (x >= y) result |= 8;
    if (x == y) result |= 16;
    if (x != y) result |= 32;
    
    return result;
}

/* Test function specifically for unordered comparisons */
int test_unordered_comparisons(long double nan_val, long double normal_val, long double inf_val) {
    int result = 0;
    
    /* Compare NaN with normal numbers - should trigger UNORDERED cases */
    if (nan_val < normal_val)   result |= 1;    /* false, unordered */
    if (nan_val > normal_val)   result |= 2;    /* false, unordered */
    if (nan_val <= normal_val)  result |= 4;    /* false, unordered */
    if (nan_val >= normal_val)  result |= 8;    /* false, unordered */
    if (nan_val == normal_val)  result |= 16;   /* false, unordered */
    if (nan_val != normal_val)  result |= 32;   /* true, unordered */
    
    /* Compare NaN with infinity */
    if (nan_val < inf_val)      result |= 64;
    if (nan_val > -inf_val)     result |= 128;
    
    /* Compare two NaNs */
    long double another_nan = nan_val + 1.0L;  /* Still NaN */
    if (nan_val == another_nan) result |= 256; /* false */
    if (nan_val != another_nan) result |= 512; /* true */
    
    return result;
}

/* Function with mixed precision comparisons */
int test_mixed_precision(long double ld, double d, float f) {
    int result = 0;
    
    /* Mixed precision comparisons causing promotions */
    if (ld < (long double)d)    result |= 1;
    if ((long double)f > ld)    result |= 2;
    if (ld == (long double)d)   result |= 4;
    if (ld != (long double)f)   result |= 8;
    
    /* Compare with integer constant cast to long double */
    if (ld < (long double)100)  result |= 16;
    if (ld > (long double)-50)  result |= 32;
    
    return result;
}

/* Loop with long double termination condition */
int test_loop_comparisons(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition based on long double comparison */
    while (counter < limit && iterations < 100) {
        /* Nested comparison inside loop */
        if (counter != start && counter <= limit / 2.0L) {
            iterations++;
        } else if (counter > start * 2.0L) {
            iterations += 2;
        }
        
        counter += 1.5L;
        
        /* Additional comparison in loop body */
        if (!(counter >= start)) {
            break;
        }
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int test_switch_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* Complex condition for switch */
    switch ((a < b) + (b > c) * 2 + (a == c) * 4) {
        case 0:
            result = 100;
            break;
        case 1:
            result = 200;
            if (a != b && c <= a) {
                result += 50;
            }
            break;
        case 2:
            result = 300;
            break;
        case 3:
            result = 400;
            if (b >= c || a < b) {
                result += 75;
            }
            break;
        default:
            result = 500;
            if (!(a == b) && !(c > a)) {
                result += 25;
            }
    }
    
    return result;
}

/* Generate NaN values using different methods */
long double generate_nan(int method) {
    switch (method) {
        case 0:
            return __builtin_nanl("");  /* Quiet NaN */
        case 1:
            return 0.0L / 0.0L;         /* NaN from division */
        case 2:
            return sqrtl(-1.0L);        /* NaN from sqrt(-1) */
        case 3:
            return __builtin_nanl("0x1234"); /* NaN with payload */
        default:
            return __builtin_infl() * 0.0L;  /* Infinity * 0 = NaN */
    }
}

int main() {
    /* Initialize test array with various values */
    global_ldbl_array[0] = 1.0L;
    global_ldbl_array[1] = 3.14159265358979323846L; /* Pi */
    global_ldbl_array[2] = -2.71828182845904523536L; /* -e */
    global_ldbl_array[3] = 0.0L;
    global_ldbl_array[4] = -0.0L;
    global_ldbl_array[5] = __builtin_infl();  /* Positive infinity */
    global_ldbl_array[6] = -__builtin_infl(); /* Negative infinity */
    global_ldbl_array[7] = generate_nan(0);   /* Quiet NaN */
    global_ldbl_array[8] = 100.0L;
    global_ldbl_array[9] = 1.0e-10L;
    global_ldbl_array[10] = 1.0e10L;
    global_ldbl_array[11] = generate_nan(1);  /* NaN from 0/0 */
    global_ldbl_array[12] = 42.0L;
    global_ldbl_array[13] = -999.999L;
    global_ldbl_array[14] = generate_nan(2);  /* NaN from sqrt(-1) */
    global_ldbl_array[15] = 0.5L;
    
    int result_hash = 0;
    
    /* Test 1: Ordered comparisons with normal numbers */
    for (int i = 0; i < 8; i += 2) {
        long double a = get_ldbl_value(i);
        long double b = get_ldbl_value(i + 1);
        int res = test_ordered_comparisons(a, b);
        result_hash ^= res;
    }
    
    /* Test 2: Unordered comparisons with NaN */
    long double nan_val = get_ldbl_value(7);
    long double normal_val = get_ldbl_value(0);
    long double inf_val = get_ldbl_value(5);
    int unordered_res = test_unordered_comparisons(nan_val, normal_val, inf_val);
    result_hash ^= unordered_res;
    
    /* Test 3: Mixed precision */
    long double ld_val = get_ldbl_value(3);
    double d_val = 2.718281828459045;
    float f_val = 3.14159265f;
    int mixed_res = test_mixed_precision(ld_val, d_val, f_val);
    result_hash ^= mixed_res;
    
    /* Test 4: Complex comparison function */
    for (int i = 0; i < 4; i++) {
        int complex_res = complex_x87_comparison(
            get_ldbl_value(i),
            get_ldbl_value(i + 4),
            get_ldbl_value(i + 8),
            get_ldbl_value(i + 12)
        );
        result_hash ^= complex_res;
    }
    
    /* Test 5: Loop with comparison */
    int loop_res = test_loop_comparisons(get_ldbl_value(0), get_ldbl_value(8));
    result_hash ^= loop_res;
    
    /* Test 6: Switch based on comparisons */
    int switch_res = test_switch_comparison(
        get_ldbl_value(1),
        get_ldbl_value(2),
        get_ldbl_value(3)
    );
    result_hash ^= switch_res;
    
    /* Test 7: Direct unordered comparisons in main */
    volatile long double volatile_nan = generate_nan(3);
    volatile long double volatile_num = 123.456L;
    
    /* These should generate the specific x87 condition codes we want to cover */
    if (volatile_nan < volatile_num) {
        result_hash ^= 0x1000;  /* Should not execute */
    }
    
    if (!(volatile_nan >= volatile_num)) {
        result_hash ^= 0x2000;  /* Should execute */
    }
    
    if (volatile_nan == volatile_nan) {
        result_hash ^= 0x4000;  /* Should not execute */
    }
    
    if (volatile_nan != volatile_num) {
        result_hash ^= 0x8000;  /* Should execute */
    }
    
    /* Test 8: Arithmetic producing NaN followed by comparison */
    long double nan_prod = get_ldbl_value(5) * get_ldbl_value(3); /* inf * 0 = NaN */
    if (nan_prod == nan_prod) {
        result_hash ^= 0x10000;  /* Should not execute */
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result hash: 0x%08x\n", result_hash);
    printf("Test completed. Check assembly output for x87 comparison instructions.\n");
    
    return 0;
}
