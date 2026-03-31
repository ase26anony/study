/* Fixed-point range analysis test targeting GCC's fixed-value.cc overflow checks */
/* Compile with: gcc -O3 -ffixed-point -ftree-vrp -fwrapv -fdump-tree-vrp-details -c fixed-point-test.c */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 0;
static volatile unsigned int vu1 = 0xFFFFFFFF;
static volatile unsigned int vu2 = 1;

/* Dummy function to prevent optimization */
__attribute__((noinline, noipa))
void consume_result(const void *ptr, int size) {
    volatile char sink;
    const char *p = (const char *)ptr;
    for (int i = 0; i < size; i++) {
        sink = p[i];
    }
    (void)sink;
}

/* Hash function to mix results */
__attribute__((noinline))
int compute_hash(const long long *arr, int n) {
    int hash = 0;
    for (int i = 0; i < n; i++) {
        hash ^= (arr[i] >> 32) ^ arr[i];
        hash = (hash << 13) | (hash >> 19);
    }
    return hash;
}

int main(void) {
    /* Array to accumulate results */
    long long results[32] = {0};
    int result_idx = 0;
    
    /* Test various fixed-point types with edge-case values */
    
    /* 1. Test _Accum (signed 16.16) near maximum */
    {
        _Accum a1 = 0.999999k;  /* Very close to 1.0 */
        _Accum a2 = 0.999999k;
        _Accum a3 = -0.999999k; /* Very close to -1.0 */
        
        /* Force range analysis with volatile-dependent values */
        int scale = vi1 + 1;  /* Could be 1 or 2 */
        
        /* Operations that could overflow */
        _Accum prod1 = a1 * a2;  /* ~0.999998, should be fine */
        _Accum prod2 = a1 * (_Accum)scale;  /* Could be ~1.999998 or ~0.999999 */
        
        /* Store as integer for hashing */
        results[result_idx++] = *((long long*)&prod1);
        results[result_idx++] = *((long long*)&prod2);
    }
    
    /* 2. Test long _Accum with extreme values */
    {
        long _Accum la1 = 0.999999999999k;  /* Very close to 1.0L */
        long _Accum la2 = 0.999999999999k;
        
        /* Multiplication that mathematically exceeds range */
        long _Accum la_prod = la1 * la2;  /* ~0.999999999998 */
        
        /* Left shift simulation through multiplication */
        int shift = vi2 < 0 ? 1 : 2;  /* Data-dependent shift */
        long _Accum la_shifted = la1 * (1 << shift);
        
        results[result_idx++] = *((long long*)&la_prod);
        results[result_idx++] = *((long long*)&la_shifted);
    }
    
    /* 3. Test unsigned _Fract at boundaries */
    {
        unsigned _Fract uf1 = 0.9999999ur;  /* Max unsigned fract */
        unsigned _Fract uf2 = 0.0000001ur;  /* Small positive */
        
        /* Operations near overflow boundary */
        unsigned _Fract uf_sum = uf1 + uf2;  /* Could overflow to 0.0 */
        unsigned _Fract uf_prod = uf1 * uf1;  /* ~0.9999998 */
        
        /* Conditional with overflow check */
        unsigned _Fract uf3 = (uf_sum == 0.0ur) ? 0.5ur : uf_sum;
        
        results[result_idx++] = *((int*)&uf_sum);
        results[result_idx++] = *((int*)&uf_prod);
        results[result_idx++] = *((int*)&uf3);
    }
    
    /* 4. Test signed _Fract with negative boundary */
    {
        _Fract sf1 = -0.9999999r;  /* Min signed fract */
        _Fract sf2 = 0.9999999r;   /* Max signed fract */
        
        /* Operations that could underflow/overflow */
        _Fract sf_diff = sf1 - sf2;  /* ~ -1.9999998 (would underflow) */
        _Fract sf_sum = sf1 + sf2;   /* ~ 0.0 */
        
        /* Data-dependent scaling */
        int mult = vu1 > 0 ? 1 : 2;  /* Always 1 due to vu1 = 0xFFFFFFFF */
        _Fract sf_scaled = sf2 * (_Fract)mult;
        
        results[result_idx++] = *((int*)&sf_diff);
        results[result_idx++] = *((int*)&sf_sum);
        results[result_idx++] = *((int*)&sf_scaled);
    }
    
    /* 5. Test short _Fract with integer promotion */
    {
        short _Fract sfr1 = 0.9999r;  /* Max short fract */
        short _Fract sfr2 = -0.9999r; /* Min short fract */
        
        /* Integer promotion in expression */
        int int_val = vi3 + 256;  /* Could be 256 or 257 */
        short _Fract sfr3 = sfr1 * (short _Fract)(int_val / 256);
        
        /* Complex expression with multiple steps */
        short _Fract temp = sfr1 * sfr2;  /* ~ -0.9998 */
        short _Fract sfr4 = temp + sfr1;  /* ~ 0.0001 */
        
        results[result_idx++] = *((short*)&sfr3);
        results[result_idx++] = *((short*)&sfr4);
    }
    
    /* 6. Loop with varying fixed-point values */
    {
        _Accum accum_array[8];
        long _Accum long_accum_array[8];
        
        for (int i = 0; i < 8; i++) {
            /* Data-dependent fixed-point values */
            int base = vu2 + i;  /* 1 + i */
            _Accum a = (_Accum)base / 8.0k;  /* Ranges from 0.125 to 1.0 */
            long _Accum la = (long _Accum)base / 8.0k;
            
            /* Operations that scale with iteration */
            accum_array[i] = a * a * (_Accum)(i + 1);
            long_accum_array[i] = la * la * (long _Accum)(i + 1);
            
            /* Store results */
            results[result_idx++] = *((int*)&accum_array[i]);
            results[result_idx++] = *((long long*)&long_accum_array[i]);
        }
        
        /* Force analysis on array elements */
        consume_result(accum_array, sizeof(accum_array));
        consume_result(long_accum_array, sizeof(long_accum_array));
    }
    
    /* 7. Test saturating arithmetic simulation */
    {
        /* Simulate overflow check logic */
        _Accum test_val = 0.999999k;
        _Accum increment = 0.000001k;
        
        /* Manual overflow check */
        _Accum sum = test_val + increment;
        int overflow = 0;
        
        /* Check if sum exceeds maximum */
        if (sum > 0.999999k && increment > 0.0k) {
            overflow = 1;
            sum = 0.999999k;  /* Saturate */
        }
        
        results[result_idx++] = *((int*)&sum);
        results[result_idx++] = overflow;
    }
    
    /* 8. Test mixed-type expressions */
    {
        unsigned _Fract uf = 0.9999999ur;
        _Fract sf = 0.9999999r;
        _Accum acc = 0.999999k;
        
        /* Mixed operations with casts */
        _Accum mixed1 = (_Accum)uf * acc;      /* unsigned fract to accum */
        _Accum mixed2 = (_Accum)sf * acc;      /* signed fract to accum */
        
        /* Integer to fixed-point with scaling */
        int int_val = vi1 * 65536;  /* Could be 65536 or 131072 */
        _Accum scaled = (_Accum)int_val / 65536.0k;
        
        results[result_idx++] = *((long long*)&mixed1);
        results[result_idx++] = *((long long*)&mixed2);
        results[result_idx++] = *((long long*)&scaled);
    }
    
    /* Ensure we don't exceed array bounds */
    if (result_idx > 32) result_idx = 32;
    
    /* Compute final hash to prevent optimization */
    int final_hash = compute_hash(results, result_idx);
    
    /* Consume all results to prevent dead code elimination */
    consume_result(results, sizeof(results[0]) * result_idx);
    
    return final_hash & 0xFF;  /* Return non-zero, non-constant value */
}
