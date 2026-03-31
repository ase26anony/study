/* Test program to trigger early rematerialization in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline,noipa)) 
long long test_remat(int iterations, int seed, double factor) {
    /* Large vectors to create virtual registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    int arr_int[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = (i + seed) * 1.5;
        arr_ll[i] = (i * seed) ^ 0x12345678;
        arr_int[i] = i * seed + get_volatile_value(); /* Barrier */
    }
    
    /* Complex intermediate results */
    v4si vec_a, vec_b, vec_c;
    v2df vec_d, vec_e, vec_f;
    
    /* Initialize vectors */
    for (int i = 0; i < 4; i++) {
        vec_a[i] = arr_int[i * 2] + arr_int[i * 2 + 1];
        vec_b[i] = arr_int[15 - i] * seed;
        vec_d[i % 2] = arr_dbl[i * 2] * factor;
        vec_e[i % 2] = arr_dbl[i * 2 + 1] / factor;
    }
    
    long long result = 0;
    double accum_dbl = 0.0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = (outer * seed) & 0xF;
        
        /* Inner if with complex expression */
        if ((invariant + get_volatile_value()) % 3 == 0) {
            /* REGISTER PRESSURE BLOCK - many temporaries */
            
            /* Vector operations creating virtual registers */
            vec_c = __builtin_shuffle(vec_a, vec_b, 
                (v4si){0, 2, 1, 3});
            
            /* Mixed integer/FP operations */
            double temp1 = arr_dbl[invariant % 16] * 3.14159;
            double temp2 = arr_dbl[(invariant + 1) % 16] / 2.71828;
            
            /* Complex expression with many intermediates */
            long long temp3 = arr_ll[invariant % 16] * 
                             (long long)(temp1 * 1000.0);
            long long temp4 = arr_ll[(invariant + 1) % 16] + 
                             (long long)(temp2 * 1000.0);
            
            /* More vector operations */
            vec_f = __builtin_shuffle(vec_d, vec_e, 
                (v2df){0, 1});
            
            /* Barrier inside expression */
            double temp5 = temp1 + temp2 + get_volatile_value();
            
            /* Integer arithmetic with FP conversion */
            int temp6 = (int)(temp5 * 100.0) ^ 
                       arr_int[invariant % 16];
            
            /* Another complex expression */
            long long temp7 = (temp3 ^ temp4) * temp6;
            
            /* Use __builtin_shuffle again for more virtual regs */
            v4si vec_g = __builtin_shuffle(vec_a, vec_c,
                (v4si){3, 1, 2, 0});
            
            /* More mixed operations */
            for (int i = 0; i < 4; i++) {
                temp7 += vec_g[i] * (i + 1);
            }
            
            /* Final accumulation */
            result += temp7;
            accum_dbl += temp5;
            
            /* Additional vector operation to increase pressure */
            vec_a = vec_a + vec_b * vec_c;
            vec_d = vec_d + vec_e * vec_f;
        }
        
        /* Alternate path with different operations */
        else {
            /* Still complex but different operations */
            double alt_temp = arr_dbl[(invariant + 5) % 16] * 
                             arr_dbl[(invariant + 3) % 16];
            long long alt_ll = arr_ll[invariant % 16] + 
                              (long long)(alt_temp * 100.0);
            
            /* Vector shuffle with different pattern */
            v4si vec_h = __builtin_shuffle(vec_b, vec_a,
                (v4si){2, 0, 3, 1});
            
            for (int i = 0; i < 4; i++) {
                alt_ll += vec_h[i] * (4 - i);
            }
            
            result ^= alt_ll;
            accum_dbl -= alt_temp;
        }
        
        /* Modify arrays to prevent optimization */
        arr_int[outer % 16] += get_volatile_value();
        arr_dbl[outer % 16] *= 1.0001;
    }
    
    /* Combine results */
    return result + (long long)(accum_dbl * 1000.0);
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50, i, 1.0 + i * 0.01);
        
        /* Prevent tail recursion optimization */
        if (i % 10 == 0) {
            printf("Progress: %d iterations, total so far: %lld\n", 
                   i, total);
        }
    }
    
    printf("Final result: %lld\n", total);
    printf("Barrier counter: %d\n", barrier_counter);
    
    return 0;
}
