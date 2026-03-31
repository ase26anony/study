/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];

/* Helper to create complex comparison using multiple condition codes */
static int complex_x87_comparison(long double a, long double b, 
                                  long double c, long double d) {
    /* This should generate multiple x87 comparison instructions */
    if ((a != b) && (c <= d)) {
        if (a > 0.0L && b < 0.0L) {
            return 1;
        } else if (c == d || a < c) {
            return 2;
        }
    } else if (a >= b || c > d) {
        if (!(a == b) && (c < d)) {
            return 3;
        }
    }
    return 0;
}

/* Test ordered comparisons with normal numbers */
static int test_ordered_comparisons(void) {
    volatile long double x = 3.14159265358979323846L;
    volatile long double y = 2.71828182845904523536L;
    volatile long double z = -1.5L;
    volatile long double w = 0.0L;
    
    int result = 0;
    
    /* All standard relational operators */
    if (x < y) result |= 0x01;
    if (x > y) result |= 0x02;
    if (z <= w) result |= 0x04;
    if (x >= y) result |= 0x08;
    if (x == x) result |= 0x10;  /* Should be true */
    if (x != y) result |= 0x20;
    
    /* Complex compound condition */
    if ((x > z) && (y < x) && (w == 0.0L)) {
        result |= 0x40;
    }
    
    /* Mixed precision comparisons */
    double d = 2.5;
    float f = -3.0f;
    if (x > d) result |= 0x80;
    if ((long double)f <= z) result |= 0x100;
    
    return result;
}

/* Test unordered comparisons with NaN */
static int test_unordered_comparisons(void) {
    /* Generate NaN values using different methods */
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    volatile long double nan3 = sqrtl(-1.0L);
    volatile long double inf = __builtin_infl();
    volatile long double normal = 42.0L;
    
    int result = 0;
    
    /* Compare NaN with normal numbers */
    if (nan1 < normal) result |= 0x01;   /* false, unordered */
    if (nan1 > normal) result |= 0x02;   /* false, unordered */
    if (nan1 <= normal) result |= 0x04;  /* false, unordered */
    if (nan1 >= normal) result |= 0x08;  /* false, unordered */
    if (nan1 == normal) result |= 0x10;  /* false, unordered */
    if (nan1 != normal) result |= 0x20;  /* true! NaN != normal */
    
    /* Compare NaN with infinity */
    if (nan2 < inf) result |= 0x40;
    if (nan2 > -inf) result |= 0x80;
    if (nan2 == inf) result |= 0x100;
    if (nan2 != inf) result |= 0x200;    /* true */
    
    /* Compare NaN with NaN */
    if (nan1 < nan2) result |= 0x400;
    if (nan1 > nan2) result |= 0x800;
    if (nan1 == nan2) result |= 0x1000;  /* false! NaN != NaN */
    if (nan1 != nan2) result |= 0x2000;  /* true! NaN != NaN */
    
    /* Check for NaN using comparisons */
    if (!(nan3 == nan3)) result |= 0x4000;  /* NaN != itself */
    if (nan3 != nan3) result |= 0x8000;     /* Also true for NaN */
    
    return result;
}

/* Test with values read from global array to prevent optimization */
static int test_array_based_comparisons(void) {
    int result = 0;
    
    /* Initialize array with mixed values */
    for (int i = 0; i < 16; i++) {
        switch (i % 5) {
            case 0: global_ldbl_array[i] = (long double)i * 1.5L; break;
            case 1: global_ldbl_array[i] = __builtin_nanl(""); break;
            case 2: global_ldbl_array[i] = __builtin_infl(); break;
            case 3: global_ldbl_array[i] = -__builtin_infl(); break;
            case 4: global_ldbl_array[i] = 0.0L / 0.0L; break;
        }
    }
    
    /* Perform comparisons between array elements */
    for (int i = 0; i < 15; i++) {
        volatile long double a = global_ldbl_array[i];
        volatile long double b = global_ldbl_array[i + 1];
        
        /* Complex condition that may use various x87 condition codes */
        if ((a < b) || (a != a) || (b != b)) {
            result ^= (i << 1);
        }
        
        if ((a >= 0.0L) && (b <= 0.0L) && (a == a) && (b == b)) {
            result ^= (i << 2);
        }
    }
    
    return result;
}

