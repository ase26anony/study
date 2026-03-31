/* test_fixed_point_ranges.c
 * Designed to trigger GCC's fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
volatile int vi1 = 1000;
volatile int vi2 = -1000;
volatile unsigned int vu1 = 2000;
volatile _Fract vf = 0.5r;
volatile _Accum va = 0.5k;
volatile unsigned _Fract vuf = 0.5ur;

/* Noinline function to prevent dead code elimination */
__attribute__((noinline)) void consume_result(const void *ptr, int size) {
    volatile int sink = 0;
    const char *p = (const char *)ptr;
    for (int i = 0; i < size; i++) {
        sink += p[i];
    }
    (void)sink;
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[8] = {0.0k};
    int result_index = 0;
    
    /* Test various fixed-point types with edge-case values */
    
    /* 1. Signed accumulative types near maximum range */
    {
        /* Values very close to maximum representable */
        long _Accum a = 0.999999999k;  /* Near max for long _Accum */
        long _Accum b = 0.999999999k;
        
        /* Multiplication that would overflow the fixed-point range */
        long _Accum prod = a * b;
        results[result_index++] = (_Accum)prod;
        
        /* Left shift that could overflow */
        _Accum shifted = (_Accum)a << 1;
        results[result_index++] = shifted;
    }
    
    /* 2. Unsigned fractional types near 1.0 */
    {
        unsigned _Fract u1 = 0.9999999ur;  /* Very close to 1.0 */
        unsigned _Fract u2 = 0.0000001ur;
        
        /* Addition that could wrap */
        unsigned _Fract sum = u1 + u2;
        results[result_index++] = (_Accum)sum;
        
        /* Multiplication near overflow */
        unsigned _Fract prod = u1 * u1;
        results[result_index++] = (_Accum)prod;
    }
    
    /* 3. Signed fractional types near -1.0 and 1.0 */
    {
        _Fract s1 = 0.9999999r;   /* Near +1.0 */
        _Fract s2 = -0.9999999r;  /* Near -1.0 */
        
        /* Complex expression mixing near-boundary values */
        _Fract complex_expr = (s1 * s2) + (s1 - s2);
        results[result_index++] = (_Accum)complex_expr;
        
        /* Conditional expression with boundary values */
        _Fract cond_expr = (vi1 > 0) ? s1 : s2;
        results[result_index++] = (_Accum)cond_expr;
    }
    
    /* 4. Mix with integer promotions and volatile inputs */
    {
        int i = vi1;  /* Volatile prevents constant folding */
        unsigned int u = vu1;
        
        /* Casts between integer and fixed-point */
        _Accum from_int = (_Accum)i * 0.5k;
        unsigned _Fract from_uint = (unsigned _Fract)u * 0.0001ur;
        
        results[result_index++] = from_int + (_Accum)from_uint;
    }
    
    /* 5. Loop with varying fixed-point values */
    {
        /* Use volatile to force range analysis */
        _Accum base = (_Accum)vf * 2.0k;
        unsigned _Fract ubase = vuf;
        
        for (int i = 0; i < 4; i++) {
            /* Vary the values based on iteration */
            _Accum multiplier = (_Accum)i * 0.25k + 0.5k;
            unsigned _Fract ufactor = (unsigned _Fract)i * 0.25ur + 0.25ur;
            
            /* Operations that approach boundaries */
            _Accum temp = base * multiplier;
            unsigned _Fract utemp = ubase * ufactor;
            
            /* Left shift that could overflow */
            if (i % 2 == 0) {
                temp = temp << 1;
            }
            
            /* Accumulate results */
            results[result_index % 8] += temp + (_Accum)utemp;
            result_index = (result_index + 1) % 8;
        }
    }
    
    /* 6. Explicit overflow checking pattern */
    {
        /* Simulate overflow check logic */
        long _Accum max_val = 0.999999999k;
        long _Accum test_val = 0.999999999k;
        
        /* Operation that would overflow */
        long _Accum dangerous = test_val * 1.1k;
        
        /* Check if we're at or near maximum */
        int near_max = (test_val > 0.9k) && (test_val <= max_val);
        if (near_max) {
            /* Force compiler to consider overflow path */
            results[0] += (_Accum)dangerous * 0.001k;
        }
    }
    
    /* 7. Nested expressions with multiple fixed-point types */
    {
        short _Fract sf = 0.9999hr;
        _Accum acc = 0.9999k;
        long _Accum lacc = 0.99999999lk;
        
        /* Chain of operations */
        _Accum step1 = (_Accum)sf * acc;
        long _Accum step2 = (long _Accum)step1 * lacc;
        _Accum final = (_Accum)step2 * 1.01k;  /* Intentional slight overflow */
        
        results[result_index % 8] += final;
    }
    
    /* 8. Array operations with fixed-point */
    {
        _Fract farray[4] = {0.9r, 0.99r, 0.999r, 0.9999r};
        _Accum aarray[4] = {0.0k};
        
        /* Compute products that approach overflow */
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                aarray[i] += (_Accum)farray[i] * (_Accum)farray[j];
            }
            results[i] += aarray[i];
        }
    }
    
    /* Prevent dead code elimination */
    consume_result(results, sizeof(results));
    
    /* Return a hash of results */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        /* Convert to integer for hashing */
        int val = (int)(results[i] * 1000.0k);
        hash = hash * 31 + val;
    }
    
    return hash & 0xFF;  /* Return non-zero to indicate execution */
}
