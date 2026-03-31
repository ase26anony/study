/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];

/* Helper to get unpredictable long double values */
long double get_ldbl(int idx) {
    return global_ldbl_array[idx];
}

/* Complex comparison function designed to use multiple x87 condition codes */
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
        if (a != c && b != d) {
            return 3;
        }
    }
    
    return 0;
}

/* Function with unordered comparisons involving NaN */
int unordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* Generate NaN values */
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    long double inf = __builtin_infl();
    
    /* Ordered comparisons (should generate "ord" type mnemonics) */
    if (x < y) result |= 1;
    if (x > y) result |= 2;
    if (x <= y) result |= 4;
    if (x >= y) result |= 8;
    if (x == y) result |= 16;
    if (x != y) result |= 32;
    
    /* Unordered comparisons with NaN (should generate "unord", "ueq", etc.) */
    if (nan1 == nan1) result |= 64;        /* false - NaN != NaN */
    if (nan1 != nan1) result |= 128;       /* true - NaN != NaN */
    if (x == nan1) result |= 256;          /* unordered comparison */
    if (x != nan1) result |= 512;          /* unordered comparison */
    if (x < nan1) result |= 1024;          /* unordered comparison */
    if (x > nan1) result |= 2048;          /* unordered comparison */
    
    /* Comparisons with infinity */
    if (x < inf) result |= 4096;
    if (x > -inf) result |= 8192;
    if (inf == inf) result |= 16384;
    
    /* Mixed NaN comparisons */
    if (nan1 == nan2) result |= 32768;     /* false - different NaNs */
    if (nan1 != nan2) result |= 65536;     /* true - different NaNs */
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ldbl_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Force runtime evaluation */
    while (counter < limit && !__builtin_isnan(counter)) {
        counter += 1.0L;
        iterations++;
        if (iterations > 100) break; /* Safety limit */
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* Nested comparisons to force x87 code generation */
    if (a < b) {
        if (b > c) {
            result = 1;
        } else if (b == c) {
            result = 2;
        } else {
            result = 3;
        }
    } else if (a > b) {
        if (__builtin_isunordered(a, c)) {
            result = 4;
        } else if (a <= c) {
            result = 5;
        } else {
            result = 6;
        }
    } else { /* a == b */
        if (__builtin_isnan(c)) {
            result = 7;
        } else {
            result = 8;
        }
    }
    
    return result;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* These should promote to long double for x87 comparison */
    if (f < ld) result |= 1;
    if ((long double)d > ld) result |= 2;
    if ((long double)f == ld) result |= 4;
    if (ld != (long double)d) result |= 8;
    
    /* Integer constant comparisons */
    if (ld < 100.0L) result |= 16;
    if (ld > -50.0L) result |= 32;
    if (ld == 0.0L) result |= 64;
    if (ld != 1.0L) result |= 128;
    
    return result;
}

/* Main test function */
int main(void) {
    /* Initialize array with mixed values */
    global_ldbl_array[0] = 1.0L;
    global_ldbl_array[1] = 2.0L;
    global_ldbl_array[2] = -3.5L;
    global_ldbl_array[3] = 0.0L;
    global_ldbl_array[4] = __builtin_nanl("");
    global_ldbl_array[5] = __builtin_infl();
    global_ldbl_array[6] = -__builtin_infl();
    global_ldbl_array[7] = 0.0L / 0.0L; /* Another NaN */
    global_ldbl_array[8] = 100.0L;
    global_ldbl_array[9] = -100.0L;
    
    /* Results array to prevent dead code elimination */
    int results[50];
    int result_index = 0;
    
    /* Test 1: Basic ordered comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ldbl(0), get_ldbl(1), get_ldbl(2), get_ldbl(3));
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_index++] = unordered_comparisons(get_ldbl(0), get_ldbl(4));
    results[result_index++] = unordered_comparisons(get_ldbl(4), get_ldbl(5));
    results[result_index++] = unordered_comparisons(get_ldbl(5), get_ldbl(6));
    
    /* Test 3: Loop with long double condition */
    results[result_index++] = loop_with_ldbl_condition(get_ldbl(3), get_ldbl(8));
    
    /* Test 4: Switch based on comparisons */
    results[result_index++] = switch_on_comparison(
        get_ldbl(0), get_ldbl(1), get_ldbl(2));
    results[result_index++] = switch_on_comparison(
        get_ldbl(4), get_ldbl(5), get_ldbl(6));
    
    /* Test 5: Mixed precision */
    float f = 3.14f;
    double d = 2.718281828459045;
    results[result_index++] = mixed_precision_comparisons(f, d, get_ldbl(0));
    
    /* Test 6: More complex unordered scenarios */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Compare all combinations, including NaN values */
            long double a = get_ldbl(i);
            long double b = get_ldbl(j);
            
            /* Force evaluation of all comparison types */
            int cmp_result = 0;
            if (a < b) cmp_result |= 1;
            if (a > b) cmp_result |= 2;
            if (a <= b) cmp_result |= 4;
            if (a >= b) cmp_result |= 8;
            if (a == b) cmp_result |= 16;
            if (a != b) cmp_result |= 32;
            if (__builtin_isunordered(a, b)) cmp_result |= 64;
            
            results[result_index++] = cmp_result;
            if (result_index >= 50) break;
        }
        if (result_index >= 50) break;
    }
    
    /* Compute verification hash (XOR of all results) */
    int verification_hash = 0;
    for (int i = 0; i < result_index; i++) {
        verification_hash ^= results[i];
    }
    
    /* Print hash to prevent dead code elimination */
    printf("Verification hash: %d\n", verification_hash);
    printf("Number of tests executed: %d\n", result_index);
    
    return 0;
}
