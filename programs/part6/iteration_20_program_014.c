/* test_fixed_point_range.c
 * Designed to trigger fixed-point range analysis overflow checks
 * in GCC's fixed-value.cc lines 264-277
 */

/* Prevent constant folding with volatile inputs */
volatile int vi1 = 100;
volatile int vi2 = -100;
volatile unsigned int vu1 = 200;
volatile _Fract vf = 0.5r;
volatile _Accum va = 0.5k;
volatile unsigned _Fract vuf = 0.5ur;

/* Noinline function to prevent dead code elimination */
__attribute__((noinline)) 
void consume_result(const void *ptr, int size) {
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
    
    /* Test 1: Signed _Accum near maximum range */
    {
        /* Values that should approach max representable */
        long _Accum a = 0.999999999k;  /* Very close to 1.0 */
        long _Accum b = 0.999999999k;
        
        /* Multiplication that could overflow range */
        long _Accum product = a * b;
        
        /* Left shift to potentially exceed range */
        long _Accum shifted = product;
        for (int i = 0; i < 3; i++) {
            shifted = shifted * 2.0k;  /* Simulate left shift */
        }
        
        results[result_index++] = (_Accum)shifted;
    }
    
    /* Test 2: Unsigned _Fract near 1.0 */
    {
        unsigned _Fract u1 = 0.9999999ur;  /* Max representable */
        unsigned _Fract u2 = 0.0000001ur;
        
        /* Addition that could wrap */
        unsigned _Fract sum = u1 + u2;
        
        /* Multiplication that could overflow */
        unsigned _Fract prod = u1 * u1;
        
        results[result_index++] = (_Accum)sum;
        results[result_index++] = (_Accum)prod;
    }
    
    /* Test 3: Signed _Fract near boundaries */
    {
        _Fract near_max = 0.9999999r;
        _Fract near_min = -0.9999999r;
        
        /* Operations that could exceed range */
        _Fract diff = near_max - near_min;  /* Could approach 2.0 */
        _Fract square = near_max * near_max;
        
        results[result_index++] = (_Accum)diff;
        results[result_index++] = (_Accum)square;
    }
    
    /* Test 4: Complex expression with volatile inputs */
    {
        /* Use volatile to prevent constant folding */
        int i = vi1;
        unsigned int u = vu1;
        
        /* Mixed integer/fixed-point operations */
        _Accum c1 = (_Accum)i * 0.123456k;
        _Accum c2 = (_Accum)u * 0.987654k;
        
        /* Chain of operations */
        _Accum temp = c1 * c2;
        
        /* Simulate left shift through multiplication */
        for (int j = 0; j < 2; j++) {
            temp = temp * 1.5k;
        }
        
        /* Conditional that depends on range */
        _Accum final = (temp > 0.9k) ? temp * 1.1k : temp * 0.9k;
        
        results[result_index++] = final;
    }
    
    /* Test 5: Loop with varying fixed-point values */
    {
        /* Array of values near boundaries */
        _Accum values[] = {
            0.999999k, 0.888888k, 0.777777k, -0.999999k,
            0.5k, -0.5k, 0.123456k, -0.987654k
        };
        
        _Accum accumulator = 0.0k;
        
        for (int i = 0; i < 8; i++) {
            /* Use volatile to vary the index */
            int idx = (i + vi2) & 7;
            if (idx < 0) idx = -idx;
            
            /* Multiplication that could overflow */
            _Accum val = values[idx];
            accumulator = accumulator + val * val;
            
            /* Occasionally scale up */
            if (i % 3 == 0) {
                accumulator = accumulator * 1.2k;
            }
        }
        
        results[result_index++] = accumulator;
    }
    
    /* Test 6: Explicit overflow checking pattern */
    {
        /* This pattern may trigger the specific sgt/ugt comparisons */
        long _Accum max_val = 0.999999999k;
        long _Accum min_val = -1.0k;
        
        /* Operations designed to exceed representable range */
        long _Accum test1 = max_val * max_val;
        long _Accum test2 = min_val * min_val;
        long _Accum test3 = (max_val + 0.000000001k) * 0.5k;
        
        /* Chain operations to create complex range */
        long _Accum combined = test1 + test2 - test3;
        
        /* Force range analysis with conditional */
        if (combined > 0.0k) {
            combined = combined * 1.5k;
        } else {
            combined = combined * 0.5k;
        }
        
        results[result_index++] = (_Accum)combined;
    }
    
    /* Test 7: Mixed-width fixed-point operations */
    {
        short _Fract sf = 0.9999hr;
        _Fract f = 0.9999r;
        long _Accum la = 0.999999999k;
        
        /* Cross-type operations requiring conversions */
        _Accum mixed1 = (_Accum)sf * la;
        _Accum mixed2 = (_Accum)f * la;
        _Accum mixed3 = mixed1 + mixed2;
        
        /* Scale up to potentially overflow */
        for (int i = 0; i < 2; i++) {
            mixed3 = mixed3 * 1.1k;
        }
        
        results[result_index++] = mixed3;
    }
    
    /* Ensure all results are used */
    consume_result(results, sizeof(results));
    
    /* Create a simple hash to return */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        /* Convert to integer for hashing */
        int val = (int)(results[i] * 1000000);
        hash = hash * 31 + val;
    }
    
    return hash & 0xFF;
}
