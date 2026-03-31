/* x87_comparison_test.c
 * Designed to trigger x87 floating-point comparison mnemonics in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double g_values[16];
volatile int g_index = 0;

/* Helper to get dynamic long double values */
long double get_value(int idx) {
    return g_values[idx % 16];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;      /* LT */
    if (a > c) result |= 2;      /* GT */
    if (a <= d) result |= 4;     /* LE */
    if (b >= c) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (c != d) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with potential NaN */
    volatile long double nan_val = __builtin_nanl("");
    if (!(a < nan_val)) result |= 64;    /* UNORDERED/ORDERED */
    if (!(nan_val > b)) result |= 128;   /* UNORDERED/ORDERED */
    
    /* Mixed comparisons */
    if (a == a) result |= 256;   /* Self-comparison (tests for NaN) */
    if (b != b) result |= 512;   /* NaN detection */
    
    return result;
}

/* Function focusing on unordered comparisons */
int unordered_comparisons(long double x, long double y) {
    int result = 0;
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;  /* Another way to get NaN */
    volatile long double inf = __builtin_infl();
    
    /* Compare NaN with normal numbers */
    if (!(nan1 < x)) result |= 1;      /* UNGE (nlt) */
    if (!(nan1 > y)) result |= 2;      /* UNLE (ule?) */
    if (!(x > nan2)) result |= 4;      /* UNLE */
    if (!(y < nan1)) result |= 8;      /* UNGE */
    
    /* Compare NaN with NaN */
    if (nan1 == nan2) result |= 16;    /* UNEQ? Actually false for NaN==NaN */
    if (nan1 != nan2) result |= 32;    /* Always true for NaN!=NaN */
    
    /* Compare with infinity */
    if (!(inf < nan1)) result |= 64;   /* UNORDERED */
    if (!(nan2 > inf)) result |= 128;  /* UNORDERED */
    
    /* LTGT comparison (unordered or not equal) */
    if (x != y) result |= 256;         /* LTGT (une) */
    
    return result;
}

