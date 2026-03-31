/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbls[16];
int global_index = 0;

/* Helper to get dynamic long double values */
long double get_ldbl(int idx) {
    return global_ldbls[idx % 16];
}

/* Complex multi-operand comparison designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    volatile long double v1 = a;
    volatile long double v2 = b;
    volatile long double v3 = c;
    volatile long double v4 = d;
    
    /* Mix of ordered and unordered comparisons */
    int result = 0;
    
    /* UNORDERED case: check if either operand is NaN */
    if (!(v1 == v1) || !(v2 == v2)) {  /* NaN check */
        result |= 1;
    }
    
    /* ORDERED case: check if both are non-NaN */
    if ((v1 == v1) && (v2 == v2)) {
        result |= 2;
    }
    
    /* UNEQ: unordered or equal */
    if (!(v1 < v2) && !(v1 > v2)) {
        result |= 4;
    }
    
    /* UNGE: not less than (greater than or equal OR unordered) */
    if (!(v1 < v2)) {
        result |= 8;
    }
    
    /* UNGT: not less than or equal (greater than OR unordered) */
    if (!(v1 <= v2)) {
        result |= 16;
    }
    
    /* UNLE: less than or equal OR unordered */
    if (!(v1 > v2)) {
        result |= 32;
    }
    
    /* UNLT: less than OR unordered */
    if (!(v1 >= v2)) {
        result |= 64;
    }
    
    /* LTGT: less than or greater than (not equal AND ordered) */
    if ((v1 < v2) || (v1 > v2)) {
        result |= 128;
    }
    
    /* Additional complex condition mixing multiple comparisons */
    if ((v1 != v2) && (v3 <= v4) && (v1 == v1) && (v2 == v2)) {
        result |= 256;
    }
    
    return result;
}

/* Test function for standard comparisons */
int test_standard_comparisons(long double x, long double y) {
    volatile long double a = x;
    volatile long double b = y;
    int results = 0;
    
    /* All standard relational operators */
    if (a < b)  results |= 1;
    if (a > b)  results |= 2;
    if (a <= b) results |= 4;
    if (a >= b) results |= 8;
    if (a == b) results |= 16;
    if (a != b) results |= 32;
    
    /* Compound conditions */
    if ((a < b) && (a != b)) results |= 64;
    if ((a > b) || (a == b)) results |= 128;
    
    return results;
}

/* Test function with NaN values */
int test_nan_comparisons(long double normal, long double nan_val) {
    volatile long double n = normal;
    volatile long double nan = nan_val;
    int results = 0;
    
    /* Comparisons with NaN (should trigger unordered cases) */
    if (n < nan)  results |= 1;    /* Always false */
    if (n > nan)  results |= 2;    /* Always false */
    if (n <= nan) results |= 4;    /* Always false */
    if (n >= nan) results |= 8;    /* Always false */
    if (n == nan) results |= 16;   /* Always false */
    if (n != nan) results |= 32;   /* Always true */
    
    /* NaN vs NaN comparisons */
    volatile long double nan2 = nan_val * 2.0L; /* Still NaN */
    if (nan < nan2)  results |= 64;
    if (nan > nan2)  results |= 128;
    if (nan == nan2) results |= 256;
    if (nan != nan2) results |= 512;
    
    /* Check for NaN using builtin */
    if (__builtin_isnan(nan)) results |= 1024;
    
    return results;
}

/* Test with infinity */
int test_infinity_comparisons(void) {
    volatile long double pos_inf = __builtin_infl();
    volatile long double neg_inf = -__builtin_infl();
    volatile long double large = 1.0e300L;
    volatile long double small = -1.0e300L;
    
    int results = 0;
    
    if (large < pos_inf) results |= 1;
    if (small > neg_inf) results |= 2;
    if (pos_inf == pos_inf) results |= 4;
    if (neg_inf <= pos_inf) results |= 8;
    if (pos_inf > neg_inf) results |= 16;
    
    /* Infinity vs NaN */
    volatile long double nan = 0.0L / 0.0L;
    if (pos_inf > nan) results |= 32;
    if (neg_inf < nan) results |= 64;
    if (pos_inf == nan) results |= 128;
    if (pos_inf != nan) results |= 256;
    
    return results;
}

/* Loop with long double termination condition */
int test_loop_comparisons(long double start, long double limit) {
    volatile long double x = start;
    volatile long double step = 0.1L;
    int count = 0;
    
    /* Loop condition based on long double comparison */
    while (x < limit && !__builtin_isnan(x)) {
        x += step;
        count++;
        if (count > 100) break; /* Safety limit */
    }
    
    /* Another loop with compound condition */
    x = start;
    int count2 = 0;
    do {
        x -= step;
        count2++;
    } while (x > 0.0L && x <= limit && count2 < 50);
    
    return count + count2;
}

/* Mixed precision comparisons */
int test_mixed_precision(float f, double d, long double ld) {
    int results = 0;
    
    /* Comparisons causing promotions to long double */
    volatile long double v1 = f;
    volatile long double v2 = d;
    volatile long double v3 = ld;
    
    if (v1 < v2) results |= 1;
    if ((long double)f > d) results |= 2;
    if (ld <= (long double)f) results |= 4;
    if (d >= ld) results |= 8;
    
    /* Integer constant cast to long double */
    if (v3 > (long double)100) results |= 16;
    if ((long double)0 < v1) results |= 32;
    
    /* Mixed in expression */
    if (v1 + v2 < v3 * 2.0L) results |= 64;
    
    return results;
}

