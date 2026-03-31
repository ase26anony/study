/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in GCC's i386 backend
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double g_ld_array[16];
volatile int g_index = 0;

/* Helper to get dynamic long double values */
long double get_ld_value(int idx) {
    return g_ld_array[idx % 16];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons (should generate "ord" type mnemonics) */
    if (a < b && !(a != a) && !(b != b)) {
        result |= 1;  /* LT - less than */
    }
    
    if (c > d && c == c && d == d) {
        result |= 2;  /* GT - greater than */
    }
    
    if (a <= b && a == a) {
        result |= 4;  /* LE - less or equal */
    }
    
    if (c >= d && d == d) {
        result |= 8;  /* GE - greater or equal */
    }
    
    /* Equality comparisons */
    if (a == b) {
        result |= 16;  /* EQ - equal */
    }
    
    if (c != d) {
        result |= 32;  /* NEQ - not equal */
    }
    
    /* Unordered comparisons with explicit NaN checks */
    if (a != a || b != b) {  /* UNORDERED */
        result |= 64;
    }
    
    if (!(a != a) && !(b != b)) {  /* ORDERED */
        result |= 128;
    }
    
    /* More specific unordered comparisons */
    if ((a != a || b != b) || a == b) {  /* UNEQ */
        result |= 256;
    }
    
    if (!(a < b) || (a != a) || (b != b)) {  /* UNGE */
        result |= 512;
    }
    
    if (!(a <= b) || (a != a) || (b != b)) {  /* UNGT */
        result |= 1024;
    }
    
    if ((a < b) || (a != a) || (b != b)) {  /* UNLT (ult) */
        result |= 2048;
    }
    
    if ((a <= b) || (a != a) || (b != b)) {  /* UNLE (ule) */
        result |= 4096;
    }
    
    if ((a < b) || (a > b)) {  /* LTGT (une) */
        result |= 8192;
    }
    
    return result;
}

/* Test function for ordered comparisons */
int test_ordered_comparisons(void) {
    volatile long double x = get_ld_value(0);
    volatile long double y = get_ld_value(1);
    volatile long double z = get_ld_value(2);
    volatile long double w = get_ld_value(3);
    
    int result = 0;
    
    /* Basic ordered comparisons */
    if (x < y) result ^= 1;
    if (y > z) result ^= 2;
    if (z <= w) result ^= 4;
    if (w >= x) result ^= 8;
    if (x == y) result ^= 16;
    if (y != z) result ^= 32;
    
    /* Compound conditions */
    if ((x < y) && (z > w)) result ^= 64;
    if ((x == y) || (z != w)) result ^= 128;
    
    /* Nested comparisons */
    if (x < y) {
        if (z > w) {
            result ^= 256;
        } else if (z <= w) {
            result ^= 512;
        }
    }
    
    return result;
}

/* Test function for unordered comparisons with NaN */
int test_unordered_comparisons(void) {
    /* Generate NaN values in various ways */
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    volatile long double nan3 = sqrtl(-1.0L);
    volatile long double inf = __builtin_infl();
    volatile long double normal = 3.14159265358979323846L;
    
    int result = 0;
    
    /* Compare NaN with normal numbers */
    if (nan1 < normal) result ^= 1;      /* Should be false, unordered */
    if (nan1 > normal) result ^= 2;      /* Should be false, unordered */
    if (nan1 <= normal) result ^= 4;     /* Should be false, unordered */
    if (nan1 >= normal) result ^= 8;     /* Should be false, unordered */
    if (nan1 == normal) result ^= 16;    /* Should be false */
    if (nan1 != normal) result ^= 32;    /* Should be true */
    
    /* Compare NaN with infinity */
    if (nan2 < inf) result ^= 64;
    if (nan2 > inf) result ^= 128;
    if (nan2 == inf) result ^= 256;
    if (nan2 != inf) result ^= 512;
    
    /* Compare NaN with NaN */
    if (nan1 < nan2) result ^= 1024;
    if (nan1 > nan2) result ^= 2048;
    if (nan1 <= nan2) result ^= 4096;
    if (nan1 >= nan2) result ^= 8192;
    if (nan1 == nan2) result ^= 16384;
    if (nan1 != nan2) result ^= 32768;
    
    /* Explicit unordered checks */
    if (nan3 != nan3) result ^= 65536;  /* NaN check */
    if (normal == normal) result ^= 131072; /* Normal number check */
    
    return result;
}

