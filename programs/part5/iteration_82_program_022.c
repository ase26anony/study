/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld[16];
volatile int global_index = 0;

/* Helper to get dynamic long double values */
long double get_ld(int idx) {
    return global_ld[idx % 16];
}

/* Complex comparison function using multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;      /* LT */
    if (a > c) result |= 2;      /* GT */
    if (a <= d) result |= 4;     /* LE */
    if (b >= c) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (c != d) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with explicit NaN checks */
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    volatile long double inf = __builtin_infl();
    
    /* UNORDERED: (a != a) | (b != b) */
    if (!(a == a) || !(b == b)) result |= 64;
    
    /* UNEQ: (a == b) | unordered */
    if (!(a != b)) result |= 128;  /* Using !(a != b) for UNEQ logic */
    
    /* UNGE: !(a < b) */
    if (!(a < b)) result |= 256;
    
    /* UNGT: !(a <= b) */
    if (!(a <= b)) result |= 512;
    
    /* UNLE: !(a > b) */
    if (!(a > b)) result |= 1024;
    
    /* UNLT: !(a >= b) */
    if (!(a >= b)) result |= 2048;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) result |= 4096;
    
    return result;
}

/* Function with nested comparisons */
int nested_x87_comparisons(long double x, long double y, long double z) {
    if (x != y) {
        if (z <= x) {
            volatile long double temp = sqrtl(-1.0L); /* Generate NaN */
            if (temp == temp) { /* Always false for NaN */
                return 1;
            } else {
                /* UNORDERED path - NaN comparison */
                if (!(temp < x) && !(temp > x) && !(temp == x)) {
                    return 2; /* Should take this path */
                }
            }
        } else if (z >= y) {
            return 3;
        }
    }
    
    /* Mixed comparisons */
    volatile double d = 3.14159;
    volatile float f = 2.71828f;
    
    if (x > (long double)d && y < (long double)f) {
        return 4;
    }
    
    return 0;
}

/* Loop with x87 termination condition */
int x87_controlled_loop(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Use volatile to prevent optimization */
    volatile long double increment = 0.1L;
    volatile long double eps = 0.00001L;
    
    while ((counter < limit) && (iterations < 100)) {
        /* Complex condition mixing ordered and unordered checks */
        volatile long double nan_test = __builtin_nanl("");
        if (counter != nan_test) { /* Ordered inequality */
            counter += increment;
            iterations++;
        } else {
            break; /* Should never happen */
        }
        
        /* Near equality check */
        if (fabsl(counter - limit) < eps) {
            break;
        }
    }
    
    return iterations;
}

/* Switch based on comparison results */
int x87_switch_logic(long double a, long double b) {
    int result = 0;
    
    /* Force evaluation of multiple comparisons */
    if (a < b) result = 1;
    else if (a > b) result = 2;
    else if (a == b) result = 3;
    else result = 4; /* Unordered (NaN involved) */
    
    switch (result) {
        case 1: /* LT */
            return a * 2.0L;
        case 2: /* GT */
            return b * 2.0L;
        case 3: /* EQ */
            return a + b;
        case 4: /* UNORDERED */
            return __builtin_nanl("");
        default:
            return 0.0L;
    }
}

/* Test function focusing on unordered comparisons */
int test_unordered_comparisons(void) {
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    volatile long double inf_pos = __builtin_infl();
    volatile long double inf_neg = -__builtin_infl();
    volatile long double normal = 3.14159265358979323846L;
    
    int results = 0;
    
    /* NaN vs Normal */
    results |= (nan1 < normal) ? 0 : 1;
    results |= (nan1 > normal) ? 0 : 2;
    results |= (nan1 <= normal) ? 0 : 4;
    results |= (nan1 >= normal) ? 0 : 8;
    results |= (nan1 == normal) ? 0 : 16;
    results |= (nan1 != normal) ? 32 : 0;
    
    /* NaN vs NaN */
    results |= (nan1 < nan2) ? 0 : 64;
    results |= (nan1 > nan2) ? 0 : 128;
    results |= (nan1 <= nan2) ? 0 : 256;
    results |= (nan1 >= nan2) ? 0 : 512;
    results |= (nan1 == nan2) ? 0 : 1024;
    results |= (nan1 != nan2) ? 2048 : 0;
    
    /* NaN vs Inf */
    results |= (nan1 < inf_pos) ? 0 : 4096;
    results |= (nan1 > inf_neg) ? 0 : 8192;
    
    /* Inf comparisons */
    results |= (inf_pos > inf_neg) ? 16384 : 0;
    results |= (inf_pos == inf_pos) ? 32768 : 0;
    
    return results;
}

