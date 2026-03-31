/* x87_comparison_test.c
 * Designed to trigger x87 floating-point comparison condition codes
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double g_vals[16];
volatile int g_index = 0;

/* Helper to get dynamic long double values */
long double get_val(int idx) {
    return g_vals[idx % 16];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons (should generate "ord" type codes) */
    if (a < b && !(a != a) && !(b != b)) {
        result |= 1;  /* LT with ordered operands */
    }
    
    if (c > d && c == c && d == d) {
        result |= 2;  /* GT with ordered operands */
    }
    
    /* Equality with ordered check */
    if (a == b && a == a && b == b) {
        result |= 4;  /* EQ with ordered operands */
    }
    
    /* Unordered comparisons (should generate "unord", "nlt", "nle", etc.) */
    if (a != a || b != b) {  /* UNORDERED check */
        result |= 8;
    }
    
    /* UNEQ: unordered or equal */
    if (!(a < b) && !(b < a)) {  /* Not less and not greater */
        result |= 16;
    }
    
    /* UNGE: not less than (or unordered) */
    if (!(a < b)) {
        result |= 32;
    }
    
    /* UNGT: not less than or equal (or unordered) */
    if (!(a <= b)) {
        result |= 64;
    }
    
    /* UNLE: less than or equal or unordered */
    if (a <= b || a != a || b != b) {
        result |= 128;
    }
    
    /* UNLT: less than or unordered */
    if (a < b || a != a || b != b) {
        result |= 256;
    }
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if ((a < b || a > b) && a == a && b == b) {
        result |= 512;
    }
    
    return result;
}

/* Test function focusing on NaN comparisons */
int nan_comparison_tests(void) {
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;  /* Another NaN */
    volatile long double inf = __builtin_infl();
    volatile long double normal = 3.14159265358979323846L;
    
    int results = 0;
    
    /* Compare NaN with normal number (all should be false for ordered comparisons) */
    if (nan1 < normal) results |= 1;
    if (nan1 > normal) results |= 2;
    if (nan1 <= normal) results |= 4;
    if (nan1 >= normal) results |= 8;
    if (nan1 == normal) results |= 16;
    if (nan1 != normal) results |= 32;  /* This should be true */
    
    /* Compare NaN with NaN */
    if (nan1 < nan2) results |= 64;
    if (nan1 > nan2) results |= 128;
    if (nan1 <= nan2) results |= 256;
    if (nan1 >= nan2) results |= 512;
    if (nan1 == nan2) results |= 1024;
    if (nan1 != nan2) results |= 2048;  /* This should be true */
    
    /* Compare NaN with infinity */
    if (nan1 < inf) results |= 4096;
    if (nan1 > inf) results |= 8192;
    if (nan1 <= inf) results |= 16384;
    if (nan1 >= inf) results |= 32768;
    
    return results;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Compare long double with double (promotion to long double) */
    if (ld < (long double)d) result |= 1;
    if ((long double)f > ld) result |= 2;
    
    /* Compare with integer constant cast to long double */
    if (ld <= (long double)100) result |= 4;
    if ((long double)-50 >= ld) result |= 8;
    
    /* Complex expression with mixed types */
    if ((ld + (long double)f) * (long double)d < (long double)1000.0) {
        result |= 16;
    }
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_x87_condition(void) {
    volatile long double counter = 0.0L;
    volatile long double limit = get_val(5);
    int iterations = 0;
    
    /* Loop condition uses x87 comparison */
    while (counter < limit && counter == counter && limit == limit) {
        counter += get_val(iterations % 3 + 1);
        iterations++;
        if (iterations > 100) break; /* Safety limit */
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b) {
    int result = 0;
    
    /* This switch depends on multiple comparison results */
    if (a < b && a == a && b == b) {
        result = 1;  /* Ordered less than */
    } else if (a > b && a == a && b == b) {
        result = 2;  /* Ordered greater than */
    } else if (a == b && a == a && b == b) {
        result = 3;  /* Ordered equal */
    } else if (a != a || b != b) {
        result = 4;  /* Unordered */
    } else if (!(a < b) && !(a > b)) {
        result = 5;  /* UNEQ */
    } else if (!(a < b)) {
        result = 6;  /* UNGE */
    } else if (!(a <= b)) {
        result = 7;  /* UNGT */
    } else if (a <= b || a != a || b != b) {
        result = 8;  /* UNLE */
    } else if (a < b || a != a || b != b) {
        result = 9;  /* UNLT */
    } else if ((a < b || a > b) && a == a && b == b) {
        result = 10; /* LTGT */
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    /* Initialize array with mixed values */
    g_vals[0] = 1.0L;
    g_vals[1] = 2.0L;
    g_vals[2] = -1.0L;
    g_vals[3] = 0.0L;
    g_vals[4] = __builtin_infl();
    g_vals[5] = -__builtin_infl();
    g_vals[6] = __builtin_nanl("");
    g_vals[7] = 0.0L / 0.0L;
    g_vals[8] = sqrtl(-1.0L);
    g_vals[9] = 100.0L;
    g_vals[10] = 1.0e-10L;
    g_vals[11] = 1.0e10L;
    g_vals[12] = 3.14159265358979323846L;
    g_vals[13] = 2.71828182845904523536L;
    g_vals[14] = __builtin_nanl("0xdead");
    g_vals[15] = 42.0L;
    
    int results[20];
    int result_index = 0;
    
    /* Run various comparison tests */
    results[result_index++] = complex_x87_comparison(
        get_val(0), get_val(1), get_val(2), get_val(3));
    
    results[result_index++] = complex_x87_comparison(
        get_val(6), get_val(0), get_val(4), get_val(5));  /* With NaN and INF */
    
    results[result_index++] = nan_comparison_tests();
    
    results[result_index++] = mixed_precision_comparisons(
        3.14f, 2.71828, get_val(12));
    
    results[result_index++] = loop_with_x87_condition();
    
    /* Test all switch cases */
    for (int i = 0; i < 10; i++) {
        results[result_index++] = switch_on_comparison(
            get_val(i), get_val(i + 1));
    }
    
    /* Additional complex expressions */
    volatile long double x = get_val(0);
    volatile long double y = get_val(1);
    volatile long double z = get_val(6);  /* NaN */
    
    /* Complex boolean expression that should generate various condition codes */
    if ((x < y) && (z != z) && (x == x) && !(y != y)) {
        results[result_index++] = 1;
    } else {
        results[result_index++] = 0;
    }
    
    if (!(x >= y) || (z == z) || !(x == x)) {
        results[result_index++] = 2;
    } else {
        results[result_index++] = 0;
    }
    
    /* Compute hash to prevent dead code elimination */
    uint32_t hash = 0;
    for (int i = 0; i < result_index; i++) {
        hash ^= (results[i] + i) * 0x9e3779b9;
    }
    
    printf("Test hash: 0x%08x\n", hash);
    printf("Number of tests executed: %d\n", result_index);
    
    return (hash != 0) ? 0 : 1;
}