/* Function with control flow based on long double comparisons */
int control_flow_test(long double base) {
    volatile long double accum = base;
    int count = 0;
    
    /* Loop with long double condition */
    while (accum < 100.0L && !(accum != accum)) {  /* accum != accum checks for NaN */
        accum *= 1.5L;
        count++;
        
        /* Nested if with complex condition */
        if (accum > 50.0L && accum <= 75.0L) {
            accum += 10.0L;
        }
        
        /* Switch-like behavior based on comparison results */
        if (accum < 0.0L) {
            count += 10;
        } else if (!(accum >= 0.0L)) {  /* UNLT case */
            count += 20;
        }
    }
    
    /* Final comparison for return */
    if (accum != accum) {  /* is NaN */
        return -count;
    } else if (!(accum < 0.0L)) {  /* UNGE (nlt) */
        return count * 2;
    }
    
    return count;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(long double ld, double d, float f) {
    int result = 0;
    
    /* Compare long double with double */
    if (ld < (long double)d) result |= 1;
    if ((long double)f > ld) result |= 2;
    
    /* Compare with integer constant */
    if (ld <= 100.0L) result |= 4;
    if (!(ld >= 0.0L)) result |= 8;  /* UNLT */
    
    /* Complex mixed expression */
    long double temp = ld + (long double)d + (long double)f;
    if (temp == temp) {  /* Not NaN */
        if (temp < 1000.0L) result |= 16;
    }
    
    return result;
}

/* Test function that uses all comparison types */
int exhaustive_comparisons(long double a, long double b, long double c) {
    int result = 0;
    
    /* Generate some NaN values */
    volatile long double nan_val = sqrtl(-1.0L);
    volatile long double zero = 0.0L;
    volatile long double inf = 1.0L / zero;  /* Infinity */
    
    /* Test all relational operators with NaN */
    result |= (a < nan_val) ? 0 : 1;        /* UNORDERED */
    result |= (nan_val > b) ? 0 : 2;        /* UNORDERED */
    result |= (c <= nan_val) ? 0 : 4;       /* UNORDERED */
    result |= (nan_val >= a) ? 0 : 8;       /* UNORDERED */
    result |= (nan_val == nan_val) ? 0 : 16; /* UNORDERED/EQ */
    result |= (nan_val != b) ? 32 : 0;      /* Always true */
    
    /* Test UNEQ, UNGE, UNGT, UNLE, UNLT */
    result |= (!(a < b) && (a == a && b == b)) ? 64 : 0;  /* GE (not LT) */
    result |= (!(a > b) && (a == a && b == b)) ? 128 : 0; /* LE (not GT) */
    
    /* LTGT - unordered or not equal */
    result |= (a != b) ? 256 : 0;
    
    /* Compare normal numbers */
    result |= (a < b) ? 512 : 0;
    result |= (a > c) ? 1024 : 0;
    result |= (b <= c) ? 2048 : 0;
    result |= (c >= a) ? 4096 : 0;
    result |= (a == c) ? 8192 : 0;
    result |= (b != a) ? 16384 : 0;
    
    return result;
}

int main() {
    /* Initialize array with mixed values */
    g_values[0] = 1.0L;
    g_values[1] = 2.5L;
    g_values[2] = -3.75L;
    g_values[3] = 100.0L;
    g_values[4] = 0.0L;
    g_values[5] = -0.0L;
    g_values[6] = __builtin_infl();
    g_values[7] = -__builtin_infl();
    g_values[8] = __builtin_nanl("");
    g_values[9] = 0.0L / 0.0L;
    g_values[10] = sqrtl(-1.0L);
    g_values[11] = 1.0e-10L;
    g_values[12] = 1.0e10L;
    g_values[13] = 3.14159265358979323846L;
    g_values[14] = 2.71828182845904523536L;
    g_values[15] = 42.0L;
    
    int results[32];
    int result_count = 0;
    
    /* Test 1: Complex x87 comparisons */
    results[result_count++] = complex_x87_comparison(
        get_value(0), get_value(1), get_value(2), get_value(3));
    
    /* Test 2: Unordered comparisons */
    results[result_count++] = unordered_comparisons(get_value(5), get_value(6));
    
    /* Test 3: Control flow */
    results[result_count++] = control_flow_test(get_value(0));
    
    /* Test 4: Mixed precision */
    results[result_count++] = mixed_precision_comparisons(
        get_value(13), (double)get_value(14), (float)get_value(15));
    
    /* Test 5: Exhaustive comparisons */
    results[result_count++] = exhaustive_comparisons(
        get_value(7), get_value(8), get_value(9));
    
    /* Additional tests with different value combinations */
    for (int i = 0; i < 10; i++) {
        results[result_count++] = complex_x87_comparison(
            get_value(i), get_value(i+1), get_value(i+2), get_value(i+3));
        results[result_count++] = unordered_comparisons(
            get_value(i), get_value(15-i));
    }
    
    /* Compute verification hash (XOR of all results) */
    int verification = 0;
    for (int i = 0; i < result_count; i++) {
        verification ^= results[i];
    }
    
    /* Print verification to prevent dead code elimination */
    printf("Verification hash: %d\n", verification);
    printf("Number of tests executed: %d\n", result_count);
    
    /* Additional print to ensure all code paths are considered */
    printf("Sample comparisons:\n");
    printf("  NaN < 1.0: %d\n", (get_value(8) < 1.0L) ? 1 : 0);
    printf("  1.0 > NaN: %d\n", (1.0L > get_value(8)) ? 1 : 0);
    printf("  NaN == NaN: %d\n", (get_value(8) == get_value(9)) ? 1 : 0);
    printf("  NaN != NaN: %d\n", (get_value(8) != get_value(9)) ? 1 : 0);
    printf("  1.0 != 2.0: %d\n", (1.0L != 2.0L) ? 1 : 0);
    
    return 0;
}
