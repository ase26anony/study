/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl[16];
volatile int global_index = 0;

/* Helper to get dynamic long double values */
long double get_ldbl(int idx) {
    return global_ldbl[idx % 16];
}

/* Complex multi-operand comparison using various condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison patterns */
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;
    if (c > d) result |= 2;
    if (a <= b) result |= 4;
    if (c >= d) result |= 8;
    if (a == b) result |= 16;
    if (c != d) result |= 32;
    
    /* Unordered comparisons with potential NaN */
    volatile long double nan_val = __builtin_nanl("");
    if (!(a < nan_val)) result |= 64;    /* May generate UNORDERED/UNGE */
    if (!(nan_val > b)) result |= 128;   /* May generate UNORDERED/UNLE */
    if (nan_val == nan_val) result |= 256; /* Always false for NaN */
    
    /* Mixed comparisons */
    if ((a != b) && (c <= d)) result |= 512;
    if ((a == b) || (c >= d)) result |= 1024;
    
    return result;
}

/* Test function focusing on unordered comparisons */
int test_unordered_comparisons(void) {
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;  /* Another NaN */
    volatile long double inf = __builtin_infl();
    volatile long double normal = 3.14159265358979323846L;
    
    int results = 0;
    
    /* Explicit unordered comparisons */
    results |= (nan1 != nan1) ? 1 : 0;        /* NaN != NaN is true */
    results |= (nan1 == nan1) ? 2 : 0;        /* NaN == NaN is false */
    results |= (nan1 < normal) ? 4 : 0;       /* NaN < anything is false (unordered) */
    results |= (normal > nan2) ? 8 : 0;       /* anything > NaN is false (unordered) */
    results |= (nan1 <= inf) ? 16 : 0;        /* NaN <= inf is false */
    results |= (inf >= nan2) ? 32 : 0;        /* inf >= NaN is false */
    
    /* More complex unordered expressions */
    results |= ((nan1 < normal) || (normal > nan2)) ? 64 : 0;
    results |= (!(nan1 == nan1) && (normal == normal)) ? 128 : 0;
    
    return results;
}

/* Test function with loops and control flow */
int test_loop_comparisons(void) {
    volatile long double x = 1.0L;
    volatile long double y = 2.0L;
    volatile long double nan = sqrtl(-1.0L);  /* Generate NaN */
    int count = 0;
    
    /* Loop with long double comparison */
    while (x < y && x == x) {  /* x == x checks for NaN */
        x *= 1.1L;
        count++;
        if (count > 10) break;
    }
    
    /* Switch based on comparison results */
    int result = 0;
    switch(count) {
        case 0: result = (x > y) ? 1 : 0; break;
        case 1: result = (x <= y) ? 2 : 0; break;
        case 2: result = (x >= y) ? 4 : 0; break;
        default: result = (x != y) ? 8 : 0; break;
    }
    
    /* Test with NaN in loop condition */
    volatile long double z = nan;
    for (int i = 0; i < 5 && !(z != z); i++) {  /* z != z is true for NaN */
        result |= (1 << i);
    }
    
    return result | (count << 8);
}

/* Function with mixed precision comparisons */
int test_mixed_precision(void) {
    volatile float f = 2.5f;
    volatile double d = 3.75;
    volatile long double ld = 4.125L;
    volatile long double nan = __builtin_nanl("");
    
    int result = 0;
    
    /* Mixed type comparisons (promotions to long double) */
    result |= ((long double)f < ld) ? 1 : 0;
    result |= (d > (long double)f) ? 2 : 0;
    result |= (ld <= (long double)d) ? 4 : 0;
    result |= ((long double)100 >= ld) ? 8 : 0;
    
    /* Comparisons with NaN */
    result |= ((long double)f < nan) ? 16 : 0;
    result |= (nan > (long double)d) ? 32 : 0;
    result |= (ld <= nan) ? 64 : 0;
    result |= (nan >= (long double)f) ? 128 : 0;
    
    /* Integer constant comparisons */
    result |= (ld == 4.125L) ? 256 : 0;
    result |= (ld != 5.0L) ? 512 : 0;
    result |= (4.0L < ld) ? 1024 : 0;
    result |= (ld > 3.0L) ? 2048 : 0;
    
    return result;
}

