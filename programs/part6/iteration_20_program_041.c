/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 10;
static volatile int vi2 = -5;
static volatile unsigned int vu1 = 100;
static volatile _Fract vf1 = 0.5r;
static volatile _Accum va1 = 0.5k;

/* Noinline function to prevent dead code elimination */
__attribute__((noinline)) 
void consume_results(const _Accum *arr, int n) {
    volatile _Accum sink = 0.0k;
    for (int i = 0; i < n; i++) {
        sink = arr[i];
    }
    (void)sink;
}

/* Helper to create complex data-dependent expressions */
__attribute__((noinline))
_Accum complex_accum_expr(_Accum a, _Accum b, int shift) {
    /* Multi-step expression that could overflow */
    _Accum temp = a * b;                    /* Multiplication near limits */
    
    /* Simulate left shift via multiplication */
    for (int i = 0; i < shift && i < 4; i++) {
        temp = temp * 2.0k;                 /* Could overflow */
    }
    
    /* Conditional that forces range analysis */
    return (temp > 0.9k) ? temp * 0.9k : temp * 1.1k;
}

__attribute__((noinline))
_Fract complex_fract_expr(_Fract a, _Fract b) {
    /* Operations near fractional limits */
    _Fract sum = a + b;                     /* Could saturate near 1.0 */
    _Fract prod = sum * 0.9999999r;         /* Near max fractional */
    
    /* Mix with integer promotion */
    int i_val = (int)(prod * 1000);
    return (_Fract)i_val / 1000.0r;
}

int main(void) {
    /* Array to store results and prevent optimization */
    _Accum results[20];
    _Fract f_results[20];
    
    /* Seed values at or near type boundaries */
    const long _Accum MAX_LACCUM = 0.999999999999999999k;
    const long _Accum MIN_LACCUM = -1.0k;
    const unsigned _Fract MAX_UFRACT = 0.9999999ur;
    const _Fract MAX_FRACT = 0.9999999r;
    const _Fract MIN_FRACT = -1.0r;
    
    /* Test 1: Signed accumulative types near maximum */
    for (int i = 0; i < 10; i++) {
        /* Vary the multiplier near 1.0 */
        _Accum multiplier = (_Accum)(0.9k + (i * 0.01k));
        
        /* These multiplications may overflow the fixed-point range */
        _Accum a = MAX_LACCUM * multiplier;
        _Accum b = MIN_LACCUM * multiplier;
        
        /* Complex expression with intermediate overflow checks */
        results[i] = complex_accum_expr(a, b, vi1 % 4);
        
        /* Force signed comparison in range analysis */
        if (a > b && a > 0.0k && b < 0.0k) {
            results[i] = results[i] * 0.5k;
        }
    }
    
    /* Test 2: Unsigned fractional types at boundary */
    unsigned _Fract uf_array[5];
    for (int i = 0; i < 5; i++) {
        /* Operations that could wrap around 1.0 */
        unsigned _Fract uf = MAX_UFRACT - (i * 0.0000001ur);
        uf = uf + (vu1 % 100) * 0.00000001ur;  /* Potential overflow */
        
        /* Cast to signed to trigger different range analysis */
        f_results[i] = (_Fract)uf * 0.5r;
        uf_array[i] = uf;
    }
    
    /* Test 3: Signed fractional types with negative values */
    for (int i = 0; i < 5; i++) {
        /* Alternate between positive and negative extremes */
        _Fract f = (i % 2 == 0) ? MAX_FRACT : MIN_FRACT;
        f = f * (_Fract)(0.5r + (i * 0.1r));
        
        /* Data-dependent shift simulation */
        int shift_amt = (vi2 + i) & 3;
        for (int s = 0; s < shift_amt; s++) {
            f = f * 2.0r;  /* Could overflow for values > 0.5 */
        }
        
        f_results[10 + i] = complex_fract_expr(f, vf1);
    }
    
    /* Test 4: Mixed-type expressions with integer promotion */
    for (int i = 0; i < 5; i++) {
        int int_val = vi1 + i * 1000;
        
        /* Conversion from integer to fixed-point with scaling */
        _Accum scaled = (_Accum)int_val / 1000.0k;
        
        /* Multiplication that could exceed fixed-point range */
        results[10 + i] = scaled * MAX_LACCUM;
        
        /* Conditional based on overflow check pattern */
        if (scaled > 0.5k && results[10 + i] < 0.0k) {
            /* This branch should be taken only if overflow occurred */
            results[10 + i] = 0.0k;
        }
    }
    
    /* Test 5: Nested expressions with volatile intermediates */
    volatile _Accum volatile_acc = va1;
    for (int i = 0; i < 5; i++) {
        /* The compiler can't fold this due to volatility */
        _Accum temp = volatile_acc * (_Accum)(i + 1);
        
        /* Chain operations to create complex range */
        temp = temp * temp;  /* Square - high overflow probability */
        temp = temp * 0.25k; /* Scale back */
        
        results[15 + i] = temp;
        
        /* Update volatile for next iteration */
        volatile_acc = volatile_acc * 0.9k;
    }
    
    /* Test 6: Explicit overflow checking pattern */
    /* This mimics the kind of comparison the uncovered code performs */
    for (int i = 0; i < 3; i++) {
        _Accum test_val = MAX_LACCUM * (0.95k + i * 0.025k);
        _Accum increment = 0.1k;
        
        /* Manual overflow check similar to the uncovered logic */
        _Accum potential_overflow = test_val + increment;
        
        /* Check if we would exceed maximum */
        if (potential_overflow > MAX_LACCUM || 
            (potential_overflow == MAX_LACCUM && increment > 0.0k)) {
            results[18 + i] = MAX_LACCUM;  /* Saturate */
        } else {
            results[18 + i] = potential_overflow;
        }
    }
    
    /* Ensure all results are used */
    consume_results(results, 20);
    
    /* Create a hash to return (prevents optimization of entire program) */
    int hash = 0;
    for (int i = 0; i < 20; i++) {
        hash ^= (int)(results[i] * 1000.0k);
    }
    for (int i = 0; i < 15; i++) {
        hash ^= (int)(f_results[i] * 1000.0r);
    }
    
    return hash & 0xFF;
}
