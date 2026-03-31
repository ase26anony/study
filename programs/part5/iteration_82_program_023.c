/* x87_comparison_test.c
 * Designed to trigger x87 floating-point comparison mnemonics in GCC's i386 backend
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];

/* Helper to create complex comparison that may use multiple condition codes */
static int complex_ldbl_comparison(long double a, long double b, 
                                   long double c, long double d) {
    /* This complex expression should generate various x87 condition codes */
    if ((a != b) && (c <= d)) {
        if ((a > 0.0L) || (b < c)) {
            return 1;
        }
    }
    if ((a == c) || (d >= b)) {
        if (!(a < d) && (b != c)) {
            return 2;
        }
    }
    return 0;
}

/* Test ordered comparisons with normal numbers */
int test_ordered_comparisons(void) {
    volatile long double x = 3.14159265358979323846L;
    volatile long double y = 2.71828182845904523536L;
    volatile long double z = 1.41421356237309504880L;
    
    int result = 0;
    
    /* All standard relational operators */
    if (x > y) result |= 1;
    if (y < z) result |= 2;
    if (x >= y) result |= 4;
    if (z <= x) result |= 8;
    if (x == x) result |= 16;  /* Should be true */
    if (y != z) result |= 32;  /* Should be true */
    
    /* Nested comparisons */
    if ((x > y) && (y > z)) result |= 64;
    if ((x != y) || (z < x)) result |= 128;
    
    return result;
}

/* Test unordered comparisons with NaN */
int test_unordered_comparisons(void) {
    /* Generate NaN values using different methods */
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    volatile long double nan3 = sqrtl(-1.0L);
    volatile long double inf = __builtin_infl();
    volatile long double normal = 42.0L;
    
    int result = 0;
    
    /* Comparisons involving NaN - these are unordered */
    if (nan1 == nan1) result |= 1;      /* false - NaN != NaN */
    if (nan1 != nan1) result |= 2;      /* true - NaN != NaN */
    if (nan1 < normal) result |= 4;     /* false - unordered */
    if (nan1 > normal) result |= 8;     /* false - unordered */
    if (normal < nan2) result |= 16;    /* false - unordered */
    if (normal > nan2) result |= 32;    /* false - unordered */
    
    /* Compare NaN with infinity */
    if (nan3 < inf) result |= 64;       /* false - unordered */
    if (nan3 > -inf) result |= 128;     /* false - unordered */
    
    /* Compare two different NaNs */
    if (nan1 == nan2) result |= 256;    /* false */
    if (nan1 != nan2) result |= 512;    /* true - both are NaN but different */
    
    return result;
}

/* Test mixed comparisons that may generate UNEQ, UNGE, UNGT, etc. */
int test_mixed_unordered_comparisons(void) {
    volatile long double nan = __builtin_nanl("");
    volatile long double inf = __builtin_infl();
    volatile long double neg_inf = -__builtin_infl();
    volatile long double zero = 0.0L;
    volatile long double one = 1.0L;
    
    int result = 0;
    
    /* These comparisons may generate various unordered condition codes */
    
    /* UNORDERED: (a UNORDERED b) - true if either operand is NaN */
    if (!(nan == nan)) result |= 1;     /* Equivalent to UNORDERED check */
    
    /* UNEQ: unordered or equal */
    /* We'll create a situation that might use UNEQ */
    volatile long double a = nan;
    volatile long double b = one;
    if (!(a < b) && !(a > b)) result |= 2;  /* Could use UNEQ */
    
    /* UNGE: unordered or greater-or-equal */
    /* NOT (a < b) where a or b is NaN */
    if (!(nan < one)) result |= 4;
    
    /* UNGT: unordered or greater-than */
    /* NOT (a <= b) where a or b is NaN */
    if (!(nan <= one)) result |= 8;
    
    /* UNLE: unordered or less-or-equal */
    /* NOT (a > b) where a or b is NaN */
    if (!(nan > one)) result |= 16;
    
    /* UNLT: unordered or less-than */
    /* NOT (a >= b) where a or b is NaN */
    if (!(nan >= one)) result |= 32;
    
    /* LTGT: less, greater, or unordered (but not equal) */
    /* (a < b) || (a > b) which is !(a == b) for non-NaN, 
       but different for NaN */
    if (one < zero || one > zero) result |= 64;
    
    return result;
}

