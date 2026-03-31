/* x87_comparison_test.c
 * Designed to trigger x87 floating-point comparison output logic
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -o x87_test x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbls[16];
volatile int global_index = 0;

/* Helper to get dynamic long double values */
long double get_ldbl(int idx) {
    return global_ldbls[idx % 16];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    /* This should generate various x87 comparison patterns */
    if (a != b) {
        if (c <= d) {
            if (a > c) {
                return 1;
            } else if (b < d) {
                return 2;
            }
        }
    }
    
    if (a == b && c >= d) {
        return 3;
    }
    
    /* Unordered comparisons with explicit NaN checks */
    long double nan_val = __builtin_nanl("");
    if (a != a) {  /* Traditional NaN check */
        if (b == b) {  /* b is not NaN */
            return 4;
        }
    }
    
    return 0;
}

/* Function focusing on unordered comparisons */
int unordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* Generate NaN through arithmetic */
    long double zero = 0.0L;
    long double nan1 = zero / zero;  /* Should produce NaN */
    long double nan2 = __builtin_nanl("");
    
    /* Comparisons involving NaN (should trigger UNORDERED cases) */
    if (nan1 < x) result |= 1;      /* UNORDERED/LT? */
    if (nan1 > y) result |= 2;      /* UNORDERED/GT? */
    if (nan1 <= x) result |= 4;     /* UNORDERED/LE? */
    if (nan1 >= y) result |= 8;     /* UNORDERED/GE? */
    if (nan1 == x) result |= 16;    /* UNORDERED/EQ? */
    if (nan1 != y) result |= 32;    /* UNORDERED/NEQ? */
    
    /* NaN vs NaN comparisons */
    if (nan1 == nan2) result |= 64;     /* Both NaN */
    if (nan1 != nan2) result |= 128;    /* Different NaN representations */
    
    /* Ordered comparisons (non-NaN) */
    if (x < y) result |= 256;
    if (x > y) result |= 512;
    
    return result;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(long double ld, double d, float f) {
    int result = 0;
    
    /* Comparisons with different floating types (promotions to long double) */
    if (ld < (long double)d) result |= 1;
    if ((long double)f > ld) result |= 2;
    if (ld == (long double)d) result |= 4;
    if ((long double)f != ld) result |= 8;
    
    /* Integer constant comparisons */
    if (ld < 10.0L) result |= 16;
    if (ld > -5.0L) result |= 32;
    
    return result;
}

