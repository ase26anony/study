/* test_fixed_point_ranges.c
 * Designed to trigger GCC's fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent dead code elimination */
static void __attribute__((noinline)) consume_result(const void *p, int size) {
    volatile char sink;
    const char *bytes = (const char *)p;
    for (int i = 0; i < size; i++) {
        sink = bytes[i];
    }
}

int main(void) {
    /* Volatile seeds to prevent constant folding */
    volatile int seed1 = 0x7FFFFFFF;
    volatile int seed2 = 0x80000000;
    volatile unsigned int useed = 0xFFFFFFFF;
    
    /* Array to accumulate results */
    long _Accum results[8] = {0};
    int result_idx = 0;
    
    /* Test 1: Signed _Accum near maximum range */
    {
        /* Values that should trigger max range checks */
        _Accum a = (_Accum)(seed1) / 32768.0r;  /* Near 0.999... */
        _Accum b = (_Accum)(seed1 - 1) / 32768.0r;
        
        /* Multiplication that could overflow fixed-point range */
        _Accum prod = a * b;
        
        /* Left shift to potentially exceed range */
        _Accum shifted = prod << 1;
        
        /* Complex expression with intermediate range checks */
        _Accum temp = (a + b) / 2.0r;
        _Accum final = temp * temp * 2.0r;
        
        results[result_idx++] = (_Accum)prod;
        results[result_idx++] = (_Accum)shifted;
        results[result_idx++] = (_Accum)final;
    }
    
    /* Test 2: Long _Accum with extreme values */
    {
        long _Accum la = 0.999999999k;  /* Very close to 1.0 */
        long _Accum lb = 0.999999999k;
        
        /* This product mathematically exceeds the fixed-point range */
        long _Accum lprod = la * lb;
        
        /* Chain operations to force range analysis */
        long _Accum ltemp = lprod * 1.000000001k;
        ltemp = ltemp << 2;
        
        results[result_idx++] = (long _Accum)lprod;
        results[result_idx++] = (long _Accum)ltemp;
    }
    
    /* Test 3: Unsigned _Fract near 1.0 */
    {
        unsigned _Fract uf1 = 0.9999999ur;
        unsigned _Fract uf2 = 0.0000001ur;
        
        /* Operations that could wrap around */
        unsigned _Fract sum = uf1 + uf2;
        unsigned _Fract prod = uf1 * uf1;
        
        /* Convert to _Accum and back to trigger range conversions */
        _Accum conv = (_Accum)sum * 2.0r;
        
        results[result_idx++] = (_Accum)sum;
        results[result_idx++] = (_Accum)conv;
    }
    
    /* Test 4: Signed _Fract with negative values */
    {
        _Fract sf1 = -0.9999999r;
        _Fract sf2 = 0.9999999r;
        
        /* Mixed sign operations */
        _Fract diff = sf2 - sf1;  /* Could approach 2.0, exceeding range */
        _Fract mix_prod = sf1 * sf2;
        
        /* Shift operation on fractional type */
        _Fract shifted = mix_prod << 1;
        
        results[result_idx++] = (_Accum)diff;
        results[result_idx++] = (_Accum)shifted;
    }
    
    /* Test 5: Loop with varying fixed-point values */
    {
        volatile int counter = 4;
        _Accum accum = 0.5r;
        
        for (int i = 0; i < counter; i++) {
            /* Varying multiplier based on loop iteration */
            _Fract multiplier = (_Fract)(seed1 + i) / 32768.0r;
            
            /* Complex data-dependent expression */
            accum = accum * multiplier;
            
            /* Conditional that depends on overflow-like check */
            if (accum > 0.99r) {
                accum = accum * 0.5r;
            } else {
                accum = accum * 1.5r;
            }
            
            /* Left shift that could overflow */
            _Accum temp = accum << (i + 1);
            
            results[result_idx % 8] += (_Accum)temp;
            result_idx = (result_idx + 1) % 8;
        }
    }
    
    /* Test 6: Integer to fixed-point conversions with range limits */
    {
        int large_int = seed1;
        int neg_large_int = seed2;
        
        /* Conversions that should trigger range boundary checks */
        _Accum from_large = (_Accum)large_int;
        _Accum from_neg = (_Accum)neg_large_int;
        
        /* Operations on converted values */
        _Accum conv_prod = from_large * from_neg;
        conv_prod = conv_prod << 1;
        
        results[result_idx++] = (_Accum)from_large;
        results[result_idx++] = (_Accum)conv_prod;
    }
    
    /* Test 7: Short fixed-point types */
    {
        short _Fract sf = 0.9999hr;
        short _Accum sa = 0.9999hk;
        
        /* Operations that might overflow short types */
        short _Fract sf_sq = sf * sf;
        short _Accum sa_sq = sa * sa;
        
        /* Promotion to larger type and back */
        _Accum promoted = (_Accum)sf_sq * (_Accum)sa_sq;
        
        results[result_idx++] = (_Accum)promoted;
    }
    
    /* Test 8: Ternary operator with fixed-point ranges */
    {
        _Accum a = (_Accum)(seed1 & 0xFFF) / 4096.0r;
        _Accum b = (_Accum)((seed1 >> 12) & 0xFFF) / 4096.0r;
        
        /* Data-dependent ternary */
        _Accum choice = (seed1 & 1) ? a : b;
        
        /* Operation that could overflow based on choice */
        _Accum result = choice * 2.0r;
        result = result << 1;
        
        /* Nested ternary with arithmetic */
        _Accum final = (result > 1.0r) ? (result * 0.5r) : (result * 2.0r);
        
        results[result_idx++] = (_Accum)final;
    }
    
    /* Prevent optimization */
    consume_result(results, sizeof(results));
    
    /* Return hash of results to prevent complete optimization */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        hash ^= *((int*)&results[i]);
        hash ^= *((int*)&results[i] + 1);
    }
    
    return hash & 0xFF;
}
