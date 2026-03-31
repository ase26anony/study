/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -o x87_test x87_comparison_test.c
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
    /* This should generate various x87 comparison patterns */
    if (a != b) {
        if (c <= d) {
            if (a > c) {
                return 1;
            } else if (b < d) {
                return 2;
            }
        }
        if (a == c || b == d) {
            return 3;
        }
    }
    
    /* Unordered comparisons with explicit NaN checks */
    if (!(a == a) || !(b == b)) {  /* NaN detection */
        if (c >= d) {
            return 4;
        }
    }
    
    return 0;
}

/* Test ordered comparisons */
int test_ordered_comparisons(long double x, long double y) {
    int result = 0;
    
    /* All standard ordered comparisons */
    if (x < y)  result |= 1;
    if (x > y)  result |= 2;
    if (x <= y) result |= 4;
    if (x >= y) result |= 8;
    if (x == y) result |= 16;
    if (x != y) result |= 32;
    
    return result;
}

/* Test unordered comparisons with NaN */
int test_unordered_comparisons(long double nan_val, long double normal) {
    int result = 0;
    
    /* Comparisons involving NaN (unordered cases) */
    if (nan_val < normal)   result |= 1;    /* false, may generate unordered */
    if (nan_val > normal)   result |= 2;    /* false, may generate unordered */
    if (nan_val <= normal)  result |= 4;    /* false, may generate unordered */
    if (nan_val >= normal)  result |= 8;    /* false, may generate unordered */
    if (nan_val == normal)  result |= 16;   /* false, may generate unordered */
    if (nan_val != normal)  result |= 32;   /* true, may generate UNEQ or similar */
    
    /* Compare NaN with NaN */
    long double nan2 = __builtin_nanl("");
    if (nan_val < nan2)     result |= 64;
    if (nan_val > nan2)     result |= 128;
    if (nan_val == nan2)    result |= 256;
    if (nan_val != nan2)    result |= 512;
    
    return result;
}

/* Mixed precision comparisons */
int test_mixed_precision(long double ld, double d, float f) {
    int result = 0;
    
    /* Compare long double with double (promotion happens) */
    if (ld < d)     result |= 1;
    if (ld > d)     result |= 2;
    if (ld == d)    result |= 4;
    
    /* Compare long double with float */
    if (ld < f)     result |= 8;
    if (ld > f)     result |= 16;
    if (ld != f)    result |= 32;
    
    /* Compare with integer constant cast to long double */
    if (ld < (long double)10)   result |= 64;
    if (ld > (long double)-5)   result |= 128;
    
    return result;
}

/* Loop with long double termination condition */
int test_loop_comparisons(void) {
    volatile long double x = 0.0L;
    volatile long double limit = 10.0L;
    int count = 0;
    
    /* Loop condition based on long double comparison */
    while (x < limit && x == x) {  /* x == x checks for NaN */
        x += 1.5L;
        count++;
        
        /* Nested comparison inside loop */
        if (x > 5.0L && x <= 8.0L) {
            count += 10;
        }
    }
    
    /* Do-while with complex condition */
    do {
        x -= 0.5L;
        if (x != 0.0L && !(x < 0.0L)) {
            count--;
        }
    } while (x > 0.0L);
    
    return count;
}

/* Switch statement based on comparison results */
int test_switch_comparison(long double a, long double b, long double c) {
    int result = 0;
    
    /* Complex condition for switch */
    switch ((a < b) + (b > c) * 2 + (a == c) * 4) {
        case 0:
            result = 100;
            break;
        case 1:
            result = 200;
            if (a != b && c <= a) {
                result += 50;
            }
            break;
        case 2:
            result = 300;
            break;
        case 3:
            result = 400;
            /* Nested comparison */
            if (!(b < c) && a >= b) {
                result += 75;
            }
            break;
        default:
            result = 500;
            if (a == a && b == b && c == c) {  /* All are not NaN */
                result += (a > 0.0L) ? 25 : 50;
            }
    }
    
    return result;
}

/* Generate NaN values in various ways */
long double generate_nan(int method) {
    switch (method) {
        case 0: return __builtin_nanl("");          /* Quiet NaN */
        case 1: return 0.0L / 0.0L;                 /* Division by zero */
        case 2: return __builtin_sqrtl(-1.0L);      /* sqrt(-1) */
        case 3: return __builtin_fabsl(__builtin_nanl("")); /* abs(NaN) */
        case 4: return __builtin_nanl("0xABC");     /* NaN with payload */
        default: return __builtin_infl() * 0.0L;    /* Infinity * 0 */
    }
}