/* Loop with long double termination condition */
static int test_loop_comparisons(void) {
    volatile long double counter = 0.0L;
    volatile long double limit = 10.0L;
    int iterations = 0;
    
    while (counter < limit && !(counter != counter)) {  /* Check not NaN */
        counter += 1.0L;
        iterations++;
        
        /* Nested comparison inside loop */
        if (counter > limit / 2.0L) {
            volatile long double temp = counter * 0.5L;
            if (temp < limit && temp > 0.0L) {
                iterations ^= (int)temp;
            }
        }
    }
    
    /* Another loop with decreasing counter */
    volatile long double dec = 20.0L;
    while (dec > 0.0L && dec == dec) {  /* dec == dec checks for NaN */
        dec -= 2.5L;
        iterations++;
        
        /* Complex comparison */
        if ((dec <= 10.0L) != (dec >= 5.0L)) {
            iterations ^= 0x55;
        }
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
static int test_switch_on_comparisons(long double a, long double b) {
    int result = 0;
    
    /* Use comparison results to drive switch */
    if (a < b) {
        result = 1;
    } else if (a > b) {
        result = 2;
    } else if (a == b) {
        result = 3;
    } else {
        /* This catches unordered cases (NaN involved) */
        result = 4;
    }
    
    /* Nested switch based on multiple comparisons */
    switch (result) {
        case 1:
            if ((a != a) || (b != b)) result = 10;  /* NaN detected */
            break;
        case 2:
            if (a > 0.0L && b < 0.0L) result = 20;
            break;
        case 3:
            if (a == 0.0L && b == 0.0L) result = 30;
            break;
        case 4:
            /* Definitely unordered */
            if (!(a == a)) result = 41;  /* a is NaN */
            else if (!(b == b)) result = 42;  /* b is NaN */
            else result = 40;
            break;
    }
    
    return result;
}

/* Function with multiple parameters for complex comparison */
static int multi_operand_comparison(long double a, long double b, 
                                    long double c, long double d,
                                    long double e, long double f) {
    /* Designed to use multiple x87 condition codes */
    int result = 0;
    
    /* Complex boolean expression */
    if ((a < b) && (c > d) && (e != f)) {
        result = 1;
    } else if ((a >= b) || (c <= d) || (e == f)) {
        result = 2;
    }
    
    /* Nested comparisons */
    if (!(a != a) && !(b != b)) {  /* Both are not NaN */
        if ((a == c) != (b == d)) {  /* XOR-like condition */
            result |= 0x10;
        }
    }
    
    /* Mixed type comparisons */
    if ((long double)(int)a > b && (long double)(float)c < d) {
        result |= 0x20;
    }
    
    return result;
}

int main(void) {
    int result_hash = 0;
    
    printf("Starting x87 comparison tests...\n");
    
    /* Test 1: Ordered comparisons */
    int r1 = test_ordered_comparisons();
    printf("Ordered comparisons result: 0x%08x\n", r1);
    result_hash ^= r1;
    
    /* Test 2: Unordered comparisons with NaN */
    int r2 = test_unordered_comparisons();
    printf("Unordered comparisons result: 0x%08x\n", r2);
    result_hash ^= r2;
    
    /* Test 3: Array-based comparisons */
    int r3 = test_array_based_comparisons();
    printf("Array comparisons result: 0x%08x\n", r3);
    result_hash ^= r3;
    
    /* Test 4: Loop comparisons */
    int r4 = test_loop_comparisons();
    printf("Loop comparisons result: 0x%08x\n", r4);
    result_hash ^= r4;
    
    /* Test 5: Switch on comparisons */
    int r5 = test_switch_on_comparisons(5.0L, 3.0L);
    result_hash ^= r5;
    r5 = test_switch_on_comparisons(__builtin_nanl(""), 3.0L);
    result_hash ^= r5;
    r5 = test_switch_on_comparisons(0.0L, 0.0L);
    result_hash ^= r5;
    
    /* Test 6: Complex helper function */
    int r6 = complex_x87_comparison(1.0L, 2.0L, 3.0L, 4.0L);
    result_hash ^= r6;
    r6 = complex_x87_comparison(__builtin_nanl(""), 2.0L, 3.0L, 4.0L);
    result_hash ^= r6;
    
    /* Test 7: Multi-operand comparison */
    int r7 = multi_operand_comparison(1.0L, 2.0L, 3.0L, 4.0L, 5.0L, 6.0L);
    result_hash ^= r7;
    r7 = multi_operand_comparison(__builtin_nanl(""), 2.0L, 
                                  __builtin_infl(), 4.0L, 5.0L, 6.0L);
    result_hash ^= r7;
    
    printf("Final result hash: 0x%08x\n", result_hash);
    
    /* Use result to prevent dead code elimination */
    if (result_hash == 0x12345678) {  /* Extremely unlikely */
        printf("Unexpected hash value!\n");
    }
    
    return 0;
}
