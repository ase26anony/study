/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -o x87_test x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld[16];

/* Helper to get unpredictable long double values */
long double get_ld(int idx) {
    static volatile long double values[] = {
        1.0L, 2.0L, 3.0L, 4.0L,
        0.0L, -1.0L, 100.0L, -100.0L,
        __builtin_infl(), -__builtin_infl(),
        __builtin_nanl(""), __builtin_nanl("0xdead"),
        0.0L/0.0L, sqrtl(-1.0L), 1.0L/0.0L, -1.0L/0.0L
    };
    return values[idx % 16];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* Ordered comparisons */
    int r1 = (a < b);   /* LT */
    int r2 = (a > c);   /* GT */
    int r3 = (d <= a);  /* LE */
    int r4 = (b >= c);  /* GE */
    int r5 = (a == d);  /* EQ */
    int r6 = (b != c);  /* NEQ */
    
    /* Unordered comparisons with explicit NaN checks */
    int r7 = !(a == a) || !(b == b);  /* UNORDERED check */
    int r8 = (a != a) && (b != b);    /* Both NaN -> UNEQ? */
    
    /* Mixed comparisons that could produce UNORDERED results */
    long double nan1 = __builtin_nanl("");
    int r9 = (a < nan1);   /* Comparison with NaN -> unordered */
    int r10 = (nan1 > b);  /* Another NaN comparison */
    
    /* Complex expression combining ordered and unordered */
    int r11 = ((a < b) && (c > d)) || (a != a);
    int r12 = ((a >= c) || (b <= d)) && (c == c);
    
    return (r1 ^ r2 ^ r3 ^ r4 ^ r5 ^ r6 ^ r7 ^ r8 ^ r9 ^ r10 ^ r11 ^ r12) & 1;
}

/* Function focusing on unordered comparisons */
int unordered_comparisons(long double x, long double y) {
    int results = 0;
    
    /* Generate NaN values in different ways */
    long double nan_q = __builtin_nanl("");
    long double nan_s = __builtin_nanl("0x1234");
    long double nan_div = 0.0L / 0.0L;
    long double nan_sqrt = sqrtl(-1.0L);
    
    /* Compare NaN with normal numbers (all should be unordered) */
    results |= (nan_q < x) ? 0 : 1;
    results |= (nan_q > y) ? 0 : 2;
    results |= (nan_q <= x) ? 0 : 4;
    results |= (nan_q >= y) ? 0 : 8;
    results |= (nan_q == x) ? 0 : 16;
    results |= (nan_q != y) ? 1 : 32;
    
    /* Compare NaN with NaN */
    results |= (nan_q < nan_s) ? 0 : 64;
    results |= (nan_q > nan_div) ? 0 : 128;
    results |= (nan_q == nan_sqrt) ? 0 : 256;
    results |= (nan_q != nan_div) ? 1 : 512;
    
    /* Compare normal numbers with NaN */
    results |= (x < nan_s) ? 0 : 1024;
    results |= (y > nan_q) ? 0 : 2048;
    
    /* Infinity comparisons */
    long double inf_p = __builtin_infl();
    long double inf_n = -__builtin_infl();
    
    results |= (inf_p > x) ? 1 : 4096;
    results |= (inf_n < y) ? 1 : 8192;
    results |= (inf_p == inf_p) ? 1 : 16384;
    results |= (inf_p < nan_q) ? 0 : 32768;  /* Inf < NaN is unordered */
    
    return results;
}

/* Function with control flow based on long double comparisons */
void control_flow_test(long double a, long double b, long double c, int *counter) {
    /* if-else chain with long double comparisons */
    if (a < b) {
        *counter += 1;
    } else if (a > c) {
        *counter += 2;
    } else if (b <= c) {
        *counter += 3;
    } else if (a >= b) {
        *counter += 4;
    }
    
    /* Nested comparisons */
    if ((a != b) && (c == c)) {
        *counter += 5;
    }
    
    /* Comparison with NaN check */
    long double nan = __builtin_nanl("");
    if (!(a == a)) {  /* a is NaN */
        *counter += 6;
    } else if (a < nan) {  /* This comparison is always unordered */
        *counter += 7;
    }
    
    /* Switch based on comparison results */
    int cmp_result = 0;
    if (a < b) cmp_result = 1;
    else if (a > b) cmp_result = 2;
    else if (a == b) cmp_result = 3;
    else cmp_result = 4;  /* unordered */
    
    switch (cmp_result) {
        case 1: *counter += 10; break;
        case 2: *counter += 20; break;
        case 3: *counter += 30; break;
        case 4: *counter += 40; break;
    }
}

