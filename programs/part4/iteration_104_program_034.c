/* Fixed-point range analysis test targeting GCC's fixed-value.cc logic */
#include <stdio.h>

/* Enable fixed-point types */
#pragma GCC diagnostic ignored "-Wpedantic"

/* Bit-field structure for shift operations */
struct bitfield_fract {
    unsigned int mantissa : 16;
    unsigned int exponent : 8;
    unsigned int sign : 1;
};

/* Separate function using bit-fields and shifts */
void fixed_bitfield_ops(struct bitfield_fract *bf, volatile short _Fract f) {
    /* Operations that may trigger alshift/zext/sext logic */
    bf->mantissa = (unsigned int)(f * 32767.0hr) & 0xFFFF;
    
    /* Shift operations requiring range analysis */
    unsigned int shifted = bf->mantissa << bf->exponent;
    shifted = shifted >> 3;
    
    /* Conditional with potential overflow */
    if (shifted > 32767) {
        bf->mantissa = 32767;
    }
}

int main() {
    /* Declare fixed-point variables spanning different ranges */
    volatile short _Fract f1 = 0.5hr;
    volatile short _Fract f2 = -0.25hr;
    volatile long _Accum a1 = 100.0lk;
    volatile long _Accum a2 = -50.0lk;
    volatile _Sat short _Fract fsat = 0.75hr;
    volatile _Sat long _Accum asat = 200.0lk;
    
    /* Accumulators for loop operations */
    short _Fract f_acc = 0.0hr;
    long _Accum a_acc = 0.0lk;
    _Sat short _Fract fsat_acc = 0.0hr;
    
    /* Non-constant bounds to prevent folding */
    volatile int bound1 = 10;
    volatile int bound2 = 20;
    
    /* Bit-field structure */
    struct bitfield_fract bf = {0, 3, 0};
    
    /* Loop forcing range widening */
    for (int i = 0; i < 100; i++) {
        /* Mixed precision arithmetic */
        f_acc = f_acc + f1;
        a_acc = a_acc * a1 + a2;
        
        /* Saturated operations */
        fsat_acc = fsat_acc + fsat;
        
        /* Explicit range comparisons - may trigger sgt/ugt logic */
        if (f_acc > 0.8hr) {
            f_acc = f_acc - 0.1hr;
        }
        
        /* Complex conditional with AND/OR - targeting the uncovered if condition */
        if ((a_acc > 1000.0lk) || 
            (a_acc == 1000.0lk && f_acc > 0.9hr)) {
            a_acc = a_acc / 2.0lk;
        }
        
        /* Mixed integer/fixed-point comparison */
        int int_val = (int)(a_acc / 10.0lk);
        if (int_val > bound1 && int_val < bound2) {
            f_acc = f_acc * 0.5hr;
        }
        
        /* Ternary operator with range check */
        fsat_acc = (fsat_acc > 0.95hr) ? 0.95hr : fsat_acc;
        
        /* Update volatile bounds to prevent optimization */
        bound1 = (bound1 + 1) % 15;
        bound2 = (bound2 + 2) % 25;
        
        /* Call bitfield function periodically */
        if (i % 7 == 0) {
            fixed_bitfield_ops(&bf, f_acc);
        }
    }
    
    /* Additional overflow-triggering operations */
    _Sat long _Accum overflow_test = 500.0lk;
    for (int i = 0; i < 5; i++) {
        overflow_test = overflow_test * 2.0lk;  /* Should saturate */
    }
    
    /* Explicit comparisons that might hit the uncovered block */
    long _Accum test_val = a_acc;
    short _Fract test_frac = f_acc;
    
    /* Multi-step comparison similar to uncovered code pattern */
    if (test_val > 10000.0lk || 
        (test_val == 10000.0lk && test_frac > 0.99hr)) {
        test_val = test_val / 10.0lk;
    }
    
    /* Compute checksum */
    unsigned long checksum = 0;
    checksum += (unsigned int)(f_acc * 1000.0hr);
    checksum += (unsigned int)(a_acc / 10.0lk);
    checksum += (unsigned int)(fsat_acc * 1000.0hr);
    checksum += (unsigned int)(overflow_test / 100.0lk);
    checksum += bf.mantissa;
    
    printf("Checksum: %lu\n", checksum);
    
    /* Additional test with shift operations on fixed-point results */
    unsigned int shifted_val = (unsigned int)(a_acc * 100.0lk);
    for (int shift = 1; shift < 8; shift++) {
        shifted_val = (shifted_val << shift) | (shifted_val >> (32 - shift));
        
        /* Conditional that might trigger range analysis */
        if (shifted_val > 0x7FFFFFFF) {
            shifted_val = 0x7FFFFFFF;
        }
    }
    
    printf("Final shifted: %u\n", shifted_val);
    
    return 0;
}
