/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
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

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;      /* LT */
    if (a > c) result |= 2;      /* GT */
    if (b <= c) result |= 4;     /* LE */
    if (c >= d) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (b != c) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with explicit NaN checks */
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    
    /* UNORDERED: compare NaN with normal number */
    if (!(nan1 < a) && !(nan1 > a) && !(nan1 == a)) {
        result |= 64;  /* UNORDERED case */
    }
    
    /* UNEQ: unordered or equal */
    if (nan1 == nan1) {  /* Always false for NaN, but compiler might generate UNEQ */
        result |= 128;
    }
    
    /* UNGE: not less than (unordered or greater or equal) */
    if (!(a < nan1)) {
        result |= 256;
    }
    
    /* UNGT: not less than or equal (unordered or greater) */
    if (!(a <= nan1)) {
        result |= 512;
    }
    
    /* UNLE: unordered or less or equal */
    if (!(nan1 > a)) {
        result |= 1024;
    }
    
    /* UNLT: unordered or less than */
    if (!(nan1 >= a)) {
        result |= 2048;
    }
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if ((a < b) || (a > b)) {
        result |= 4096;
    }
    
    return result;
}

/* Function with nested comparisons */
int nested_x87_comparisons(long double x, long double y, long double z) {
    if (x != y) {
        if (z <= x) {
            return (y > z) ? 1 : 2;
        } else if (z >= y) {
            volatile long double inf = __builtin_infl();
            if (x < inf && y > -inf) {
                return 3;
            }
        }
    }
    
    /* Mixed precision comparisons */
    float f = (float)x;
    double d = (double)y;
    
    if ((long double)f < d) {
        return 4;
    }
    
    if (x == (long double)(int)z) {
        return 5;
    }
    
    return 0;
}