/* Switch statement based on comparison results */
int test_switch_comparison(long double a, long double b) {
    volatile long double x = a;
    volatile long double y = b;
    int result = 0;
    
    /* Use comparison in switch through intermediate */
    int cmp_result;
    if (x < y) cmp_result = 1;
    else if (x > y) cmp_result = 2;
    else if (x == y) cmp_result = 3;
    else cmp_result = 4; /* unordered */
    
    switch (cmp_result) {
        case 1: result = 100; break;
        case 2: result = 200; break;
        case 3: result = 300; break;
        case 4: result = 400; break;
    }
    
    /* Nested if-else chain with complex conditions */
    if (x != y && x == x && y == y) {
        if (x < y) {
            result += 10;
        } else {
            result += 20;
        }
    } else if (!(x == x) || !(y == y)) {
        result += 30;
    }
    
    return result;
}

/* Generate NaN values in various ways */
long double generate_nan(int method) {
    switch (method) {
        case 0: return 0.0L / 0.0L;
        case 1: return __builtin_nanl("");
        case 2: return sqrtl(-1.0L);
        case 3: return asinl(2.0L);
        case 4: return logl(-1.0L);
        default: return __builtin_nanl("0xdead");
    }
}

int main(void) {
    /* Initialize global array with mixed values */
    for (int i = 0; i < 8; i++) {
        global_ldbls[i] = (long double)i * 1.5L;
    }
    global_ldbls[8] = __builtin_infl();      /* +Inf */
    global_ldbls[9] = -__builtin_infl();     /* -Inf */
    global_ldbls[10] = generate_nan(0);      /* NaN from 0/0 */
    global_ldbls[11] = generate_nan(1);      /* NaN from builtin */
    global_ldbls[12] = 1.0e100L;             /* Large number */
    global_ldbls[13] = -1.0e100L;            /* Large negative */
    global_ldbls[14] = 0.0L;                 /* Zero */
    global_ldbls[15] = -0.0L;                /* Negative zero */
    
    /* Array to store all boolean results */
    int bool_results[256];
    int result_count = 0;
    
    /* Test 1: Standard comparisons */
    for (int i = 0; i < 8; i += 2) {
        bool_results[result_count++] = test_standard_comparisons(
            get_ldbl(i), get_ldbl(i+1));
    }
    
    /* Test 2: NaN comparisons */
    bool_results[result_count++] = test_nan_comparisons(
        get_ldbl(0), get_ldbl(10));
    bool_results[result_count++] = test_nan_comparisons(
        get_ldbl(5), get_ldbl(11));
    
    /* Test 3: Infinity comparisons */
    bool_results[result_count++] = test_infinity_comparisons();
    
    /* Test 4: Complex x87 comparisons */
    for (int i = 0; i < 4; i++) {
        bool_results[result_count++] = complex_x87_comparison(
            get_ldbl(i*3), get_ldbl(i*3+1),
            get_ldbl(i*3+2), get_ldbl((i*3+3) % 16));
    }
    
    /* Test 5: Loop comparisons */
    bool_results[result_count++] = test_loop_comparisons(0.0L, 5.0L);
    bool_results[result_count++] = test_loop_comparisons(10.0L, 0.0L);
    
    /* Test 6: Mixed precision */
    float f = 3.14159f;
    double d = 2.718281828459045;
    bool_results[result_count++] = test_mixed_precision(f, d, get_ldbl(0));
    bool_results[result_count++] = test_mixed_precision(f * 2, d / 2, get_ldbl(7));
    
    /* Test 7: Switch-based comparisons */
    for (int i = 0; i < 8; i += 2) {
        bool_results[result_count++] = test_switch_comparison(
            get_ldbl(i), get_ldbl(i+1));
    }
    
    /* Test 8: Direct NaN generation and comparison */
    volatile long double nan1 = generate_nan(0);
    volatile long double nan2 = generate_nan(1);
    volatile long double normal = get_ldbl(3);
    
    /* Perform all possible comparisons */
    int nan_test_results = 0;
    nan_test_results |= (nan1 < normal) ? 1 : 0;
    nan_test_results |= (nan1 > normal) ? 2 : 0;
    nan_test_results |= (nan1 <= normal) ? 4 : 0;
    nan_test_results |= (nan1 >= normal) ? 8 : 0;
    nan_test_results |= (nan1 == normal) ? 16 : 0;
    nan_test_results |= (nan1 != normal) ? 32 : 0;
    nan_test_results |= (nan1 < nan2) ? 64 : 0;
    nan_test_results |= (nan1 > nan2) ? 128 : 0;
    nan_test_results |= (nan1 == nan2) ? 256 : 0;
    nan_test_results |= (nan1 != nan2) ? 512 : 0;
    
    bool_results[result_count++] = nan_test_results;
    
    /* Compute verification hash (XOR of all results) */
    unsigned int verification_hash = 0;
    for (int i = 0; i < result_count; i++) {
        verification_hash ^= (unsigned int)bool_results[i];
    }
    
    /* Print hash to prevent dead code elimination */
    printf("Verification hash: 0x%08x\n", verification_hash);
    printf("Total tests executed: %d\n", result_count);
    
    return (verification_hash == 0) ? 0 : 1;
}
