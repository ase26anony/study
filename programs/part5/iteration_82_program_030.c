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
    
    /* Ordered comparisons (should generate "ord" type mnemonics) */
    if (a < b && !(a != a) && !(b != b)) {
        result |= 1;  /* LT with ordered operands */
    }
    
    if (c > d && c == c && d == d) {
        result |= 2;  /* GT with ordered operands */
    }
    
    /* Equality comparisons */
    if (a == b) {
        result |= 4;  /* EQ */
    }
    
    if (c != d) {
        result |= 8;  /* NEQ/UNE */
    }
    
    /* Unordered comparisons with NaN */
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    
    /* UNORDERED: compare NaN with anything */
    if (!(nan1 <= b) && !(nan1 >= b)) {
        result |= 16;  /* UNORDERED case */
    }
    
    /* UNEQ: unordered or equal */
    if (!(nan1 < a) && !(nan1 > a)) {
        result |= 32;  /* UNEQ case */
    }
    
    /* UNGE: not less than (unordered or greater or equal) */
    if (!(a < nan2)) {
        result |= 64;  /* UNGE/nlt case */
    }
    
    /* UNGT: not less than or equal (unordered or greater) */
    if (!(b <= nan1)) {
        result |= 128;  /* UNGT/nle case */
    }
    
    /* UNLE: unordered or less or equal */
    if (!(nan2 > c)) {
        result |= 256;  /* UNLE/ule case */
    }
    
    /* UNLT: unordered or less than */
    if (!(nan1 >= d)) {
        result |= 512;  /* UNLT/ult case */
    }
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if ((a < b) || (a > b)) {
        result |= 1024;  /* LTGT/une case */
    }
    
    return result;
}

/* Test function for ordered comparisons */
int test_ordered_comparisons(long double x, long double y) {
    int results = 0;
    
    /* Basic ordered comparisons */
    if (x < y)  results |= 1;
    if (x > y)  results |= 2;
    if (x <= y) results |= 4;
    if (x >= y) results |= 8;
    if (x == y) results |= 16;
    if (x != y) results |= 32;
    
    /* Compound ordered comparisons */
    if (x < y && x > 0.0L) results |= 64;
    if (x <= y || x == 0.0L) results |= 128;
    
    return results;
}

/* Test function for unordered comparisons */
int test_unordered_comparisons(long double x, long double y) {
    int results = 0;
    volatile long double nan = __builtin_nanl("");
    volatile long double inf = __builtin_infl();
    
    /* Compare with NaN (triggers unordered conditions) */
    if (x == nan) results |= 1;        /* Always false, but compiler must generate code */
    if (x != nan) results |= 2;        /* Always true for non-NaN x */
    if (nan < y)  results |= 4;        /* Unordered comparison */
    if (nan > y)  results |= 8;        /* Unordered comparison */
    if (nan <= y) results |= 16;       /* UNLE */
    if (nan >= y) results |= 32;       /* UNGE */
    
    /* Compare NaN with NaN */
    long double nan2 = sqrtl(-1.0L);
    if (nan == nan2) results |= 64;    /* UNEQ */
    if (nan != nan2) results |= 128;   /* Always true */
    
    /* Compare with infinity */
    if (x == inf) results |= 256;
    if (x < inf)  results |= 512;
    if (x > -inf) results |= 1024;
    
    return results;
}

