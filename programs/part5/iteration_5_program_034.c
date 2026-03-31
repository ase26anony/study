/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barrier */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int start, int iterations, double init_val) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_int[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = init_val * i;
        arr_int[i] = start + i * 3;
    }
    
    /* Loop-invariant variable for control flow */
    int invariant = get_volatile_value() % 4;
    long long total = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex control flow */
        if (outer % (invariant + 1) == 0) {
            /* Register pressure inducing expression */
            v4df vec1 = {arr_dbl[outer & 0xF], arr_dbl[(outer + 1) & 0xF], 
                         arr_dbl[(outer + 2) & 0xF], arr_dbl[(outer + 3) & 0xF]};
            v4df vec2 = {arr_dbl[(outer + 4) & 0xF], arr_dbl[(outer + 5) & 0xF],
                         arr_dbl[(outer + 6) & 0xF], arr_dbl[(outer + 7) & 0xF]};
            
            /* Vector operations creating many temporaries */
            v4df temp1 = vec1 * vec2;
            v4df temp2 = vec1 + vec2;
            v4df temp3 = temp1 - temp2;
            
            /* Shuffle operations that create virtual registers */
            v4df shuffled = __builtin_shuffle(temp1, temp2, 
                (v4di){2, 3, 0, 1});
            
            /* Mix integer and floating point */
            v4di int_vec = (v4di)shuffled;
            v4di int_temp = int_vec >> (outer & 7);
            
            /* More operations with barrier */
            double barrier_val = get_volatile_value();
            v4df temp4 = shuffled * barrier_val;
            
            /* Convert back and forth */
            v4di int_result = (v4di)temp4 + int_temp;
            
            /* Accumulate results with mixed types */
            for (int i = 0; i < 4; i++) {
                total += int_result[i] + (long long)(temp3[i] * 100.0);
            }
            
            /* More register pressure with array accesses */
            double dbl_temp = arr_dbl[outer & 0xF] * arr_dbl[(outer + 1) & 0xF];
            long long int_temp2 = arr_int[outer & 0xF] * arr_int[(outer + 2) & 0xF];
            
            /* Complex expression with many intermediate values */
            total += (long long)(dbl_temp * 1000.0) + int_temp2;
            
            /* Another vector operation chain */
            v4df vec3 = {dbl_temp, dbl_temp * 2.0, dbl_temp * 3.0, dbl_temp * 4.0};
            v4df vec4 = vec3 * shuffled;
            v4di vec4_int = (v4di)vec4;
            
            for (int i = 0; i < 4; i++) {
                total += vec4_int[i] >> (i + 1);
            }
        } else {
            /* Alternative path with different operations */
            double alt_val = arr_dbl[(outer + 8) & 0xF];
            long long alt_int = arr_int[(outer + 8) & 0xF];
            
            /* More mixed operations */
            for (int j = 0; j < 4; j++) {
                double calc = alt_val * j + get_volatile_value();
                total += (long long)(calc * alt_int);
                alt_val = alt_val * 1.5 - j;
            }
        }
        
        /* Modify arrays to prevent optimization */
        arr_dbl[outer & 0xF] += get_volatile_value() * 0.01;
        arr_int[outer & 0xF] += get_volatile_value() % 100;
    }
    
    return total;
}

int main(void) {
    long long checksum = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        checksum += test_remat(i, 50, i * 1.5);
        checksum += test_remat(i * 2, 25, i * 0.75);
    }
    
    printf("Final checksum: %lld\n", checksum);
    printf("Barrier counter: %d\n", barrier_counter);
    
    return 0;
}
