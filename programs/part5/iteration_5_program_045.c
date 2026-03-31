/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter;
}

/* Non-inlineable function with __attribute__((noinline)) */
static long long __attribute__((noinline,noipa))
test_remat(int iterations, int seed, double factor) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays with values that depend on arguments */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = (seed + i) * factor;
        arr_ll[i] = (seed * i) ^ 0x12345678;
    }
    
    /* Complex expression results */
    v4df vec_result = {0};
    long long scalar_result = 0;
    double fp_result = 0.0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = get_volatile_value() & 0xF;
        
        /* Inner if with register pressure */
        if ((outer + seed) % 3 == 0) {
            /* High register pressure section with mixed operations */
            
            /* Vector operations that create virtual registers */
            v4df vec1 = {arr_dbl[invariant % 16], 
                         arr_dbl[(invariant + 1) % 16],
                         arr_dbl[(invariant + 2) % 16],
                         arr_dbl[(invariant + 3) % 16]};
            
            v4df vec2 = {arr_dbl[(invariant + 4) % 16],
                         arr_dbl[(invariant + 5) % 16],
                         arr_dbl[(invariant + 6) % 16],
                         arr_dbl[(invariant + 7) % 16]};
            
            /* Shuffle operation that often needs virtual registers */
            v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
                (v4di){0, 2, 1, 3});
            
            /* Mixed integer/floating computations */
            for (int inner = 0; inner < 4; inner++) {
                /* Complex expression with many temporaries */
                double temp1 = vec_shuffled[inner] * factor;
                double temp2 = temp1 + arr_dbl[(inner + invariant) % 16];
                long long temp3 = (long long)(temp2 * 1000.0);
                double temp4 = (double)temp3 / 1000.0;
                
                /* Integer operations */
                long long temp5 = arr_ll[(inner + invariant) % 16];
                long long temp6 = temp5 ^ temp3;
                long long temp7 = temp6 * (inner + 1);
                
                /* More floating point */
                double temp8 = temp4 * temp4;
                double temp9 = temp8 - temp2;
                double temp10 = temp9 * (inner % 2 ? -1.0 : 1.0);
                
                /* Barrier to prevent optimization */
                if (get_volatile_value() & 1) {
                    temp10 += 0.001;
                }
                
                /* Accumulate results */
                fp_result += temp10;
                scalar_result += temp7;
                
                /* More vector operations */
                vec_result[inner] += temp10;
            }
            
            /* Additional computations to increase register pressure */
            for (int i = 0; i < 8; i++) {
                double a = arr_dbl[(i * 2) % 16];
                double b = arr_dbl[(i * 2 + 1) % 16];
                long long c = arr_ll[i];
                long long d = arr_ll[15 - i];
                
                /* Complex chain of operations */
                double t1 = a * b + (double)c;
                long long t2 = (long long)t1 ^ d;
                double t3 = (double)t2 / (a + 1.0);
                long long t4 = (long long)(t3 * 100.0);
                double t5 = t3 * t3 - a * b;
                
                /* Use results */
                if (t4 > 0) {
                    fp_result += t5;
                    scalar_result += t4;
                }
            }
        } else {
            /* Alternative path with different computations */
            for (int i = 0; i < 4; i++) {
                double base = arr_dbl[(invariant + i * 3) % 16];
                long long mask = arr_ll[(invariant + i * 2) % 16];
                
                /* Different computation pattern */
                double r1 = base * (i + 1);
                long long r2 = (long long)r1 & mask;
                double r3 = (double)r2 / (base + 1.0);
                
                fp_result += r3;
                scalar_result += r2;
            }
        }
        
        /* Cross-mode operations */
        if (outer % 5 == 0) {
            /* Convert between double and long long */
            double dval = fp_result;
            long long llval = scalar_result;
            
            /* Operations that might need rematerialization */
            double converted = (double)llval;
            long long reconverted = (long long)dval;
            
            /* Mix them */
            fp_result = fp_result * 0.99 + converted * 0.01;
            scalar_result = (scalar_result + reconverted) / 2;
        }
    }
    
    /* Final mixing */
    long long final_ll = scalar_result;
    double final_dbl = fp_result;
    
    /* Use vector result */
    for (int i = 0; i < 4; i++) {
        final_ll ^= (long long)vec_result[i];
        final_dbl += vec_result[i];
    }
    
    return final_ll + (long long)final_dbl;
}

int main(void) {
    long long total = 0;
    
    /* Call with different parameters to avoid constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50 + (i % 10), i * 123, 1.0 + i * 0.01);
        
        /* Prevent loop optimization */
        if (get_volatile_value() & 0x100) {
            total ^= 0x5555555555555555LL;
        }
    }
    
    printf("Result: %lld\n", total);
    printf("Barrier calls: %d\n", barrier_counter);
    
    return 0;
}