/* Loop with long double termination condition */
int loop_test(long double start, long double limit) {
    volatile long double x = start;
    int iterations = 0;
    
    /* Loop while x < limit, but watch for NaN */
    while (x < limit && x == x) {  /* x == x checks for NaN */
        x = x * 1.1L;
        iterations++;
        if (iterations > 100) break;  /* Safety break */
    }
    
    /* Do-while with comparison */
    volatile long double y = start;
    do {
        y = y / 1.5L;
        iterations++;
    } while (y > 0.001L && y == y);
    
    return iterations;
}

/* Mixed precision comparisons */
int mixed_precision_test(float f, double d, long double ld) {
    int results = 0;
    
    /* Compare different precisions (will promote to long double) */
    results |= (ld > f) ? 1 : 0;
    results |= (d < ld) ? 2 : 0;
    results |= ((long double)f == d) ? 4 : 0;
    
    /* With integer constants */
    results |= (ld > 10) ? 8 : 0;
    results |= (f < 100L) ? 16 : 0;
    results |= ((long double)25 == d) ? 32 : 0;
    
    /* Complex mixed expression */
    results |= ((ld * 2.0L) > (d + f)) ? 64 : 0;
    
    return results;
}

/* Main test driver */
int main() {
    int bool_results[100];
    int result_index = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 16; i++) {
        global_ld[i] = get_ld(i);
    }
    
    /* Test 1: Complex x87 comparisons */
    for (int i = 0; i < 8; i++) {
        long double a = get_ld(i);
        long double b = get_ld(i+1);
        long double c = get_ld(i+2);
        long double d = get_ld(i+3);
        
        bool_results[result_index++] = complex_x87_comparison(a, b, c, d);
    }
    
    /* Test 2: Unordered comparisons */
    for (int i = 0; i < 4; i++) {
        long double x = get_ld(i*2);
        long double y = get_ld(i*2+1);
        
        int unordered_result = unordered_comparisons(x, y);
        /* Store individual bits as separate boolean results */
        for (int bit = 0; bit < 16 && result_index < 100; bit++) {
            bool_results[result_index++] = (unordered_result >> bit) & 1;
        }
    }
    
    /* Test 3: Control flow */
    int counter = 0;
    for (int i = 0; i < 4; i++) {
        long double a = get_ld(i);
        long double b = get_ld(i+4);
        long double c = get_ld(i+8);
        
        control_flow_test(a, b, c, &counter);
        bool_results[result_index++] = (counter >> i) & 1;
    }
    
    /* Test 4: Loop tests */
    for (int i = 0; i < 4; i++) {
        long double start = get_ld(i);
        long double limit = get_ld(i+8);
        
        int iter = loop_test(start, limit);
        bool_results[result_index++] = (iter > 0);
        bool_results[result_index++] = (iter % 2);
    }
    
    /* Test 5: Mixed precision */
    for (int i = 0; i < 4; i++) {
        float f = (float)get_ld(i);
        double d = (double)get_ld(i+4);
        long double ld = get_ld(i+8);
        
        int mixed = mixed_precision_test(f, d, ld);
        for (int bit = 0; bit < 7 && result_index < 100; bit++) {
            bool_results[result_index++] = (mixed >> bit) & 1;
        }
    }
    
    /* Test 6: Direct volatile comparisons */
    volatile long double v1 = get_ld(0);
    volatile long double v2 = get_ld(5);
    volatile long double v3 = get_ld(10);
    volatile long double v4 = get_ld(15);
    
    bool_results[result_index++] = (v1 < v2);
    bool_results[result_index++] = (v2 > v3);
    bool_results[result_index++] = (v3 <= v4);
    bool_results[result_index++] = (v4 >= v1);
    bool_results[result_index++] = (v1 == v3);
    bool_results[result_index++] = (v2 != v4);
    
    /* Compare with NaN directly */
    volatile long double v_nan = __builtin_nanl("");
    bool_results[result_index++] = (v1 < v_nan);
    bool_results[result_index++] = (v_nan > v2);
    bool_results[result_index++] = (v_nan == v_nan);
    bool_results[result_index++] = (v_nan != v3);
    
    /* Compute final hash to prevent dead code elimination */
    int final_hash = 0;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= (bool_results[i] << (i % 16));
    }
    
    printf("Test completed. Final hash: 0x%08x\n", final_hash);
    printf("Total comparisons performed: %d\n", result_index);
    
    return final_hash != 0 ? 0 : 1;
}