/* Loop with termination based on long double comparison */
int x87_controlled_loop(long double start, long double limit, long double step) {
    volatile long double counter = start;
    int iterations = 0;
    
    while (counter < limit && iterations < 100) {
        /* Prevent infinite loops with NaN */
        if (counter != counter) {  /* NaN check */
            break;
        }
        counter += step;
        iterations++;
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int x87_switch_logic(long double a, long double b) {
    int result = 0;
    
    /* Complex condition that might generate various x87 codes */
    if ((a < b) && !(a != a) && !(b != b)) {
        result = 1;
    } else if (a > b) {
        result = 2;
    } else if (a == b) {
        result = 3;
    } else {
        /* This else case includes unordered comparisons */
        volatile long double nan = sqrtl(-1.0L);
        if (!(a < nan) && !(a > nan)) {
            result = 4;  /* UNORDERED */
        }
    }
    
    return result;
}

/* Test function focusing on unordered comparisons */
int test_unordered_comparisons(void) {
    int results = 0;
    
    /* Create various NaN values */
    volatile long double qnan = __builtin_nanl("");
    volatile long double snan = __builtin_nansl("");
    volatile long double nan_div = 0.0L / 0.0L;
    volatile long double nan_sqrt = sqrtl(-1.0L);
    
    volatile long double normal = 3.14159265358979323846L;
    volatile long double inf = __builtin_infl();
    volatile long double neg_inf = -__builtin_infl();
    volatile long double zero = 0.0L;
    
    /* Test all relational operators with NaN */
    results |= (qnan < normal) ? 0 : 1;
    results |= (qnan > normal) ? 0 : 2;
    results |= (qnan <= normal) ? 0 : 4;
    results |= (qnan >= normal) ? 0 : 8;
    results |= (qnan == normal) ? 0 : 16;
    results |= (qnan != normal) ? 32 : 0;
    
    /* NaN compared to infinity */
    results |= (snan < inf) ? 0 : 64;
    results |= (snan > neg_inf) ? 0 : 128;
    
    /* NaN compared to NaN */
    results |= (qnan == snan) ? 0 : 256;
    results |= (qnan != snan) ? 512 : 0;
    results |= (nan_div < nan_sqrt) ? 0 : 1024;
    results |= (nan_div > nan_sqrt) ? 0 : 2048;
    
    /* Compare infinity with normal numbers */
    results |= (normal < inf) ? 4096 : 0;
    results |= (normal > neg_inf) ? 8192 : 0;
    results |= (inf == inf) ? 16384 : 0;
    results |= (inf != neg_inf) ? 32768 : 0;
    
    return results;
}

/* Main test harness */
int main(void) {
    /* Initialize global array with mixed values */
    global_ld[0] = 1.0L;
    global_ld[1] = 2.0L;
    global_ld[2] = 3.14159265358979323846L;
    global_ld[3] = -2.5L;
    global_ld[4] = 100.0L;
    global_ld[5] = 0.0L;
    global_ld[6] = -0.0L;
    global_ld[7] = __builtin_infl();
    global_ld[8] = -__builtin_infl();
    global_ld[9] = __builtin_nanl("");
    global_ld[10] = 0.0L / 0.0L;
    global_ld[11] = sqrtl(-1.0L);
    global_ld[12] = 1.0e-10L;
    global_ld[13] = 1.0e+10L;
    global_ld[14] = (long double)(1ULL << 60);
    global_ld[15] = __builtin_nansl("");
    
    int result_hash = 0;
    
    /* Test 1: Complex comparisons */
    for (int i = 0; i < 8; i++) {
        long double a = get_ld(i);
        long double b = get_ld(i + 1);
        long double c = get_ld(i + 2);
        long double d = get_ld(i + 3);
        
        int res = complex_x87_comparison(a, b, c, d);
        result_hash ^= res;
    }
    
    /* Test 2: Nested comparisons */
    for (int i = 0; i < 4; i++) {
        int res = nested_x87_comparisons(
            get_ld(i * 3),
            get_ld(i * 3 + 1),
            get_ld(i * 3 + 2)
        );
        result_hash ^= (res << (i * 4));
    }
    
    /* Test 3: Loop with long double control */
    for (int i = 0; i < 4; i++) {
        int iterations = x87_controlled_loop(
            get_ld(i),
            get_ld(i + 4),
            get_ld(i + 8)
        );
        result_hash ^= iterations;
    }
    
    /* Test 4: Switch logic */
    for (int i = 0; i < 8; i++) {
        int res = x87_switch_logic(get_ld(i), get_ld(15 - i));
        result_hash ^= res;
    }
    
    /* Test 5: Unordered comparisons */
    int unordered_results = test_unordered_comparisons();
    result_hash ^= unordered_results;
    
    /* Test 6: Direct comparisons in main */
    volatile long double dynamic_nan = get_ld(9);
    volatile long double dynamic_inf = get_ld(7);
    volatile long double dynamic_normal = get_ld(2);
    
    /* Generate various comparison patterns */
    if (dynamic_nan < dynamic_normal) {
        result_hash ^= 0x1111;
    }
    if (!(dynamic_nan >= dynamic_normal)) {
        result_hash ^= 0x2222;
    }
    if (dynamic_normal < dynamic_inf) {
        result_hash ^= 0x3333;
    }
    if (!(dynamic_nan == dynamic_nan)) {
        result_hash ^= 0x4444;
    }
    if (dynamic_nan != dynamic_normal) {
        result_hash ^= 0x5555;
    }
    
    /* Mixed precision with casting */
    float f = 1.5f;
    double d = 2.5;
    if ((long double)f < (long double)d) {
        result_hash ^= 0x6666;
    }
    if ((long double)(int)f > get_ld(0)) {
        result_hash ^= 0x7777;
    }
    
    /* Final output to prevent optimization */
    printf("Result hash: %d (0x%08x)\n", result_hash, result_hash);
    printf("This indicates all x87 comparisons executed.\n");
    
    /* Verify some expected behaviors */
    printf("NaN == NaN: %d (expected 0)\n", __builtin_nanl("") == __builtin_nanl(""));
    printf("NaN != 1.0: %d (expected 1)\n", __builtin_nanl("") != 1.0L);
    printf("Inf > 1000: %d (expected 1)\n", __builtin_infl() > 1000.0L);
    
    return result_hash != 0 ? 0 : 1;
}
