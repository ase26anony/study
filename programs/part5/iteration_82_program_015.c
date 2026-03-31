/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld[16];

/* Helper to create complex comparison that may use multiple condition codes */
int complex_ld_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison patterns */
    if ((a != b) && (c <= d)) {
        if (a > 0.0L && b < 0.0L) {
            return 1;
        }
        if (c == d || a >= c) {
            return 2;
        }
    }
    
    if ((a < b) || (c > d)) {
        if (a <= 0.0L && b >= 0.0L) {
            return 3;
        }
    }
    
    /* Unordered comparisons with explicit NaN checks */
    if (a != a) {  /* NaN check */
        return 4;
    }
    
    if (b != b) {
        return 5;
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

/* Test function for unordered comparisons with NaN */
int test_unordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* Comparisons that should trigger UNORDERED/UNEQ/etc. cases */
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    volatile long double inf = __builtin_infl();
    
    /* Compare NaN with normal numbers */
    if (nan1 < x)  result |= 1;    /* Should be false, may use unordered */
    if (nan1 > x)  result |= 2;    /* Should be false, may use unordered */
    if (nan1 <= x) result |= 4;    /* Should be false, may use unordered */
    if (nan1 >= x) result |= 8;    /* Should be false, may use unordered */
    if (nan1 == x) result |= 16;   /* Should be false, may use UNEQ */
    if (nan1 != x) result |= 32;   /* Should be true, may use UNORDERED */
    
    /* Compare NaN with NaN */
    if (nan1 < nan2)  result |= 64;
    if (nan1 > nan2)  result |= 128;
    if (nan1 <= nan2) result |= 256;
    if (nan1 >= nan2) result |= 512;
    if (nan1 == nan2) result |= 1024;  /* Should be false */
    if (nan1 != nan2) result |= 2048;  /* Should be true */
    
    /* Compare with infinity */
    if (x < inf)  result |= 4096;
    if (x > -inf) result |= 8192;
    
    /* Arithmetic producing NaN */
    volatile long double sqrt_neg = sqrtl(-1.0L);
    if (sqrt_neg == sqrt_neg) result |= 16384;  /* NaN != NaN, so false */
    
    return result;
}

/* Mixed precision comparisons */
int test_mixed_precision(long double ld, double d, float f) {
    int result = 0;
    
    /* Compare long double with double (promotion to long double) */
    if (ld < (long double)d) result |= 1;
    if (ld > (long double)d) result |= 2;
    
    /* Compare long double with float */
    if (ld == (long double)f) result |= 4;
    if (ld != (long double)f) result |= 8;
    
    /* Compare with integer constant cast to long double */
    if (ld < (long double)100) result |= 16;
    if (ld > (long double)-50) result |= 32;
    
    return result;
}

