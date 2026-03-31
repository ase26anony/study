/* x87_comparison_test.c
 * Designed to trigger x87 condition code output (unord, nlt, ule, etc.)
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];
volatile int array_index = 0;

/* Helper to get unpredictable long double values */
long double get_ldbl(int idx) {
    return global_ldbl_array[idx % 16];
}

/* Complex multi-operand comparison function - designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison patterns */
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;      /* LT */
    if (a > c) result |= 2;      /* GT */
    if (b <= c) result |= 4;     /* LE */
    if (c >= d) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (b != c) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with potential NaN */
    volatile long double nan_val = __builtin_nanl("");
    if (!(a < nan_val)) result |= 64;    /* UNORDERED or UNGE */
    if (!(nan_val > b)) result |= 128;   /* UNORDERED or UNLE */
    if (nan_val == nan_val) result |= 256; /* Always false for NaN - tests UNEQ */
    
    return result;
}

/* Function focusing on unordered comparisons with NaN */
int unordered_comparisons(long double x, long double y) {
    int results = 0;
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;  /* Another way to get NaN */
    long double inf = __builtin_infl();
    
    /* Compare NaN with normal numbers */
    results |= (x < nan1) ? 0 : 1;        /* UNORDERED/UNGE when x is normal */
    results |= (nan1 > y) ? 0 : 2;        /* UNORDERED/UNLE when y is normal */
    results |= (nan1 == x) ? 4 : 0;       /* UNORDERED/UNEQ */
    results |= (nan1 != nan2) ? 8 : 0;    /* UNORDERED - both are NaN */
    
    /* Compare with infinity */
    results |= (inf < nan1) ? 16 : 0;     /* UNORDERED */
    results |= (nan1 > inf) ? 32 : 0;     /* UNORDERED */
    
    /* Complex condition that might use UNLT or UNGT */
    results |= ((x < y) || (x != x)) ? 64 : 0;  /* UNLT when x is NaN */
    
    return results;
}

/* Function with mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Mixed precision - forces promotions to long double */
    if (f < ld) result |= 1;
    if ((long double)d > ld) result |= 2;
    if (ld <= (long double)f) result |= 4;
    if ((long double)3.14 >= ld) result |= 8;  /* Integer constant cast */
    
    /* Arithmetic that might produce NaN */
    long double maybe_nan = sqrtl(ld - 100.0L);  /* Could be NaN if ld < 100 */
    if (maybe_nan == maybe_nan) result |= 16;    /* False for NaN */
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ldbl_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition using long double comparison */
    while (counter < limit && iterations < 100) {
        /* Prevent infinite loops with NaN */
        if (counter != counter) break;
        
        counter += 1.0L;
        iterations++;
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b, long double c) {
    int value = 0;
    
    /* Build a value based on multiple comparisons */
    if (a < b) value = 1;
    else if (a > b) value = 2;
    else if (a == b) value = 3;
    else value = 4;  /* Unordered (NaN involved) */
    
    /* Switch on the comparison result */
    switch (value) {
        case 1:
            return c + 1;
        case 2:
            return c + 2;
        case 3:
            return c + 3;
        case 4:
            return c + 4;
        default:
            return c;
    }
}

/* Main test function */
int main() {
    /* Initialize array with mixed values */
    global_ldbl_array[0] = 1.0L;
    global_ldbl_array[1] = 3.14159265358979323846L;
    global_ldbl_array[2] = __builtin_infl();
    global_ldbl_array[3] = -__builtin_infl();
    global_ldbl_array[4] = __builtin_nanl("");
    global_ldbl_array[5] = 0.0L / 0.0L;  /* NaN */
    global_ldbl_array[6] = 1.0L / 0.0L;  /* Inf */
    global_ldbl_array[7] = -1.0L / 0.0L; /* -Inf */
    global_ldbl_array[8] = 100.0L;
    global_ldbl_array[9] = -100.0L;
    global_ldbl_array[10] = 0.0L;
    global_ldbl_array[11] = -0.0L;
    global_ldbl_array[12] = 1e-4900L;    /* Subnormal */
    global_ldbl_array[13] = sqrtl(-1.0L); /* NaN */
    global_ldbl_array[14] = logl(0.0L);   /* -Inf */
    global_ldbl_array[15] = expl(1000.0L); /* Inf */
    
    int results[20];
    int result_index = 0;
    
    /* Test 1: Complex x87 comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ldbl(0), get_ldbl(1), get_ldbl(2), get_ldbl(3));
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_index++] = unordered_comparisons(get_ldbl(4), get_ldbl(5));
    
    /* Test 3: Mixed precision */
    results[result_index++] = mixed_precision_comparisons(
        3.14f, 2.718281828459045, get_ldbl(6));
    
    /* Test 4: Loop with long double condition */
    results[result_index++] = loop_with_ldbl_condition(get_ldbl(7), get_ldbl(8));
    
    /* Test 5: Switch on comparison */
    results[result_index++] = switch_on_comparison(
        get_ldbl(9), get_ldbl(10), get_ldbl(11));
    
    /* Additional direct comparison tests */
    volatile long double v1 = get_ldbl(12);
    volatile long double v2 = get_ldbl(13);
    volatile long double v3 = get_ldbl(14);
    
    /* Generate various comparison types */
    results[result_index++] = (v1 < v2) ? 1 : 0;
    results[result_index++] = (v2 > v3) ? 1 : 0;
    results[result_index++] = (v1 <= v3) ? 1 : 0;
    results[result_index++] = (v2 >= v1) ? 1 : 0;
    results[result_index++] = (v1 == v2) ? 1 : 0;
    results[result_index++] = (v2 != v3) ? 1 : 0;
    
    /* NaN comparisons that should trigger unordered condition codes */
    long double nan_val = get_ldbl(4);
    results[result_index++] = (nan_val < 1.0L) ? 1 : 0;      /* UNORDERED */
    results[result_index++] = (1.0L > nan_val) ? 1 : 0;      /* UNORDERED */
    results[result_index++] = (nan_val <= 2.0L) ? 1 : 0;     /* UNORDERED */
    results[result_index++] = (2.0L >= nan_val) ? 1 : 0;     /* UNORDERED */
    results[result_index++] = (nan_val == 3.0L) ? 1 : 0;     /* UNORDERED */
    results[result_index++] = (nan_val != nan_val) ? 1 : 0;  /* UNORDERED - always true for NaN */
    
    /* Compute hash to prevent dead code elimination */
    int hash = 0;
    for (int i = 0; i < result_index; i++) {
        hash ^= results[i];
    }
    
    printf("Result hash: %d\n", hash);
    printf("Test completed. Hash value should be non-deterministic due to NaN comparisons.\n");
    
    return 0;
}
