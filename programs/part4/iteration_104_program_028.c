/* Test program to trigger fixed-point range analysis logic in fixed-value.cc */
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
    unsigned int flag : 1;
    ufract frac_part : 7;  /* 7-bit unsigned fixed-point */
    fract signed_part : 8; /* 8-bit signed fixed-point */
    int padding : 16;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bitfield(struct fixed_bitfield bf, int shift) {
    /* Shift operations that may trigger alshift logic */
    unsigned int temp = (unsigned int)bf.frac_part;
    temp = temp << shift;  /* Left shift */
    
    /* Range check with explicit comparison */
    if (temp > 0x7F) {  /* Compare against max 7-bit value */
        bf.frac_part = 0x7F;
    } else {
        bf.frac_part = temp;
    }
    
    /* Signed fixed-point operation with potential overflow */
    fract scaled = bf.signed_part;
    for (int i = 0; i < shift; i++) {
        scaled *= 0.5r;  /* Repeated multiplication */
    }
    
    /* Explicit range comparison */
    if (scaled > 0.9r || scaled < -0.9r) {
        bf.flag = 1;
    } else {
        bf.flag = 0;
    }
    
    return bf;
}

/* Main function with complex fixed-point operations */
int main(void) {
    /* Initialize fixed-point variables with various ranges */
    volatile sfract sf1 = 0.5hr;      /* Positive */
    volatile sfract sf2 = -0.75hr;    /* Negative */
    volatile laccum acc1 = 100.0lk;   /* Large positive */
    volatile laccum acc2 = -50.0lk;   /* Large negative */
    volatile sat_fract sat1 = 0.8r;   /* Saturated type */
    volatile sat_accum sat2 = 200.0k; /* Saturated accum */
    
    /* Mixed integer/fixed-point variables */
    unsigned long ul_bound = 1000;
    int int_bound = 500;
    
    /* Accumulators for loop operations */
    fract f_acc = 0.0r;
    laccum l_acc = 0.0lk;
    sat_fract sat_acc = 0.0r;
    
    /* Bit-field structure */
    struct fixed_bitfield bf = {0, 0x40, 0.5r, 0};
    
    /* Loop with range-widening operations */
    for (volatile int i = 0; i < 100; i++) {
        /* Multi-step arithmetic that forces range analysis */
        f_acc = f_acc + sf1 * 0.1hr;
        
        /* Conditional with explicit range comparison */
        if (f_acc > 0.9r) {
            f_acc = 0.9r;  /* Clamp to upper bound */
        } else if (f_acc < -0.9r) {
            f_acc = -0.9r; /* Clamp to lower bound */
        }
        
        /* Mixed-type operation with cast */
        l_acc = l_acc + (laccum)((int)f_acc * 10);
        
        /* Range check against volatile bounds */
        if (l_acc > (laccum)ul_bound) {
            l_acc = (laccum)ul_bound;
        }
        if (l_acc < (laccum)(-int_bound)) {
            l_acc = (laccum)(-int_bound);
        }
        
        /* Saturated arithmetic with overflow potential */
        sat_acc = sat_acc + sat1;
        
        /* Ternary operator with range comparison */
        sat2 = (sat_acc > 0.5r) ? sat_acc * 2.0k : sat_acc / 2.0k;
        
        /* Complex conditional with multiple comparisons */
        if (acc1 > l_acc && acc2 < l_acc) {
            /* Nested range check */
            fract temp = (fract)(l_acc / 10.0lk);
            if (temp > sf1 || temp < sf2) {
                f_acc = f_acc * 0.5r;
            }
        }
        
        /* Bit-field operations every 10 iterations */
        if (i % 10 == 0) {
            bf = process_fixed_bitfield(bf, (i / 10) % 4);
            
            /* Mixed comparison with bit-field member */
            if ((fract)bf.frac_part > f_acc) {
                f_acc = (fract)bf.frac_part;
            }
        }
        
        /* Shift-based operation that may trigger alshift logic */
        unsigned int shift_temp = (unsigned int)(i % 8);
        ufract shifted = (ufract)((unsigned int)sat_acc << shift_temp);
        
        /* Range comparison after shift */
        if (shifted > 0.9ur) {
            sat_acc = sat_acc * 0.8r;
        }
    }
    
    /* Post-loop computations with explicit range checks */
    laccum final_acc = acc1 + acc2 + l_acc;
    
    /* Final range comparison - may trigger the specific sgt/ugt logic */
    if (final_acc > 1000.0lk || final_acc < -1000.0lk) {
        printf("Range exceeded: %ld\n", (long)(final_acc));
    }
    
    /* Compute checksum from all results */
    unsigned long checksum = 0;
    checksum += (unsigned long)(f_acc * 1000);
    checksum += (unsigned long)l_acc;
    checksum += (unsigned long)(sat_acc * 1000);
    checksum += (unsigned long)(sat2 / 10);
    checksum += bf.frac_part;
    checksum += (unsigned long)(bf.signed_part * 100);
    
    printf("Checksum: %lu\n", checksum);
    
    return 0;
}
