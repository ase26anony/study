/* test_fixed_point_range.c
 * Designed to trigger fixed-point range analysis overflow checks in GCC's VRP
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -fwrapv -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi = 0;
static volatile unsigned int vu = 0;
static volatile long vl = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
}

/* Complex fixed-point computations that should trigger range analysis */
int main(void) {
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = vi;
    volatile unsigned int useed = vu;
    
    /* Arrays to store results */
    _Accum acc_results[8];
    unsigned _Fract ufract_results[8];
    signed _Fract sfract_results[8];
    long _Accum laccum_results[8];
    short _Fract shfract_results[8];
    
    /* Initialize with boundary values */
    _Accum max_acc = 0.999999999k;  /* Near max for _Accum */
    _Accum min_acc = -0.999999999k; /* Near min for _Accum */
    unsigned _Fract max_uf = 0.9999999ur; /* Near max for unsigned _Fract */
    signed _Fract max_sf = 0.9999999r;    /* Near max for signed _Fract */
    signed _Fract min_sf = -0.9999999r;   /* Near min for signed _Fract */
    
    /* Loop with varying values to force dynamic range analysis */
    for (int i = 0; i < 8; i++) {
        /* Vary the seed each iteration */
        int idx = (seed + i) & 7;
        unsigned int uidx = (useed + i) & 7;
        
        /* 1. Multiplication near overflow boundaries for _Accum */
        /* This should trigger the signed greater-than comparison in range analysis */
        _Accum a1 = max_acc - (_Accum)(idx * 0.000000001k);
        _Accum a2 = max_acc - (_Accum)(uidx * 0.000000001k);
        _Accum product = a1 * a2;  /* May overflow mathematically */
        acc_results[i] = product;
        
        /* 2. Left shift operations that could overflow */
        /* Fixed-point left shift expands to multiplication by power of two */
        signed _Fract sf = max_sf - (signed _Fract)(i * 0.0000001r);
        /* Simulate left shift: multiply by 2, 4, 8... */
        int shift = (i % 3) + 1;
        signed _Fract shifted = sf;
        for (int s = 0; s < shift; s++) {
            shifted = shifted + shifted;  /* Multiply by 2 each time */
        }
        sfract_results[i] = shifted;
        
        /* 3. Complex expression with mixed types and intermediate overflow checks */
        unsigned _Fract uf = max_uf;
        /* Add small increments that could cause wrapping */
        unsigned _Fract increment = (unsigned _Fract)(uidx * 0.0000001ur);
        uf = uf + increment;
        
        /* Check for overflow using comparison (simulating builtin overflow check) */
        if (uf < increment) {  /* Would indicate wrap-around */
            uf = max_uf;  /* Saturate */
        }
        ufract_results[i] = uf;
        
        /* 4. Operations with long _Accum - larger range, similar overflow issues */
        long _Accum la = 0.99999999999999999lk;
        long _Accum lb = la - (long _Accum)(i * 0.00000000000000001lk);
        long _Accum lproduct = la * lb;
        laccum_results[i] = lproduct;
        
        /* 5. Mix fixed-point with integer promotions */
        int int_val = idx - 4;  /* Range -4 to 3 */
        short _Fract shf = (short _Fract)int_val * 0.5hr;
        /* Conditional expression that depends on range analysis */
        shf = (shf > 0.9hr) ? 0.9hr : 
              (shf < -0.9hr) ? -0.9hr : shf;
        shfract_results[i] = shf;
        
        /* 6. Additional overflow-prone expression */
        /* Chain of operations that could overflow at intermediate steps */
        _Accum temp = a1 * a2;
        /* Simulate left shift by 2 (multiply by 4) */
        _Accum shifted_temp = temp + temp;  /* ×2 */
        shifted_temp = shifted_temp + shifted_temp;  /* ×4 */
        acc_results[i] = shifted_temp;
    }
    
    /* 7. Boundary case: values exactly at or beyond representable range */
    /* Force compiler to consider extreme values */
    signed _Fract boundary_test;
    if (seed > 0) {
        boundary_test = max_sf;
    } else {
        boundary_test = min_sf;
    }
    
    /* Multiply by value > 1 to potentially exceed range */
    boundary_test = boundary_test * 1.1r;
    sfract_results[0] = boundary_test;
    
    /* 8. Use volatile long to force re-evaluation */
    long volatile_seed = vl;
    _Accum volatile_acc = (_Accum)volatile_seed * 0.000000001k;
    if (volatile_seed != 0) {
        volatile_acc = volatile_acc * max_acc;
    }
    acc_results[7] = volatile_acc;
    
    /* Prevent dead code elimination */
    consume(acc_results, sizeof(acc_results));
    consume(ufract_results, sizeof(ufract_results));
    consume(sfract_results, sizeof(sfract_results));
    consume(laccum_results, sizeof(laccum_results));
    consume(shfract_results, sizeof(shfract_results));
    
    /* Return hash of results to ensure all computations matter */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        hash ^= *(int*)&acc_results[i];
        hash ^= *(unsigned int*)&ufract_results[i];
        hash ^= *(int*)&sfract_results[i];
        hash ^= *(long*)&laccum_results[i];
        hash ^= *(short*)&shfract_results[i];
    }
    
    return hash & 0xFF;
}
