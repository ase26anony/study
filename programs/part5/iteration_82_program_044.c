/* x87_comparison_test.c
 * Designed to trigger x87 floating-point comparison condition codes
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];
volatile int array_index = 0;

/* Helper to get dynamic long double values */
long double get_ldbl_value(int idx) {
    return global_ldbl_array[idx % 16];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    volatile long double x = a;
    volatile long double y = b;
    volatile long double z = c;
    volatile long double w = d;
    
    /* Mix of ordered and unordered comparisons */
    if ((x != y) && (z <= w)) {
        if (x > 0.0L && !(y == y)) {  /* y is NaN check */
            return 1;
        }
        if (z < w || w != w) {  /* w is NaN check */
            return 2;
        }
    }
    
    if (x >= y) {
        if (z == w) {
            return 3;
        }
        if (z > w && w == w) {  /* w is not NaN */
            return 4;
        }
    }
    
    /* Unordered comparison scenarios */
    if (!(x == x) || !(y == y)) {  /* Either is NaN */
        if (x != x && y != y) {    /* Both are NaN */
            return 5;
        }
        if (!(x == x) && y > 0.0L) {  /* x is NaN, y is positive */
            return 6;
        }
    }
    
    return 0;
}

/* Test function for ordered comparisons */
int test_ordered_comparisons(long double a, long double b) {
    int result = 0;
    
    /* All standard ordered comparisons */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    return result;
}

/* Test function for unordered comparisons with NaN */
int test_unordered_comparisons(long double a, long double b) {
    int result = 0;
    
    /* Comparisons that should trigger UNORDERED/UNEQ/etc. codes */
    volatile long double nan_val = __builtin_nanl("");
    
    /* Compare NaN with normal number */
    if (!(a == a)) result |= 1;      /* UNORDERED: a is NaN */
    if (a != a) result |= 2;         /* UNORDERED: a is NaN */
    
    /* Compare two NaNs */
    if (!(nan_val == nan_val)) result |= 4;
    if (nan_val != nan_val) result |= 8;
    
    /* Compare normal number with NaN */
    if (a == nan_val) result |= 16;  /* Always false, but generates comparison */
    if (a != nan_val) result |= 32;  /* Always true for normal a */
    
    /* Mixed comparisons */
    if (a < nan_val) result |= 64;
    if (a > nan_val) result |= 128;
    
    return result;
}

/* Function with switch based on comparison results */
int comparison_switch(long double x, long double y, long double z) {
    volatile long double a = x;
    volatile long double b = y;
    volatile long double c = z;
    
    int result = 0;
    
    /* Complex condition that may use various x87 codes */
    if ((a < b) && (b > c) && (a != c)) {
        result = 1;
    } else if ((a >= b) || (b <= c)) {
        result = 2;
    } else if (!(a == a) || !(b == b)) {  /* NaN checks */
        result = 3;
    } else if ((a == b) && (b == c)) {
        result = 4;
    }
    
    /* Additional unordered checks */
    if (a != a) {  /* a is NaN */
        result += 10;
    }
    if (0.0L / 0.0L == b) {  /* Generate NaN and compare */
        result += 20;
    }
    
    return result;
}

/* Loop with floating-point termination condition */
int floating_point_loop(long double start, long double limit) {
    volatile long double x = start;
    int count = 0;
    
    /* Loop condition uses x87 comparison */
    while (x < limit && x == x) {  /* x == x checks for NaN */
        x = x * 1.1L;
        count++;
        if (count > 100) break;  /* Safety break */
    }
    
    /* Another loop with different comparison */
    volatile long double y = limit;
    while (y > start && !(y != y)) {  /* y != y would be true for NaN */
        y = y / 1.1L;
        count++;
        if (count > 200) break;
    }
    
    return count;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Compare different precisions (promotions to long double) */
    if ((long double)f < ld) result |= 1;
    if ((long double)d > ld) result |= 2;
    if (f == (float)ld) result |= 4;
    if (d != (double)ld) result |= 8;
    
    /* Integer constant comparisons */
    if (ld < 100.0L) result |= 16;
    if (ld > -50.0L) result |= 32;
    if ((long double)(int)f == ld) result |= 64;
    
    return result;
}

