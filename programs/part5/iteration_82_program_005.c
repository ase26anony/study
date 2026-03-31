/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld_array[16];
volatile int global_index = 0;

/* Helper to get unpredictable long double values */
long double get_ld_value(int idx) {
    return global_ld_array[idx & 15];
}

/* Complex comparison function using multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
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
    volatile long double inf_val = __builtin_infl();
    
    /* Comparisons that may be UNORDERED */
    if (!(a < nan_val)) result |= 64;    /* May generate "unord" or "nlt" */
    if (!(nan_val > b)) result |= 128;   /* May generate "unord" or "nle" */
    
    /* Direct unordered comparisons */
    if (a != a) result |= 256;           /* NaN check - may use UNEQ/UNORDERED */
    if (nan_val == nan_val) result |= 512; /* Always false for NaN - may use UNORDERED */
    
    return result;
}

/* Function focusing on unordered comparison scenarios */
int unordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* Generate NaN values */
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    long double nan3 = sqrtl(-1.0L);
    
    /* Compare NaN with normal numbers */
    if (x < nan1) result |= 1;      /* UNORDERED case */
    if (nan1 > y) result |= 2;      /* UNORDERED case */
    if (x <= nan2) result |= 4;     /* UNORDERED/UNLE case */
    if (nan2 >= y) result |= 8;     /* UNORDERED/UNGE case */
    if (x == nan3) result |= 16;    /* UNORDERED/UNEQ case */
    if (nan3 != y) result |= 32;    /* UNORDERED case */
    
    /* Compare NaN with NaN */
    if (nan1 < nan2) result |= 64;  /* UNORDERED case */
    if (nan2 > nan3) result |= 128; /* UNORDERED case */
    if (nan1 == nan3) result |= 256; /* UNORDERED/UNEQ case */
    
    /* Compare with infinity */
    long double inf_pos = __builtin_infl();
    long double inf_neg = -__builtin_infl();
    
    if (nan1 < inf_pos) result |= 512;   /* UNORDERED case */
    if (inf_neg > nan2) result |= 1024;  /* UNORDERED case */
    
    return result;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Promote float/double to long double */
    if (f < ld) result |= 1;
    if ((long double)d > ld) result |= 2;
    if ((float)ld <= f) result |= 4;
    if (ld >= (long double)d) result |= 8;
    
    /* Integer constant comparisons */
    if (ld < 10.0L) result |= 16;
    if ((long double)42 > ld) result |= 32;
    if (ld == 0.0L) result |= 64;
    if (ld != 3.14159265358979323846L) result |= 128;
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ld_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* While loop with complex condition */
    while (counter < limit && !(counter != counter)) {  /* counter is not NaN */
        counter += 0.5L;
        iterations++;
        if (iterations > 100) break; /* Safety limit */
    }
    
    /* Do-while with comparison */
    volatile long double temp = start;
    do {
        temp *= 1.1L;
        iterations++;
    } while (temp < limit && temp == temp); /* temp is not NaN */
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* Multiple comparisons to generate various condition codes */
    if (a < b) {
        if (b > c) result = 1;
        else if (b == c) result = 2;
        else result = 3;
    } else if (a > b) {
        if (b < c) result = 4;
        else if (b != c) result = 5;
        else result = 6;
    } else { /* a == b */
        if (c != c) result = 7;  /* c is NaN */
        else if (c < 0.0L) result = 8;
        else result = 9;
    }
    
    return result;
}

/* Main test function */
int main() {
    /* Initialize array with mixed values */
    global_ld_array[0] = 1.0L;
    global_ld_array[1] = 2.0L;
    global_ld_array[2] = 3.14159265358979323846L;
    global_ld_array[3] = __builtin_infl();
    global_ld_array[4] = -__builtin_infl();
    global_ld_array[5] = __builtin_nanl("");
    global_ld_array[6] = 0.0L;
    global_ld_array[7] = -0.0L;
    global_ld_array[8] = 1.0L / 0.0L;  /* Infinity */
    global_ld_array[9] = 0.0L / 0.0L;  /* NaN */
    global_ld_array[10] = sqrtl(-1.0L); /* NaN */
    global_ld_array[11] = 100.0L;
    global_ld_array[12] = 1.0e-10L;
    global_ld_array[13] = 1.0e10L;
    global_ld_array[14] = -42.0L;
    global_ld_array[15] = 99.999L;
    
    int results[50];
    int result_index = 0;
    
    /* Test 1: Complex x87 comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ld_value(0), get_ld_value(1),
        get_ld_value(2), get_ld_value(3));
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_index++] = unordered_comparisons(
        get_ld_value(4), get_ld_value(5));
    
    /* Test 3: Mixed precision */
    results[result_index++] = mixed_precision_comparisons(
        3.14f, 2.718281828459045, get_ld_value(6));
    
    /* Test 4: Loop conditions */
    results[result_index++] = loop_with_ld_condition(
        get_ld_value(7), get_ld_value(8));
    
    /* Test 5: Switch on comparison */
    results[result_index++] = switch_on_comparison(
        get_ld_value(9), get_ld_value(10), get_ld_value(11));
    
    /* Additional random comparisons to exercise more code paths */
    for (int i = 0; i < 10; i++) {
        volatile long double a = get_ld_value(i);
        volatile long double b = get_ld_value(i + 1);
        volatile long double c = get_ld_value(i + 2);
        
        /* Generate various comparison patterns */
        int r = 0;
        r |= (a < b) ? 1 : 0;
        r |= (b > c) ? 2 : 0;
        r |= (a <= b) ? 4 : 0;
        r |= (b >= c) ? 8 : 0;
        r |= (a == b) ? 16 : 0;
        r |= (b != c) ? 32 : 0;
        
        /* Unordered checks */
        volatile long double nan = __builtin_nanl("");
        r |= (a < nan) ? 64 : 0;
        r |= (nan > b) ? 128 : 0;
        r |= (a != a) ? 256 : 0;
        
        results[result_index++] = r;
    }
    
    /* Compute hash to prevent dead code elimination */
    int hash = 0;
    for (int i = 0; i < result_index; i++) {
        hash ^= results[i];
        hash = (hash << 1) | (hash >> 31); /* Simple rotation */
    }
    
    printf("Result hash: %d\n", hash);
    printf("Number of tests executed: %d\n", result_index);
    
    return 0;
}