int main(void) {
    /* Initialize test array with various values */
    global_ld[0] = 1.0L;
    global_ld[1] = 2.5L;
    global_ld[2] = -3.75L;
    global_ld[3] = 0.0L;
    global_ld[4] = -0.0L;
    global_ld[5] = __builtin_infl();      /* Positive infinity */
    global_ld[6] = -__builtin_infl();     /* Negative infinity */
    global_ld[7] = __builtin_nanl("");    /* Quiet NaN */
    global_ld[8] = 0.0L / 0.0L;           /* Signaling NaN (typically) */
    global_ld[9] = 100.0L;
    global_ld[10] = 1.0e-10L;
    global_ld[11] = 1.0e10L;
    global_ld[12] = __builtin_sqrtl(-1.0L); /* Another NaN */
    global_ld[13] = 3.14159265358979323846L; /* Pi */
    global_ld[14] = 2.71828182845904523536L; /* e */
    global_ld[15] = generate_nan(4);       /* Generated NaN */
    
    int results[50];
    int result_index = 0;
    
    /* Test 1: Ordered comparisons between normal values */
    results[result_index++] = test_ordered_comparisons(get_ld(0), get_ld(1));
    results[result_index++] = test_ordered_comparisons(get_ld(2), get_ld(3));
    results[result_index++] = test_ordered_comparisons(get_ld(4), get_ld(5));
    results[result_index++] = test_ordered_comparisons(get_ld(9), get_ld(10));
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_index++] = test_unordered_comparisons(get_ld(7), get_ld(0));
    results[result_index++] = test_unordered_comparisons(get_ld(8), get_ld(6));
    results[result_index++] = test_unordered_comparisons(get_ld(12), get_ld(15));
    
    /* Test 3: Complex multi-operand comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ld(0), get_ld(1), get_ld(2), get_ld(3));
    results[result_index++] = complex_x87_comparison(
        get_ld(5), get_ld(6), get_ld(7), get_ld(8));
    results[result_index++] = complex_x87_comparison(
        get_ld(9), get_ld(10), get_ld(11), get_ld(12));
    
    /* Test 4: Mixed precision */
    results[result_index++] = test_mixed_precision(
        get_ld(0), 2.5, 3.14f);
    results[result_index++] = test_mixed_precision(
        get_ld(13), 3.14159, 2.71828f);
    
    /* Test 5: Loop comparisons */
    results[result_index++] = test_loop_comparisons();
    
    /* Test 6: Switch based on comparisons */
    results[result_index++] = test_switch_comparison(
        get_ld(0), get_ld(1), get_ld(2));
    results[result_index++] = test_switch_comparison(
        get_ld(7), get_ld(8), get_ld(9));
    
    /* Test 7: Direct NaN comparisons that should trigger specific mnemonics */
    volatile long double nan1 = generate_nan(0);
    volatile long double nan2 = generate_nan(1);
    volatile long double inf = __builtin_infl();
    volatile long double normal = 42.0L;
    
    /* These comparisons should generate the specific x87 condition codes */
    int unordered_tests = 0;
    unordered_tests |= (nan1 == nan1) ? 0 : 1;      /* UNORDERED/UNEQ */
    unordered_tests |= (nan1 < normal) ? 0 : 2;     /* UNORDERED/UNLT */
    unordered_tests |= (nan1 > normal) ? 0 : 4;     /* UNORDERED/UNGT */
    unordered_tests |= (normal <= nan1) ? 0 : 8;    /* UNORDERED/UNLE */
    unordered_tests |= (normal >= nan1) ? 0 : 16;   /* UNORDERED/UNGE */
    unordered_tests |= (nan1 != normal) ? 32 : 0;   /* UNEQ */
    unordered_tests |= (nan1 == nan2) ? 0 : 64;     /* UNORDERED/LTGT */
    unordered_tests |= (nan1 != nan2) ? 128 : 0;    /* UNORDERED */
    
    results[result_index++] = unordered_tests;
    
    /* Test 8: Infinity comparisons */
    int inf_tests = 0;
    inf_tests |= (inf > normal) ? 1 : 0;
    inf_tests |= (inf < -inf) ? 2 : 0;
    inf_tests |= (inf == inf) ? 4 : 0;
    inf_tests |= (inf != inf) ? 8 : 0;
    inf_tests |= (-inf < normal) ? 16 : 0;
    inf_tests |= (normal > -inf) ? 32 : 0;
    
    results[result_index++] = inf_tests;
    
    /* Final verification hash to prevent dead code elimination */
    uint32_t hash = 0;
    for (int i = 0; i < result_index; i++) {
        hash ^= (results[i] + i) * 0x9e3779b9;
    }
    
    printf("Test results hash: 0x%08x\n", hash);
    printf("Total tests executed: %d\n", result_index);
    
    /* Print some results for debugging */
    printf("Sample results: %d, %d, %d\n", results[0], results[5], results[10]);
    
    return (hash != 0) ? 0 : 1;
}
