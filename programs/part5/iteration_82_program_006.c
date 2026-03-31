/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
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

/* Complex multi-operand comparison designed to use various x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate multiple x87 comparisons with different condition codes */
    int result = 0;
    
    /* Ordered comparisons (should use ORDERED/UNORDERED) */
    if (a == b) result |= 1;          /* EQ/UEQ */
    if (a != b) result |= 2;          /* NEQ/UNEQ */
    if (a < b)  result |= 4;          /* LT/UNLT */
    if (a > b)  result |= 8;          /* GT/UNGT */
    if (a <= b) result |= 16;         /* LE/UNLE */
    if (a >= b) result |= 32;         /* GE/UNGE */
    
    /* Unordered comparisons with NaN involvement */
    long double nan_val = __builtin_nanl("");
    if (c != nan_val) result |= 64;   /* Should use UNEQ/NEQ */
    if (c == nan_val) result |= 128;  /* Should use UNORDERED */
    
    /* Mixed comparisons */
    if (a < c && c > d) result |= 256;
    if (a >= nan_val || b <= nan_val) result |= 512;
    
    /* Complex condition that might use LTGT */
    if ((a < b) != (c > d)) result |= 1024;
    
    return result;
}

/* Test function focusing on unordered comparisons with NaN */
int test_nan_comparisons(void) {
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;  /* Another way to get NaN */
    volatile long double inf = __builtin_infl();
    volatile long double normal = 3.14159265358979323846L;
    
    int results = 0;
    
    /* UNORDERED comparisons (NaN compared with anything is unordered) */
    if (nan1 == normal)   results |= 1;    /* false */
    if (nan1 != normal)   results |= 2;    /* true (UNE) */
    if (nan1 < normal)    results |= 4;    /* false */
    if (nan1 > normal)    results |= 8;    /* false */
    if (nan1 <= normal)   results |= 16;   /* false */
    if (nan1 >= normal)   results |= 32;   /* false */
    
    /* NaN compared with NaN */
    if (nan1 == nan2)     results |= 64;   /* false */
    if (nan1 != nan2)     results |= 128;  /* true (UNE) */
    
    /* UNGE/UNLE/UNGT/UNLT scenarios */
    if (!(nan1 < inf))    results |= 256;  /* true (NLT/UNGE) */
    if (!(nan1 > inf))    results |= 512;  /* true (NGT/UNLE) */
    if (!(nan1 <= normal)) results |= 1024; /* true (NLE/UNGT) */
    if (!(nan1 >= normal)) results |= 2048; /* true (NLT/UNGE) */
    
    /* ORDERED comparisons (normal numbers) */
    if (normal == 3.14159265358979323846L) results |= 4096;
    if (normal < 4.0L)    results |= 8192;
    if (normal > 3.0L)    results |= 16384;
    
    return results;
}

/* Test function with loops and conditional flow */
int test_conditional_flow(void) {
    volatile long double x = 1.0L;
    volatile long double y = 2.0L;
    volatile long double z = __builtin_nanl("");
    int count = 0;
    
    /* Loop with long double comparison condition */
    while (x < 10.0L) {
        if (x != z) {  /* Should use UNEQ */
            count++;
        }
        
        /* Switch based on comparison results */
        if (x < y) {
            count += 10;
        } else if (x > y) {
            count += 20;
        } else if (x == y) {
            count += 30;
        }
        
        /* Complex condition */
        if ((x < 5.0L) && (y > 1.0L) && (z != z)) {
            count += 100;
        }
        
        x += 1.0L;
    }
    
    /* Do-while with NaN check */
    volatile long double w = 0.0L;
    do {
        if (w == w) {  /* false for NaN, true for normal numbers */
            count++;
        }
        w += 1.0L;
    } while (w <= 5.0L);
    
    return count;
}

/* Test mixed precision comparisons */
int test_mixed_precision(void) {
    volatile float f = 2.5f;
    volatile double d = 3.75;
    volatile long double ld = 4.125L;
    int result = 0;
    
    /* Mixed precision comparisons (will promote to long double) */
    if (f < ld) result |= 1;
    if ((long double)d > ld) result |= 2;
    if (ld == (long double)f + (long double)d) result |= 4;
    
    /* Integer constant comparisons */
    if (ld > 3) result |= 8;
    if (ld < 5L) result |= 16;
    if (ld == 4.125L) result |= 32;
    
    /* Arithmetic producing NaN */
    volatile long double zero = 0.0L;
    volatile long double neg = -1.0L;
    volatile long double nan_arith = zero / zero;
    volatile long double nan_sqrt = sqrtl(neg);
    
    if (nan_arith != nan_sqrt) result |= 64;  /* UNEQ */
    if (!(nan_arith < 100.0L)) result |= 128; /* NLT/UNGE */
    
    return result;
}

/* Main test harness */
int main(void) {
    /* Initialize array with mixed values */
    for (int i = 0; i < 16; i++) {
        switch (i % 5) {
            case 0: global_ldbl_array[i] = (long double)i * 1.23456789L; break;
            case 1: global_ldbl_array[i] = __builtin_infl(); break;
            case 2: global_ldbl_array[i] = -__builtin_infl(); break;
            case 3: global_ldbl_array[i] = __builtin_nanl(""); break;
            case 4: global_ldbl_array[i] = 0.0L / 0.0L; break;
        }
    }
    
    /* Run all tests */
    int result1 = complex_x87_comparison(
        get_ldbl_value(0),
        get_ldbl_value(1),
        get_ldbl_value(2),
        get_ldbl_value(3)
    );
    
    int result2 = test_nan_comparisons();
    int result3 = test_conditional_flow();
    int result4 = test_mixed_precision();
    
    /* Additional direct tests */
    int result5 = 0;
    volatile long double* ptr = (volatile long double*)global_ldbl_array;
    
    /* Direct pointer-based comparisons to inhibit optimization */
    for (int i = 0; i < 8; i += 2) {
        if (ptr[i] < ptr[i+1]) result5 ^= (1 << i);
        if (ptr[i] > ptr[i+1]) result5 ^= (2 << i);
        if (ptr[i] == ptr[i+1]) result5 ^= (4 << i);
        if (ptr[i] != ptr[i+1]) result5 ^= (8 << i);
        
        /* Unordered checks */
        if (!(ptr[i] < ptr[i+1])) result5 ^= (16 << i);  /* NLT/UNGE */
        if (!(ptr[i] > ptr[i+1])) result5 ^= (32 << i);  /* NGT/UNLE */
    }
    
    /* Compute final hash to prevent dead code elimination */
    int final_hash = result1 ^ result2 ^ result3 ^ result4 ^ result5;
    
    /* Print results to ensure code executes */
    printf("Test results:\n");
    printf("Complex comparison: 0x%08x\n", result1);
    printf("NaN comparisons:    0x%08x\n", result2);
    printf("Conditional flow:   0x%08x\n", result3);
    printf("Mixed precision:    0x%08x\n", result4);
    printf("Direct comparisons: 0x%08x\n", result5);
    printf("Final hash:         0x%08x\n", final_hash);
    
    return (final_hash != 0) ? 0 : 1;
}
