/* x87_comparison_tests.c - Targeting uncovered lines 13992-14017 in i386.cc */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];
volatile int array_index = 0;

/* Helper to get unpredictable long double values */
long double get_ldbl(int idx) {
    return global_ldbl_array[idx % 16];
}

/* Complex comparison function using multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison patterns */
    if ((a != b) && (c <= d)) {
        if (a > 0.0L && b < 0.0L) {
            return 1;
        } else if (a == c || b == d) {
            return 2;
        }
    }
    
    if ((a >= b) || (c < d)) {
        if (!(a == b) && (c > d)) {
            return 3;
        }
    }
    
    /* Unordered comparisons with explicit NaN checks */
    if (a != a) {  /* NaN check */
        if (b == b) {  /* Not NaN */
            return 4;
        }
    }
    
    return 0;
}

/* Function focusing on unordered comparisons with NaN */
int unordered_comparisons(long double nan_val, long double normal, long double inf) {
    int result = 0;
    
    /* UNORDERED cases (any operand is NaN) */
    if (nan_val < normal) result |= 1;      /* Should be false */
    if (nan_val > normal) result |= 2;      /* Should be false */
    if (nan_val <= normal) result |= 4;     /* Should be false */
    if (nan_val >= normal) result |= 8;     /* Should be false */
    if (nan_val == normal) result |= 16;    /* Should be false */
    if (nan_val != normal) result |= 32;    /* Should be true */
    
    /* UNEQ (unordered or equal) */
    long double another_nan = nan_val + normal;  /* Produces NaN */
    if (nan_val == another_nan) result |= 64;    /* UNEQ comparison */
    
    /* UNGE (not less than) - nlt */
    if (!(normal < inf)) result |= 128;          /* Should be true */
    
    /* UNGT (not less than or equal) - nle */
    if (!(normal <= 0.0L)) result |= 256;        /* Depends on normal */
    
    /* UNLE (unordered or less than or equal) - ule */
    if (nan_val <= nan_val) result |= 512;       /* UNLE comparison */
    
    /* UNLT (unordered or less than) - ult */
    if (nan_val < nan_val) result |= 1024;       /* UNLT comparison */
    
    /* LTGT (less than or greater than) - une */
    if (normal != 0.0L) result |= 2048;          /* LTGT comparison */
    
    return result;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Comparisons causing type promotions */
    if (f < ld) result |= 1;
    if ((long double)d > ld) result |= 2;
    if (f == (long double)42) result |= 4;
    if ((double)ld != d) result |= 8;
    
    /* Integer constant comparisons */
    if (ld > 100L) result |= 16;
    if ((long double)(int)f <= ld) result |= 32;
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ldbl_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* While loop with compound condition */
    while (counter < limit && counter == counter) {  /* counter == counter checks for NaN */
        counter += 1.0L;
        iterations++;
        if (iterations > 100) break;  /* Safety limit */
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b) {
    int result = 0;
    
    /* Use comparison results in switch */
    if (a < b) {
        result = 1;
    } else if (a > b) {
        result = 2;
    } else if (a == b) {
        result = 3;
    } else {
        /* This catches NaN comparisons (unordered) */
        result = 4;
    }
    
    /* Nested comparisons */
    if ((a != b) && (a > 0.0L) && (b < 0.0L)) {
        result += 10;
    }
    
    return result;
}

/* Main test function */
int main(void) {
    /* Initialize array with various values */
    global_ldbl_array[0] = 0.0L;
    global_ldbl_array[1] = 1.0L;
    global_ldbl_array[2] = -1.0L;
    global_ldbl_array[3] = 3.14159265358979323846L;
    global_ldbl_array[4] = __builtin_infl();      /* Positive infinity */
    global_ldbl_array[5] = -__builtin_infl();     /* Negative infinity */
    global_ldbl_array[6] = __builtin_nanl("");    /* Quiet NaN */
    global_ldbl_array[7] = 0.0L / 0.0L;           /* Another NaN */
    global_ldbl_array[8] = sqrtl(-1.0L);          /* NaN from sqrt(-1) */
    global_ldbl_array[9] = 1.0L / 0.0L;           /* Infinity */
    global_ldbl_array[10] = -1.0L / 0.0L;         /* -Infinity */
    global_ldbl_array[11] = 100.0L;
    global_ldbl_array[12] = 1.0e-10L;
    global_ldbl_array[13] = 1.0e10L;
    global_ldbl_array[14] = __builtin_nanl("0xdead"); /* NaN with payload */
    global_ldbl_array[15] = 42.0L;
    
    int results[50];
    int result_index = 0;
    
    /* Test 1: Complex comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ldbl(0), get_ldbl(1), get_ldbl(2), get_ldbl(3));
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_index++] = unordered_comparisons(
        get_ldbl(6),  /* NaN */
        get_ldbl(1),  /* 1.0 */
        get_ldbl(4)   /* +Inf */
    );
    
    /* Test 3: More unordered comparisons */
    results[result_index++] = unordered_comparisons(
        get_ldbl(7),  /* 0/0 NaN */
        get_ldbl(0),  /* 0.0 */
        get_ldbl(5)   /* -Inf */
    );
    
    /* Test 4: Mixed precision */
    float f = 3.14f;
    double d = 2.718281828459045;
    results[result_index++] = mixed_precision_comparisons(f, d, get_ldbl(3));
    
    /* Test 5: Loop with long double condition */
    results[result_index++] = loop_with_ldbl_condition(get_ldbl(0), get_ldbl(11));
    
    /* Test 6: Switch on comparison results */
    results[result_index++] = switch_on_comparison(get_ldbl(1), get_ldbl(2));
    results[result_index++] = switch_on_comparison(get_ldbl(6), get_ldbl(1)); /* NaN vs normal */
    results[result_index++] = switch_on_comparison(get_ldbl(4), get_ldbl(5)); /* +Inf vs -Inf */
    
    /* Test 7: Direct comparisons of all types */
    for (int i = 0; i < 8; i++) {
        volatile long double a = get_ldbl(i);
        volatile long double b = get_ldbl(i + 1);
        
        results[result_index++] = (a < b) ? 1 : 0;
        results[result_index++] = (a > b) ? 1 : 0;
        results[result_index++] = (a <= b) ? 1 : 0;
        results[result_index++] = (a >= b) ? 1 : 0;
        results[result_index++] = (a == b) ? 1 : 0;
        results[result_index++] = (a != b) ? 1 : 0;
    }
    
    /* Test 8: NaN with arithmetic */
    volatile long double nan1 = get_ldbl(6);
    volatile long double nan2 = get_ldbl(7);
    results[result_index++] = (nan1 == nan2) ? 1 : 0;  /* UNEQ */
    results[result_index++] = (nan1 < nan2) ? 1 : 0;   /* UNLT */
    results[result_index++] = (nan1 > nan2) ? 1 : 0;   /* UNGT */
    results[result_index++] = (nan1 <= nan2) ? 1 : 0;  /* UNLE */
    results[result_index++] = (nan1 >= nan2) ? 1 : 0;  /* UNGE */
    
    /* Test 9: Infinity comparisons */
    volatile long double pos_inf = get_ldbl(4);
    volatile long double neg_inf = get_ldbl(5);
    results[result_index++] = (pos_inf > neg_inf) ? 1 : 0;
    results[result_index++] = (pos_inf == pos_inf) ? 1 : 0;
    results[result_index++] = (neg_inf < 0.0L) ? 1 : 0;
    
    /* Test 10: Compound conditions with NaN */
    volatile long double x = get_ldbl(1);
    volatile long double y = get_ldbl(6);  /* NaN */
    results[result_index++] = (x > 0.0L && y != y) ? 1 : 0;  /* y != y is true for NaN */
    results[result_index++] = (x == x || y == y) ? 1 : 0;    /* y == y is false for NaN */
    
    /* Compute verification hash (XOR of all results) */
    int verification_hash = 0;
    for (int i = 0; i < result_index; i++) {
        verification_hash ^= results[i];
    }
    
    /* Print hash to prevent dead code elimination */
    printf("Verification hash: %d\n", verification_hash);
    printf("Total tests executed: %d\n", result_index);
    
    return verification_hash != 0 ? 0 : 1;
}