/* Loop with long double termination condition */
int loop_with_ldbl_condition(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition based on long double comparison */
    while (counter < limit && iterations < 100) {
        counter += 0.5L;
        iterations++;
        
        /* Nested comparison inside loop */
        if (counter != start && counter <= limit) {
            iterations++;  /* Extra increment */
        }
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* Build conditions through comparisons */
    if (a < b) {
        result = 1;
    } else if (a > b) {
        result = 2;
    } else if (a == b) {
        result = 3;
    } else {
        /* NaN case - unordered */
        result = 4;
    }
    
    /* Additional switch based on result */
    switch (result) {
        case 1:
            if (c > a) result += 10;
            break;
        case 2:
            if (c < b) result += 20;
            break;
        case 3:
            if (c == a) result += 30;
            break;
        case 4:
            /* NaN handling */
            if (c != c) result += 40;  /* c is also NaN */
            break;
    }
    
    return result;
}

/* Test function with all comparison operators */
int all_comparison_operators(long double x, long double y) {
    int results[6] = {0};
    
    results[0] = (x < y) ? 1 : 0;    /* LT */
    results[1] = (x > y) ? 1 : 0;    /* GT */
    results[2] = (x <= y) ? 1 : 0;   /* LE */
    results[3] = (x >= y) ? 1 : 0;   /* GE */
    results[4] = (x == y) ? 1 : 0;   /* EQ */
    results[5] = (x != y) ? 1 : 0;   /* NE */
    
    /* Combine results */
    int combined = 0;
    for (int i = 0; i < 6; i++) {
        combined = (combined << 1) | results[i];
    }
    
    return combined;
}

/* Main test function */
int main() {
    /* Initialize array with various long double values */
    global_ldbls[0] = 1.0L;
    global_ldbls[1] = 2.0L;
    global_ldbls[2] = 3.14159265358979323846L; /* Pi */
    global_ldbls[3] = -1.5L;
    global_ldbls[4] = 0.0L;
    global_ldbls[5] = -0.0L;
    global_ldbls[6] = __builtin_infl();  /* Positive infinity */
    global_ldbls[7] = -__builtin_infl(); /* Negative infinity */
    global_ldbls[8] = __builtin_nanl(""); /* Quiet NaN */
    global_ldbls[9] = 0.0L / 0.0L;       /* Signaling NaN (typically) */
    global_ldbls[10] = sqrtl(-1.0L);     /* NaN from sqrt(-1) */
    global_ldbls[11] = 100.0L;
    global_ldbls[12] = 1.0e-10L;
    global_ldbls[13] = 1.0e10L;
    global_ldbls[14] = -3.0L;
    global_ldbls[15] = 42.0L;
    
    /* Array to store comparison results */
    int results[50];
    int result_index = 0;
    
    /* Test 1: Complex comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ldbl(0), get_ldbl(1), get_ldbl(2), get_ldbl(3));
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_index++] = unordered_comparisons(get_ldbl(0), get_ldbl(1));
    
    /* Test 3: Mixed precision */
    results[result_index++] = mixed_precision_comparisons(
        get_ldbl(2), 2.71828, 3.14f);
    
    /* Test 4: Loop with long double condition */
    results[result_index++] = loop_with_ldbl_condition(get_ldbl(0), get_ldbl(11));
    
    /* Test 5: Switch on comparison */
    results[result_index++] = switch_on_comparison(
        get_ldbl(8), get_ldbl(0), get_ldbl(8));  /* NaN vs normal vs NaN */
    
    /* Test 6: All comparison operators */
    for (int i = 0; i < 10; i++) {
        results[result_index++] = all_comparison_operators(
            get_ldbl(i), get_ldbl(i+1));
    }
    
    /* Test 7: Direct NaN comparisons */
    long double nan1 = get_ldbl(8);
    long double nan2 = get_ldbl(9);
    long double normal = get_ldbl(0);
    
    /* These should trigger UNORDERED/UNEQ/UNGE/etc. cases */
    results[result_index++] = (nan1 == normal) ? 1 : 0;
    results[result_index++] = (nan1 != normal) ? 1 : 0;
    results[result_index++] = (nan1 < normal) ? 1 : 0;
    results[result_index++] = (nan1 > normal) ? 1 : 0;
    results[result_index++] = (nan1 <= normal) ? 1 : 0;
    results[result_index++] = (nan1 >= normal) ? 1 : 0;
    
    results[result_index++] = (nan1 == nan2) ? 1 : 0;
    results[result_index++] = (nan1 != nan2) ? 1 : 0;
    
    /* Test 8: Infinity comparisons */
    long double pos_inf = get_ldbl(6);
    long double neg_inf = get_ldbl(7);
    
    results[result_index++] = (pos_inf > normal) ? 1 : 0;
    results[result_index++] = (neg_inf < normal) ? 1 : 0;
    results[result_index++] = (pos_inf == pos_inf) ? 1 : 0;
    results[result_index++] = (pos_inf > neg_inf) ? 1 : 0;
    
    /* Test 9: Complex expression with multiple comparisons */
    long double a = get_ldbl(0);
    long double b = get_ldbl(1);
    long double c = get_ldbl(2);
    long double d = get_ldbl(3);
    
    results[result_index++] = ((a < b) && (c > d) && (a != c)) ? 1 : 0;
    results[result_index++] = ((a == b) || (c <= d) || (b >= a)) ? 1 : 0;
    
    /* Test 10: More unordered scenarios */
    volatile long double volatile_nan = __builtin_nanl("");
    for (int i = 0; i < 5; i++) {
        results[result_index++] = (volatile_nan < get_ldbl(i)) ? 1 : 0;
        results[result_index++] = (get_ldbl(i) > volatile_nan) ? 1 : 0;
    }
    
    /* Compute hash to prevent dead code elimination */
    uint32_t hash = 0;
    for (int i = 0; i < result_index; i++) {
        hash ^= (results[i] + i);
    }
    
    printf("Result hash: 0x%08x\n", hash);
    printf("Number of tests executed: %d\n", result_index);
    
    /* Print some results for verification */
    printf("Sample results: ");
    for (int i = 0; i < 10 && i < result_index; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    return (hash == 0) ? 1 : 0;  /* Non-zero return if all results were 0 */
}
