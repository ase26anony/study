/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld[16];
volatile int global_index = 0;

/* Helper to get dynamic long double values */
long double get_ld(int idx) {
    return global_ld[idx % 16];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    volatile long double v1 = a;
    volatile long double v2 = b;
    volatile long double v3 = c;
    volatile long double v4 = d;
    
    /* Mix of ordered and unordered comparisons */
    if ((v1 != v2) && (v3 <= v4)) {
        if (v1 < v2 || v3 > v4) {
            return 1;
        } else if (!(v1 == v2) && (v3 >= v4)) {
            return 2;
        }
    }
    
    /* Explicit unordered check */
    if (v1 != v1 || v2 != v2) {  /* NaN check */
        if (v3 == v3) {  /* Not NaN */
            return 3;
        }
    }
    
    return 0;
}

/* Function with nested comparisons for flow control */
int nested_x87_compare(long double x, long double y, long double z) {
    int result = 0;
    
    /* Chain of if-else based on x87 comparisons */
    if (x < y) {
        result = 1;
        if (z > 0.0L) {
            result = 2;
            if (x != z) {
                result = 3;
            }
        }
    } else if (x > y) {
        result = 4;
        if (z <= x) {
            result = 5;
        }
    } else if (x == y) {  /* Ordered equality */
        result = 6;
        if (z >= y) {
            result = 7;
        }
    } else {
        /* Unordered case (x or y is NaN) */
        result = 8;
    }
    
    return result;
}

/* Loop with x87 termination condition */
int x87_controlled_loop(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition based on x87 comparison */
    while (counter < limit && iterations < 100) {
        counter += 0.5L;
        iterations++;
        
        /* Additional x87 comparison inside loop */
        if (counter != limit && !(counter != counter)) {  /* Not NaN check */
            counter *= 1.01L;
        }
    }
    
    return iterations;
}

/* Test all standard comparisons */
void test_standard_comparisons(long double a, long double b, int* results, int* idx) {
    results[(*idx)++] = (a < b)   ? 1 : 0;   /* LT */
    results[(*idx)++] = (a > b)   ? 1 : 0;   /* GT */
    results[(*idx)++] = (a <= b)  ? 1 : 0;   /* LE */
    results[(*idx)++] = (a >= b)  ? 1 : 0;   /* GE */
    results[(*idx)++] = (a == b)  ? 1 : 0;   /* EQ */
    results[(*idx)++] = (a != b)  ? 1 : 0;   /* NEQ */
}

/* Test with NaN values */
void test_nan_comparisons(long double nan_val, long double normal, int* results, int* idx) {
    /* Compare NaN with normal number */
    results[(*idx)++] = (nan_val < normal)   ? 1 : 0;  /* Should be false (unordered) */
    results[(*idx)++] = (nan_val > normal)   ? 1 : 0;  /* Should be false (unordered) */
    results[(*idx)++] = (nan_val <= normal)  ? 1 : 0;  /* Should be false (unordered) */
    results[(*idx)++] = (nan_val >= normal)  ? 1 : 0;  /* Should be false (unordered) */
    results[(*idx)++] = (nan_val == normal)  ? 1 : 0;  /* Should be false (unordered) */
    results[(*idx)++] = (nan_val != normal)  ? 1 : 0;  /* Should be true (UNEQ/UNORDERED) */
    
    /* Compare NaN with NaN */
    results[(*idx)++] = (nan_val == nan_val) ? 1 : 0;  /* Should be false (UNORDERED) */
    results[(*idx)++] = (nan_val != nan_val) ? 1 : 0;  /* Should be true (UNORDERED) */
}

/* Mixed precision comparisons */
void test_mixed_precision(float f, double d, long double ld, int* results, int* idx) {
    /* Float to long double (promotion) */
    results[(*idx)++] = (f < ld) ? 1 : 0;
    results[(*idx)++] = ((long double)f > ld) ? 1 : 0;
    
    /* Double to long double (promotion) */
    results[(*idx)++] = (d <= ld) ? 1 : 0;
    results[(*idx)++] = (ld >= (long double)d) ? 1 : 0;
    
    /* Integer constant to long double */
    results[(*idx)++] = (ld == 42.0L) ? 1 : 0;
    results[(*idx)++] = (ld != (long double)100) ? 1 : 0;
}

