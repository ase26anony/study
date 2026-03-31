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
    
    /* Ordered comparisons (should generate "ord" type mnemonics) */
    if (a < b && !(a != a) && !(b != b)) {
        result |= 1;  /* LT - Less Than (ordered) */
    }
    
    if (c > d && c == c && d == d) {
        result |= 2;  /* GT - Greater Than (ordered) */
    }
    
    if (a <= b && a == a && b == b) {
        result |= 4;  /* LE - Less or Equal (ordered) */
    }
    
    if (c >= d && c == c && d == d) {
        result |= 8;  /* GE - Greater or Equal (ordered) */
    }
    
    /* Equality comparisons */
    if (a == b) {
        result |= 16; /* EQ - Equal */
    }
    
    if (c != d) {
        result |= 32; /* NEQ - Not Equal */
    }
    
    /* Unordered comparisons with explicit NaN checks */
    if (a != a || b != b) {
        result |= 64; /* UNORDERED - either operand is NaN */
    }
    
    /* UNEQ: unordered or equal */
    if (!(a < b || a > b) || (a != a) || (b != b)) {
        result |= 128;
    }
    
    /* UNGE: not less than (unordered) - should generate "nlt" */
    if (!(a < b) || (a != a) || (b != b)) {
        result |= 256;
    }
    
    /* UNGT: not less or equal (unordered) - should generate "nle" */
    if (!(a <= b) || (a != a) || (b != b)) {
        result |= 512;
    }
    
    /* UNLE: unordered or less or equal - should generate "ule" */
    if ((a <= b) || (a != a) || (b != b)) {
        result |= 1024;
    }
    
    /* UNLT: unordered or less than - should generate "ult" */
    if ((a < b) || (a != a) || (b != b)) {
        result |= 2048;
    }
    
    /* LTGT: less than or greater than (ordered) - should generate "une" */
    if ((a < b || a > b) && (a == a) && (b == b)) {
        result |= 4096;
    }
    
    return result;
}

/* Function with switch based on long double comparisons */
int x87_switch_logic(long double x, long double y) {
    int code = 0;
    
    /* This complex condition may generate various x87 comparison codes */
    if (x != x || y != y) {
        code = 1;  /* UNORDERED */
    } else if (!(x < y || x > y)) {
        code = 2;  /* UNEQ or EQ */
    } else if (!(x < y)) {
        code = 3;  /* UNGE */
    } else if (!(x <= y)) {
        code = 4;  /* UNGT */
    } else if (x <= y) {
        code = 5;  /* UNLE */
    } else if (x < y) {
        code = 6;  /* UNLT */
    } else if (x < y || x > y) {
        code = 7;  /* LTGT */
    }
    
    return code;
}

/* Loop with long double termination condition */
int x87_loop_comparisons(long double start, long double limit) {
    volatile long double x = start;
    int count = 0;
    
    /* Loop condition using long double comparison */
    while (x < limit && x == x) {  /* x == x checks for NaN */
        x = x * 1.1L;  /* Force runtime computation */
        count++;
        if (count > 100) break; /* Safety limit */
    }
    
    return count;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int result = 0;
    
    /* These will promote to long double for x87 comparison */
    if (f < ld) result |= 1;
    if ((long double)d > ld) result |= 2;
    if ((long double)f <= (long double)d) result |= 4;
    if (ld >= (long double)f) result |= 8;
    
    /* Compare with integer constant cast to long double */
    if (ld == (long double)42) result |= 16;
    if (ld != (long double)-1) result |= 32;
    
    return result;
}

