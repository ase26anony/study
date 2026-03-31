/* x87_comparison_tests.c
 * Designed to trigger x87 comparison mnemonics in GCC's i386 backend
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_tests.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl[16];
volatile int global_index = 0;

/* Helper to get dynamic long double values */
long double get_ldbl(int idx) {
    return global_ldbl[idx];
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
    
    /* Unordered comparisons with explicit NaN checks */
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    
    /* UNORDERED: (a != a) || (b != b) */
    if (a != a || b != b) result |= 64;
    
    /* ORDERED: (a == a) && (b == b) */
    if (a == a && b == b) result |= 128;
    
    /* UNEQ: unordered or equal */
    if (!(a < b || a > b)) result |= 256;
    
    /* UNGE: not less than (unordered or greater or equal) */
    if (!(a < b)) result |= 512;
    
    /* UNGT: not less than or equal (unordered or greater) */
    if (!(a <= b)) result |= 1024;
    
    /* UNLE: unordered or less or equal */
    if (!(a > b)) result |= 2048;
    
    /* UNLT: unordered or less than */
    if (!(a >= b)) result |= 4096;
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if (a < b || a > b) {
        if (a == a && b == b) result |= 8192;
    }
    
    return result;
}

/* Test function focusing on NaN comparisons */
int nan_comparison_tests(void) {
    int results = 0;
    volatile long double nan_q = __builtin_nanl("");
    volatile long double nan_s = sqrtl(-1.0L);
    volatile long double inf_p = __builtin_infl();
    volatile long double inf_n = -__builtin_infl();
    volatile long double normal = 3.14159265358979323846L;
    volatile long double zero = 0.0L;
    
    /* Compare NaN with various values */
    results |= (nan_q < normal) ? 1 : 0;
    results |= (nan_q > normal) ? 2 : 0;
    results |= (nan_q <= normal) ? 4 : 0;
    results |= (nan_q >= normal) ? 8 : 0;
    results |= (nan_q == normal) ? 16 : 0;
    results |= (nan_q != normal) ? 32 : 0;
    
    /* Compare NaN with infinity */
    results |= (nan_q < inf_p) ? 64 : 0;
    results |= (nan_q > inf_n) ? 128 : 0;
    results |= (nan_q == inf_p) ? 256 : 0;
    
    /* Compare NaN with NaN */
    results |= (nan_q < nan_s) ? 512 : 0;
    results |= (nan_q > nan_s) ? 1024 : 0;
    results |= (nan_q == nan_s) ? 2048 : 0;
    results |= (nan_q != nan_s) ? 4096 : 0;
    
    /* Arithmetic producing NaN */
    volatile long double nan_div = zero / zero;
    results |= (nan_div != nan_div) ? 8192 : 0;
    
    return results;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Compare long double with float/double (causing promotions) */
    result |= (ld > (long double)f) ? 1 : 0;
    result |= (ld < (long double)d) ? 2 : 0;
    result |= ((long double)f == ld) ? 4 : 0;
    result |= ((long double)d != ld) ? 8 : 0;
    
    /* Integer constant comparisons */
    result |= (ld > 10.0L) ? 16 : 0;
    result |= (ld < -5.0L) ? 32 : 0;
    result |= ((long double)(int)f == ld) ? 64 : 0;
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ldbl_condition(long double start, long double end, long double step) {
    int count = 0;
    volatile long double x = start;
    
    /* While loop with complex condition */
    while (x < end && x == x) {  /* x == x checks for NaN */
        count++;
        x += step;
        
        /* Nested if with long double comparison */
        if (x > end / 2.0L) {
            count += 10;
        }
        
        /* Prevent infinite loops */
        if (count > 1000) break;
    }
    
    return count;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b) {
    int result = 0;
    
    /* Complex condition to force multiple comparisons */
    if (a < b) {
        result = 1;
    } else if (a > b) {
        result = 2;
    } else if (a == b) {
        result = 3;
    } else {
        /* This branch is taken for unordered comparisons (NaN involved) */
        result = 4;
    }
    
    /* Nested switch based on the result */
    switch (result) {
        case 1:
            if (a != a || b != b) result += 10;  /* UNORDERED check */
            break;
        case 2:
            if (!(a <= b)) result += 20;  /* UNGT check */
            break;
        case 3:
            if (!(a < b || a > b)) result += 30;  /* UNEQ check */
            break;
        case 4:
            if (!(a >= b)) result += 40;  /* UNLT check */
            break;
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    /* Initialize array with mixed values */
    global_ldbl[0] = 1.0L;
    global_ldbl[1] = 2.0L;
    global_ldbl[2] = __builtin_nanl("");
    global_ldbl[3] = __builtin_infl();
    global_ldbl[4] = -__builtin_infl();
    global_ldbl[5] = 0.0L;
    global_ldbl[6] = -0.0L;
    global_ldbl[7] = 3.14159265358979323846L;
    global_ldbl[8] = 0.0L / 0.0L;  /* Generate signaling NaN */
    global_ldbl[9] = sqrtl(-1.0L); /* Another NaN */
    global_ldbl[10] = 100.0L;
    global_ldbl[11] = -100.0L;
    global_ldbl[12] = 1e-10L;
    global_ldbl[13] = 1e10L;
    global_ldbl[14] = (long double)(1ULL << 63);
    global_ldbl[15] = 1.0L / global_ldbl[5];  /* Infinity */
    
    int results[20];
    int result_index = 0;
    
    /* Test 1: Complex x87 comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ldbl(0), get_ldbl(1), get_ldbl(2), get_ldbl(3));
    
    /* Test 2: NaN-focused comparisons */
    results[result_index++] = nan_comparison_tests();
    
    /* Test 3: Mixed precision */
    results[result_index++] = mixed_precision_comparisons(
        1.5f, 2.5, get_ldbl(7));
    
    /* Test 4: Loop with long double conditions */
    results[result_index++] = loop_with_ldbl_condition(
        get_ldbl(0), get_ldbl(10), get_ldbl(1));
    
    /* Test 5: Switch based on comparisons */
    results[result_index++] = switch_on_comparison(
        get_ldbl(2), get_ldbl(0));  /* NaN vs normal */
    results[result_index++] = switch_on_comparison(
        get_ldbl(0), get_ldbl(1));  /* normal < normal */
    results[result_index++] = switch_on_comparison(
        get_ldbl(1), get_ldbl(0));  /* normal > normal */
    results[result_index++] = switch_on_comparison(
        get_ldbl(0), get_ldbl(0));  /* normal == normal */
    
    /* Test 6: More complex scenarios */
    volatile long double x = get_ldbl(0);
    volatile long double y = get_ldbl(1);
    volatile long double z = get_ldbl(2);
    volatile long double w = get_ldbl(3);
    
    /* Complex boolean expression */
    results[result_index++] = (x < y) && (z <= w) && (x != y) && !(z == w);
    
    /* Chained comparisons */
    results[result_index++] = (x < y && y < get_ldbl(10) && x != z);
    
    /* Test 7: Direct unordered comparisons */
    results[result_index++] = !(get_ldbl(2) < get_ldbl(0));  /* UNGE: nlt */
    results[result_index++] = !(get_ldbl(2) <= get_ldbl(0)); /* UNGT: nle */
    results[result_index++] = !(get_ldbl(0) > get_ldbl(2));  /* UNLE: ule */
    results[result_index++] = !(get_ldbl(0) >= get_ldbl(2)); /* UNLT: ult */
    
    /* Test 8: Ordered vs unordered explicit checks */
    results[result_index++] = (get_ldbl(0) == get_ldbl(0) && get_ldbl(1) == get_ldbl(1));
    results[result_index++] = (get_ldbl(2) != get_ldbl(2) || get_ldbl(0) != get_ldbl(0));
    
    /* Compute final hash to prevent dead code elimination */
    int final_hash = 0;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= results[i];
        /* Also use the results to affect control flow */
        if (results[i] & 1) {
            final_hash += i;
        }
    }
    
    printf("Test results hash: %d\n", final_hash);
    printf("Number of tests executed: %d\n", result_index);
    
    return final_hash != 0 ? 0 : 1;
}