/* Loop with long double termination condition */
int test_loop_comparisons(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition based on long double comparison */
    while (counter < limit && iterations < 100) {
        /* Nested comparison inside loop */
        if (counter != 0.0L && !(counter != counter)) {  /* Not zero and not NaN */
            iterations++;
        }
        counter += 0.5L;
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int test_switch_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* Complex condition that might use multiple x87 comparisons */
    if ((a < b) && (b > c)) {
        result = 1;
    } else if ((a >= b) || (c <= a)) {
        result = 2;
    } else if (a != a) {  /* a is NaN */
        result = 3;
    } else if (b == c && a != 0.0L) {
        result = 4;
    }
    
    return result;
}

/* Initialize global array with various values */
void init_global_array() {
    global_ld[0] = 1.0L;
    global_ld[1] = -1.0L;
    global_ld[2] = 0.0L;
    global_ld[3] = __builtin_infl();      /* Positive infinity */
    global_ld[4] = -__builtin_infl();     /* Negative infinity */
    global_ld[5] = __builtin_nanl("");    /* Quiet NaN */
    global_ld[6] = 0.0L / 0.0L;           /* Another NaN */
    global_ld[7] = 3.14159265358979323846L; /* Pi */
    global_ld[8] = 2.71828182845904523536L; /* e */
    global_ld[9] = 100.0L;
    global_ld[10] = -100.0L;
    global_ld[11] = 1.0e-10L;
    global_ld[12] = 1.0e10L;
    global_ld[13] = sqrtl(-1.0L);         /* NaN from sqrt(-1) */
    global_ld[14] = 1.0L / 0.0L;          /* Infinity */
    global_ld[15] = -1.0L / 0.0L;         /* -Infinity */
}

int main() {
    init_global_array();
    
    /* Array to store comparison results */
    int results[32];
    int result_index = 0;
    
    /* Read values from global array to prevent optimization */
    volatile long double* ptr = (volatile long double*)global_ld;
    
    /* Test 1: Ordered comparisons between normal numbers */
    results[result_index++] = test_ordered_comparisons(ptr[0], ptr[1]);
    results[result_index++] = test_ordered_comparisons(ptr[7], ptr[8]);
    results[result_index++] = test_ordered_comparisons(ptr[9], ptr[10]);
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_index++] = test_unordered_comparisons(ptr[0], ptr[1]);
    results[result_index++] = test_unordered_comparisons(ptr[5], ptr[0]);  /* NaN vs normal */
    results[result_index++] = test_unordered_comparisons(ptr[5], ptr[6]);  /* NaN vs NaN */
    
    /* Test 3: Mixed precision */
    results[result_index++] = test_mixed_precision(ptr[0], 1.0, 1.0f);
    results[result_index++] = test_mixed_precision(ptr[7], 3.14, 3.14f);
    
    /* Test 4: Complex comparisons */
    results[result_index++] = complex_ld_comparison(ptr[0], ptr[1], ptr[2], ptr[7]);
    results[result_index++] = complex_ld_comparison(ptr[5], ptr[6], ptr[3], ptr[4]);  /* With NaN and Inf */
    
    /* Test 5: Loop comparisons */
    results[result_index++] = test_loop_comparisons(ptr[0], ptr[9]);
    
    /* Test 6: Switch based on comparisons */
    results[result_index++] = test_switch_comparison(ptr[0], ptr[1], ptr[2]);
    results[result_index++] = test_switch_comparison(ptr[5], ptr[0], ptr[1]);  /* With NaN */
    
    /* Test 7: Direct comparisons that might trigger specific condition codes */
    volatile long double nan_val = ptr[5];
    volatile long double inf_val = ptr[3];
    volatile long double normal = ptr[0];
    
    /* These should generate various x87 condition codes */
    int direct_results = 0;
    if (!(nan_val == nan_val)) direct_results |= 1;   /* UNORDERED/UNEQ */
    if (nan_val != normal) direct_results |= 2;       /* UNORDERED */
    if (!(nan_val < normal)) direct_results |= 4;     /* UNGE (nlt) */
    if (!(nan_val <= normal)) direct_results |= 8;    /* UNGT (nle) */
    if (normal <= inf_val) direct_results |= 16;      /* UNLE (ule) */
    if (normal < inf_val) direct_results |= 32;       /* UNLT (ult) */
    if (normal != normal) direct_results |= 64;       /* Should be 0 */
    if (normal == normal) direct_results |= 128;      /* Should be 1 */
    
    results[result_index++] = direct_results;
    
    /* Test 8: More edge cases */
    volatile long double zero = 0.0L;
    volatile long double neg_zero = -0.0L;
    if (zero == neg_zero) results[result_index++] = 1;  /* Should be true */
    if (1.0L / zero == inf_val) results[result_index++] = 2;  /* Should be true */
    if (-1.0L / zero == -inf_val) results[result_index++] = 4;  /* Should be true */
    
    /* Compute a simple hash of all results to prevent dead code elimination */
    int final_hash = 0;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= results[i];
    }
    
    printf("Result hash: %d\n", final_hash);
    printf("Number of tests executed: %d\n", result_index);
    
    return final_hash != 0 ? 0 : 1;
}
