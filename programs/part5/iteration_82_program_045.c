/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in GCC's i386 backend
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math
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
    volatile long double v1 = a;
    volatile long double v2 = b;
    volatile long double v3 = c;
    volatile long double v4 = d;
    
    /* Mix of ordered and unordered comparisons */
    int result = 0;
    
    /* UNORDERED case: (a != a) || (b != b) */
    if (!(v1 == v1) || !(v2 == v2)) {
        result |= 1;
    }
    
    /* ORDERED case: (c == c) && (d == d) */
    if ((v3 == v3) && (v4 == v4)) {
        result |= 2;
    }
    
    /* UNEQ case: (a == b) || (a != a) || (b != b) */
    if ((v1 == v2) || !(v1 == v1) || !(v2 == v2)) {
        result |= 4;
    }
    
    /* UNGE case: !(a < b) */
    if (!(v1 < v2)) {
        result |= 8;
    }
    
    /* UNGT case: !(a <= b) */
    if (!(v1 <= v2)) {
        result |= 16;
    }
    
    /* UNLE case: (a <= b) || (a != a) || (b != b) */
    if ((v1 <= v2) || !(v1 == v1) || !(v2 == v2)) {
        result |= 32;
    }
    
    /* UNLT case: (a < b) || (a != a) || (b != b) */
    if ((v1 < v2) || !(v1 == v1) || !(v2 == v2)) {
        result |= 64;
    }
    
    /* LTGT case: (a != b) && (a == a) && (b == b) */
    if ((v1 != v2) && (v1 == v1) && (v2 == v2)) {
        result |= 128;
    }
    
    return result;
}

/* Test function with explicit NaN comparisons */
int test_nan_comparisons(void) {
    int results = 0;
    
    /* Generate various NaN values */
    long double nan1 = __builtin_nanl("");
    long double nan2 = 0.0L / 0.0L;
    long double nan3 = sqrtl(-1.0L);
    long double inf = __builtin_infl();
    long double normal = 3.14159265358979323846L;
    
    volatile long double v_nan1 = nan1;
    volatile long double v_nan2 = nan2;
    volatile long double v_inf = inf;
    volatile long double v_normal = normal;
    
    /* Test all comparison operators with NaN */
    results |= (v_nan1 == v_normal) ? 0x01 : 0;
    results |= (v_nan1 != v_normal) ? 0x02 : 0;
    results |= (v_nan1 < v_normal) ? 0x04 : 0;
    results |= (v_nan1 > v_normal) ? 0x08 : 0;
    results |= (v_nan1 <= v_normal) ? 0x10 : 0;
    results |= (v_nan1 >= v_normal) ? 0x20 : 0;
    
    /* NaN vs NaN comparisons */
    results |= (v_nan1 == v_nan2) ? 0x40 : 0;
    results |= (v_nan1 != v_nan2) ? 0x80 : 0;
    results |= (v_nan1 < v_nan2) ? 0x100 : 0;
    results |= (v_nan1 > v_nan2) ? 0x200 : 0;
    
    /* NaN vs Inf comparisons */
    results |= (v_nan1 == v_inf) ? 0x400 : 0;
    results |= (v_nan1 != v_inf) ? 0x800 : 0;
    results |= (v_nan1 < v_inf) ? 0x1000 : 0;
    results |= (v_nan1 > v_inf) ? 0x2000 : 0;
    
    return results;
}

/* Test function with mixed precision comparisons */
int test_mixed_precision(void) {
    int results = 0;
    
    /* Mix of types to force promotions */
    float f1 = 1.5f;
    double d1 = 2.718281828459045;
    long double ld1 = 3.14159265358979323846L;
    
    volatile float vf = f1;
    volatile double vd = d1;
    volatile long double vld = ld1;
    
    /* Comparisons with explicit casts */
    results |= ((long double)vf < vld) ? 1 : 0;
    results |= (vld > (long double)vd) ? 2 : 0;
    results |= ((long double)vf == vld) ? 4 : 0;
    results |= (vld != (long double)vd) ? 8 : 0;
    
    /* Integer constant comparisons */
    results |= (vld < 5.0L) ? 16 : 0;
    results |= (vld > 2.0L) ? 32 : 0;
    results |= ((long double)10 == vld) ? 64 : 0;
    results |= ((long double)3 != vld) ? 128 : 0;
    
    return results;
}

/* Loop with long double termination condition */
int test_loop_comparisons(void) {
    volatile long double counter = 0.0L;
    volatile long double limit = 10.0L;
    int iterations = 0;
    
    /* Loop condition uses x87 comparison */
    while (counter < limit && counter == counter) {  /* Check for NaN */
        counter += 1.0L;
        iterations++;
        
        /* Nested comparison inside loop */
        if (counter > 5.0L && counter <= 9.0L) {
            iterations += 10;
        }
    }
    
    /* Another loop with complex condition */
    volatile long double x = 100.0L;
    volatile long double y = 1.0L;
    while (x > y && !(x != x)) {  /* x != x checks for NaN */
        x /= 2.0L;
        if (x <= 1.0L || x != x) {
            break;
        }
    }
    
    return iterations + (int)x;
}