/* Loop with long double termination condition */
int loop_with_ld_comparison(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Loop condition using long double comparison */
    while (counter < limit && iterations < 100) {
        /* Mixed precision comparison inside loop */
        float f = (float)counter;
        double d = (double)counter;
        
        if (counter > (long double)f && counter < (long double)d + 1.0L) {
            iterations++;
        }
        
        counter += 0.5L;
        
        /* Break on NaN detection */
        if (counter != counter) {
            break;
        }
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int switch_on_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* First level of comparisons */
    if (a < b) {
        result = 1;
        /* Nested comparison */
        if (b > c) {
            result = 2;
            if (a == c) {
                result = 3;
            }
        }
    } else if (a > b) {
        result = 4;
        if (b < c) {
            result = 5;
        }
    } else {  /* a == b or unordered */
        result = 6;
        /* Check for NaN */
        if (a != a || b != b) {
            result = 7;  /* Unordered case */
        }
    }
    
    return result;
}

/* Initialize test data */
void init_test_data(void) {
    /* Normal numbers */
    global_ld[0] = 0.0L;
    global_ld[1] = 1.0L;
    global_ld[2] = -1.0L;
    global_ld[3] = 3.14159265358979323846L;
    global_ld[4] = 2.71828182845904523536L;
    
    /* Large and small numbers */
    global_ld[5] = 1.0e100L;
    global_ld[6] = 1.0e-100L;
    global_ld[7] = -1.0e100L;
    
    /* Special values */
    global_ld[8] = __builtin_infl();      /* Positive infinity */
    global_ld[9] = -__builtin_infl();     /* Negative infinity */
    global_ld[10] = __builtin_nanl("");   /* Quiet NaN */
    global_ld[11] = 0.0L / 0.0L;          /* NaN from division */
    global_ld[12] = sqrtl(-1.0L);         /* NaN from sqrt */
    
    /* Denormal candidates */
    global_ld[13] = 1.0e-4900L;
    global_ld[14] = -1.0e-4900L;
    
    /* Integer cast to long double */
    global_ld[15] = (long double)(1ULL << 63);
}

int main(void) {
    init_test_data();
    
    int bool_results[256];
    int result_index = 0;
    uint32_t hash = 0;
    
    /* Test 1: Ordered comparisons with normal numbers */
    bool_results[result_index++] = test_ordered_comparisons(
        get_ld(1), get_ld(3));
    bool_results[result_index++] = test_ordered_comparisons(
        get_ld(2), get_ld(0));
    bool_results[result_index++] = test_ordered_comparisons(
        get_ld(4), get_ld(4));  /* Equal values */
    
    /* Test 2: Unordered comparisons with NaN and Inf */
    bool_results[result_index++] = test_unordered_comparisons(
        get_ld(0), get_ld(10));  /* 0 vs NaN */
    bool_results[result_index++] = test_unordered_comparisons(
        get_ld(8), get_ld(9));   /* +Inf vs -Inf */
    bool_results[result_index++] = test_unordered_comparisons(
        get_ld(10), get_ld(11)); /* NaN vs NaN */
    
    /* Test 3: Complex x87 comparison function */
    bool_results[result_index++] = complex_x87_comparison(
        get_ld(1), get_ld(2), get_ld(3), get_ld(4));
    bool_results[result_index++] = complex_x87_comparison(
        get_ld(10), get_ld(1), get_ld(11), get_ld(0));  /* With NaNs */
    
    /* Test 4: Loop with long double comparison */
    bool_results[result_index++] = loop_with_ld_comparison(
        get_ld(0), get_ld(5));
    
    /* Test 5: Switch based on comparisons */
    bool_results[result_index++] = switch_on_comparison(
        get_ld(1), get_ld(3), get_ld(2));
    bool_results[result_index++] = switch_on_comparison(
        get_ld(10), get_ld(1), get_ld(0));  /* With NaN */
    
    /* Test 6: Mixed precision comparisons */
    float f1 = 1.5f;
    double d1 = 2.5;
    long double ld1 = get_ld(3);
    
    bool_results[result_index++] = (ld1 > (long double)f1);
    bool_results[result_index++] = (ld1 < (long double)d1);
    bool_results[result_index++] = ((long double)f1 == ld1);
    
    /* Test 7: Integer constant comparisons */
    bool_results[result_index++] = (ld1 > 3.0L);
    bool_results[result_index++] = (ld1 < 4.0L);
    bool_results[result_index++] = (get_ld(15) == (long double)(1ULL << 63));
    
    /* Test 8: More unordered scenarios */
    volatile long double nan = get_ld(10);
    volatile long double num = get_ld(1);
    
    /* Direct unordered comparisons that should hit the uncovered cases */
    int unordered_test = 0;
    unordered_test |= !(nan < num) ? 1 : 0;   /* UNGE/nlt */
    unordered_test |= !(nan <= num) ? 2 : 0;  /* UNGT/nle */
    unordered_test |= !(nan > num) ? 4 : 0;   /* UNLE/ule */
    unordered_test |= !(nan >= num) ? 8 : 0;  /* UNLT/ult */
    unordered_test |= (nan != nan) ? 16 : 0;  /* Always true for NaN */
    
    bool_results[result_index++] = unordered_test;
    
    /* Compute hash to prevent dead code elimination */
    for (int i = 0; i < result_index; i++) {
        hash ^= (uint32_t)bool_results[i] + (i * 0x9e3779b9);
    }
    
    printf("Test completed. Hash: 0x%08x\n", hash);
    printf("Number of tests executed: %d\n", result_index);
    
    return (int)(hash & 0x7FFFFFFF);
}
