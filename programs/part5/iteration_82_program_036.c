/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math -o x87_test x87_comparison_test.c
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
    if (b <= c) result |= 4;     /* LE */
    if (c >= d) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (b != c) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with potential NaN */
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    
    /* UNORDERED: (a != a) | (b != b) */
    if (!(a == a) || !(b == b)) result |= 64;
    
    /* UNEQ: unordered or equal */
    if (!(a == a) || !(b == b) || a == b) result |= 128;
    
    /* UNGE: not less than (unordered or greater or equal) */
    if (!(a < b)) result |= 256;
    
    /* UNGT: not less or equal (unordered or greater) */
    if (!(a <= b)) result |= 512;
    
    /* UNLE: unordered or less or equal */
    if (!(a == a) || !(b == b) || a <= b) result |= 1024;
    
    /* UNLT: unordered or less than */
    if (!(a == a) || !(b == b) || a < b) result |= 2048;
    
    /* LTGT: less or greater (ordered and not equal) */
    if ((a == a) && (b == b) && a != b) result |= 4096;
    
    return result;
}

/* Test function with explicit NaN comparisons */
int test_nan_comparisons(void) {
    volatile long double nan = __builtin_nanl("");
    volatile long double inf = __builtin_infl();
    volatile long double neg_inf = -__builtin_infl();
    volatile long double zero = 0.0L;
    volatile long double one = 1.0L;
    
    int results = 0;
    
    /* Compare NaN with various values */
    results |= (nan < one) ? 1 : 0;
    results |= (nan > one) ? 2 : 0;
    results |= (nan <= zero) ? 4 : 0;
    results |= (nan >= inf) ? 8 : 0;
    results |= (nan == nan) ? 16 : 0;  /* Always false for NaN */
    results |= (nan != one) ? 32 : 0;  /* Always true for NaN */
    
    /* Compare two different NaNs */
    volatile long double nan2 = sqrtl(-1.0L);
    results |= (nan == nan2) ? 64 : 0;
    results |= (nan != nan2) ? 128 : 0;
    
    /* Compare infinity */
    results |= (inf > one) ? 256 : 0;
    results |= (neg_inf < -one) ? 512 : 0;
    
    return results;
}

/* Function with mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Mixed precision - will promote to long double */
    if ((long double)f < ld) result |= 1;
    if (d > (long double)f) result |= 2;
    if ((long double)f <= (long double)d) result |= 4;
    if (ld >= (long double)d) result |= 8;
    
    /* With integer casts */
    if (ld > (long double)(int)f) result |= 16;
    if ((long double)(int)d < ld) result |= 32;
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ld_condition(long double start, long double end, long double step) {
    volatile long double sum = 0.0L;
    volatile long double x = start;
    int iterations = 0;
    
    /* Use complex condition to force x87 comparisons */
    while ((x < end) && (x == x) && !(x != x)) {  /* x < end AND x not NaN */
        sum += x;
        x += step;
        iterations++;
        if (iterations > 1000) break; /* Safety */
    }
    
    /* Another loop with different comparison */
    x = end;
    while ((x > start) || (x != x)) {  /* x > start OR x is NaN */
        sum -= x;
        x -= step / 2.0L;
        iterations++;
        if (iterations > 2000) break; /* Safety */
    }
    
    return (int)(sum * 100.0L);
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b) {
    int result = 0;
    
    /* Force generation of comparison instructions */
    if (a < b) result = 1;
    else if (a > b) result = 2;
    else if (a == b) result = 3;
    else result = 4; /* unordered */
    
    /* Nested comparisons in switch */
    switch(result) {
        case 1:
            if (!(a >= b)) result *= 10;
            break;
        case 2:
            if (!(a <= b)) result *= 20;
            break;
        case 3:
            if (a == b) result *= 30;
            break;
        case 4:
            if (!(a == a) || !(b == b)) result *= 40;
            break;
    }
    
    return result;
}

/* Main test function */
int main(void) {
    /* Initialize array with mixed values */
    global_ld[0] = 0.0L;
    global_ld[1] = 1.0L;
    global_ld[2] = -1.0L;
    global_ld[3] = 3.14159265358979323846L;
    global_ld[4] = __builtin_infl();
    global_ld[5] = -__builtin_infl();
    global_ld[6] = __builtin_nanl("");
    global_ld[7] = 0.0L / 0.0L;
    global_ld[8] = sqrtl(-1.0L);
    global_ld[9] = 100.0L;
    global_ld[10] = 1.0e-10L;
    global_ld[11] = 1.0e10L;
    global_ld[12] = -3.14159265358979323846L;
    global_ld[13] = 2.71828182845904523536L;
    global_ld[14] = 1.0L / 0.0L;  /* Infinity */
    global_ld[15] = logl(-1.0L);  /* NaN */
    
    int results[20];
    int result_index = 0;
    
    /* Test 1: Complex x87 comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ld(0), get_ld(1), get_ld(2), get_ld(3));
    
    /* Test 2: NaN comparisons */
    results[result_index++] = test_nan_comparisons();
    
    /* Test 3: Mixed precision */
    results[result_index++] = mixed_precision_comparisons(
        3.14f, 2.71828, get_ld(3));
    
    /* Test 4: Loop conditions */
    results[result_index++] = loop_with_ld_condition(
        get_ld(0), get_ld(9), get_ld(1));
    
    /* Test 5: Switch on comparison */
    results[result_index++] = switch_on_comparison(
        get_ld(1), get_ld(2));
    
    /* Test 6: More complex scenarios */
    volatile long double x = get_ld(global_index++);
    volatile long double y = get_ld(global_index++);
    volatile long double z = get_ld(global_index++);
    
    /* Chain of comparisons */
    int chain_result = 0;
    if (x < y && y > z) chain_result |= 1;
    if (x == x && y == y && z == z) chain_result |= 2;  /* All ordered */
    if (!(x == x) || !(y == y)) chain_result |= 4;      /* Any unordered */
    if (x >= y || y <= z) chain_result |= 8;
    if (x != y && y != z && x != z) chain_result |= 16;
    
    results[result_index++] = chain_result;
    
    /* Test 7: Arithmetic producing NaN then compare */
    volatile long double nan_prod = get_ld(6) * get_ld(7);
    results[result_index++] = (nan_prod == nan_prod) ? 0 : 1;
    results[result_index++] = (nan_prod < 0.0L) ? 0 : 1;
    results[result_index++] = (nan_prod > 0.0L) ? 0 : 1;
    
    /* Test 8: Ordered comparison with infinity */
    results[result_index++] = (get_ld(4) > get_ld(9)) ? 100 : 200;
    results[result_index++] = (get_ld(5) < get_ld(10)) ? 300 : 400;
    
    /* Test 9: Unordered comparisons explicitly */
    volatile long double u1 = get_ld(6);
    volatile long double u2 = get_ld(8);
    results[result_index++] = !(u1 < u2) ? 1 : 0;   /* UNGE: nlt */
    results[result_index++] = !(u1 <= u2) ? 1 : 0;  /* UNGT: nle */
    results[result_index++] = (!(u1 == u1) || !(u2 == u2) || u1 <= u2) ? 1 : 0; /* UNLE: ule */
    results[result_index++] = (!(u1 == u1) || !(u2 == u2) || u1 < u2) ? 1 : 0;  /* UNLT: ult */
    
    /* Compute final hash to prevent dead code elimination */
    int final_hash = 0;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= results[i];
    }
    
    printf("Test completed. Result hash: %d\n", final_hash);
    printf("(This hash varies based on NaN/infinity comparisons)\n");
    
    return final_hash != 0 ? 0 : 1;
}
