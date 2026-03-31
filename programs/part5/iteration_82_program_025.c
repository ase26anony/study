/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in GCC's i386 backend
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

/* Complex multi-operand comparison using various x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison patterns */
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;      /* LT */
    if (a > c) result |= 2;      /* GT */
    if (a <= d) result |= 4;     /* LE */
    if (b >= c) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (c != d) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with potential NaN */
    volatile long double nan_val = __builtin_nanl("");
    if (!(a < nan_val)) result |= 64;    /* UNORDERED or UNGE */
    if (!(nan_val > b)) result |= 128;   /* UNORDERED or UNLE */
    
    return result;
}

/* Test function focusing on unordered comparisons with NaN */
int test_nan_comparisons(void) {
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;  /* Another way to get NaN */
    long double inf = __builtin_infl();
    long double normal = 3.14159265358979323846L;
    
    int results = 0;
    
    /* Compare NaN with various values */
    results |= (nan1 == nan2) ? 0 : 1;           /* Should be false, NaN != NaN */
    results |= (nan1 != normal) ? 2 : 0;         /* Should be true */
    results |= (nan1 < inf) ? 4 : 0;             /* Should be false, unordered */
    results |= (nan1 > -inf) ? 8 : 0;            /* Should be false, unordered */
    results |= (normal <= nan1) ? 16 : 0;        /* Should be false, unordered */
    results |= (normal >= nan1) ? 32 : 0;        /* Should be false, unordered */
    
    /* Explicit unordered comparisons */
    volatile long double x = nan1;
    volatile long double y = normal;
    
    /* These should generate UNORDERED/UNxx condition codes */
    if (!(x < y)) results |= 64;     /* UNGE or UNORDERED */
    if (!(x > y)) results |= 128;    /* UNLE or UNORDERED */
    if (!(x == y)) results |= 256;   /* UNEQ or UNORDERED */
    if (!(x != y)) results |= 512;   /* Should be UNORDERED */
    
    return results;
}

/* Test mixed precision comparisons */
int test_mixed_precision(void) {
    volatile float f = 2.71828f;
    volatile double d = 1.41421356237;
    volatile long double ld = get_ld(3);
    
    int result = 0;
    
    /* Mixed precision comparisons (will promote to long double) */
    if (f < ld) result |= 1;
    if ((long double)d > ld) result |= 2;
    if (ld <= (long double)f) result |= 4;
    if (ld >= d) result |= 8;  /* d promoted to long double */
    
    /* Compare with integer constant */
    if (ld == 10.0L) result |= 16;
    if (ld != 5L) result |= 32;  /* Integer promoted to long double */
    
    return result;
}

/* Loop with long double termination condition */
int test_loop_comparisons(void) {
    volatile long double counter = 0.0L;
    volatile long double limit = get_ld(5);
    int iterations = 0;
    
    /* Loop condition uses x87 comparison */
    while (counter < limit && iterations < 100) {
        counter += 1.0L;
        iterations++;
        
        /* Nested comparison in loop body */
        if (counter != limit / 2.0L) {
            iterations++;
        }
    }
    
    /* Another loop with complex condition */
    volatile long double x = get_ld(6);
    volatile long double y = get_ld(7);
    int count = 0;
    
    do {
        x *= 0.9L;
        count++;
    } while (x > y && !isnan(x) && count < 50);
    
    return iterations + count;
}

/* Switch statement based on comparison results */
int test_switch_comparisons(long double a, long double b) {
    int result = 0;
    
    /* Multiple comparisons whose results direct control flow */
    if (a < b) {
        result = 1;
    } else if (a > b) {
        result = 2;
    } else if (a == b) {
        result = 3;
    } else {
        /* NaN comparison - unordered */
        result = 4;
    }
    
    /* Nested switch based on comparison chain */
    volatile long double c = get_ld(8);
    volatile long double d = get_ld(9);
    
    switch (result) {
        case 1:
            if (c <= d && a != b) result += 10;
            break;
        case 2:
            if (c >= d || a == a) result += 20;
            break;
        case 3:
            if (!(c < d)) result += 30;  /* UNGE */
            break;
        case 4:
            if (!(c > d)) result += 40;  /* UNLE */
            break;
    }
    
    return result;
}

/* Generate NaN through various operations */
long double generate_nan(int method) {
    switch (method) {
        case 0: return __builtin_nanl("");
        case 1: return 0.0L / 0.0L;
        case 2: return sqrtl(-1.0L);
        case 3: return __builtin_infl() - __builtin_infl();
        case 4: return __builtin_nanl("0xdeadbeef");
        default: return 1.0L;
    }
}

/* Main test harness */
int main(void) {
    /* Initialize array with mixed values */
    global_ld[0] = 1.0L;
    global_ld[1] = 2.0L;
    global_ld[2] = 3.14159265358979323846L;
    global_ld[3] = -10.5L;
    global_ld[4] = __builtin_infl();
    global_ld[5] = -__builtin_infl();
    global_ld[6] = generate_nan(0);
    global_ld[7] = generate_nan(1);
    global_ld[8] = 100.0L;
    global_ld[9] = 200.0L;
    global_ld[10] = 0.0L;
    global_ld[11] = -0.0L;
    global_ld[12] = 1e-10L;
    global_ld[13] = 1e10L;
    global_ld[14] = generate_nan(2);
    global_ld[15] = generate_nan(3);
    
    int hash = 0;
    
    /* Run all tests, XOR results to create verification hash */
    hash ^= complex_x87_comparison(
        get_ld(0), get_ld(1), get_ld(2), get_ld(3)
    );
    
    hash ^= test_nan_comparisons();
    
    hash ^= test_mixed_precision();
    
    hash ^= test_loop_comparisons();
    
    hash ^= test_switch_comparisons(get_ld(4), get_ld(5));
    hash ^= test_switch_comparisons(get_ld(6), get_ld(7));  /* NaN vs NaN */
    hash ^= test_switch_comparisons(get_ld(0), get_ld(6));  /* Normal vs NaN */
    
    /* Additional direct unordered comparison tests */
    volatile long double nan_val = generate_nan(4);
    volatile long double normal_val = get_ld(8);
    
    /* Force generation of specific unordered condition codes */
    int unordered_results = 0;
    unordered_results |= (!(nan_val < normal_val)) ? 1 : 0;   /* UNGE */
    unordered_results |= (!(nan_val > normal_val)) ? 2 : 0;   /* UNLE */
    unordered_results |= (!(nan_val == normal_val)) ? 4 : 0;  /* UNEQ */
    unordered_results |= (!(normal_val < nan_val)) ? 8 : 0;   /* UNGE */
    unordered_results |= (!(normal_val > nan_val)) ? 16 : 0;  /* UNLE */
    
    hash ^= unordered_results;
    
    /* Test LTGT (unordered and not equal) */
    volatile long double a = get_ld(10);
    volatile long double b = get_ld(11);
    if (a != b && !isnan(a) && !isnan(b)) {
        hash ^= 0x100;
    }
    
    printf("Result hash: %d\n", hash);
    printf("Test completed. Compile with -m32 -O2 -ffloat-store -mfpmath=387\n");
    
    return 0;
}
