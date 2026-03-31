/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];
volatile int array_index = 0;

/* Helper to get dynamic long double values */
long double get_ldbl(int idx) {
    return global_ldbl_array[idx % 16];
}

/* Complex multi-operand comparison designed to use multiple x87 condition codes */
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
    
    if (a == b && c == d) {
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
    
    /* All standard ordered comparisons */
    if (x < y)  result |= 1;
    if (x > y)  result |= 2;
    if (x <= y) result |= 4;
    if (x >= y) result |= 8;
    if (x == y) result |= 16;
    if (x != y) result |= 32;
    
    return result;
}

/* Test function specifically for unordered comparisons with NaN */
int test_unordered_comparisons(long double nan_val, long double normal_val, long double inf_val) {
    int result = 0;
    
    /* Comparisons involving NaN - should trigger UNORDERED, UNEQ, etc. */
    if (nan_val < normal_val)   result |= 1;    /* false, unordered */
    if (nan_val > normal_val)   result |= 2;    /* false, unordered */
    if (nan_val <= normal_val)  result |= 4;    /* false, unordered */
    if (nan_val >= normal_val)  result |= 8;    /* false, unordered */
    if (nan_val == normal_val)  result |= 16;   /* false, unordered */
    if (nan_val != normal_val)  result |= 32;   /* true, unordered */
    
    /* NaN vs NaN comparisons */
    long double nan_val2 = __builtin_nanl("");
    if (nan_val < nan_val2)     result |= 64;   /* false, unordered */
    if (nan_val == nan_val2)    result |= 128;  /* false, unordered */
    if (nan_val != nan_val2)    result |= 256;  /* true, unordered */
    
    /* NaN vs Infinity */
    if (nan_val < inf_val)      result |= 512;  /* false, unordered */
    if (nan_val > -inf_val)     result |= 1024; /* false, unordered */
    
    return result;
}

/* Mixed precision comparisons */
int test_mixed_precision(long double ld, double d, float f) {
    int result = 0;
    
    /* Comparisons with different precisions */
    if (ld < (long double)d)    result |= 1;
    if ((long double)f > ld)    result |= 2;
    if (ld == (long double)d)   result |= 4;
    if (ld != (long double)f)   result |= 8;
    
    /* With integer constants */
    if (ld < 10.0L)             result |= 16;
    if ((long double)5 > ld)    result |= 32;
    
    return result;
}

/* Loop with long double termination condition */
int test_loop_comparisons(long double start, long double limit, long double step) {
    int count = 0;
    volatile long double x = start;  /* volatile to prevent optimization */
    
    while (x < limit && x == x) {  /* x == x checks for NaN */
        count++;
        x += step;
        
        /* Prevent infinite loops */
        if (count > 100) break;
    }
    
    return count;
}

/* Switch statement based on comparison results */
int test_switch_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* Nested comparisons in switch conditions */
    if (a < b) {
        if (c != 0.0L) {
            result = 1;
        } else {
            result = 2;
        }
    } else if (a > b) {
        if (c <= 1.0L) {
            result = 3;
        } else {
            result = 4;
        }
    } else { /* a == b */
        if (c >= -1.0L) {
            result = 5;
        } else {
            result = 6;
        }
    }
    
    return result;
}

/* Generate NaN values through various methods */
long double generate_nan(int method) {
    switch (method) {
        case 0: return __builtin_nanl("");  /* Quiet NaN */
        case 1: return 0.0L / 0.0L;         /* NaN through division */
        case 2: return sqrtl(-1.0L);        /* NaN through sqrt(-1) */
        case 3: return __builtin_nanl("0xABC"); /* NaN with payload */
        default: return __builtin_infl() * 0.0L; /* Infinity * 0 = NaN */
    }
}

