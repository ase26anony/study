/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in GCC's i386 backend
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
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

/* Complex multi-operand comparison using various condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons (should generate "ord" type mnemonics) */
    if (a < b && !(a != a) && !(b != b)) {
        result |= 1;  /* LT with ordered operands */
    }
    
    if (c >= d && c == c && d == d) {
        result |= 2;  /* GE with ordered operands */
    }
    
    /* Unordered comparisons (should generate "unord", "nlt", "nle", etc.) */
    if (!(a <= b) || (a != a) || (b != b)) {
        result |= 4;  /* UNGT or unordered case */
    }
    
    if (!(c > d) || (c != c) || (d != d)) {
        result |= 8;  /* UNLE or unordered case */
    }
    
    /* Equality with NaN handling */
    if (a == b || ((a != a) && (b != b))) {
        result |= 16;  /* EQ or UNEQ (both NaNs) */
    }
    
    if (a != b && !((a != a) && (b != b))) {
        result |= 32;  /* NEQ but not both NaN (LTGT) */
    }
    
    return result;
}

/* Test function focusing on unordered comparisons with NaN */
int test_nan_comparisons(void) {
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    long double inf = __builtin_infl();
    long double normal = 3.14159265358979323846L;
    
    volatile long double v_nan1 = nan1;
    volatile long double v_nan2 = nan2;
    volatile long double v_inf = inf;
    volatile long double v_normal = normal;
    
    int results = 0;
    
    /* UNORDERED comparisons (NaN compared with anything) */
    if (v_nan1 < v_normal) results |= 1;      /* false, unordered */
    if (v_nan1 <= v_inf) results |= 2;        /* false, unordered */
    if (v_nan1 > v_normal) results |= 4;      /* false, unordered */
    if (v_nan1 >= v_inf) results |= 8;        /* false, unordered */
    if (v_nan1 == v_nan2) results |= 16;      /* false, unordered (even NaN != NaN) */
    if (v_nan1 != v_normal) results |= 32;    /* true, unordered */
    
    /* UNEQ: either equal or unordered */
    if (!(v_nan1 < v_nan2) && !(v_nan1 > v_nan2)) results |= 64;
    
    /* UNGE: not less than or unordered (nlt) */
    if (!(v_nan1 < v_normal)) results |= 128;
    
    /* UNGT: not less than or equal or unordered (nle) */
    if (!(v_nan1 <= v_inf)) results |= 256;
    
    /* UNLE: less than or equal or unordered (ule) */
    if ((v_normal <= v_nan1) || (v_nan1 != v_nan1)) results |= 512;
    
    /* UNLT: less than or unordered (ult) */
    if ((v_normal < v_nan1) || (v_nan1 != v_nan1)) results |= 1024;
    
    /* LTGT: less than or greater than (une) - not unordered */
    if ((v_normal < v_inf) || (v_inf > v_normal)) results |= 2048;
    
    return results;
}

/* Test ordered comparisons with normal numbers */
int test_ordered_comparisons(void) {
    volatile long double a = get_ld(0);
    volatile long double b = get_ld(1);
    volatile long double c = get_ld(2);
    volatile long double d = get_ld(3);
    
    int results = 0;
    
    /* Basic ordered comparisons */
    if (a < b) results |= 1;
    if (c > d) results |= 2;
    if (a <= b) results |= 4;
    if (c >= d) results |= 8;
    if (a == b) results |= 16;
    if (a != b) results |= 32;
    
    /* Compound conditions */
    if ((a < b) && (c > d)) results |= 64;
    if ((a <= b) || (c >= d)) results |= 128;
    
    /* Nested comparisons */
    if (a < b) {
        if (c > d) {
            results |= 256;
        } else if (a == b) {
            results |= 512;
        }
    }
    
    return results;
}

/* Loop with long double termination condition */
int test_loop_comparisons(void) {
    volatile long double counter = get_ld(4);
    volatile long double limit = get_ld(5);
    int iterations = 0;
    
    /* Loop condition using long double comparison */
    while (counter < limit && iterations < 100) {
        if (counter != counter) break;  /* Check for NaN */
        counter += get_ld(6);
        iterations++;
    }
    
    return iterations;
}

