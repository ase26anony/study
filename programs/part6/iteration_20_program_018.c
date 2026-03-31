/* Fixed-point range analysis test targeting GCC's fixed-value.cc */
/* Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test.c -o test.o */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int seed1 = 0x7FFFFFFF;
static volatile int seed2 = 0x80000000;
static volatile unsigned int useed1 = 0xFFFFFFFF;
static volatile float fseed = 0.999999f;

/* Dummy function to prevent optimization */
__attribute__((noinline, noipa))
void consume_result(const void *ptr, int size) {
    asm volatile("" : : "r"(ptr), "r"(size) : "memory");
}

/* Test various fixed-point types near their boundaries */
int main(void) {
    /* Array to accumulate results */
    long _Accum results[16];
    int result_idx = 0;
    
    /* Test 1: Signed _Accum near maximum (0.999999999k ~= 1-2^-31) */
    {
        _Accum a = 0.999999999k;  /* Very close to max */
        _Accum b = 0.999999999k;
        _Accum c = a * b;  /* Product should mathematically overflow */
        
        /* Force range analysis with volatile intermediate */
        volatile _Accum temp = a;
        _Accum d = temp * 0.999999999k;
        
        results[result_idx++] = (_Accum)c;
        results[result_idx++] = (_Accum)d;
    }
    
    /* Test 2: Long _Accum with left shift simulation */
    {
        long _Accum la = 0.999999999999999k;  /* ~1-2^-63 */
        long _Accum lb = 0.999999999999999k;
        
        /* Create expression that might overflow in range analysis */
        long _Accum lc = la * lb;
        long _Accum ld = lc * 0.999999999999999k;
        
        /* Mix with integer promotion */
        int shift = 1;
        long _Accum le = ld * (long _Accum)(1 << shift);
        
        results[result_idx++] = le;
    }
    
    /* Test 3: Unsigned _Fract near 1.0 */
    {
        unsigned _Fract uf1 = 0.9999999ur;  /* Very close to 1.0 */
        unsigned _Fract uf2 = 0.0000001ur;
        
        /* Operation that could wrap to 0 */
        unsigned _Fract uf3 = uf1 + uf2;
        
        /* Multiplication near boundary */
        unsigned _Fract uf4 = uf1 * 0.9999999ur;
        
        results[result_idx++] = (long _Accum)uf3;
        results[result_idx++] = (long _Accum)uf4;
    }
    
    /* Test 4: Signed _Fract near -1.0 and 1.0 */
    {
        _Fract sf1 = 0.9999999r;   /* Near +1 */
        _Fract sf2 = -0.9999999r;  /* Near -1 */
        
        /* Complex expression with conditional */
        _Fract sf3 = (seed1 > 0) ? sf1 : sf2;
        _Fract sf4 = sf3 * 0.9999999r;
        
        /* Simulate left shift through multiplication */
        _Fract sf5 = sf4 * (_Fract)(1 << 1);
        
        results[result_idx++] = (long _Accum)sf5;
    }
    
    /* Test 5: Short fixed-point types */
    {
        short _Fract sf = 0.9999sr;
        short _Accum sa = 0.9999sk;
        
        /* Operations that might overflow short types */
        short _Fract sf_mul = sf * 0.9999sr;
        short _Accum sa_mul = sa * 0.9999sk;
        
        results[result_idx++] = (long _Accum)sf_mul;
        results[result_idx++] = (long _Accum)sa_mul;
    }
    
    /* Test 6: Loop with varying fixed-point values */
    {
        /* Use volatile seeds to prevent constant propagation */
        int base = seed1;
        unsigned int ubase = useed1;
        
        long _Accum accum = 0.5k;
        
        for (int i = 0; i < 4; i++) {
            /* Create data-dependent fixed-point values */
            _Accum a = (_Accum)(base + i) / 1000000000k;
            _Accum b = (_Accum)(ubase - i) / 1000000000k;
            
            /* Operations that need range analysis */
            _Accum prod = a * b;
            _Accum shifted = prod * (_Accum)(1 << (i & 1));
            
            accum = accum * 0.999999999k + shifted;
            
            /* Conditional that depends on overflow-like check */
            if (accum > 0.999999998k) {
                accum = accum * 0.5k;
            }
        }
        
        results[result_idx++] = accum;
    }
    
    /* Test 7: Explicit overflow checking pattern */
    {
        _Accum max_val = 0.999999999k;
        _Accum test_val = max_val * 0.999999999k;
        
        /* Pattern similar to overflow check */
        int overflow = 0;
        if (test_val > max_val || (test_val == max_val && 0)) {
            overflow = 1;
            test_val = max_val;
        }
        
        results[result_idx++] = test_val;
        results[result_idx] = (long _Accum)overflow;
        result_idx++;
    }
    
    /* Test 8: Mixed integer and fixed-point */
    {
        int int_val = seed2;  /* Negative value */
        unsigned int uint_val = useed1;
        
        /* Conversions that need range analysis */
        _Accum from_signed = (_Accum)int_val * 0.000000001k;
        _Accum from_unsigned = (_Accum)uint_val * 0.000000001k;
        
        /* Multiplication near boundaries */
        _Accum mixed = from_signed * from_unsigned;
        
        results[result_idx++] = mixed;
    }
    
    /* Ensure we use all results */
    consume_result(results, sizeof(results[0]) * result_idx);
    
    /* Create a simple hash of results for return value */
    int hash = 0;
    for (int i = 0; i < result_idx; i++) {
        /* Access as bytes to mix bits */
        unsigned char *bytes = (unsigned char*)&results[i];
        for (size_t j = 0; j < sizeof(results[0]); j++) {
            hash = (hash * 31) + bytes[j];
        }
    }
    
    return hash & 0xFF;
}