/* Generate NaN values through various methods */
long double generate_nan(int method) {
    volatile long double result;
    
    switch (method % 4) {
        case 0:
            result = __builtin_nanl("");
            break;
        case 1:
            result = 0.0L / 0.0L;
            break;
        case 2:
            result = __builtin_nanl("0xdead");
            break;
        case 3:
            /* sqrt(-1) generates NaN */
            result = sqrtl(-1.0L);
            break;
        default:
            result = 1.0L;
    }
    
    return result;
}

int main() {
    /* Initialize array with mixed values */
    global_ldbl_array[0] = 1.0L;
    global_ldbl_array[1] = 2.0L;
    global_ldbl_array[2] = 3.14159265358979323846L;
    global_ldbl_array[3] = -1.0L;
    global_ldbl_array[4] = 0.0L;
    global_ldbl_array[5] = -0.0L;
    global_ldbl_array[6] = __builtin_infl();
    global_ldbl_array[7] = -__builtin_infl();
    global_ldbl_array[8] = __builtin_nanl("");
    global_ldbl_array[9] = 0.0L / 0.0L;
    global_ldbl_array[10] = sqrtl(-1.0L);
    global_ldbl_array[11] = 1.0e100L;
    global_ldbl_array[12] = 1.0e-100L;
    global_ldbl_array[13] = 2.71828182845904523536L; /* e */
    global_ldbl_array[14] = 1.61803398874989484820L; /* phi */
    global_ldbl_array[15] = generate_nan(2);
    
    int results[50];
    int result_index = 0;
    
    /* Test various comparison scenarios */
    for (int i = 0; i < 8; i++) {
        long double a = get_ldbl_value(i);
        long double b = get_ldbl_value(i + 1);
        long double c = get_ldbl_value(i + 2);
        long double d = get_ldbl_value(i + 3);
        
        results[result_index++] = test_ordered_comparisons(a, b);
        results[result_index++] = test_unordered_comparisons(a, b);
        results[result_index++] = complex_x87_comparison(a, b, c, d);
        results[result_index++] = comparison_switch(a, b, c);
    }
    
    /* Test mixed precision */
    float f = 3.14f;
    double d = 2.718281828459045;
    long double ld = get_ldbl_value(0);
    results[result_index++] = mixed_precision_comparisons(f, d, ld);
    
    /* Test floating point loops */
    results[result_index++] = floating_point_loop(1.0L, 10.0L);
    results[result_index++] = floating_point_loop(-5.0L, 5.0L);
    
    /* Explicit NaN comparisons to trigger UNORDERED/UNEQ/etc. */
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    volatile long double normal = 42.0L;
    volatile long double inf = __builtin_infl();
    
    /* These should generate the specific x87 condition codes */
    if (nan1 == normal) results[result_index++] = 1000;
    if (nan1 != normal) results[result_index++] = 1001;
    if (nan1 < normal)  results[result_index++] = 1002;
    if (nan1 > normal)  results[result_index++] = 1003;
    if (nan1 <= normal) results[result_index++] = 1004;
    if (nan1 >= normal) results[result_index++] = 1005;
    
    if (normal == nan1) results[result_index++] = 1010;
    if (normal != nan1) results[result_index++] = 1011;
    if (normal < nan1)  results[result_index++] = 1012;
    if (normal > nan1)  results[result_index++] = 1013;
    if (normal <= nan1) results[result_index++] = 1014;
    if (normal >= nan1) results[result_index++] = 1015;
    
    /* NaN vs NaN comparisons */
    if (nan1 == nan2) results[result_index++] = 1020;
    if (nan1 != nan2) results[result_index++] = 1021;
    if (nan1 < nan2)  results[result_index++] = 1022;
    if (nan1 > nan2)  results[result_index++] = 1023;
    
    /* Infinity comparisons */
    if (inf == inf)   results[result_index++] = 1030;
    if (inf > normal) results[result_index++] = 1031;
    if (normal < inf) results[result_index++] = 1032;
    
    /* NaN vs Infinity */
    if (nan1 == inf)  results[result_index++] = 1040;
    if (nan1 != inf)  results[result_index++] = 1041;
    if (nan1 < inf)   results[result_index++] = 1042;
    if (nan1 > inf)   results[result_index++] = 1043;
    
    /* Compute verification hash to prevent dead code elimination */
    uint32_t hash = 0;
    for (int i = 0; i < result_index; i++) {
        hash ^= (uint32_t)results[i] + (i * 0x5bd1e995);
    }
    
    printf("Verification hash: 0x%08x\n", hash);
    printf("Total comparisons performed: %d\n", result_index);
    
    return (hash != 0) ? 0 : 1;
}