/* NaN-specific comparisons */
int nan_comparison_tests(void) {
    int result = 0;
    
    /* Generate NaNs in various ways */
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    long double nan3 = sqrtl(-1.0L);
    long double inf = __builtin_infl();
    long double normal = 3.14159265358979323846L;
    
    /* Compare NaN with normal numbers (all should be false for ordered comparisons) */
    if (!(nan1 < normal)) result |= 1;      /* UNGE: "nlt" */
    if (!(nan1 <= normal)) result |= 2;     /* UNGT: "nle" */
    if (!(nan1 > normal)) result |= 4;      /* UNLE: "ule"? Actually should be unordered */
    if (!(nan1 >= normal)) result |= 8;     /* UNLT: "ult"? Actually should be unordered */
    
    /* Compare NaN with NaN */
    if (nan1 == nan2) result |= 16;         /* Always false, but compiler might not know */
    if (nan1 != nan2) result |= 32;         /* Always true for quiet NaNs? */
    
    /* Compare NaN with infinity */
    if (!(nan1 < inf)) result |= 64;        /* UNGE: "nlt" */
    if (nan1 != inf) result |= 128;         /* Always true */
    
    /* Complex expression designed to trigger UNORDERED/ORDERED codes */
    if ((nan1 == nan1) && (normal == normal)) {
        /* Both are ordered */
        result |= 256;
    }
    
    if ((nan1 != nan1) || (normal != normal)) {
        /* At least one is unordered */
        result |= 512;
    }
    
    /* LTGT: ordered and not equal */
    if ((normal < inf || normal > inf) && (normal == normal) && (inf == inf)) {
        result |= 1024;
    }
    
    return result;
}

/* Main test function */
int main(void) {
    /* Initialize array with mix of values */
    global_ld[0] = 1.0L;
    global_ld[1] = 2.0L;
    global_ld[2] = 3.14159265358979323846L;
    global_ld[3] = -1.0L;
    global_ld[4] = 0.0L;
    global_ld[5] = __builtin_infl();
    global_ld[6] = -__builtin_infl();
    global_ld[7] = __builtin_nanl("");
    global_ld[8] = 0.0L / 0.0L;
    global_ld[9] = sqrtl(-1.0L);
    global_ld[10] = 100.0L;
    global_ld[11] = 1.0e-10L;
    global_ld[12] = 1.0e10L;
    global_ld[13] = -3.14159265358979323846L;
    global_ld[14] = 42.0L;
    global_ld[15] = -42.0L;
    
    int results[20];
    int result_index = 0;
    
    /* Test 1: Complex x87 comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ld(0), get_ld(1), get_ld(2), get_ld(3));
    
    /* Test 2: More complex with NaN */
    results[result_index++] = complex_x87_comparison(
        get_ld(7), get_ld(0), get_ld(5), get_ld(6));
    
    /* Test 3: Switch-based logic */
    results[result_index++] = x87_switch_logic(get_ld(0), get_ld(1));
    results[result_index++] = x87_switch_logic(get_ld(7), get_ld(0));  /* NaN vs normal */
    results[result_index++] = x87_switch_logic(get_ld(7), get_ld(8));  /* NaN vs NaN */
    
    /* Test 4: Loop comparisons */
    results[result_index++] = x87_loop_comparisons(get_ld(0), get_ld(10));
    
    /* Test 5: Mixed precision */
    results[result_index++] = mixed_precision_comparisons(
        3.14f, 2.718281828459045, get_ld(2));
    
    /* Test 6: NaN-specific tests */
    results[result_index++] = nan_comparison_tests();
    
    /* Test 7: Direct comparisons that might trigger specific mnemonics */
    volatile long double x = get_ld(global_index++);
    volatile long double y = get_ld(global_index++);
    
    /* Generate various comparison results */
    results[result_index++] = (x < y) ? 1 : 0;
    results[result_index++] = (x > y) ? 1 : 0;
    results[result_index++] = (x <= y) ? 1 : 0;
    results[result_index++] = (x >= y) ? 1 : 0;
    results[result_index++] = (x == y) ? 1 : 0;
    results[result_index++] = (x != y) ? 1 : 0;
    
    /* Unordered comparison patterns */
    results[result_index++] = (!(x < y) || (x != x) || (y != y)) ? 1 : 0;  /* UNGE */
    results[result_index++] = (!(x <= y) || (x != x) || (y != y)) ? 1 : 0; /* UNGT */
    results[result_index++] = ((x <= y) || (x != x) || (y != y)) ? 1 : 0;  /* UNLE */
    results[result_index++] = ((x < y) || (x != x) || (y != y)) ? 1 : 0;   /* UNLT */
    
    /* Final verification hash to prevent dead code elimination */
    int final_hash = 0;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= results[i];
    }
    
    printf("Test results hash: %d\n", final_hash);
    printf("Number of tests executed: %d\n", result_index);
    
    return final_hash != 0 ? 0 : 1;
}
