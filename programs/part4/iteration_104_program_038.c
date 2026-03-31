/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point types with various precisions */
typedef short _Fract sfract;
typedef _Fract fract;
typedef long _Fract lfract;
typedef short _Accum saccum;
typedef _Accum accum;
typedef long _Accum laccum;
typedef _Sat _Fract sat_fract;
typedef _Sat _Accum sat_accum;

/* Bit-field structure for mixed operations */
struct mixed_bf {
    unsigned int ubf1 : 7;
    signed int sbf1 : 9;
    unsigned int ubf2 : 4;
};

/* Separate function using bit-fields and shifts */
static int process_with_bitfields(volatile sat_accum sa, volatile fract f) {
    struct mixed_bf bf = {0};
    int result = 0;
    
    /* Initialize bit-fields with fixed-point derived values */
    bf.ubf1 = (unsigned int)((long long)(sa * 64) & 0x7F);
    bf.sbf1 = (int)((long long)(f * 256) & 0x1FF);
    bf.ubf2 = (unsigned int)((long long)(sa * 8) & 0xF);
    
    /* Shift operations that require range analysis */
    unsigned int shifted = bf.ubf1 << bf.ubf2;
    shifted = shifted >> (bf.ubf2 / 2 + 1);
    
    /* Mixed comparisons with fixed-point */
    if ((shifted > (unsigned int)(sa * 128)) && 
        (bf.sbf1 < (int)(f * 512))) {
        result = 1;
    }
    
    /* Explicit range check triggering sgt/ugt */
    volatile unsigned int bound = bf.ubf2 * 16;
    if (shifted > bound) {
        result += 2;
    }
    
    return result;
}

int main(void) {
    /* Initialize with values spanning positive, negative, and saturated ranges */
    volatile sfract sf1 = 0.25hr;
    volatile sfract sf2 = -0.75hr;
    volatile accum a1 = 100.0k;
    volatile accum a2 = -200.0k;
    volatile sat_accum sa1 = 0.9k;
    volatile sat_fract sf_sat = 0.5hr;
    
    /* Accumulators for loop operations */
    accum acc1 = 0.0k;
    accum acc2 = 0.0k;
    fract f_acc = 0.0r;
    
    /* Non-constant bounds to prevent folding */
    volatile int iter_bound = 100;
    volatile fract f_bound = 0.8r;
    volatile accum a_bound = 50.0k;
    
    int checksum = 0;
    
    /* Loop forcing range widening */
    for (int i = 0; i < iter_bound; i++) {
        /* Mixed arithmetic operations */
        acc1 += a1 * (accum)i / 100.0k;
        acc2 = acc2 * 0.99k + a2;
        f_acc = f_acc + sf1 - sf2;
        
        /* Conditional blocks with range comparisons */
        if (acc1 > a_bound) {
            /* Operations when above bound */
            a_bound = a_bound * 1.1k;
            acc1 = acc1 * 0.9k;
            
            /* Nested comparison */
            if (acc2 < -a_bound) {
                acc2 = -a_bound + 10.0k;
            }
        } else if (acc1 < -a_bound) {
            /* Operations when below negative bound */
            a_bound = a_bound * 0.9k;
            acc1 = acc1 * 1.1k;
        }
        
        /* Ternary operator with fixed-point comparison */
        f_acc = (f_acc > f_bound) ? f_bound : 
                (f_acc < -f_bound) ? -f_bound : f_acc;
        
        /* Mixed integer/fixed-point context */
        unsigned long ul_temp = (unsigned long)((acc1 > 0) ? acc1 * 1000 : -acc1 * 1000);
        checksum += (int)(ul_temp % 256);
        
        /* Periodic saturation check */
        if (i % 7 == 0) {
            sa1 = sa1 * 1.2k;  /* May saturate */
            sf_sat = sf_sat + 0.3hr;  /* May saturate */
            
            /* Explicit overflow check */
            if (sa1 == 1.0k || sa1 == -1.0k) {
                sa1 = 0.5k;
            }
        }
        
        /* Call bitfield function periodically */
        if (i % 13 == 0) {
            checksum += process_with_bitfields(sa1, f_acc);
        }
    }
    
    /* Additional range-intensive operations outside loop */
    laccum lacc = (laccum)acc1 * (laccum)acc2 / 1000.0lk;
    
    /* Complex conditional with multiple comparisons */
    if ((acc1 > 0.0k && acc2 < 0.0k) || 
        (acc1 < 0.0k && acc2 > 0.0k)) {
        lacc = -lacc;
    }
    
    /* Final checksum computation */
    checksum += (int)((long long)(acc1 * 100) & 0xFF);
    checksum += (int)((long long)(acc2 * 100) & 0xFF);
    checksum += (int)((long long)(f_acc * 1000) & 0xFF);
    checksum += (int)((long long)(lacc / 10) & 0xFF);
    checksum += (int)((long long)(sa1 * 100) & 0xFF);
    checksum += (int)((long long)(sf_sat * 1000) & 0xFF);
    
    printf("Checksum: %d\n", checksum);
    printf("Final values: acc1=%.3k, acc2=%.3k, f_acc=%.3r\n", 
           (accum)acc1, (accum)acc2, (fract)f_acc);
    
    return 0;
}
