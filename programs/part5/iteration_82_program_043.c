/* x87_comparison_test.c
 * Designed to trigger x87 floating-point comparison output logic in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -o x87_test x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld[16];
volatile int global_index = 0;

/* Helper to get dynamic long double values */
long double get_ld(int idx) {
    return global_ld[idx % 16];
}

/* Complex comparison function using multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison patterns */
    if (a != b) {
        if (c <= d) {
            if (a > 0.0L) {
                return 1;
            } else if (b < 0.0L) {
                return 2;
            }
        }
        if (c >= d && !(a == b)) {
            return 3;
        }
    }
    
    /* Unordered comparisons with explicit NaN checks */
    if (a != a || b != b) {  /* NaN checks */
        if (c == c && d == d) {  /* Non-NaN checks */
            return 4;
        }
    }
    
    /* Mixed comparisons */
    if ((a < b) != (c > d)) {
        return 5;
    }
    
    return 0;
}

/* Function focusing on unordered comparisons with NaN */
int unordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* Generate NaN values */
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    long double nan3 = sqrtl(-1.0L);
    
    /* Compare NaN with normal numbers */
    if (x != nan1) result |= 1;
    if (nan2 == nan2) result |= 2;  /* Always false for NaN */
    if (nan3 < x) result |= 4;      /* Unordered comparison */
    if (x >= nan1) result |= 8;     /* Unordered comparison */
    
    /* Compare NaN with NaN */
    if (nan1 == nan2) result |= 16;
    if (nan1 != nan2) result |= 32;
    if (nan1 < nan2) result |= 64;
    if (nan1 > nan2) result |= 128;
    
    /* Compare with infinity */
    long double inf = __builtin_infl();
    if (x < inf) result |= 256;
    if (nan1 > inf) result |= 512;
    if (inf == inf) result |= 1024;
    
    return result;
}

/* Function with structured control flow based on long double comparisons */
int control_flow_comparisons(long double a, long double b, long double c) {
    int counter = 0;
    
    /* if-else chain */
    if (a < b) {
        counter += 1;
    } else if (a > b) {
        counter += 2;
    } else if (a == b) {
        counter += 3;
    } else {
        /* This executes for unordered comparisons (NaN involved) */
        counter += 4;
    }
    
    /* Nested comparisons */
    if ((a != b) && (c <= 100.0L)) {
        counter += 10;
    }
    
    if ((a >= b) || (c != 0.0L)) {
        counter += 20;
    }
    
    /* Switch based on comparison results */
    switch((a > b) + 2 * (a < b) + 4 * (a == b)) {
        case 1:
            counter += 100;
            break;
        case 2:
            counter += 200;
            break;
        case 4:
            counter += 300;
            break;
        default:
            /* Unordered case */
            counter += 400;
            break;
    }
    
    return counter;
}

/* Loop with long double termination condition */
int loop_comparisons(long double start, long double limit) {
    volatile long double x = start;
    int iterations = 0;
    
    /* while loop with compound condition */
    while (x < limit && x == x) {  /* x == x checks for NaN */
        x = x * 1.1L;
        iterations++;
        if (iterations > 100) break;
    }
    
    /* do-while with comparison */
    volatile long double y = limit;
    do {
        y = y / 1.5L;
        iterations++;
    } while (y > start && y == y);
    
    return iterations;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(long double ld, double d, float f) {
    int result = 0;
    
    /* Compare long double with double */
    if (ld > (long double)d) result |= 1;
    if ((long double)f <= ld) result |= 2;
    
    /* Compare with integer constants */
    if (ld < 100L) result |= 4;
    if ((long double)50 >= ld) result |= 8;
    
    /* Complex mixed expression */
    if ((ld + (long double)d) > (long double)(f * 2.0f)) {
        result |= 16;
    }
    
    return result;
}

/* Main test function that exercises all comparison types */
int test_all_comparisons(void) {
    int results[100];
    int result_index = 0;
    
    /* Initialize test values */
    global_ld[0] = 1.0L;
    global_ld[1] = 2.0L;
    global_ld[2] = 0.0L;
    global_ld[3] = -1.0L;
    global_ld[4] = __builtin_infl();      /* Positive infinity */
    global_ld[5] = -__builtin_infl();     /* Negative infinity */
    global_ld[6] = __builtin_nanl("");    /* Quiet NaN */
    global_ld[7] = 0.0L / 0.0L;           /* Another NaN */
    global_ld[8] = 3.14159265358979323846L; /* Pi */
    global_ld[9] = 2.71828182845904523536L; /* e */
    
    /* Test all standard comparisons */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            long double a = get_ld(i);
            long double b = get_ld(j);
            
            results[result_index++] = (a < b);
            results[result_index++] = (a > b);
            results[result_index++] = (a <= b);
            results[result_index++] = (a >= b);
            results[result_index++] = (a == b);
            results[result_index++] = (a != b);
            
            if (result_index > 90) break;
        }
        if (result_index > 90) break;
    }
    
    /* Test complex comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ld(0), get_ld(1), get_ld(2), get_ld(3));
    
    /* Test unordered comparisons */
    results[result_index++] = unordered_comparisons(get_ld(0), get_ld(6));
    
    /* Test control flow */
    results[result_index++] = control_flow_comparisons(
        get_ld(0), get_ld(1), get_ld(2));
    
    /* Test loops */
    results[result_index++] = loop_comparisons(get_ld(0), get_ld(4));
    
    /* Test mixed precision */
    results[result_index++] = mixed_precision_comparisons(
        get_ld(0), 3.14159, 2.71828f);
    
    /* Compute hash to prevent dead code elimination */
    int hash = 0;
    for (int i = 0; i < result_index; i++) {
        hash ^= results[i] * (i + 1);
    }
    
    return hash;
}

int main(void) {
    /* Run the comprehensive test */
    int final_hash = test_all_comparisons();
    
    /* Print result to ensure execution */
    printf("Test hash: %d\n", final_hash);
    printf("If you see this and a number above, all comparisons executed.\n");
    
    return 0;
}