/* Main test harness */
int main(void) {
    /* Initialize with mixed values */
    global_ld[0] = 1.0L;
    global_ld[1] = 2.0L;
    global_ld[2] = 3.14159265358979323846L;
    global_ld[3] = __builtin_infl();
    global_ld[4] = -__builtin_infl();
    global_ld[5] = __builtin_nanl("");
    global_ld[6] = 0.0L;
    global_ld[7] = -0.0L;
    global_ld[8] = 1.0L / 0.0L; /* Another Inf */
    global_ld[9] = -1.0L / 0.0L; /* -Inf */
    global_ld[10] = sqrtl(-1.0L); /* NaN */
    global_ld[11] = 100.0L;
    global_ld[12] = 1.0e-10L;
    global_ld[13] = 1.0e10L;
    global_ld[14] = (long double)(1ULL << 60);
    global_ld[15] = 0.0L / 0.0L; /* NaN */
    
    int bool_results[64];
    int result_index = 0;
    
    /* Test 1: Complex comparisons */
    bool_results[result_index++] = complex_x87_comparison(
        get_ld(0), get_ld(1), get_ld(2), get_ld(3)
    ) != 0;
    
    /* Test 2: Nested comparisons */
    bool_results[result_index++] = nested_x87_comparisons(
        get_ld(2), get_ld(3), get_ld(4)
    ) != 0;
    
    /* Test 3: Loop with x87 control */
    bool_results[result_index++] = x87_controlled_loop(
        get_ld(0), get_ld(11)
    ) > 0;
    
    /* Test 4: Unordered comparisons */
    bool_results[result_index++] = test_unordered_comparisons() != 0;
    
    /* Test 5: Direct NaN comparisons */
    volatile long double nan = get_ld(5);
    volatile long double normal = get_ld(2);
    
    bool_results[result_index++] = (nan < normal);
    bool_results[result_index++] = (nan > normal);
    bool_results[result_index++] = (nan <= normal);
    bool_results[result_index++] = (nan >= normal);
    bool_results[result_index++] = (nan == normal);
    bool_results[result_index++] = (nan != normal);
    
    /* Test 6: Normal number comparisons */
    bool_results[result_index++] = (get_ld(0) < get_ld(1));
    bool_results[result_index++] = (get_ld(1) > get_ld(0));
    bool_results[result_index++] = (get_ld(0) <= get_ld(0));
    bool_results[result_index++] = (get_ld(2) >= get_ld(1));
    bool_results[result_index++] = (get_ld(0) == get_ld(0));
    bool_results[result_index++] = (get_ld(0) != get_ld(1));
    
    /* Test 7: Infinity comparisons */
    bool_results[result_index++] = (get_ld(3) > get_ld(4));
    bool_results[result_index++] = (get_ld(4) < get_ld(3));
    bool_results[result_index++] = (get_ld(3) == get_ld(8));
    bool_results[result_index++] = (get_ld(4) == get_ld(9));
    
    /* Test 8: Mixed precision */
    volatile double d = 2.0;
    volatile float f = 3.0f;
    bool_results[result_index++] = (get_ld(0) < (long double)d);
    bool_results[result_index++] = ((long double)f > get_ld(1));
    bool_results[result_index++] = (get_ld(2) == (long double)42);
    
    /* Test 9: Compound conditions */
    bool_results[result_index++] = (get_ld(0) < get_ld(1) && get_ld(1) > get_ld(0));
    bool_results[result_index++] = (get_ld(2) != get_ld(5) || get_ld(3) == get_ld(8));
    bool_results[result_index++] = (!(get_ld(5) < get_ld(0)) && !(get_ld(5) > get_ld(0)));
    
    /* Test 10: More unordered scenarios */
    bool_results[result_index++] = !(get_ld(5) == get_ld(5)); /* NaN != NaN */
    bool_results[result_index++] = !(get_ld(10) < get_ld(10)); /* UNGE */
    bool_results[result_index++] = !(get_ld(15) <= get_ld(15)); /* UNGT */
    bool_results[result_index++] = !(get_ld(5) > get_ld(2)); /* UNLE */
    bool_results[result_index++] = !(get_ld(10) >= get_ld(2)); /* UNLT */
    bool_results[result_index++] = (get_ld(0) < get_ld(1) || get_ld(0) > get_ld(1)); /* LTGT */
    
    /* Compute verification hash */
    int verification_hash = 0;
    for (int i = 0; i < result_index; i++) {
        verification_hash ^= (bool_results[i] << (i % 16));
    }
    
    printf("Verification hash: %d\n", verification_hash);
    printf("Total comparisons tested: %d\n", result_index);
    
    /* Use results to prevent dead code elimination */
    if (verification_hash == 0) {
        printf("All comparisons returned false (unlikely)\n");
    }
    
    return verification_hash != 0 ? 0 : 1;
}