/* Test function with mixed precision */
int test_mixed_precision(void) {
    volatile float f = 2.71828f;
    volatile double d = 1.41421356237;
    volatile long double ld = get_ld_value(4);
    
    int result = 0;
    
    /* Mixed precision comparisons (will promote to long double) */
    if (ld > (long double)f) result ^= 1;
    if ((long double)d < ld) result ^= 2;
    if (f == (float)ld) result ^= 4;
    if (d != (double)ld) result ^= 8;
    
    /* Integer to long double comparisons */
    if (ld > (long double)42) result ^= 16;
    if ((long double)100 < ld) result ^= 32;
    
    return result;
}

/* Loop with long double termination condition */
int test_loop_comparisons(void) {
    volatile long double counter = get_ld_value(5);
    volatile long double limit = get_ld_value(6);
    int iterations = 0;
    int result = 0;
    
    /* Loop with long double condition */
    while (counter < limit && iterations < 100) {
        if (counter != counter) break;  /* NaN check */
        result ^= (int)counter;
        counter += 1.0L;
        iterations++;
    }
    
    /* Do-while with complex condition */
    volatile long double x = get_ld_value(7);
    volatile long double y = get_ld_value(8);
    
    do {
        result ^= iterations;
        x *= 0.9L;
        iterations++;
    } while (x > y && x == x && iterations < 50);
    
    return result;
}

/* Switch statement based on comparison results */
int test_switch_comparisons(void) {
    volatile long double a = get_ld_value(9);
    volatile long double b = get_ld_value(10);
    int result = 0;
    
    /* Use comparison in switch (indirectly) */
    int cmp_result = 0;
    if (a < b) cmp_result = 1;
    else if (a > b) cmp_result = 2;
    else if (a == b) cmp_result = 3;
    else cmp_result = 4;  /* unordered */
    
    switch (cmp_result) {
        case 1: result = 111; break;  /* less than */
        case 2: result = 222; break;  /* greater than */
        case 3: result = 333; break;  /* equal */
        case 4: result = 444; break;  /* unordered */
        default: result = 0;
    }
    
    return result;
}

int main(void) {
    /* Initialize array with mixed values */
    g_ld_array[0] = 1.0L;
    g_ld_array[1] = 2.0L;
    g_ld_array[2] = 3.0L;
    g_ld_array[3] = 4.0L;
    g_ld_array[4] = 5.0L;
    g_ld_array[5] = 0.0L;
    g_ld_array[6] = 10.0L;
    g_ld_array[7] = 100.0L;
    g_ld_array[8] = 1.0L;
    g_ld_array[9] = __builtin_nanl("");
    g_ld_array[10] = __builtin_infl();
    g_ld_array[11] = -__builtin_infl();
    g_ld_array[12] = 0.0L;
    g_ld_array[13] = -0.0L;
    g_ld_array[14] = 1.0L / 0.0L;  /* infinity */
    g_ld_array[15] = 0.0L / 0.0L;  /* NaN */
    
    int final_hash = 0;
    
    /* Run all test functions */
    final_hash ^= test_ordered_comparisons();
    final_hash ^= test_unordered_comparisons();
    final_hash ^= test_mixed_precision();
    final_hash ^= test_loop_comparisons();
    final_hash ^= test_switch_comparisons();
    
    /* Complex multi-operand comparison */
    volatile long double v1 = get_ld_value(0);
    volatile long double v2 = get_ld_value(1);
    volatile long double v3 = get_ld_value(9);  /* NaN */
    volatile long double v4 = get_ld_value(10); /* Inf */
    
    int complex_result = complex_x87_comparison(v1, v2, v3, v4);
    final_hash ^= complex_result;
    
    /* Additional direct comparisons to ensure code generation */
    volatile long double* ptr = (volatile long double*)&g_ld_array[0];
    
    /* Chain of comparisons */
    if ((ptr[0] < ptr[1]) && (ptr[2] > ptr[3]) && !(ptr[9] == ptr[9])) {
        final_hash ^= 0xAAAA;
    }
    
    if (!(ptr[0] < ptr[1]) || (ptr[9] != ptr[9]) || (ptr[10] == ptr[11])) {
        final_hash ^= 0x5555;
    }
    
    /* Print final hash to prevent dead code elimination */
    printf("Result hash: %d\n", final_hash);
    printf("Test completed - check generated assembly for x87 comparison mnemonics\n");
    
    return final_hash != 0 ? 0 : 1;
}
