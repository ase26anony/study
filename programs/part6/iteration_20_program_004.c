/* test_fixed_range.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
volatile int vi1 = 1;
volatile int vi2 = -1;
volatile int vi3 = 100;
volatile int vi4 = -100;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
}

/* Hash function to return non-deterministic result */
__attribute__((noinline))
int hash_result(const void *data, int size) {
    const unsigned char *p = (const unsigned char *)data;
    int h = 0;
    for (int i = 0; i < size; i++) {
        h = (h * 31) + p[i];
    }
    return h;
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[16];
    int result_idx = 0;
    
    /* Initialize with volatile to prevent constant propagation */
    volatile _Accum va = (_Accum)vi1 * 0.0009765625k;  /* ~1/1024 */
    volatile _Accum vb = (_Accum)vi2 * 0.0009765625k;
    volatile long _Accum vla = 0.999999999k;
    volatile long _Accum vlb = -0.999999999k;
    volatile unsigned _Fract vuf = 0.9999999ur;
    volatile _Fract vf = 0.9999999r;
    volatile _Fract vfn = -0.9999999r;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 8; i++) {
        /* Vary inputs based on iteration */
        _Accum a = va * (_Accum)i;
        _Accum b = vb * (_Accum)(8 - i);
        long _Accum la = vla + (long _Accum)(i * 0.0001k);
        long _Accum lb = vlb - (long _Accum)(i * 0.0001k);
        unsigned _Fract uf = vuf - (unsigned _Fract)(i * 0.0000001ur);
        _Fract f = vf - (_Fract)(i * 0.0000001r);
        _Fract fn = vfn + (_Fract)(i * 0.0000001r);
        
        /* Test 1: Multiplication near max _Accum range */
        /* This should trigger max range check for signed accum */
        _Accum prod1 = a * b;
        if (i % 2 == 0) {
            /* Additional scaling to push beyond limits */
            prod1 = prod1 * (_Accum)1.5k;
        }
        results[result_idx++] = prod1;
        
        /* Test 2: long _Accum multiplication at extreme values */
        /* This is likely to trigger the specific uncovered lines */
        long _Accum prod2 = la * lb;
        /* Shift-like operation through multiplication */
        prod2 = prod2 * (long _Accum)1.999k;
        results[result_idx++] = (_Accum)prod2;  /* Truncate to _Accum */
        
        /* Test 3: Operations that require checking against min/max */
        /* Mix with integer promotions */
        int scale = vi3 + i;
        _Accum scaled = (_Accum)scale * 0.5k;
        
        /* Conditional expression with fixed-point */
        _Accum cond_result = (scaled > 0.0k) ? 
            (scaled * scaled) : (scaled * -scaled);
        
        /* Left-shift simulation via multiplication */
        for (int j = 0; j < 3; j++) {
            cond_result = cond_result * 2.0k;
        }
        results[result_idx++] = cond_result;
        
        /* Test 4: unsigned _Fract near overflow */
        unsigned _Fract uf_result = uf;
        for (int j = 0; j < 2; j++) {
            uf_result = uf_result + (unsigned _Fract)0.0000001ur;
        }
        results[result_idx++] = (_Accum)uf_result;
        
        /* Test 5: signed _Fract boundary checks */
        _Fract f_result = (i % 3 == 0) ? f : fn;
        f_result = f_result * (_Fract)1.1r;
        results[result_idx++] = (_Accum)f_result;
        
        /* Test 6: Complex expression with multiple steps */
        /* This creates intermediate ranges needing analysis */
        _Accum temp1 = a + b;
        _Accum temp2 = temp1 * (_Accum)0.25k;
        _Accum temp3 = temp2 * temp2;
        _Accum temp4 = temp3 * (_Accum)(vi4 + i);
        
        /* Simulate left shift with saturation check */
        if (temp4 > 0.9k || temp4 < -0.9k) {
            temp4 = (temp4 > 0) ? 0.999999k : -0.999999k;
        } else {
            temp4 = temp4 * 4.0k;  /* Effectively << 2 */
        }
        results[result_idx++] = temp4;
        
        /* Ensure we don't overflow results array */
        if (result_idx >= 14) break;
    }
    
    /* Test 7: Direct boundary value tests */
    /* These should directly exercise the range comparison logic */
    _Accum boundary_test = 0.0k;
    
    /* Approach max from below */
    for (int i = 0; i < 10; i++) {
        boundary_test = boundary_test + 0.1k;
        _Accum test_prod = boundary_test * boundary_test;
        results[result_idx++] = test_prod;
    }
    
    /* Approach min from above */
    boundary_test = 0.0k;
    for (int i = 0; i < 10; i++) {
        boundary_test = boundary_test - 0.1k;
        _Accum test_prod = boundary_test * boundary_test;
        results[result_idx++] = test_prod;
    }
    
    /* Test 8: Fixed-point with integer casts */
    int int_val = vi1 * 1000;
    _Accum from_int = (_Accum)int_val * 0.001k;
    results[result_idx++] = from_int * from_int;
    
    /* Prevent dead code elimination */
    consume(results, sizeof(results));
    
    /* Return hash to make result observable */
    return hash_result(results, sizeof(results)) & 0xFF;
}
