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

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;      /* LT */
    if (a > b) result |= 2;      /* GT */
    if (a <= b) result |= 4;     /* LE */
    if (a >= b) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (a != b) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with potential NaN */
    if (!(c < d)) result |= 64;  /* NLT (UNGE) */
    if (!(c <= d)) result |= 128; /* NLE (UNGT) */
    
    /* Mixed comparisons */
    if ((a != b) && (c <= d)) result |= 256;
    if ((a == b) || !(c > d)) result |= 512;
    
    return result;
}

/* Function focusing on unordered NaN comparisons */
int nan_comparison_tests(long double nan1, long double nan2, long double normal) {
    int results = 0;
    
    /* UNORDERED comparisons (NaN vs anything) */
    if (nan1 != nan1) results |= 1;          /* NaN self-comparison */
    if (nan1 == nan1) results |= 2;          /* Should be false for NaN */
    
    /* UNEQ: unordered or equal */
    if (!(nan1 < normal) && !(nan1 > normal)) results |= 4;
    
    /* UNGE: not less than (greater than or equal or unordered) */
    if (!(nan1 < normal)) results |= 8;
    
    /* UNGT: not less than or equal (greater than or unordered) */
    if (!(nan1 <= normal)) results |= 16;
    
    /* UNLE: less than or equal or unordered */
    if ((nan1 < normal) || !(nan1 == nan1)) results |= 32;
    
    /* UNLT: less than or unordered */
    if ((nan1 < normal) || (nan1 != nan1)) results |= 64;
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if ((normal < 10.0L) != (normal > 10.0L)) results |= 128;
    
    /* NaN vs NaN comparisons */
    if (nan1 == nan2) results |= 256;        /* Usually false */
    if (nan1 != nan2) results |= 512;        /* Usually true */
    
    return results;
}

/* Function with switch based on comparison results */
int comparison_switch(long double x, long double y) {
    int ret = 0;
    
    /* Force runtime evaluation */
    volatile long double vx = x;
    volatile long double vy = y;
    
    /* Chain of if-else comparisons */
    if (vx < vy) {
        ret = 1;
    } else if (!(vx < vy)) {  /* NLT */
        ret = 2;
    } else if (vx == vy) {
        ret = 3;
    } else if (vx != vy) {
        ret = 4;
    } else if (!(vx <= vy)) { /* NLE */
        ret = 5;
    } else if (!(vx > vy)) {  /* NGT */
        ret = 6;
    }
    
    return ret;
}

/* Loop with long double termination condition */
int loop_with_ld_condition(long double start, long double limit) {
    volatile long double x = start;
    int count = 0;
    
    /* Loop condition using multiple comparisons */
    while ((x < limit) && (x == x) && !(x != x)) {
        x = x * 1.1L;
        count++;
        if (count > 100) break; /* Safety */
    }
    
    return count;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Promote float to long double */
    if ((long double)f < ld) result |= 1;
    
    /* Promote double to long double */
    if (d > (long double)f) result |= 2;
    
    /* Integer constant cast to long double */
    if (ld == (long double)42) result |= 4;
    
    /* Complex mixed expression */
    if ((f * d) != ld) result |= 8;
    
    return result;
}

