/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];

/* Helper to get unpredictable long double values */
long double get_ldbl(int idx) {
    static volatile long double values[] = {
        1.0L, 2.0L, 3.0L, 4.0L,
        0.0L, -1.0L, 100.0L, 0.001L,
        __builtin_infl(), -__builtin_infl(),
        __builtin_nanl(""), __builtin_nanl("0xdead"),
        0.0L/0.0L, sqrtl(-1.0L)
    };
    return values[idx % 14];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison patterns */
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b && !__builtin_isnan(a) && !__builtin_isnan(b)) {
        result |= 1;
    }
    
    /* Unordered comparisons with NaN */
    if (a != b) {  /* May generate UNEQ or LTGT */
        result |= 2;
    }
    
    /* UNGE: not less than (nlt) */
    if (!(c < d)) {
        result |= 4;
    }
    
    /* UNGT: not less than or equal (nle) */
    if (!(c <= d)) {
        result |= 8;
    }
    
    /* UNLE: unordered or less than or equal (ule) */
    if (a <= b || __builtin_isnan(a) || __builtin_isnan(b)) {
        result |= 16;
    }
    
    /* UNLT: unordered or less than (ult) */
    if (a < b || __builtin_isnan(a) || __builtin_isnan(b)) {
        result |= 32;
    }
    
    /* UNORDERED: check if either is NaN */
    if (__builtin_isnan(c) || __builtin_isnan(d)) {
        result |= 64;
    }
    
    /* ORDERED: check if both are not NaN */
    if (!__builtin_isnan(c) && !__builtin_isnan(d)) {
        result |= 128;
    }
    
    /* LTGT: less than or greater than (unequal but ordered) */
    if ((a < b || a > b) && !__builtin_isnan(a) && !__builtin_isnan(b)) {
        result |= 256;
    }
    
    return result;
}

/* Test function focusing on unordered NaN comparisons */
int test_nan_comparisons(void) {
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = __builtin_nanl("0xbeef");
    volatile long double inf = __builtin_infl();
    volatile long double normal = 3.14159265358979323846L;
    volatile long double zero = 0.0L;
    
    int results = 0;
    
    /* UNORDERED comparisons */
    if (nan1 == nan1) results |= 1;  /* false */
    if (nan1 != nan1) results |= 2;  /* true - UNEQ? */
    if (nan1 < normal) results |= 4;  /* false */
    if (nan1 > normal) results |= 8;  /* false */
    if (normal < nan1) results |= 16; /* false */
    if (normal > nan1) results |= 32; /* false */
    
    /* UNGE: not less than (nlt) with NaN */
    if (!(nan1 < inf)) results |= 64;  /* true */
    
    /* UNGT: not less than or equal (nle) with NaN */
    if (!(nan1 <= inf)) results |= 128; /* true */
    
    /* UNLE: unordered or less than or equal */
    if (nan1 <= inf) results |= 256;  /* true (because unordered) */
    
    /* UNLT: unordered or less than */
    if (nan1 < inf) results |= 512;   /* true (because unordered) */
    
    /* Compare two different NaNs */
    if (nan1 == nan2) results |= 1024; /* false */
    if (nan1 != nan2) results |= 2048; /* true */
    
    /* Arithmetic producing NaN */
    volatile long double nan3 = zero / zero;
    if (__builtin_isnan(nan3)) results |= 4096;
    
    return results;
}

/* Mixed precision comparisons */
int test_mixed_precision(float f, double d, long double ld) {
    int result = 0;
    
    /* Promote float to long double */
    if ((long double)f < ld) {
        result |= 1;
    }
    
    /* Promote double to long double */
    if (d > (long double)f) {
        result |= 2;
    }
    
    /* Integer constant cast to long double */
    if (ld < (long double)100) {
        result |= 4;
    }
    
    /* Complex expression with mixed types */
    if ((long double)f * d != ld / 2.0L) {
        result |= 8;
    }
    
    return result;
}

/* Loop with long double termination condition */
int test_loop_comparisons(void) {
    volatile long double x = 0.0L;
    volatile long double limit = 10.0L;
    int count = 0;
    
    /* Loop condition using long double comparison */
    while (x < limit && !__builtin_isnan(x)) {
        x += 1.0L;
        count++;
        
        /* Nested comparison inside loop */
        if (x != limit / 2.0L) {
            count += 10;
        }
    }
    
    /* Do-while with complex condition */
    volatile long double y = 100.0L;
    do {
        y /= 2.0L;
        if (y <= 1.0L || __builtin_isnan(y)) {
            break;
        }
    } while (y > 0.001L);
    
    return count + (int)y;
}

