/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
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

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;
    if (a > c) result |= 2;
    if (b <= c) result |= 4;
    if (c >= d) result |= 8;
    if (a == b) result |= 16;
    if (b != c) result |= 32;
    
    /* Unordered comparisons with potential NaN */
    if (!(a < b)) result |= 64;        /* May generate UNGE/UNGT */
    if (!(a > b)) result |= 128;       /* May generate UNLE/UNLT */
    if (!(a <= b)) result |= 256;      /* May generate UNGT */
    if (!(a >= b)) result |= 512;      /* May generate UNLT */
    
    /* Explicit unordered checks */
    if (a != a || b != b) result |= 1024;  /* UNORDERED check */
    
    return result;
}

/* Function focusing on unordered comparisons with NaN */
int unordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* Generate NaN through arithmetic */
    long double nan1 = x / 0.0L;           /* Should produce NaN if x is 0 */
    long double nan2 = 0.0L / 0.0L;        /* Explicit NaN */
    long double nan3 = __builtin_nanl(""); /* Built-in quiet NaN */
    
    /* Compare NaN with normal numbers (all should be false for ordered comparisons) */
    if (nan1 < y) result |= 1;      /* Should be false, may generate UNGE */
    if (nan1 > y) result |= 2;      /* Should be false, may generate UNLE */
    if (nan1 <= y) result |= 4;     /* Should be false, may generate UNGT */
    if (nan1 >= y) result |= 8;     /* Should be false, may generate UNLT */
    if (nan1 == y) result |= 16;    /* Should be false */
    if (nan1 != y) result |= 32;    /* Should be true, may generate UNEQ */
    
    /* Compare NaN with NaN */
    if (nan1 == nan2) result |= 64;     /* Should be false */
    if (nan1 != nan2) result |= 128;    /* Should be true, may generate UNEQ */
    
    /* Ordered vs unordered comparisons */
    if (!(nan1 < y)) result |= 256;     /* Should be true, may generate UNGE */
    if (!(nan1 > y)) result |= 512;     /* Should be true, may generate UNLE */
    
    return result;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(long double ld, double d, float f) {
    int result = 0;
    
    /* Compare long double with double (promotion happens) */
    if (ld < (long double)d) result |= 1;
    if ((long double)f > ld) result |= 2;
    
    /* Compare with integer constants */
    if (ld < 10.0L) result |= 4;
    if ((long double)5 > ld) result |= 8;
    
    /* Complex expression */
    if ((ld < d) && (f > 0.0f) && (ld != 0.0L)) result |= 16;
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ld_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition using long double comparison */
    while (counter < limit && iterations < 100) {
        counter += 1.0L;
        iterations++;
        
        /* Nested comparison inside loop */
        if (counter != limit && !(counter > limit * 2.0L)) {
            iterations += (int)(counter / 10.0L);
        }
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparisons(long double a, long double b, long double c) {
    int result = 0;
    
    /* Build complex condition */
    int condition = 0;
    if (a < b) condition = 1;
    else if (a > c) condition = 2;
    else if (b <= c) condition = 3;
    else if (a >= b) condition = 4;
    else if (a == c) condition = 5;
    else if (b != a) condition = 6;
    else condition = 7;
    
    /* Switch on the comparison result */
    switch (condition) {
        case 1: result = 100; break;
        case 2: result = 200; break;
        case 3: result = 300; break;
        case 4: result = 400; break;
        case 5: result = 500; break;
        case 6: result = 600; break;
        default: result = 700; break;
    }
    
    /* Additional unordered check */
    if (!(a < b) && (b == b)) {
        result += 1000;
    }
    
    return result;
}

/* Initialize global array with mix of values */
void init_global_values() {
    global_ld[0] = 0.0L;
    global_ld[1] = 1.0L;
    global_ld[2] = -1.0L;
    global_ld[3] = 3.14159265358979323846L;
    global_ld[4] = 1.0L / 0.0L;          /* Infinity */
    global_ld[5] = -1.0L / 0.0L;         /* -Infinity */
    global_ld[6] = 0.0L / 0.0L;          /* NaN */
    global_ld[7] = __builtin_nanl("");   /* Quiet NaN */
    global_ld[8] = 100.0L;
    global_ld[9] = 0.0001L;
    global_ld[10] = 1.0e308L;            /* Large number */
    global_ld[11] = 1.0e-308L;           /* Small number */
    global_ld[12] = -global_ld[10];
    global_ld[13] = sqrtl(-1.0L);        /* NaN from sqrt(-1) */
    global_ld[14] = __builtin_infl();    /* Infinity */
    global_ld[15] = -__builtin_infl();   /* -Infinity */
}

int main() {
    init_global_values();
    
    int results[50];
    int result_count = 0;
    
    /* Test 1: Complex x87 comparisons */
    results[result_count++] = complex_x87_comparison(
        get_ld(0), get_ld(1), get_ld(2), get_ld(3));
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_count++] = unordered_comparisons(get_ld(0), get_ld(1));
    
    /* Test 3: Mixed precision */
    results[result_count++] = mixed_precision_comparisons(
        get_ld(3), 2.718281828459045, 1.41421356237f);
    
    /* Test 4: Loop with long double condition */
    results[result_count++] = loop_with_ld_condition(get_ld(0), get_ld(8));
    
    /* Test 5: Switch on comparisons */
    results[result_count++] = switch_on_comparisons(
        get_ld(1), get_ld(2), get_ld(3));
    
    /* Test 6: More unordered scenarios */
    for (int i = 0; i < 8; i++) {
        results[result_count++] = unordered_comparisons(
            get_ld(i), get_ld(i+8));
    }
    
    /* Test 7: Direct NaN comparisons */
    volatile long double nan_val = __builtin_nanl("");
    volatile long double inf_val = __builtin_infl();
    volatile long double normal_val = 42.0L;
    
    /* These should trigger various unordered condition codes */
    int nan_test_result = 0;
    nan_test_result |= (nan_val < normal_val) ? 0 : 1;
    nan_test_result |= (nan_val > normal_val) ? 0 : 2;
    nan_test_result |= (nan_val <= normal_val) ? 0 : 4;
    nan_test_result |= (nan_val >= normal_val) ? 0 : 8;
    nan_test_result |= (nan_val == normal_val) ? 0 : 16;
    nan_test_result |= (nan_val != normal_val) ? 32 : 0;
    nan_test_result |= (nan_val < nan_val) ? 0 : 64;
    nan_test_result |= (nan_val == nan_val) ? 0 : 128;
    
    results[result_count++] = nan_test_result;
    
    /* Test 8: Infinity comparisons */
    int inf_test_result = 0;
    inf_test_result |= (inf_val > normal_val) ? 1 : 0;
    inf_test_result |= (inf_val < -inf_val) ? 2 : 0;
    inf_test_result |= (inf_val == inf_val) ? 4 : 0;
    inf_test_result |= (-inf_val <= inf_val) ? 8 : 0;
    inf_test_result |= (!(inf_val < normal_val)) ? 16 : 0;  /* May generate UNLE/UNGE */
    
    results[result_count++] = inf_test_result;
    
    /* Test 9: Complex nested comparisons */
    volatile long double x = get_ld(global_index++);
    volatile long double y = get_ld(global_index++);
    volatile long double z = get_ld(global_index++);
    
    int complex_result = 0;
    if ((x < y) && (y > z) && (x != z)) complex_result |= 1;
    if (!(x > y) || (y <= z) || (x == x)) complex_result |= 2;
    if ((x != x) || (y != y) || (z != z)) complex_result |= 4;  /* NaN check */
    if (!(x < y) && !(y < z) && !(z < x)) complex_result |= 8;  /* May generate UNGE/UNLE */
    
    results[result_count++] = complex_result;
    
    /* Compute final hash to prevent dead code elimination */
    int final_hash = 0;
    for (int i = 0; i < result_count; i++) {
        final_hash ^= results[i];
        final_hash = (final_hash << 1) | (final_hash >> 31);  /* Simple rotation */
    }
    
    printf("Test completed. Final hash: %d\n", final_hash);
    printf("Number of tests executed: %d\n", result_count);
    
    return final_hash != 0 ? 0 : 1;
}