/* Mixed precision comparisons */
int test_mixed_precision(void) {
    volatile float f = 1.5f;
    volatile double d = 2.718281828459045;
    volatile long double ld = get_ld(7);
    
    int results = 0;
    
    /* Comparisons with implicit promotions */
    if (ld > f) results |= 1;
    if (d < ld) results |= 2;
    if ((long double)f == ld) results |= 4;
    
    /* Explicit casts */
    if (ld > (long double)d) results |= 8;
    if ((long double)f <= ld) results |= 16;
    
    /* Integer constant comparisons */
    if (ld > 10.0L) results |= 32;
    if (ld < 100L) results |= 64;  /* integer constant */
    
    return results;
}

/* Switch statement based on comparison results */
int test_switch_comparisons(long double x, long double y) {
    int result = 0;
    
    /* Use comparison in switch condition */
    if (x < y) {
        result = 1;
    } else if (x > y) {
        result = 2;
    } else if (x == y) {
        result = 3;
    } else {
        /* Unordered case (one or both are NaN) */
        result = 4;
    }
    
    /* Nested switch based on multiple comparisons */
    switch(result) {
        case 1:
            if (x < 0.0L && y > 0.0L) result = 10;
            break;
        case 2:
            if (x > 0.0L && y < 0.0L) result = 20;
            break;
        case 3:
            if (x == 0.0L) result = 30;
            break;
        case 4:
            if (x != x || y != y) result = 40;
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize array with mixed values */
    global_ld[0] = 1.0L;
    global_ld[1] = 2.0L;
    global_ld[2] = 3.14159265358979323846L;
    global_ld[3] = 2.71828182845904523536L;
    global_ld[4] = 0.0L;
    global_ld[5] = 10.0L;
    global_ld[6] = 0.5L;
    global_ld[7] = 7.5L;
    global_ld[8] = __builtin_infl();      /* Positive infinity */
    global_ld[9] = -__builtin_infl();     /* Negative infinity */
    global_ld[10] = __builtin_nanl("");   /* Quiet NaN */
    global_ld[11] = 0.0L / 0.0L;          /* Another NaN */
    global_ld[12] = sqrtl(-1.0L);         /* NaN from sqrt(-1) */
    global_ld[13] = 1.0L / 0.0L;          /* Infinity */
    global_ld[14] = -5.0L;
    global_ld[15] = 100.0L;
    
    int hash = 0;
    
    /* Run all tests */
    hash ^= complex_x87_comparison(
        get_ld(0), get_ld(1), 
        get_ld(10), get_ld(2)  /* Includes NaN */
    );
    
    hash ^= test_nan_comparisons();
    hash ^= test_ordered_comparisons();
    hash ^= test_loop_comparisons();
    hash ^= test_mixed_precision();
    
    /* Test with various combinations */
    hash ^= test_switch_comparisons(get_ld(0), get_ld(1));
    hash ^= test_switch_comparisons(get_ld(10), get_ld(1));  /* NaN vs normal */
    hash ^= test_switch_comparisons(get_ld(10), get_ld(11)); /* NaN vs NaN */
    hash ^= test_switch_comparisons(get_ld(8), get_ld(9));   /* +inf vs -inf */
    
    /* Additional direct unordered comparisons */
    volatile long double nan_val = get_ld(10);
    volatile long double inf_val = get_ld(8);
    volatile long double normal_val = get_ld(3);
    
    if (!(nan_val < normal_val)) hash ^= 0x1000;      /* UNGE (nlt) */
    if (!(nan_val <= inf_val)) hash ^= 0x2000;        /* UNGT (nle) */
    if ((normal_val <= nan_val) || (nan_val != nan_val)) 
        hash ^= 0x4000;                               /* UNLE (ule) */
    if ((normal_val < nan_val) || (nan_val != nan_val)) 
        hash ^= 0x8000;                               /* UNLT (ult) */
    
    /* Final hash output to prevent dead code elimination */
    printf("Result hash: 0x%08x\n", hash);
    
    return 0;
}