/* Helper to generate various condition codes */
int generate_condition_codes(long double a, long double b) {
    /* This function is designed to use many different x87 condition codes */
    int r = 0;
    
    /* Basic ordered comparisons */
    r = (a < b) ? 1 : 0;
    r |= (a > b) ? 2 : 0;
    r |= (a <= b) ? 4 : 0;
    r |= (a >= b) ? 8 : 0;
    r |= (a == b) ? 16 : 0;
    r |= (a != b) ? 32 : 0;
    
    /* Unordered comparisons */
    volatile long double nan = __builtin_nanl("");
    if (!(a < nan)) r |= 64;      /* UNORDERED or UNGE */
    if (!(nan > b)) r |= 128;     /* UNORDERED or UNLE */
    if (a == a && b == b) r |= 256; /* Both are ordered numbers */
    
    return r;
}

int main(void) {
    /* Initialize array with mixed values */
    global_ldbl[0] = 1.0L;
    global_ldbl[1] = 2.5L;
    global_ldbl[2] = __builtin_infl();
    global_ldbl[3] = -__builtin_infl();
    global_ldbl[4] = __builtin_nanl("");
    global_ldbl[5] = 0.0L / 0.0L;
    global_ldbl[6] = sqrtl(-1.0L);
    global_ldbl[7] = 3.14159265358979323846L;
    global_ldbl[8] = 100.0L;
    global_ldbl[9] = -100.0L;
    global_ldbl[10] = 1.0e-10L;
    global_ldbl[11] = 1.0e+10L;
    global_ldbl[12] = 0.0L;
    global_ldbl[13] = -0.0L;
    global_ldbl[14] = 42.0L;
    global_ldbl[15] = __builtin_nanl("0xdeadbeef");
    
    /* Run all tests */
    int results[10];
    
    results[0] = complex_x87_comparison(
        get_ldbl(0), get_ldbl(1), 
        get_ldbl(2), get_ldbl(3)
    );
    
    results[1] = complex_x87_comparison(
        get_ldbl(4), get_ldbl(5),  /* Both NaN */
        get_ldbl(6), get_ldbl(7)
    );
    
    results[2] = test_unordered_comparisons();
    results[3] = test_loop_comparisons();
    results[4] = test_mixed_precision();
    
    /* More specific tests for condition codes */
    results[5] = generate_condition_codes(get_ldbl(0), get_ldbl(1));
    results[6] = generate_condition_codes(get_ldbl(4), get_ldbl(0));  /* NaN vs normal */
    results[7] = generate_condition_codes(get_ldbl(2), get_ldbl(3));  /* +inf vs -inf */
    results[8] = generate_condition_codes(get_ldbl(4), get_ldbl(5));  /* NaN vs NaN */
    results[9] = generate_condition_codes(get_ldbl(13), get_ldbl(12)); /* -0.0 vs +0.0 */
    
    /* Compute verification hash to prevent dead code elimination */
    int verification_hash = 0;
    for (int i = 0; i < 10; i++) {
        verification_hash ^= results[i];
        /* Also use the values to force computation */
        printf("Test %d result: 0x%08x\n", i, results[i]);
    }
    
    printf("Verification hash: 0x%08x\n", verification_hash);
    
    /* Additional complex expression that might use uncovered codes */
    volatile long double x = get_ldbl(global_index++);
    volatile long double y = get_ldbl(global_index++);
    volatile long double z = get_ldbl(global_index++);
    
    /* This complex expression should generate various condition codes */
    int final_check = 0;
    if ((x < y) && (y > z) && (x != z) && !(x != x)) {
        final_check = 1;
    }
    if ((x == x) && (y == y) && (z == z)) {  /* All are ordered */
        final_check |= 2;
    }
    if (!(x < __builtin_nanl(""))) {  /* Unordered comparison */
        final_check |= 4;
    }
    
    printf("Final check: %d\n", final_check);
    
    return verification_hash != 0 ? 0 : 1;
}
