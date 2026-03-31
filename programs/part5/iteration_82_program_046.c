/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];

/* Helper to get unpredictable values */
static long double get_value(int idx) {
    return global_ldbl_array[idx];
}

/* Complex comparison function using multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison patterns */
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
    if (!(a < nan_val)) result |= 64;    /* May generate UNORDERED/UNGE */
    if (!(nan_val > b)) result |= 128;   /* May generate UNORDERED/UNLE */
    if (nan_val == nan_val) result |= 256; /* Always false for NaN */
    
    /* Mixed comparisons */
    if ((a != b) && (c <= d)) result |= 512;
    if ((a < b) || (c > d)) result |= 1024;
    
    return result;
}

/* Function focusing on unordered comparisons */
int unordered_comparisons(long double x, long double y) {
    int result = 0;
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;  /* Another way to get NaN */
    volatile long double inf = __builtin_infl();
    
    /* Direct NaN comparisons */
    if (x == nan1) result |= 1;      /* Always false, but compiler may not know */
    if (x != nan1) result |= 2;      /* UNORDERED/UNEQ? */
    if (nan1 == nan2) result |= 4;   /* UNORDERED */
    if (nan1 != nan2) result |= 8;   /* UNORDERED */
    
    /* NaN vs normal numbers */
    if (x < nan1) result |= 16;      /* UNORDERED */
    if (nan1 > y) result |= 32;      /* UNORDERED */
    if (x <= nan1) result |= 64;     /* UNORDERED/UNGE? */
    if (nan1 >= y) result |= 128;    /* UNORDERED/UNLE? */
    
    /* NaN vs infinity */
    if (nan1 < inf) result |= 256;   /* UNORDERED */
    if (nan1 > -inf) result |= 512;  /* UNORDERED */
    
    return result;
}

/* Function with switch based on comparison results */
int comparison_switch(long double a, long double b, long double c) {
    int score = 0;
    
    /* Chain of comparisons controlling flow */
    if (a < b) {
        score += 10;
        if (b > c) {
            score += 20;
            if (a != c) {
                score += 30;
            }
        }
    } else if (a > b) {
        score += 40;
        if (b < c) {
            score += 50;
        }
    } else { /* a == b */
        score += 60;
    }
    
    /* NaN-aware comparisons */
    volatile long double nan_val = sqrtl(-1.0L);
    if (!(a < nan_val)) {
        score += 100;  /* UNORDERED/UNGE */
    }
    if (!(nan_val > b)) {
        score += 200;  /* UNORDERED/UNLE */
    }
    
    return score;
}

/* Loop with floating-point condition */
int loop_with_fp_condition(long double start, long double limit) {
    volatile long double x = start;
    int iterations = 0;
    
    /* Loop condition may generate x87 comparisons */
    while (x < limit && iterations < 100) {
        x = x * 1.1L;
        iterations++;
        
        /* Break on NaN detection */
        if (x != x) {  /* NaN check */
            break;
        }
    }
    
    return iterations;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* Promote float/double to long double for x87 comparison */
    if (f < ld) result |= 1;
    if ((long double)d > ld) result |= 2;
    if (f <= (long double)d) result |= 4;
    if ((double)ld >= d) result |= 8;
    
    /* Integer constant comparisons */
    if (ld < 100.0L) result |= 16;
    if (ld > -50.0L) result |= 32;
    if ((long double)(int)f == ld) result |= 64;
    
    return result;
}

/* Main test driver */
int main() {
    /* Initialize array with mix of values */
    global_ldbl_array[0] = 1.0L;
    global_ldbl_array[1] = 2.0L;
    global_ldbl_array[2] = 3.14159265358979323846L;
    global_ldbl_array[3] = -1.5L;
    global_ldbl_array[4] = __builtin_infl();      /* +Inf */
    global_ldbl_array[5] = -__builtin_infl();     /* -Inf */
    global_ldbl_array[6] = __builtin_nanl("");    /* NaN */
    global_ldbl_array[7] = 0.0L;
    global_ldbl_array[8] = -0.0L;
    global_ldbl_array[9] = 1.0L / 0.0L;           /* Another Inf */
    global_ldbl_array[10] = 0.0L / 0.0L;          /* NaN */
    global_ldbl_array[11] = sqrtl(-1.0L);         /* NaN */
    
    /* Get volatile values to prevent optimization */
    volatile long double v1 = get_value(0);
    volatile long double v2 = get_value(1);
    volatile long double v3 = get_value(2);
    volatile long double v4 = get_value(3);
    volatile long double v5 = get_value(4);  /* Inf */
    volatile long double v6 = get_value(6);  /* NaN */
    
    /* Run various comparison tests */
    int results[10];
    
    results[0] = complex_x87_comparison(v1, v2, v3, v4);
    results[1] = unordered_comparisons(v1, v6);  /* Normal vs NaN */
    results[2] = unordered_comparisons(v6, v5);  /* NaN vs Inf */
    results[3] = unordered_comparisons(v6, v6);  /* NaN vs NaN */
    results[4] = comparison_switch(v1, v2, v3);
    results[5] = comparison_switch(v6, v1, v2);  /* With NaN */
    results[6] = loop_with_fp_condition(v1, v3);
    results[7] = mixed_precision_comparisons(1.5f, 2.5, v3);
    
    /* Additional direct comparisons */
    results[8] = 0;
    if (v1 < v2) results[8] |= 1;
    if (v6 > v1) results[8] |= 2;      /* NaN comparison */
    if (!(v1 < v6)) results[8] |= 4;   /* UNORDERED/UNGE */
    if (!(v6 > v2)) results[8] |= 8;   /* UNORDERED/UNLE */
    if (v6 == v6) results[8] |= 16;    /* UNORDERED for NaN */
    if (v6 != v6) results[8] |= 32;    /* UNORDERED for NaN */
    
    /* Test all relational operators with NaN */
    results[9] = 0;
    volatile long double nan_val = get_value(10);
    results[9] |= (nan_val < v1) ? 1 : 0;
    results[9] |= (nan_val > v1) ? 2 : 0;
    results[9] |= (nan_val <= v1) ? 4 : 0;
    results[9] |= (nan_val >= v1) ? 8 : 0;
    results[9] |= (nan_val == v1) ? 16 : 0;
    results[9] |= (nan_val != v1) ? 32 : 0;
    
    /* Compute hash to prevent dead code elimination */
    int hash = 0;
    for (int i = 0; i < 10; i++) {
        hash ^= results[i];
        /* Also use the values to prevent optimization */
        volatile int dummy = results[i];
        (void)dummy;
    }
    
    printf("Result hash: %d\n", hash);
    printf("Test completed - check generated assembly for x87 comparison mnemonics\n");
    
    return 0;
}
