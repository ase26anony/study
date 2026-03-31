/* test-x87-comparisons.c
 * 
 * This program is specifically designed to trigger the x87 floating-point
 * comparison output logic in GCC's i386 backend, particularly the
 * UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, and LTGT cases.
 * It uses long double extensively to force x87 code generation.
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double g_values[16];

/* Helper to get values with some indirection */
static long double get_value(int idx) {
    volatile long double* ptr = &g_values[idx];
    return *ptr;
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_compare(long double a, long double b, long double c, long double d) {
    /* Ordered comparisons */
    int r1 = (a < b);   /* Should generate ordered less-than */
    int r2 = (c > d);   /* Should generate ordered greater-than */
    int r3 = (a <= c);  /* Should generate ordered less-than-or-equal */
    int r4 = (b >= d);  /* Should generate ordered greater-than-or-equal */
    
    /* Equality comparisons */
    int r5 = (a == b);  /* Should generate ordered equal */
    int r6 = (c != d);  /* Should generate ordered not-equal */
    
    /* Unordered comparisons with explicit NaN checks */
    int r7 = !(a == a) || !(b == b);  /* UNORDERED: either is NaN */
    int r8 = (a != a) || (b != b) || (a == b); /* UNEQ: unordered or equal */
    
    /* More complex unordered comparisons */
    int r9 = !(a < b);  /* UNGE: not less-than (includes unordered) */
    int r10 = !(a <= b); /* UNGT: not less-or-equal (includes unordered) */
    int r11 = (a <= b) || (a != a) || (b != b); /* UNLE: less-or-equal or unordered */
    int r12 = (a < b) || (a != a) || (b != b);  /* UNLT: less-than or unordered */
    int r13 = (a < b) || (b < a); /* LTGT: less-than or greater-than (ordered) */
    
    /* Combine results in a way that uses all comparisons */
    return (r1 ^ r2) | (r3 & r4) | (r5 ^ r6) | 
           (r7 << 1) | (r8 << 2) | (r9 << 3) |
           (r10 << 4) | (r11 << 5) | (r12 << 6) |
           (r13 << 7);
}

/* Function that performs unordered comparisons with NaN values */
int unordered_comparisons(long double x, long double y) {
    int results = 0;
    
    /* Generate NaN values in multiple ways */
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    long double nan3 = sqrtl(-1.0L);
    
    /* Compare NaN with normal numbers */
    results |= (nan1 < x) ? 0 : 1;      /* UNORDERED case */
    results |= (nan2 > y) ? 0 : 2;      /* UNORDERED case */
    results |= (nan3 <= x) ? 0 : 4;     /* UNORDERED case */
    results |= (nan1 >= y) ? 0 : 8;     /* UNORDERED case */
    
    /* Compare NaN with NaN */
    results |= (nan1 == nan2) ? 0 : 16; /* UNORDERED case */
    results |= (nan1 != nan3) ? 32 : 0; /* Always true for distinct NaNs? */
    
    /* Compare normal numbers with potential NaN */
    volatile long double maybe_nan = x;
    if (x != x) maybe_nan = nan1;  /* Force potential NaN */
    
    results |= (maybe_nan < y) ? 0 : 64;   /* Could be UNORDERED */
    results |= (x > maybe_nan) ? 0 : 128;  /* Could be UNORDERED */
    
    return results;
}

/* Function with mixed precision comparisons */
int mixed_precision_comparisons(long double ld, double d, float f) {
    int results = 0;
    
    /* Compare long double with double (promotion to long double) */
    results |= (ld < (long double)d) ? 1 : 0;
    results |= ((long double)f > ld) ? 2 : 0;
    
    /* Compare with integer constants cast to long double */
    results |= (ld <= (long double)100) ? 4 : 0;
    results |= ((long double)-50 >= ld) ? 8 : 0;
    
    /* Equality comparisons with mixed types */
    results |= (ld == (long double)d) ? 16 : 0;
    results |= ((long double)f != ld) ? 32 : 0;
    
    return results;
}

/* Loop with termination based on long double comparison */
int loop_with_fp_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop while counter < limit, but handle NaN case */
    while (counter < limit && counter == counter) {  /* counter == counter checks for NaN */
        counter += 1.0L;
        iterations++;
        if (iterations > 1000) break; /* Safety limit */
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* Use comparison results in switch */
    switch ((a < b) + 2*(a > b) + 4*(a <= c) + 8*(b >= c)) {
        case 0: result = 1; break;  /* All false */
        case 1: result = 2; break;  /* a < b only */
        case 2: result = 3; break;  /* a > b only */
        case 4: result = 4; break;  /* a <= c only */
        case 8: result = 5; break;  /* b >= c only */
        default: result = 6; break; /* Multiple true */
    }
    
    return result;
}

/* Main test function */
int main(void) {
    /* Initialize array with mix of normal numbers, infinity, and NaN */
    g_values[0] = 1.0L;
    g_values[1] = 2.0L;
    g_values[2] = 3.14159265358979323846L;
    g_values[3] = -1.5L;
    g_values[4] = 0.0L;
    g_values[5] = __builtin_infl();  /* Positive infinity */
    g_values[6] = -__builtin_infl(); /* Negative infinity */
    g_values[7] = __builtin_nanl(""); /* Quiet NaN */
    g_values[8] = 0.0L / 0.0L;       /* Another NaN */
    g_values[9] = 100.0L;
    g_values[10] = 1.0e-10L;
    g_values[11] = 1.0e10L;
    g_values[12] = sqrtl(-1.0L);     /* Another NaN */
    g_values[13] = 42.0L;
    g_values[14] = -999.999L;
    g_values[15] = 1.0L / 0.0L;      /* Infinity */
    
    int results[100];
    int result_count = 0;
    
    /* Test 1: Complex comparisons */
    results[result_count++] = complex_compare(
        get_value(0), get_value(1), 
        get_value(2), get_value(3)
    );
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_count++] = unordered_comparisons(
        get_value(4), get_value(5)
    );
    
    /* Test 3: Mixed precision */
    results[result_count++] = mixed_precision_comparisons(
        get_value(6), 
        (double)get_value(7), 
        (float)get_value(8)
    );
    
    /* Test 4: Loop with FP condition */
    results[result_count++] = loop_with_fp_condition(
        get_value(9), get_value(10)
    );
    
    /* Test 5: Switch on comparison results */
    results[result_count++] = switch_on_comparison(
        get_value(11), get_value(12), get_value(13)
    );
    
    /* Test 6: Direct comparisons of all types */
    for (int i = 0; i < 10; i += 2) {
        long double a = get_value(i);
        long double b = get_value(i + 1);
        
        /* Perform all standard comparisons */
        results[result_count++] = (a < b);
        results[result_count++] = (a > b);
        results[result_count++] = (a <= b);
        results[result_count++] = (a >= b);
        results[result_count++] = (a == b);
        results[result_count++] = (a != b);
        
        /* Explicit unordered checks */
        results[result_count++] = !(a == a) || !(b == b);  /* UNORDERED */
        results[result_count++] = (a != a) || (b != b) || (a == b); /* UNEQ */
    }
    
    /* Test 7: Compare NaN with various values */
    long double nan_val = get_value(7);
    for (int i = 0; i < 8; i++) {
        long double val = get_value(i);
        results[result_count++] = (nan_val < val);
        results[result_count++] = (nan_val > val);
        results[result_count++] = (nan_val <= val);
        results[result_count++] = (nan_val >= val);
        results[result_count++] = (nan_val == val);
        results[result_count++] = (nan_val != val);
    }
    
    /* Compute final hash to prevent dead code elimination */
    uint32_t hash = 0;
    for (int i = 0; i < result_count; i++) {
        hash ^= (results[i] << (i % 32));
    }
    
    printf("Result hash: 0x%08x\n", hash);
    printf("Number of comparisons performed: %d\n", result_count);
    
    return (hash != 0) ? 0 : 1;
}
