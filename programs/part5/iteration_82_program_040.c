/* x87_comparison_test.c
 * Designed to trigger x87 comparison condition code output in i386.cc
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld_array[16];
volatile int array_index = 0;

/* Helper to get dynamic long double values */
long double get_ld_value(int idx) {
    return global_ld_array[idx % 16];
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
    if (nan_val != nan_val) result |= 512; /* Always true for NaN */
    
    /* Mixed comparisons */
    if ((a < b) && (c > d)) result |= 1024;
    if ((a != nan_val) || (b == c)) result |= 2048;
    
    return result;
}

/* Function focusing on unordered comparisons */
int unordered_comparisons(long double x, long double y) {
    int result = 0;
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;  /* Another way to get NaN */
    long double inf = __builtin_infl();
    
    /* Explicit unordered comparisons */
    volatile int cmp1 = !(x < y);    /* May generate UNORDERED or UNGE */
    volatile int cmp2 = !(y > x);    /* May generate UNORDERED or UNLE */
    
    /* Compare NaN with various values */
    if (!(nan1 < 1.0L)) result |= 1;      /* UNORDERED/UNGE */
    if (!(1.0L > nan1)) result |= 2;      /* UNORDERED/UNLE */
    if (!(nan1 <= inf)) result |= 4;      /* UNORDERED/UNGT */
    if (!(inf >= nan1)) result |= 8;      /* UNORDERED/UNLT */
    if (nan1 == nan2) result |= 16;       /* Always false - UNEQ? */
    if (nan1 != nan1) result |= 32;       /* Always true - LTGT? */
    
    /* Complex condition that might use UNORDERED */
    if ((x != x) || (y != y)) result |= 64;  /* Check for NaN */
    
    return result;
}

/* Function with control flow based on long double comparisons */
int control_flow_test(long double a, long double b, long double c) {
    int counter = 0;
    
    /* if-else chain with long double comparisons */
    if (a < b) {
        counter += 1;
    } else if (a > b) {
        counter += 2;
    } else if (a == b) {
        counter += 3;
    } else {
        /* This branch is taken when a or b is NaN */
        counter += 4;
    }
    
    /* while loop with long double condition */
    volatile long double x = a;
    while (x < c && counter < 100) {
        x *= 1.1L;
        counter++;
    }
    
    /* switch based on comparison results */
    int cmp_result = 0;
    if (a < b) cmp_result = 1;
    else if (a > b) cmp_result = 2;
    else if (a == b) cmp_result = 3;
    else cmp_result = 4;
    
    switch (cmp_result) {
        case 1: counter += 10; break;
        case 2: counter += 20; break;
        case 3: counter += 30; break;
        case 4: counter += 40; break;  /* NaN case */
    }
    
    return counter;
}

/* Mixed precision comparisons */
int mixed_precision_test(float f, double d, long double ld) {
    int result = 0;
    
    /* Compare different floating types (promotions to long double) */
    if (f < ld) result |= 1;
    if ((long double)d > ld) result |= 2;
    if (f == (long double)d) result |= 4;
    
    /* Integer constant comparisons */
    if (ld < 100.0L) result |= 8;
    if ((long double)42 > ld) result |= 16;
    
    /* Casting in comparisons */
    volatile int cmp = ((long double)f < (long double)d);
    if (cmp) result |= 32;
    
    return result;
}

/* NaN-generating operations */
void generate_nan_operations() {
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = sqrtl(-1.0L);
    volatile long double nan3 = 0.0L / 0.0L;
    volatile long double inf = __builtin_infl();
    
    /* Store in global array for later use */
    global_ld_array[0] = nan1;
    global_ld_array[1] = nan2;
    global_ld_array[2] = nan3;
    global_ld_array[3] = inf;
    global_ld_array[4] = -inf;
}

