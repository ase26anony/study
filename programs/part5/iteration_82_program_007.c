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
    if (a < b) result |= 1;
    if (a > c) result |= 2;
    if (a <= d) result |= 4;
    if (b >= c) result |= 8;
    if (a == b) result |= 16;
    if (c != d) result |= 32;
    
    /* Unordered comparisons with potential NaN */
    long double nan_val = __builtin_nanl("");
    if (!(a < nan_val)) result |= 64;    /* May generate UNORDERED or ORDERED */
    if (!(nan_val > b)) result |= 128;   /* May generate UNORDERED */
    
    /* Mixed comparisons */
    if ((a != nan_val) && (c <= d)) result |= 256;
    if ((b == b) || (nan_val != nan_val)) result |= 512;  /* NaN != NaN is true */
    
    return result;
}

/* Function focusing on unordered comparisons */
int unordered_comparisons(long double x, long double y) {
    int results = 0;
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;  /* Another way to get NaN */
    volatile long double inf = __builtin_infl();
    
    /* Explicit unordered comparisons */
    int unordered = (x != x) || (y != y);  /* Check if either is NaN */
    if (unordered) results |= 1;
    
    /* Comparisons that should trigger UNEQ, UNGE, UNGT, UNLE, UNLT */
    if (!(nan1 < y)) results |= 2;   /* May generate UNORDERED or UNGE */
    if (!(nan1 > x)) results |= 4;   /* May generate UNORDERED or UNLE */
    if (!(x < nan2)) results |= 8;   /* May generate UNORDERED or UNGE */
    if (!(y > nan1)) results |= 16;  /* May generate UNORDERED or UNLE */
    
    /* LTGT (unordered and not equal) */
    if (x != y) results |= 32;       /* May generate LTGT for NaN comparisons */
    
    /* Compare NaN with infinity */
    if (!(nan1 < inf)) results |= 64;
    if (!(nan1 > -inf)) results |= 128;
    
    return results;
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
            count += 10;
        } else if (accum == 0.0L || accum != accum) {
            break;
        }
    }
    
    /* Switch-like behavior using if-else chain */
    volatile long double test_val = get_ld(global_index++);
    if (test_val < -10.0L) {
        count += 100;
    } else if (test_val > 10.0L) {
        count += 200;
    } else if (test_val == 0.0L) {
        count += 300;
    } else if (test_val != test_val) {  /* NaN check */
        count += 400;
    }
    
    return count;
}

/* Mixed precision comparisons */
int mixed_precision_comparisons(float f, double d, long double ld) {
    int results = 0;
    
    /* Compare different precisions (will promote to long double) */
    if ((long double)f < ld) results |= 1;
    if (d > (long double)f) results |= 2;
    if ((long double)42 == ld) results |= 4;
    if (ld != (long double)d) results |= 8;
    
    /* Complex expression with casts */
    if (((long double)f * 2.0L) <= (ld + (long double)d)) results |= 16;
    
    return results;
}

/* Generate NaN through various methods */
void generate_nan_scenarios() {
    volatile long double nan_array[6];
    
    nan_array[0] = __builtin_nanl("");      /* Quiet NaN */
    nan_array[1] = 0.0L / 0.0L;             /* NaN from 0/0 */
    nan_array[2] = __builtin_infl() / __builtin_infl();  /* inf/inf = NaN */
    nan_array[3] = __builtin_infl() - __builtin_infl();  /* inf-inf = NaN */
    nan_array[4] = sqrtl(-1.0L);            /* sqrt(-1) = NaN */
    nan_array[5] = logl(-1.0L);             /* log(-1) = NaN */
    
    /* Store in global array for later use */
    for (int i = 0; i < 6; i++) {
        global_ld[i] = nan_array[i];
    }
}

int main() {
    /* Initialize test values */
    global_ld[0] = 3.14159265358979323846L;  /* pi */
    global_ld[1] = 2.71828182845904523536L;  /* e */
    global_ld[2] = -42.5L;
    global_ld[3] = 0.0L;
    global_ld[4] = -0.0L;
    global_ld[5] = __builtin_infl();        /* +inf */
    global_ld[6] = -__builtin_infl();       /* -inf */
    
    generate_nan_scenarios();  /* Fills global_ld[7] through [12] with NaNs */
    
    /* Fill remaining slots */
    global_ld[13] = 1.0e100L;
    global_ld[14] = -1.0e-100L;
    global_ld[15] = 1.0L / 3.0L;  /* Repeating fraction */
    
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
    
    /* Test 2: Unordered comparisons */
    for (int i = 0; i < 10; i++) {
        long double x = get_ld(i);
        long double y = get_ld(15 - i);
        
        int res = unordered_comparisons(x, y);
        result_hash ^= (res << (i % 16));
    }
    
    /* Test 3: Control flow */
    for (int i = 0; i < 5; i++) {
        int res = control_flow_test(get_ld(i * 3));
        result_hash ^= res;
    }
    
    /* Test 4: Mixed precision */
    float f_vals[] = {3.14f, -2.5f, 0.0f, 1.0e10f};
    double d_vals[] = {2.718281828459045, -42.0, 1.0e100, -1.0e-100};
    
    for (int i = 0; i < 4; i++) {
        int res = mixed_precision_comparisons(f_vals[i], d_vals[i], get_ld(i * 4));
        result_hash ^= res;
    }
    
    /* Test 5: Direct NaN comparisons */
    volatile long double nan = __builtin_nanl("");
    volatile long double inf = __builtin_infl();
    volatile long double normal = 100.5L;
    
    /* These comparisons should trigger the uncovered mnemonics */
    int nan_test_results = 0;
    
    /* Force generation of specific condition codes */
    if (!(nan < normal)) nan_test_results |= 1;   /* UNORDERED or UNGE */
    if (!(nan > normal)) nan_test_results |= 2;   /* UNORDERED or UNLE */
    if (!(normal < nan)) nan_test_results |= 4;   /* UNORDERED or UNGE */
    if (!(normal > nan)) nan_test_results |= 8;   /* UNORDERED or UNLE */
    if (nan == nan) nan_test_results |= 16;       /* UNORDERED or UNEQ */
    if (nan != nan) nan_test_results |= 32;       /* Always true for NaN */
    if (!(nan <= inf)) nan_test_results |= 64;    /* UNORDERED */
    if (!(nan >= -inf)) nan_test_results |= 128;  /* UNORDERED */
    
    result_hash ^= nan_test_results;
    
    /* Final verification output */
    printf("Result hash: %d\n", result_hash);
    printf("Test completed. Check generated assembly for x87 comparison mnemonics.\n");
    
    return 0;
}
