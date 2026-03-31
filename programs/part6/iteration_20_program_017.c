/* Test program to trigger fixed-point range analysis overflow checks */
/* Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test.c -o test.o */

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
int compute_hash(_Accum *arr, int n) {
    int hash = 0;
    for (int i = 0; i < n; i++) {
        /* Access bits to create hash */
        union {
            _Accum f;
            int i;
        } u;
        u.f = arr[i];
        hash = hash * 31 + u.i;
    }
    return hash;
}

int main(void) {
    /* Array to store results and prevent optimization */
    _Accum results[32] = {0k};
    unsigned _Accum uresults[16] = {0uk};
    _Fract fract_results[24] = {0r};
    unsigned _Fract ufract_results[24] = {0ur};
    
    /* Volatile seeds for dynamic values */
    volatile int seed1 = vi1;
    volatile int seed2 = vi2;
    volatile int seed3 = vi3;
    volatile int seed4 = vi4;
    
    /* Test 1: Signed _Accum near maximum range */
    for (int i = 0; i < 8; i++) {
        /* Create values near max/min representable */
        _Accum a = (_Accum)seed1 * 0.999999k;  /* Near max */
        _Accum b = (_Accum)seed2 * 0.999999k;  /* Near min */
        
        /* Operations that could overflow */
        _Accum prod1 = a * a;  /* Square of near-max value */
        _Accum prod2 = b * b;  /* Square of near-min value */
        _Accum sum1 = a + a;   /* Sum that could overflow */
        _Accum sum2 = b + b;   /* Sum that could underflow */
        
        /* Left shift simulation with multiplication */
        _Accum shifted1 = prod1 * 2k;  /* Like << 1 */
        _Accum shifted2 = prod2 * 2k;
        
        /* Store results with conditional to force range analysis */
        results[i*2] = (prod1 > 0.9k) ? shifted1 : prod1;
        results[i*2 + 1] = (prod2 < -0.9k) ? shifted2 : prod2;
        
        /* Update seeds */
        seed1 = seed1 + vi3;
        seed2 = seed2 + vi4;
    }
    
    /* Test 2: Unsigned _Accum near 1.0 */
    for (int i = 0; i < 8; i++) {
        unsigned _Accum ua = 0.9999999uk;
        unsigned _Accum ub = (unsigned _Accum)seed3 * 0.0000001uk;
        
        /* Operations near overflow boundary */
        unsigned _Accum sum = ua + ub;
        unsigned _Accum prod = ua * ua;
        unsigned _Accum scaled = prod * 2uk;
        
        uresults[i*2] = sum;
        uresults[i*2 + 1] = (scaled > 1.0uk) ? 1.0uk : scaled;
        
        seed3 = seed3 + vi1;
    }
    
    /* Test 3: _Fract types with fractional precision */
    for (int i = 0; i < 12; i++) {
        /* Signed fract near boundaries */
        _Fract f1 = 0.999999r;  /* Near +1.0 */
        _Fract f2 = -0.999999r; /* Near -1.0 */
        _Fract f3 = (_Fract)seed4 * 0.000001r;
        
        /* Complex expressions requiring range analysis */
        _Fract expr1 = f1 * f1;  /* Square near 1.0 */
        _Fract expr2 = f2 * f2;  /* Square near 1.0 */
        _Fract expr3 = (f1 + f3) * 2r;  /* Potential overflow */
        _Fract expr4 = (f2 - f3) * 2r;  /* Potential underflow */
        
        fract_results[i*2] = (expr1 > 0r) ? expr1 : f1;
        fract_results[i*2 + 1] = (expr2 < 0r) ? expr2 : f2;
        
        /* Mix with integer promotions */
        int int_val = seed4;
        _Fract converted = (_Fract)int_val * 0.5r;
        fract_results[23 - i] = converted * expr3;
        
        seed4 = seed4 + vi2;
    }
    
    /* Test 4: Unsigned _Fract with wrap-around behavior */
    for (int i = 0; i < 12; i++) {
        unsigned _Fract uf1 = 0.9999999ur;
        unsigned _Fract uf2 = (unsigned _Fract)seed1 * 0.0000001ur;
        
        /* Operations designed to approach and exceed 1.0 */
        unsigned _Fract sum = uf1 + uf2;
        unsigned _Fract prod = uf1 * uf1;
        unsigned _Fract scaled = prod + prod;  /* ×2 */
        
        /* Conditional that depends on overflow check */
        ufract_results[i*2] = (sum >= 1.0ur) ? 1.0ur : sum;
        ufract_results[i*2 + 1] = (scaled >= 1.0ur) ? 1.0ur : scaled;
        
        /* Chain operations to create complex range */
        unsigned _Fract temp = uf1;
        for (int j = 0; j < 3; j++) {
            temp = temp * 0.999ur;
        }
        ufract_results[23 - i] = temp + uf2;
        
        seed1 = seed1 * 2 + 1;
    }
    
    /* Test 5: Mixed-type expressions with explicit casts */
    {
        long _Accum la = 0.999999999lk;
        short _Fract sf = 0.9999hr;
        _Accum ma = 0.999k;
        
        /* Cross-type multiplication requiring range conversion */
        _Accum mixed1 = (_Accum)la * ma;
        _Accum mixed2 = (_Accum)sf * 2k;
        
        /* Left-shift simulation with power-of-two multiplication */
        for (int shift = 1; shift <= 4; shift++) {
            _Accum power = 1k;
            for (int p = 0; p < shift; p++) power = power * 2k;
            _Accum shifted = mixed1 * power;
            results[16 + shift] = shifted;
        }
        
        /* Saturation-like check (emulating builtin overflow check) */
        _Accum test_val = mixed2 * 4k;
        const _Accum MAX_ACCUM = 0.999999k;
        const _Accum MIN_ACCUM = -1.0k;
        
        /* This comparison should trigger the uncovered range check */
        if (test_val > MAX_ACCUM) {
            results[20] = MAX_ACCUM;
        } else if (test_val < MIN_ACCUM) {
            results[20] = MIN_ACCUM;
        } else {
            results[20] = test_val;
        }
    }
    
    /* Test 6: Loop with data-dependent fixed-point computation */
    {
        volatile int array[8] = {127, -128, 255, 0, 1, -1, 100, -100};
        _Accum accum = 0k;
        
        for (int i = 0; i < 8; i++) {
            int val = array[i];
            _Accum converted = (_Accum)val / 256k;  /* Convert to fract-like */
            
            /* Accumulate with potential overflow */
            accum = accum + converted;
            
            /* Multiply by growing factor */
            _Accum factor = (_Accum)i * 0.25k + 1k;
            _Accum scaled = accum * factor;
            
            /* Store with overflow protection logic */
            if (scaled > 0.99k) {
                results[24 + i] = 0.99k;
            } else if (scaled < -0.99k) {
                results[24 + i] = -0.99k;
            } else {
                results[24 + i] = scaled;
            }
        }
    }
    
    /* Prevent optimization of all results */
    consume(results, sizeof(results));
    consume(uresults, sizeof(uresults));
    consume(fract_results, sizeof(fract_results));
    consume(ufract_results, sizeof(ufract_results));
    
    /* Return hash to make result observable */
    int hash1 = compute_hash(results, 32);
    int hash2 = compute_hash((_Accum *)uresults, 8);  /* Half size since different type */
    
    return (hash1 ^ hash2) & 0xFF;
}