int main(void) {
    /* Initialize global array with various values */
    global_ldbl_array[0] = 1.0L;
    global_ldbl_array[1] = 2.0L;
    global_ldbl_array[2] = 3.14159265358979323846L;
    global_ldbl_array[3] = -1.5L;
    global_ldbl_array[4] = 0.0L;
    global_ldbl_array[5] = -0.0L;
    global_ldbl_array[6] = __builtin_infl();      /* Positive infinity */
    global_ldbl_array[7] = -__builtin_infl();     /* Negative infinity */
    global_ldbl_array[8] = generate_nan(0);       /* Quiet NaN */
    global_ldbl_array[9] = generate_nan(1);       /* NaN from 0/0 */
    global_ldbl_array[10] = 100.0L;
    global_ldbl_array[11] = 1.0e-10L;
    global_ldbl_array[12] = 1.0e10L;
    global_ldbl_array[13] = -3.0L;
    global_ldbl_array[14] = 42.0L;
    global_ldbl_array[15] = generate_nan(2);      /* NaN from sqrt(-1) */
    
    /* Results array to prevent dead code elimination */
    int results[50];
    int result_index = 0;
    
    /* Test 1: Ordered comparisons with normal numbers */
    results[result_index++] = test_ordered_comparisons(
        get_ldbl(0), get_ldbl(1));
    results[result_index++] = test_ordered_comparisons(
        get_ldbl(2), get_ldbl(3));
    results[result_index++] = test_ordered_comparisons(
        get_ldbl(4), get_ldbl(5));  /* +0 vs -0 */
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_index++] = test_unordered_comparisons(
        get_ldbl(8), get_ldbl(0), get_ldbl(6));
    results[result_index++] = test_unordered_comparisons(
        get_ldbl(9), get_ldbl(10), get_ldbl(7));
    
    /* Test 3: Complex multi-operand comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ldbl(0), get_ldbl(1), get_ldbl(2), get_ldbl(3));
    results[result_index++] = complex_x87_comparison(
        get_ldbl(8), get_ldbl(0), get_ldbl(6), get_ldbl(7));  /* With NaN */
    
    /* Test 4: Mixed precision */
    double d_val = 2.5;
    float f_val = 3.14f;
    results[result_index++] = test_mixed_precision(
        get_ldbl(0), d_val, f_val);
    results[result_index++] = test_mixed_precision(
        get_ldbl(8), d_val, f_val);  /* With NaN */
    
    /* Test 5: Loop comparisons */
    results[result_index++] = test_loop_comparisons(
        get_ldbl(0), get_ldbl(10), get_ldbl(1));
    
    /* Test 6: Switch based on comparisons */
    results[result_index++] = test_switch_comparison(
        get_ldbl(0), get_ldbl(1), get_ldbl(2));
    results[result_index++] = test_switch_comparison(
        get_ldbl(8), get_ldbl(0), get_ldbl(6));  /* With NaN */
    
    /* Test 7: Direct NaN arithmetic and comparisons */
    long double nan1 = generate_nan(0);
    long double nan2 = generate_nan(1);
    long double inf = __builtin_infl();
    
    /* These should trigger various unordered condition codes */
    volatile int cmp_results = 0;
    if (nan1 == nan2) cmp_results |= 1;      /* UNORDERED/UNEQ? */
    if (nan1 != nan2) cmp_results |= 2;      /* UNORDERED */
    if (nan1 < 5.0L)  cmp_results |= 4;      /* UNORDERED/UNLT? */
    if (nan1 > 5.0L)  cmp_results |= 8;      /* UNORDERED/UNGT? */
    if (nan1 <= inf)  cmp_results |= 16;     /* UNORDERED/UNLE? */
    if (nan1 >= -inf) cmp_results |= 32;     /* UNORDERED/UNGE? */
    if (!(nan1 < 5.0L)) cmp_results |= 64;   /* ORDERED/UNGE? (nlt) */
    if (!(nan1 > 5.0L)) cmp_results |= 128;  /* ORDERED/UNLE? (nle) */
    
    results[result_index++] = cmp_results;
    
    /* Test 8: More explicit unordered pattern that might trigger LTGT */
    long double x = get_ldbl(0);
    long double y = get_ldbl(1);
    long double z = get_ldbl(8);  /* NaN */
    
    volatile int unordered_test = 0;
    /* This pattern might generate LTGT ("une") */
    if (x != y && x != z) {
        unordered_test = 1;
    }
    if (z != x && z != y) {  /* NaN comparisons */
        unordered_test |= 2;
    }
    
    results[result_index++] = unordered_test;
    
    /* Compute final hash to ensure all code executed */
    int final_hash = 0;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= results[i];
    }
    
    printf("Test completed. Final hash: %d\n", final_hash);
    printf("Number of tests executed: %d\n", result_index);
    
    return final_hash != 0 ? 0 : 1;
}
