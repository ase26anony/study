/* test_fixed_point_range.c
 * Designed to trigger GCC's fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
volatile int vi1 = 1;
volatile int vi2 = -1;
volatile int vi3 = 100;
volatile int vi4 = -100;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *ptr, int size) {
    volatile char sink;
    char *p = (char*)ptr;
    for (int i = 0; i < size; i++) {
        sink = p[i];
    }
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[8] = {0.0k};
    int result_index = 0;
    
    /* Test 1: Signed _Accum near maximum range */
    {
        /* Values that should approach max representable _Accum */
        volatile _Accum a = 0.999999999k;  /* Very close to max */
        volatile _Accum b = 0.999999999k;
        
        /* Multiplication that would overflow if not saturated */
        _Accum prod = a * b;
        results[result_index++] = prod;
        
        /* Left shift to potentially overflow */
        _Accum shifted = prod << vi1;
        results[result_index++] = shifted;
    }
    
    /* Test 2: Long _Accum with complex expression */
    {
        volatile long _Accum la = 0.9999999999999999lk;
        volatile long _Accum lb = -0.9999999999999999lk;
        
        /* Chain of operations that could overflow in intermediate steps */
        long _Accum temp1 = la * la;
        long _Accum temp2 = lb * lb;
        long _Accum sum = temp1 + temp2;
        
        /* Convert to regular _Accum with potential overflow */
        _Accum converted = (_Accum)sum;
        results[result_index++] = (_Accum)temp1;
        results[result_index++] = (_Accum)temp2;
        results[result_index++] = converted;
    }
    
    /* Test 3: Unsigned _Fract near 1.0 */
    {
        volatile unsigned _Fract uf1 = 0.9999999ur;
        volatile unsigned _Fract uf2 = 0.0000001ur;
        
        /* Addition that could wrap */
        unsigned _Fract sum = uf1 + uf2;
        
        /* Multiplication near overflow */
        unsigned _Fract prod = uf1 * uf1;
        
        /* Convert to signed with potential range issues */
        _Accum signed_from_unsigned = (_Accum)sum + (_Accum)prod;
        results[result_index++] = signed_from_unsigned;
    }
    
    /* Test 4: Signed _Fract with negative values */
    {
        volatile _Fract sf1 = -0.9999999r;
        volatile _Fract sf2 = 0.9999999r;
        
        /* Operations that could hit min/max bounds */
        _Fract diff = sf2 - sf1;  /* Could approach 2.0, overflowing for _Fract */
        _Fract prod = sf1 * sf2;  /* Negative product near -1.0 */
        
        /* Conditional expression forcing range analysis */
        _Fract selected = (vi1 > 0) ? diff : prod;
        results[result_index++] = (_Accum)selected;
    }
    
    /* Test 5: Loop with varying fixed-point values */
    {
        volatile int counter = vi3;
        _Accum accumulator = 0.0k;
        
        for (int i = 0; i < 4; i++) {
            /* Varying multiplier based on loop counter */
            _Fract multiplier = (_Fract)(counter + i) / 1000.0r;
            
            /* Base value that changes each iteration */
            _Accum base = 0.5k + (_Accum)i * 0.1k;
            
            /* Complex expression that could overflow depending on i */
            _Accum val = base * (_Accum)multiplier;
            
            /* Left shift that could overflow */
            if (i & 1) {
                val = val << 1;
            }
            
            accumulator += val;
        }
        results[result_index++] = accumulator;
    }
    
    /* Test 6: Mixed integer and fixed-point with casts */
    {
        volatile int int_val = vi4;
        
        /* Convert integer to fixed-point with potential overflow */
        _Accum from_int = (_Accum)int_val * 0.123456789k;
        
        /* Chain of operations */
        _Accum step1 = from_int * 8.0k;  /* Could overflow */
        _Accum step2 = step1 >> 2;       /* Right shift */
        _Accum step3 = step2 << vi1;     /* Left shift with volatile */
        
        /* Conditional based on overflow-like check */
        _Accum final_val;
        if (step3 > 0.999999k) {
            final_val = 0.999999k;  /* Simulate saturation */
        } else if (step3 < -0.999999k) {
            final_val = -0.999999k;
        } else {
            final_val = step3;
        }
        results[result_index++] = final_val;
    }
    
    /* Test 7: Short fixed-point types */
    {
        volatile short _Fract sf = 0.9999hr;
        volatile short _Accum sa = 0.9999999hk;
        
        /* Operations that could overflow short types */
        short _Accum prod = sa * sa;
        short _Fract sum = sf + sf;
        
        /* Promote to larger type for storage */
        results[result_index++] = (_Accum)prod + (_Accum)sum;
    }
    
    /* Test 8: Bitwise operations after conversion */
    {
        /* Start with integer, convert to fixed, do arithmetic, check bounds */
        int start = vi1 * 1000;
        _Accum fixed = (_Accum)start / 1000.0k;
        
        /* Series of multiplications that could overflow */
        for (int i = 0; i < 3; i++) {
            fixed = fixed * 1.5k;
        }
        
        /* Final operation that should trigger range check */
        _Accum final = fixed << 1;
        results[result_index++] = final;
    }
    
    /* Ensure all results are used */
    consume(results, sizeof(results));
    
    /* Return hash of results to prevent optimization */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        /* Access as bytes to create hash */
        char *bytes = (char*)&results[i];
        for (size_t j = 0; j < sizeof(_Accum); j++) {
            hash ^= (bytes[j] << ((i * 7) % 32));
        }
    }
    
    return hash & 0xFF;
}