/* Switch statement based on comparison results */
int test_switch_comparisons(long double a, long double b) {
    int result = 0;
    
    /* Chain of if-else based on comparisons */
    if (a < b && !__builtin_isnan(a) && !__builtin_isnan(b)) {
        result = 1;
    } else if (a > b && !__builtin_isnan(a) && !__builtin_isnan(b)) {
        result = 2;
    } else if (a == b && !__builtin_isnan(a) && !__builtin_isnan(b)) {
        result = 3;
    } else if (__builtin_isnan(a) || __builtin_isnan(b)) {
        result = 4;  /* UNORDERED case */
    } else if (a != b) {
        result = 5;  /* UNEQ or LTGT */
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    /* Initialize global array with mix of values */
    for (int i = 0; i < 16; i++) {
        global_ldbl_array[i] = get_ldbl(i);
    }
    
    int bool_results[64];
    int result_idx = 0;
    
    /* Test 1: Complex x87 comparisons */
    bool_results[result_idx++] = complex_x87_comparison(
        global_ldbl_array[0], global_ldbl_array[1],
        global_ldbl_array[2], global_ldbl_array[3]
    );
    
    /* Test 2: NaN-focused comparisons */
    bool_results[result_idx++] = test_nan_comparisons();
    
    /* Test 3: Mixed precision */
    bool_results[result_idx++] = test_mixed_precision(
        3.14f, 2.718281828459045, global_ldbl_array[4]
    );
    
    /* Test 4: Loop comparisons */
    bool_results[result_idx++] = test_loop_comparisons();
    
    /* Test 5: Switch/if-else comparisons */
    bool_results[result_idx++] = test_switch_comparisons(
        global_ldbl_array[5], global_ldbl_array[6]
    );
    
    /* Additional direct comparison tests */
    volatile long double* ptr = (volatile long double*)global_ldbl_array;
    
    /* Test all relational operators */
    bool_results[result_idx++] = (ptr[0] < ptr[1]) ? 1 : 0;
    bool_results[result_idx++] = (ptr[1] > ptr[2]) ? 1 : 0;
    bool_results[result_idx++] = (ptr[2] <= ptr[3]) ? 1 : 0;
    bool_results[result_idx++] = (ptr[3] >= ptr[4]) ? 1 : 0;
    bool_results[result_idx++] = (ptr[4] == ptr[5]) ? 1 : 0;
    bool_results[result_idx++] = (ptr[5] != ptr[6]) ? 1 : 0;
    
    /* NaN comparisons */
    bool_results[result_idx++] = (ptr[8] == ptr[8]) ? 1 : 0;  /* NaN == NaN is false */
    bool_results[result_idx++] = (ptr[8] != ptr[8]) ? 1 : 0;  /* NaN != NaN is true */
    bool_results[result_idx++] = (ptr[8] < 0.0L) ? 1 : 0;
    bool_results[result_idx++] = (ptr[8] > 0.0L) ? 1 : 0;
    bool_results[result_idx++] = (0.0L < ptr[8]) ? 1 : 0;
    bool_results[result_idx++] = (0.0L > ptr[8]) ? 1 : 0;
    
    /* Infinity comparisons */
    bool_results[result_idx++] = (ptr[9] > 1e100L) ? 1 : 0;
    bool_results[result_idx++] = (-ptr[9] < -1e100L) ? 1 : 0;
    
    /* Complex expressions */
    bool_results[result_idx++] = ((ptr[0] + ptr[1]) * ptr[2] != ptr[3] / ptr[4]) ? 1 : 0;
    bool_results[result_idx++] = (!(ptr[5] < ptr[6]) || __builtin_isnan(ptr[7])) ? 1 : 0;
    
    /* Compute verification hash (XOR of all results) */
    int verification_hash = 0;
    for (int i = 0; i < result_idx; i++) {
        verification_hash ^= bool_results[i];
    }
    
    /* Print hash to prevent dead code elimination */
    printf("Verification hash: %d\n", verification_hash);
    printf("Number of tests executed: %d\n", result_idx);
    
    return verification_hash != 0 ? 0 : 1;
}
