/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -o x87_test x87_comparison_test.c
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
    if (a <= b) result |= 4;     /* LE */
    if (c >= d) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (c != d) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with potential NaN */
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    
    /* UNORDERED: compare NaN with normal number */
    if (!(nan1 < b) && !(nan1 > b) && !(nan1 == b)) {
        result |= 64;  /* unordered */
    }
    
    /* UNEQ: unordered or equal */
    if (nan1 == nan1) {  /* This is false for NaN */
        result |= 128;
    }
    
    /* UNGE: not less than (unordered or greater or equal) */
    if (!(nan1 < c)) {
        result |= 256;
    }
    
    /* UNGT: not less than or equal (unordered or greater) */
    if (!(nan1 <= d)) {
        result |= 512;
    }
    
    /* UNLE: unordered or less or equal */
    if (!(nan1 > a)) {
        result |= 1024;
    }
    
    /* UNLT: unordered or less than */
    if (!(nan1 >= b)) {
        result |= 2048;
    }
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if ((a < b) || (a > b)) {
        result |= 4096;
    }
    
    return result;
}

/* Function with mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Promote float to long double */
    if ((long double)f < ld) result++;
    
    /* Promote double to long double */
    if (d > (long double)f) result++;
    
    /* Integer constant cast to long double */
    if (ld == (long double)42) result++;
    
    /* Complex expression */
    if ((f != d) && (ld <= (long double)100.0)) result++;
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ld_comparison(long double start, long double end, long double step) {
    volatile long double counter = start;
    int iterations = 0;
    
    while (counter < end && iterations < 100) {
        /* Prevent infinite loop with NaN */
        if (counter != counter) break;  /* Check for NaN */
        
        counter += step;
        iterations++;
        
        /* Nested comparison in loop */
        if (counter > start && counter < end) {
            iterations |= 0x1000;
        }
    }
    
    return iterations;
}

/* Switch based on comparison results */
int switch_on_comparison(long double a, long double b) {
    int result = 0;
    
    /* Force evaluation of multiple comparisons */
    if (a < b) result = 1;
    else if (a > b) result = 2;
    else if (a == b) result = 3;
    else result = 4;  /* unordered (NaN involved) */
    
    /* Additional switch to force jump table */
    switch (result) {
        case 1: return 100;
        case 2: return 200;
        case 3: return 300;
        case 4: return 400;
        default: return 0;
    }
}

/* Test function focusing on NaN comparisons */
int nan_comparison_tests(void) {
    volatile long double nan_q = __builtin_nanl("");  /* quiet NaN */
    volatile long double nan_s = sqrtl(-1.0L);        /* signaling NaN from sqrt(-1) */
    volatile long double inf_p = __builtin_infl();
    volatile long double inf_n = -__builtin_infl();
    volatile long double zero = 0.0L;
    volatile long double normal = 3.14159265358979323846L;
    
    int results = 0;
    
    /* Compare NaN with various values */
    results |= (nan_q == nan_q) ? 0 : 1;           /* false */
    results |= (nan_q != nan_q) ? 2 : 0;           /* true */
    results |= (nan_q < normal) ? 4 : 0;           /* false */
    results |= (nan_q > normal) ? 8 : 0;           /* false */
    results |= (nan_q <= inf_p) ? 16 : 0;          /* false */
    results |= (nan_q >= inf_n) ? 32 : 0;          /* false */
    
    /* Compare two different NaNs */
    results |= (nan_q == nan_s) ? 64 : 0;          /* false */
    results |= (nan_q != nan_s) ? 128 : 0;         /* true */
    
    /* Compare infinity */
    results |= (inf_p > normal) ? 256 : 0;         /* true */
    results |= (inf_n < normal) ? 512 : 0;         /* true */
    results |= (inf_p == inf_p) ? 1024 : 0;        /* true */
    
    /* Division by zero producing infinity */
    volatile long double div_inf = 1.0L / zero;
    results |= (div_inf == inf_p) ? 2048 : 0;      /* true */
    
    return results;
}

/* Main test harness */
int main(void) {
    /* Initialize array with mixed values */
    global_ld[0] = 1.0L;
    global_ld[1] = 2.0L;
    global_ld[2] = 3.14159265358979323846L;
    global_ld[3] = __builtin_infl();
    global_ld[4] = -__builtin_infl();
    global_ld[5] = __builtin_nanl("");
    global_ld[6] = 0.0L;
    global_ld[7] = -0.0L;
    global_ld[8] = 1.0L / 0.0L;      /* +inf */
    global_ld[9] = -1.0L / 0.0L;     /* -inf */
    global_ld[10] = 0.0L / 0.0L;     /* NaN */
    global_ld[11] = sqrtl(-1.0L);    /* NaN */
    global_ld[12] = 100.0L;
    global_ld[13] = 200.0L;
    global_ld[14] = 300.0L;
    global_ld[15] = 400.0L;
    
    int result_hash = 0;
    
    /* Test 1: Complex x87 comparisons */
    int r1 = complex_x87_comparison(
        get_ld(0), get_ld(1), get_ld(2), get_ld(3)
    );
    result_hash ^= r1;
    
    /* Test 2: Mixed precision */
    int r2 = mixed_precision_comparisons(1.5f, 2.5, get_ld(2));
    result_hash ^= r2;
    
    /* Test 3: Loop with comparison */
    int r3 = loop_with_ld_comparison(get_ld(0), get_ld(12), get_ld(1));
    result_hash ^= r3;
    
    /* Test 4: Switch on comparison */
    int r4 = switch_on_comparison(get_ld(0), get_ld(1));
    result_hash ^= r4;
    
    /* Test 5: NaN comparisons */
    int r5 = nan_comparison_tests();
    result_hash ^= r5;
    
    /* Additional direct tests for uncovered cases */
    volatile long double x = get_ld(5);  /* NaN */
    volatile long double y = get_ld(0);  /* 1.0 */
    
    /* Force generation of specific condition codes */
    if (!(x < y) && !(x > y) && !(x == y)) {
        result_hash |= 0x80000000;  /* UNORDERED */
    }
    
    if (!(x < y)) {
        result_hash ^= 0x40000000;  /* UNGE */
    }
    
    if (!(x <= y)) {
        result_hash ^= 0x20000000;  /* UNGT */
    }
    
    if (!(x > y)) {
        result_hash ^= 0x10000000;  /* UNLE */
    }
    
    if (!(x >= y)) {
        result_hash ^= 0x08000000;  /* UNLT */
    }
    
    /* Final verification output */
    printf("Result hash: 0x%08x\n", result_hash);
    
    /* Use results to prevent dead code elimination */
    volatile int sink = r1 + r2 + r3 + r4 + r5;
    
    return (result_hash == 0) ? 1 : 0;
}
