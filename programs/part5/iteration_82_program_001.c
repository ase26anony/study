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

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;      /* LT - should generate normal comparison */
    if (c > d) result |= 2;      /* GT */
    if (a <= b) result |= 4;     /* LE */
    if (c >= d) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (c != d) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with potential NaN */
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    
    /* UNORDERED: (a UNORDERED b) - true if either is NaN */
    if (!(a == a) || !(b == b)) result |= 64;
    
    /* UNEQ: unordered or equal */
    if (!(a == a) || !(b == b) || a == b) result |= 128;
    
    /* UNGE: unordered or greater than or equal */
    if (!(a == a) || !(b == b) || a >= b) result |= 256;
    
    /* UNGT: unordered or greater than */
    if (!(a == a) || !(b == b) || a > b) result |= 512;
    
    /* UNLE: unordered or less than or equal */
    if (!(a == a) || !(b == b) || a <= b) result |= 1024;
    
    /* UNLT: unordered or less than */
    if (!(a == a) || !(b == b) || a < b) result |= 2048;
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if (a == a && b == b && a != b) result |= 4096;
    
    return result;
}

/* Function with switch based on long double comparisons */
int x87_switch_logic(long double x, long double y) {
    int code = 0;
    
    /* This should generate various x87 condition code checks */
    if (x < y) code = 1;
    else if (x > y) code = 2;
    else if (x == y) code = 3;
    else if (x != y) code = 4;  /* This might be true for NaN comparisons */
    
    /* Additional unordered checks */
    volatile long double nan = __builtin_nanl("");
    if (x != x || y != y) code = 5;  /* UNORDERED */
    if (!(x >= y)) code = 6;         /* Might generate UNLT (nlt) */
    if (!(x <= y)) code = 7;         /* Might generate UNGT (nle) */
    
    return code;
}

/* Loop with long double termination condition */
int x87_loop_comparisons(long double start, long double limit) {
    volatile long double x = start;
    int iterations = 0;
    
    /* Mixed precision comparison */
    while (x < limit && x == x) {  /* x == x checks for NaN */
        x += 1.5L;
        iterations++;
        
        /* Compare with different types */
        float f = (float)x;
        double d = (double)x;
        
        if (x > (long double)f) iterations++;
        if (x < (long double)d) iterations++;
        
        /* Compare with integer cast */
        if (x > (long double)(iterations * 2)) iterations++;
    }
    
    return iterations;
}

/* NaN-specific comparison tests */
int nan_comparison_tests(void) {
    int results = 0;
    
    /* Generate various NaN values */
    volatile long double qnan = __builtin_nanl("");
    volatile long double snan = __builtin_nansl("");
    volatile long double nan_div = 0.0L / 0.0L;
    volatile long double nan_sqrt = sqrtl(-1.0L);
    
    /* Generate infinities */
    volatile long double pos_inf = __builtin_infl();
    volatile long double neg_inf = -__builtin_infl();
    
    /* Normal numbers for comparison */
    volatile long double normal = 3.14159265358979323846L;
    volatile long double zero = 0.0L;
    
    /* Test all comparisons with NaN */
    results |= (qnan < normal) ? 1 : 0;
    results |= (qnan > normal) ? 2 : 0;
    results |= (qnan <= normal) ? 4 : 0;
    results |= (qnan >= normal) ? 8 : 0;
    results |= (qnan == normal) ? 16 : 0;
    results |= (qnan != normal) ? 32 : 0;
    
    /* NaN vs NaN comparisons */
    results |= (qnan < snan) ? 64 : 0;
    results |= (qnan > snan) ? 128 : 0;
    results |= (qnan <= snan) ? 256 : 0;
    results |= (qnan >= snan) ? 512 : 0;
    results |= (qnan == snan) ? 1024 : 0;
    results |= (qnan != snan) ? 2048 : 0;
    
    /* NaN vs Infinity */
    results |= (qnan < pos_inf) ? 4096 : 0;
    results |= (qnan > neg_inf) ? 8192 : 0;
    
    /* Infinity comparisons (should be ordered) */
    results |= (pos_inf > normal) ? 16384 : 0;
    results |= (neg_inf < normal) ? 32768 : 0;
    
    return results;
}

/* Main test function */
int main(void) {
    /* Initialize array with mixed values */
    global_ld[0] = 1.0L;
    global_ld[1] = 2.0L;
    global_ld[2] = 3.14159265358979323846L;
    global_ld[3] = -2.5L;
    global_ld[4] = __builtin_infl();      /* Positive infinity */
    global_ld[5] = -__builtin_infl();     /* Negative infinity */
    global_ld[6] = __builtin_nanl("");    /* Quiet NaN */
    global_ld[7] = __builtin_nansl("");   /* Signalling NaN */
    global_ld[8] = 0.0L;
    global_ld[9] = -0.0L;
    global_ld[10] = 1.0L / 0.0L;          /* Another infinity */
    global_ld[11] = 0.0L / 0.0L;          /* NaN */
    global_ld[12] = sqrtl(-1.0L);         /* NaN from sqrt(-1) */
    global_ld[13] = 100.0L;
    global_ld[14] = 1.0e-10L;
    global_ld[15] = 1.0e10L;
    
    int result_hash = 0;
    
    /* Test 1: Complex comparisons */
    for (int i = 0; i < 8; i++) {
        long double a = get_ld(i);
        long double b = get_ld(i + 1);
        long double c = get_ld(i + 2);
        long double d = get_ld(i + 3);
        
        int res = complex_x87_comparison(a, b, c, d);
        result_hash ^= res;
    }
    
    /* Test 2: Switch logic */
    for (int i = 0; i < 10; i++) {
        long double x = get_ld(i);
        long double y = get_ld(15 - i);
        
        int code = x87_switch_logic(x, y);
        result_hash ^= (code << (i % 16));
    }
    
    /* Test 3: Loop comparisons */
    int loop_res = x87_loop_comparisons(0.0L, 10.0L);
    result_hash ^= loop_res;
    
    /* Test 4: NaN-specific tests */
    int nan_res = nan_comparison_tests();
    result_hash ^= nan_res;
    
    /* Test 5: Direct unordered comparisons */
    volatile long double nan = __builtin_nanl("");
    volatile long double num = 42.0L;
    
    /* These should generate the specific x87 condition codes */
    if (nan < num) result_hash ^= 0x1111;      /* Should be false */
    if (!(nan >= num)) result_hash ^= 0x2222;  /* UNGE/UNLT territory */
    if (nan != nan) result_hash ^= 0x3333;     /* UNORDERED */
    if (nan == nan) result_hash ^= 0x4444;     /* Should be false */
    
    /* Test 6: Mixed type comparisons */
    float f = 3.14f;
    double d = 2.718281828459045;
    long double ld = get_ld(0);
    
    if (ld > (long double)f) result_hash ^= 0x5555;
    if ((long double)d < ld) result_hash ^= 0x6666;
    if (ld == (long double)42) result_hash ^= 0x7777;
    
    /* Final output to prevent dead code elimination */
    printf("Result hash: 0x%08x\n", result_hash);
    printf("Test completed - check generated assembly for x87 comparison mnemonics\n");
    
    return result_hash != 0 ? 0 : 1;
}