/* Main test function */
int main() {
    /* Initialize test array with various values */
    global_ld[0] = 0.0L;
    global_ld[1] = 1.0L;
    global_ld[2] = -1.0L;
    global_ld[3] = 100.0L;
    global_ld[4] = 0.0L / 0.0L;                    /* NaN */
    global_ld[5] = __builtin_nanl("");             /* Quiet NaN */
    global_ld[6] = 1.0L / 0.0L;                    /* Infinity */
    global_ld[7] = -1.0L / 0.0L;                   /* -Infinity */
    global_ld[8] = sqrtl(-1.0L);                   /* NaN from sqrt(-1) */
    global_ld[9] = 3.14159265358979323846L;        /* Pi */
    global_ld[10] = 2.71828182845904523536L;       /* e */
    global_ld[11] = global_ld[4] * 2.0L;           /* Another NaN */
    global_ld[12] = global_ld[6] + global_ld[7];   /* NaN (inf - inf) */
    global_ld[13] = global_ld[6] * 0.0L;           /* NaN (inf * 0) */
    global_ld[14] = 42.0L;
    global_ld[15] = -999.999L;
    
    int results[50];
    int result_index = 0;
    
    /* Test 1: Complex x87 comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ld(0), get_ld(1), get_ld(4), get_ld(2));
    
    /* Test 2: NaN comparison tests */
    results[result_index++] = nan_comparison_tests(
        get_ld(4), get_ld(5), get_ld(1));
    
    /* Test 3: More NaN tests with different NaN sources */
    results[result_index++] = nan_comparison_tests(
        get_ld(8), get_ld(11), get_ld(14));
    
    /* Test 4: Switch-based comparisons */
    results[result_index++] = comparison_switch(get_ld(1), get_ld(3));
    results[result_index++] = comparison_switch(get_ld(4), get_ld(1)); /* NaN vs normal */
    
    /* Test 5: Loop with conditions */
    results[result_index++] = loop_with_ld_condition(get_ld(0), get_ld(3));
    
    /* Test 6: Mixed precision */
    results[result_index++] = mixed_precision_comparisons(
        3.14f, 2.718281828459045, get_ld(9));
    
    /* Test 7: Direct unordered comparisons */
    volatile long double nan_val = get_ld(4);
    volatile long double normal_val = get_ld(14);
    
    /* Generate various comparison results */
    results[result_index++] = (nan_val < normal_val) ? 1 : 0;
    results[result_index++] = (nan_val > normal_val) ? 1 : 0;
    results[result_index++] = (nan_val <= normal_val) ? 1 : 0;
    results[result_index++] = (nan_val >= normal_val) ? 1 : 0;
    results[result_index++] = (nan_val == normal_val) ? 1 : 0;
    results[result_index++] = (nan_val != normal_val) ? 1 : 0;
    
    /* Test 8: Ordered comparisons with normal numbers */
    results[result_index++] = (get_ld(1) < get_ld(3)) ? 1 : 0;
    results[result_index++] = (get_ld(9) > get_ld(10)) ? 1 : 0;
    results[result_index++] = (get_ld(0) <= get_ld(0)) ? 1 : 0;
    results[result_index++] = (get_ld(14) >= get_ld(14)) ? 1 : 0;
    
    /* Test 9: Infinity comparisons */
    results[result_index++] = (get_ld(6) > get_ld(3)) ? 1 : 0;  /* inf > 100 */
    results[result_index++] = (get_ld(7) < get_ld(2)) ? 1 : 0;  /* -inf < -1 */
    
    /* Test 10: Complex expression designed to use specific condition codes */
    long double a = get_ld(1);
    long double b = get_ld(4);  /* NaN */
    long double c = get_ld(9);
    long double d = get_ld(10);
    
    int complex_result = 0;
    if ((a != b) && (c <= d)) complex_result |= 1;      /* UNEQ/ORDERED mix */
    if (!(b < a)) complex_result |= 2;                  /* UNGE */
    if (!(b <= a)) complex_result |= 4;                 /* UNGT */
    if ((b < c) || (b != b)) complex_result |= 8;       /* UNLT */
    if ((b <= c) || !(b == b)) complex_result |= 16;    /* UNLE */
    if ((a < c) != (a > c)) complex_result |= 32;       /* LTGT */
    
    results[result_index++] = complex_result;
    
    /* Compute final hash to prevent optimization */
    int final_hash = 0;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= results[i] * (i + 1);
    }
    
    printf("Test completed. Result hash: %d\n", final_hash);
    printf("(This hash varies based on NaN comparison behavior)\n");
    
    return final_hash != 0 ? 0 : 1;
}