/* Test with function parameters to force runtime evaluation */
int test_param_comparisons(long double p1, long double p2, 
                           long double p3, long double p4) {
    int result = 0;
    
    /* Complex expression that may use multiple condition codes */
    if ((p1 < p2) && (p3 >= p4)) {
        result |= 1;
    }
    
    if ((p1 != p2) || (p3 == p4)) {
        result |= 2;
    }
    
    /* Nested ternary with comparisons */
    result |= ((p1 > p2) ? 4 : 0);
    result |= ((p3 <= p4) ? 8 : 0);
    
    /* Call complex helper */
    result |= (complex_ldbl_comparison(p1, p2, p3, p4) << 4);
    
    return result;
}

/* Loop with long double comparison as termination condition */
int test_loop_comparisons(void) {
    volatile long double x = 100.0L;
    volatile long double y = 0.0L;
    int count = 0;
    
    /* Loop condition based on long double comparison */
    while (x > y && !isnan(x)) {
        x = x / 2.0L;
        y = y + 1.0L;
        count++;
        if (count > 50) break; /* Safety */
    }
    
    return count;
}

/* Test mixed precision comparisons */
int test_mixed_precision(void) {
    volatile double d = 3.14159;
    volatile float f = 2.71828f;
    volatile long double ld = 1.41421356237309504880L;
    int result = 0;
    
    /* Mixed precision comparisons (will promote to long double) */
    if (ld > d) result |= 1;
    if (f < ld) result |= 2;
    if ((long double)d >= ld) result |= 4;
    if ((long double)f <= ld) result |= 8;
    
    /* Compare with integer constant cast to long double */
    if (ld > (long double)1) result |= 16;
    if (ld < (long double)2) result |= 32;
    
    return result;
}

/* Initialize global array with mix of values */
void init_global_array(void) {
    for (int i = 0; i < 16; i++) {
        switch (i % 5) {
            case 0: global_ldbl_array[i] = (long double)i * 1.23456789L; break;
            case 1: global_ldbl_array[i] = __builtin_nanl(""); break;
            case 2: global_ldbl_array[i] = __builtin_infl(); break;
            case 3: global_ldbl_array[i] = -__builtin_infl(); break;
            case 4: global_ldbl_array[i] = 0.0L / 0.0L; break;
        }
    }
}

int main(void) {
    init_global_array();
    
    int results[10];
    int result_index = 0;
    
    /* Run all tests */
    results[result_index++] = test_ordered_comparisons();
    results[result_index++] = test_unordered_comparisons();
    results[result_index++] = test_mixed_unordered_comparisons();
    results[result_index++] = test_loop_comparisons();
    results[result_index++] = test_mixed_precision();
    
    /* Test with global array values (prevents constant folding) */
    results[result_index++] = test_param_comparisons(
        global_ldbl_array[0], global_ldbl_array[1],
        global_ldbl_array[2], global_ldbl_array[3]);
    
    results[result_index++] = test_param_comparisons(
        global_ldbl_array[4], global_ldbl_array[5],
        global_ldbl_array[6], global_ldbl_array[7]);
    
    /* Additional complex test */
    volatile long double x = global_ldbl_array[8];
    volatile long double y = global_ldbl_array[9];
    volatile long double z = global_ldbl_array[10];
    volatile long double w = global_ldbl_array[11];
    
    int complex_result = 0;
    if ((x != y) && (z <= w)) complex_result |= 1;
    if ((x == z) || (y >= w)) complex_result |= 2;
    if (!(x < y) && !(z > w)) complex_result |= 4;
    
    results[result_index++] = complex_result;
    
    /* Switch statement based on comparison results */
    int switch_result = 0;
    switch (results[0] & 0x7) {
        case 0: switch_result = 1; break;
        case 1: switch_result = 2; break;
        case 2: switch_result = 3; break;
        case 3: switch_result = 4; break;
        case 4: switch_result = 5; break;
        case 5: switch_result = 6; break;
        case 6: switch_result = 7; break;
        case 7: switch_result = 8; break;
    }
    results[result_index++] = switch_result;
    
    /* Compute final hash to prevent dead code elimination */
    int final_hash = 0;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= results[i];
    }
    
    printf("Test results hash: 0x%08X\n", final_hash);
    printf("(This output ensures all comparisons executed)\n");
    
    return final_hash != 0 ? 0 : 1;
}
