/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type definitions */
typedef short _Fract sfract;
typedef _Fract fract;
typedef long _Accum laccum;
typedef unsigned _Fract ufract;
typedef _Sat _Fract sat_fract;
typedef _Sat _Accum sat_accum;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int sign : 1;
    unsigned int integer : 7;
    ufract fraction;  /* unsigned fixed-point */
    fract signed_fraction;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bits(ufract uf, fract f, int shift) {
    struct fixed_bitfield bf;
    volatile ufract vuf = uf;  /* Prevent constant folding */
    
    /* Operations that may trigger alshift/zext/sext logic */
    bf.sign = (uf > 0.5hr) ? 1 : 0;
    bf.integer = (unsigned int)(uf * 10) & 0x7F;
    
    /* Shift operations that require range analysis */
    unsigned int temp = (unsigned int)(uf * 256) >> shift;
    bf.fraction = (ufract)(temp / 256.0);
    
    /* Mixed signed/unsigned operations */
    bf.signed_fraction = f * (fract)bf.fraction;
    
    return bf;
}

/* Main function with complex fixed-point operations */
int main(void) {
    /* Initialize fixed-point variables across different ranges */
    sfract sf1 = 0.2hr;
    sfract sf2 = -0.7hr;
    laccum acc1 = 0.0lk;
    laccum acc2 = 0.0lk;
    sat_fract sat1 = 0.5hr;
    sat_accum sat_acc = 0.0lk;
    
    volatile fract vbound1 = 0.3r;  /* Non-constant bounds */
    volatile fract vbound2 = -0.8r;
    
    /* Bit-field variable */
    struct fixed_bitfield bf;
    
    /* Loop-based range widening */
    for (int i = 0; i < 100; i++) {
        /* Multi-step arithmetic that forces range calculations */
        acc1 = acc1 + (laccum)sf1 * 0.1lk;
        acc2 = acc2 - (laccum)sf2 * 0.05lk;
        
        /* Saturated arithmetic that may trigger overflow checks */
        sat1 = sat1 + 0.15hr;
        sat_acc = sat_acc * 1.01lk;
        
        /* Explicit range comparisons - may lower to sgt/ugt */
        if (acc1 > (laccum)vbound1 * 10.0lk) {
            /* Operation when above upper bound */
            sf1 = sf1 * 0.9hr;
            acc1 = acc1 * 0.8lk;
        } else if (acc2 < (laccum)vbound2 * 5.0lk) {
            /* Operation when below lower bound */
            sf2 = sf2 * -0.8hr;
            acc2 = acc2 * -0.5lk;
        }
        
        /* Ternary operator with mixed types */
        fract temp = (acc1 > 0.0lk) ? 
            (fract)(sf1 * 0.5hr) : 
            (fract)(sf2 * -0.3hr);
            
        /* Compare against computed bounds */
        laccum max_bound = (laccum)vbound1 * (laccum)i;
        laccum min_bound = (laccum)vbound2 * (laccum)i;
        
        if (acc1 > max_bound || acc2 < min_bound) {
            /* Force range recalculation */
            acc1 = acc1 / 2.0lk;
            acc2 = acc2 / 2.0lk;
        }
        
        /* Periodically process bit-fields */
        if (i % 10 == 0) {
            ufract uf = (ufract)((i % 100) / 100.0);
            fract f = (fract)((i % 50 - 25) / 50.0);
            bf = process_fixed_bits(uf, f, i % 8);
            
            /* Use bit-field in conditional */
            if (bf.fraction > 0.25ur && bf.signed_fraction < -0.1r) {
                sat1 = sat1 - 0.1hr;
            }
        }
        
        /* Mixed integer/fixed-point context */
        unsigned long ul_temp = (unsigned long)(acc1 * 1000);
        int int_temp = (int)(acc2 * 100);
        
        /* Conditional with mixed precision */
        if (ul_temp > 500 || int_temp < -200) {
            sat_acc = sat_acc + (sat_accum)(ul_temp / 1000.0);
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    checksum += (int)(sf1 * 1000);
    checksum += (int)(sf2 * 1000);
    checksum += (int)(acc1 * 100);
    checksum += (int)(acc2 * 100);
    checksum += (int)(sat1 * 1000);
    checksum += (int)(sat_acc * 100);
    checksum += (int)(bf.fraction * 1000);
    checksum += (int)(bf.signed_fraction * 1000);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional explicit comparisons to trigger edge cases */
    {
        /* Create scenarios that might hit the specific sgt/ugt comparisons */
        fract f_array[4] = {0.1r, 0.5r, -0.2r, -0.9r};
        ufract uf_array[4] = {0.2ur, 0.8ur, 0.1ur, 0.9ur};
        
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                /* Multiple comparison types */
                if (f_array[i] > (fract)uf_array[j]) {
                    checksum += i * j;
                }
                if ((ufract)f_array[i] < uf_array[j]) {
                    checksum -= i + j;
                }
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