/* Switch statement based on comparison results */
int x87_switch_test(long double a, long double b, long double c) {
    int result = 0;
    
    /* Complex condition that might use various x87 codes */
    if (a < b && b > c) {
        result = 1;
    } else if (a >= b || c <= a) {
        result = 2;
    } else if (a != b && b == c) {
        result = 3;
    } else {
        /* This else case catches unordered comparisons */
        result = 4;
    }
    
    /* Switch on the result to create more control flow */
    switch (result) {
        case 1:
            if (c != 0.0L) result = 10;
            break;
        case 2:
            if (a == b) result = 20;
            break;
        case 3:
            if (b < c) result = 30;
            break;
        case 4:
            /* Might involve unordered comparisons */
            if (a != a || b != b) result = 40;  /* NaN check */
            break;
    }
    
    return result;
}

int main() {
    int results[256];
    int idx = 0;
    
    /* Initialize array with mixed values */
    global_ld[0] = 1.0L;
    global_ld[1] = 2.0L;
    global_ld[2] = 3.14159265358979323846L;
    global_ld[3] = -1.5L;
    global_ld[4] = 0.0L;
    global_ld[5] = __builtin_infl();      /* Positive infinity */
    global_ld[6] = -__builtin_infl();     /* Negative infinity */
    global_ld[7] = __builtin_nanl("");    /* Quiet NaN */
    global_ld[8] = 0.0L / 0.0L;           /* Another NaN */
    global_ld[9] = sqrtl(-1.0L);          /* NaN from sqrt(-1) */
    global_ld[10] = 100.0L;
    global_ld[11] = 1.0e-10L;
    global_ld[12] = 1.0e10L;
    global_ld[13] = -0.0L;
    global_ld[14] = 42.0L;
    global_ld[15] = 99.999L;
    
    /* Test 1: Standard comparisons between normal numbers */
    test_standard_comparisons(get_ld(0), get_ld(1), results, &idx);
    test_standard_comparisons(get_ld(2), get_ld(3), results, &idx);
    
    /* Test 2: Comparisons involving infinity */
    test_standard_comparisons(get_ld(0), get_ld(5), results, &idx);  /* 1.0 < INF */
    test_standard_comparisons(get_ld(6), get_ld(1), results, &idx);  /* -INF < 2.0 */
    
    /* Test 3: NaN comparisons */
    test_nan_comparisons(get_ld(7), get_ld(0), results, &idx);  /* NaN vs 1.0 */
    test_nan_comparisons(get_ld(8), get_ld(9), results, &idx);  /* NaN vs NaN */
    
    /* Test 4: Mixed precision */
    float f_val = 3.14f;
    double d_val = 2.718281828459045;
    test_mixed_precision(f_val, d_val, get_ld(2), results, &idx);
    
    /* Test 5: Complex comparison function */
    results[idx++] = complex_x87_comparison(
        get_ld(0), get_ld(1), get_ld(2), get_ld(3));
    results[idx++] = complex_x87_comparison(
        get_ld(7), get_ld(0), get_ld(5), get_ld(6));  /* With NaN and INF */
    
    /* Test 6: Nested comparisons */
    results[idx++] = nested_x87_compare(get_ld(0), get_ld(1), get_ld(2));
    results[idx++] = nested_x87_compare(get_ld(7), get_ld(0), get_ld(2));  /* With NaN */
    
    /* Test 7: Loop with x87 condition */
    results[idx++] = x87_controlled_loop(get_ld(0), get_ld(10));
    
    /* Test 8: Switch test */
    results[idx++] = x87_switch_test(get_ld(0), get_ld(1), get_ld(2));
    results[idx++] = x87_switch_test(get_ld(7), get_ld(0), get_ld(5));  /* With NaN and INF */
    
    /* Test 9: More edge cases */
    volatile long double zero = 0.0L;
    volatile long double neg_zero = -0.0L;
    results[idx++] = (zero == neg_zero) ? 1 : 0;  /* Should be true */
    results[idx++] = (1.0L / zero == get_ld(5)) ? 1 : 0;  /* +INF == +INF */
    results[idx++] = (-1.0L / zero == get_ld(6)) ? 1 : 0;  /* -INF == -INF */
    
    /* Test 10: Arithmetic producing NaN then comparison */
    volatile long double nan_prod = get_ld(5) * get_ld(6);  /* INF * -INF = -INF? Actually NaN */
    results[idx++] = (nan_prod < get_ld(0)) ? 1 : 0;
    results[idx++] = (nan_prod == nan_prod) ? 1 : 0;
    
    /* Compute hash to prevent dead code elimination */
    uint32_t hash = 0;
    for (int i = 0; i < idx; i++) {
        hash ^= (results[i] << (i % 32));
    }
    
    printf("Test completed. Hash: 0x%08x\n", hash);
    printf("Total comparisons performed: %d\n", idx);
    
    return 0;
}