int main() {
    /* Initialize global array with various values */
    global_ld_array[0] = __builtin_nanl("");
    global_ld_array[1] = 0.0L / 0.0L;
    global_ld_array[2] = __builtin_infl();
    global_ld_array[3] = -__builtin_infl();
    global_ld_array[4] = 3.14159265358979323846L;
    global_ld_array[5] = 2.71828182845904523536L;
    global_ld_array[6] = 1.41421356237309504880L;
    global_ld_array[7] = 0.0L;
    global_ld_array[8] = -0.0L;
    global_ld_array[9] = 1.0L;
    global_ld_array[10] = -1.0L;
    global_ld_array[11] = 100.0L;
    global_ld_array[12] = 1e-10L;
    global_ld_array[13] = 1e10L;
    global_ld_array[14] = sqrtl(-1.0L);
    global_ld_array[15] = logl(-1.0L);
    
    /* Generate more NaN values */
    generate_nan_operations();
    
    /* Test results array */
    int results[20];
    int result_index = 0;
    
    /* Test 1: Complex x87 comparisons */
    results[result_index++] = complex_x87_comparison(
        get_ld_value(4),  /* pi */
        get_ld_value(5),  /* e */
        get_ld_value(6),  /* sqrt(2) */
        get_ld_value(7)   /* 0.0 */
    );
    
    /* Test 2: Unordered comparisons with NaN */
    results[result_index++] = unordered_comparisons(
        get_ld_value(0),  /* NaN */
        get_ld_value(2)   /* inf */
    );
    
    /* Test 3: Control flow based on comparisons */
    results[result_index++] = control_flow_test(
        get_ld_value(4),
        get_ld_value(5),
        get_ld_value(11)
    );
    
    /* Test 4: Mixed precision */
    results[result_index++] = mixed_precision_test(
        3.14f,
        2.718281828459045,
        get_ld_value(4)
    );
    
    /* Test 5: Direct NaN comparisons */
    volatile long double nan_val = get_ld_value(0);
    volatile long double normal_val = get_ld_value(4);
    
    /* Perform all standard comparisons with NaN */
    int nan_test_result = 0;
    nan_test_result |= (nan_val < normal_val) ? 1 : 0;
    nan_test_result |= (nan_val > normal_val) ? 2 : 0;
    nan_test_result |= (nan_val <= normal_val) ? 4 : 0;
    nan_test_result |= (nan_val >= normal_val) ? 8 : 0;
    nan_test_result |= (nan_val == normal_val) ? 16 : 0;
    nan_test_result |= (nan_val != normal_val) ? 32 : 0;
    nan_test_result |= (nan_val < nan_val) ? 64 : 0;
    nan_test_result |= (nan_val > nan_val) ? 128 : 0;
    
    results[result_index++] = nan_test_result;
    
    /* Test 6: More complex expressions */
    volatile long double a = get_ld_value(4);
    volatile long double b = get_ld_value(5);
    volatile long double c = get_ld_value(0);  /* NaN */
    
    int complex_result = 0;
    if ((a < b) && !(c == c)) complex_result |= 1;  /* c == c is false for NaN */
    if ((a != a) || (b > a)) complex_result |= 2;
    if (!(c < b) && !(b < c)) complex_result |= 4;  /* Both unordered */
    
    results[result_index++] = complex_result;
    
    /* Test 7: Loop with long double comparison */
    volatile long double x = get_ld_value(9);  /* 1.0 */
    int loop_count = 0;
    while (x < get_ld_value(11) && loop_count < 50) {  /* 100.0 */
        x *= 1.5L;
        loop_count++;
    }
    results[result_index++] = loop_count;
    
    /* Test 8: Nested comparisons */
    long double p = get_ld_value(4);
    long double q = get_ld_value(5);
    long double r = get_ld_value(6);
    long double s = get_ld_value(0);  /* NaN */
    
    int nested_result = 0;
    if ((p < q) == (r > p)) nested_result |= 1;
    if ((s != s) && (p == p)) nested_result |= 2;
    if (!(s < p) || !(p < s)) nested_result |= 4;
    
    results[result_index++] = nested_result;
    
    /* Compute final hash to prevent dead code elimination */
    uint32_t final_hash = 0;
    for (int i = 0; i < result_index; i++) {
        final_hash ^= (uint32_t)results[i];
        final_hash = (final_hash << 1) | (final_hash >> 31);  /* Rotate left */
    }
    
    /* Also mix in some volatile comparisons to ensure they're not optimized out */
    volatile int check = 0;
    for (int i = 0; i < 8; i++) {
        check += (get_ld_value(i) < get_ld_value(i+1)) ? 1 : 0;
        check += (get_ld_value(i) > get_ld_value(i+1)) ? 1 : 0;
    }
    
    final_hash ^= (uint32_t)check;
    
    printf("Test hash result: 0x%08X\n", final_hash);
    printf("(This value varies based on floating-point comparisons)\n");
    
    return (final_hash != 0) ? 0 : 1;
}
