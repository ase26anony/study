/* Compile with: gcc -O3 -fno-inline -fdump-rtl-all -fno-schedule-insns test.c */
/* Or: gcc -O2 -fsanitize=undefined -fno-omit-frame-pointer test.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile function to create code motion barrier */
static volatile int barrier_counter = 0;
int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int start_val, double init_val) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = init_val + i * 0.5;
        arr_ll[i] = start_val + i * 3;
    }
    
    /* Mix of integer and FP computations */
    double fp_acc = init_val;
    long long int_acc = start_val;
    v4df vec_acc = {init_val, init_val + 1.0, init_val + 2.0, init_val + 3.0};
    v4di vec_int = {start_val, start_val + 1, start_val + 2, start_val + 3};
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant condition */
        int invariant_cond = (outer % 4) == 0;
        
        /* Inner if block with register pressure */
        if (invariant_cond) {
            /* Complex expression with many temporaries */
            volatile int barrier = get_volatile_value();
            
            /* Vector operations that create virtual registers */
            v4df vec1 = {arr_dbl[outer & 0xF], arr_dbl[(outer + 1) & 0xF], 
                         arr_dbl[(outer + 2) & 0xF], arr_dbl[(outer + 3) & 0xF]};
            v4df vec2 = {arr_dbl[(outer + 4) & 0xF], arr_dbl[(outer + 5) & 0xF],
                         arr_dbl[(outer + 6) & 0xF], arr_dbl[(outer + 7) & 0xF]};
            
            /* Shuffle operations - often create virtual registers */
            v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
                (v4di){0, 1, 4, 5});
            
            /* Mixed integer/FP operations */
            for (int inner = 0; inner < 8; inner++) {
                /* Non-constant array indexing */
                int idx1 = (outer + inner) & 0xF;
                int idx2 = (outer + inner + 4) & 0xF;
                
                /* Complex expression with many intermediate values */
                double temp1 = arr_dbl[idx1] * 1.5 + barrier;
                double temp2 = arr_dbl[idx2] * 2.5 - barrier;
                long long temp3 = arr_ll[idx1] * 3 + barrier;
                long long temp4 = arr_ll[idx2] * 5 - barrier;
                
                /* More operations mixing types */
                fp_acc += temp1 - temp2;
                int_acc += temp3 - temp4;
                
                /* Vector accumulation */
                vec_acc += vec_shuffled * (inner + 1);
                vec_int += (v4di){temp3, temp4, temp3 + 1, temp4 + 1};
                
                /* Additional barrier to prevent optimization */
                barrier = get_volatile_value();
            }
            
            /* More vector operations */
            v4df vec3 = vec_acc + vec_shuffled;
            v4di vec4 = vec_int + (v4di){barrier, barrier + 1, barrier + 2, barrier + 3};
            
            /* Store results back to arrays */
            for (int i = 0; i < 4; i++) {
                arr_dbl[(outer + i) & 0xF] = vec3[i];
                arr_ll[(outer + i) & 0xF] = vec4[i];
            }
        } else {
            /* Alternate path with different computations */
            for (int i = 0; i < 4; i++) {
                int idx = (outer + i) & 0xF;
                arr_dbl[idx] = arr_dbl[idx] * 0.9 + fp_acc;
                arr_ll[idx] = arr_ll[idx] / 2 + int_acc;
            }
        }
        
        /* Cross-iteration dependencies */
        fp_acc = fp_acc * 0.99 + outer * 0.01;
        int_acc = int_acc * 0.9 + outer;
    }
    
    /* Final reduction */
    double final_fp = fp_acc;
    for (int i = 0; i < 16; i++) {
        final_fp += arr_dbl[i];
    }
    
    long long final_int = int_acc;
    for (int i = 0; i < 16; i++) {
        final_int += arr_ll[i];
    }
    
    /* Mix results */
    return (long long)(final_fp * 1000.0) + final_int;
}

int main(void) {
    long long total = 0;
    
    /* Call with different parameters to avoid constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50 + (i % 10), i * 100, i * 0.5);
        
        /* Print progress occasionally */
        if (i % 25 == 0) {
            printf("Iteration %d, total so far: %lld\n", i, total);
        }
    }
    
    printf("Final checksum: %lld\n", total);
    printf("Barrier calls: %d\n", barrier_counter);
    
    return 0;
}