/* Switch statement based on comparison results */
int test_switch_comparisons(long double a, long double b) {
    volatile long double va = a;
    volatile long double vb = b;
    int result = 0;
    
    /* Chain of if-else based on comparisons */
    if (va < vb && va == va && vb == vb) {
        result = 1;
    } else if (va > vb && va == va && vb == vb) {
        result = 2;
    } else if (va == vb) {
        result = 3;
    } else if (va != vb) {
        result = 4;
    } else if (!(va == va) || !(vb == vb)) {
        result = 5;  /* NaN case */
    }
    
    /* Nested comparisons */
    if ((va <= vb || !(va == va)) && (vb >= 0.0L || !(vb == vb))) {
        result += 10;
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    /* Initialize global array with mixed values */
    global_ld[0] = 1.0L;
    global_ld[1] = 2.0L;
    global_ld[2] = __builtin_nanl("");
    global_ld[3] = 0.0L / 0.0L;
    global_ld[4] = __builtin_infl();
    global_ld[5] = -__builtin_infl();
    global_ld[6] = 3.14159265358979323846L;
    global_ld[7] = sqrtl(-1.0L);
    global_ld[8] = 100.0L;
    global_ld[9] = -100.0L;
    global_ld[10] = 1.0e-10L;
    global_ld[11] = 1.0e10L;
    global_ld[12] = 0.0L;
    global_ld[13] = -0.0L;
    global_ld[14] = 42.0L;
    global_ld[15] = __builtin_nanl("0xdeadbeef");
    
    /* Array to store all boolean results */
    int bool_results[64];
    int result_index = 0;
    
    /* Test 1: Complex x87 comparisons */
    for (int i = 0; i < 8; i++) {
        long double a = get_ld(i);
        long double b = get_ld(i + 1);
        long double c = get_ld(i + 2);
        long double d = get_ld(i + 3);
        
        int res = complex_x87_comparison(a, b, c, d);
        for (int bit = 0; bit < 8; bit++) {
            bool_results[result_index++] = (res >> bit) & 1;
        }
    }
    
    /* Test 2: NaN comparisons */
    int nan_results = test_nan_comparisons();
    for (int bit = 0; bit < 16; bit++) {
        bool_results[result_index++] = (nan_results >> bit) & 1;
    }
    
    /* Test 3: Mixed precision */
    int mixed_results = test_mixed_precision();
    for (int bit = 0; bit < 8; bit++) {
        bool_results[result_index++] = (mixed_results >> bit) & 1;
    }
    
    /* Test 4: Loop comparisons */
    int loop_result = test_loop_comparisons();
    bool_results[result_index++] = (loop_result > 0);
    bool_results[result_index++] = (loop_result < 100);
    bool_results[result_index++] = (loop_result != 0);
    
    /* Test 5: Switch statement comparisons */
    for (int i = 0; i < 4; i++) {
        int switch_res = test_switch_comparisons(get_ld(i * 2), get_ld(i * 2 + 1));
        bool_results[result_index++] = (switch_res & 1);
        bool_results[result_index++] = (switch_res & 2);
        bool_results[result_index++] = (switch_res & 4);
    }
    
    /* Additional direct comparisons to cover all mnemonics */
    volatile long double v1 = get_ld(0);
    volatile long double v2 = get_ld(1);
    volatile long double v3 = get_ld(2);  /* NaN */
    volatile long double v4 = get_ld(4);  /* Inf */
    
    /* Generate specific comparison patterns */
    bool_results[result_index++] = !(v1 < v2) && (v1 == v1) && (v2 == v2);  /* UNGE */
    bool_results[result_index++] = !(v1 <= v2) && (v1 == v1) && (v2 == v2); /* UNGT */
    bool_results[result_index++] = (v1 <= v2) || !(v1 == v1) || !(v2 == v2); /* UNLE */
    bool_results[result_index++] = (v1 < v2) || !(v1 == v1) || !(v2 == v2);  /* UNLT */
    bool_results[result_index++] = (v1 != v2) && (v1 == v1) && (v2 == v2);   /* LTGT */
    
    /* Compute verification hash (XOR of all boolean results) */
    int verification_hash = 0;
    for (int i = 0; i < result_index; i++) {
        verification_hash ^= bool_results[i];
    }
    
    /* Print hash to prevent dead code elimination */
    printf("Verification hash: %d\n", verification_hash);
    printf("Total comparisons performed: %d\n", result_index);
    
    return verification_hash != 0 ? 0 : 1;
}
