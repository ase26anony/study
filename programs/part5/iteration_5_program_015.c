/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier_counter = 0;
static int get_barrier(void) {
    barrier_counter++;
    return barrier_counter;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline,noipa))
long long test_remat(int start, int iterations, double init_val) {
    /* Large vectors to force virtual register creation */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_int[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = init_val + i * 0.5;
        arr_int[i] = start + i * 3;
    }
    
    /* Complex intermediate results */
    v4df vec_acc = {0.0, 0.0, 0.0, 0.0};
    v4di int_acc = {0, 0, 0, 0};
    double fp_acc = 0.0;
    long long int_acc_scalar = 0;
    
    /* Outer loop with multiple basic blocks */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for control flow */
        int invariant = get_barrier() % 4;  /* Volatile call creates barrier */
        
        /* Inner if with complex expression */
        if (invariant > 0) {
            /* Complex expression mixing integer and FP operations */
            for (int inner = 0; inner < 8; inner++) {
                /* Non-constant array indexing */
                int idx1 = (start + outer + inner) & 0xF;
                int idx2 = (start * inner + outer) & 0xF;
                
                /* Register pressure: many intermediate values */
                double temp1 = arr_dbl[idx1] * 1.5 + arr_dbl[idx2];
                double temp2 = temp1 / (inner + 1.0);
                double temp3 = temp2 * temp2 - temp1;
                
                /* Integer operations */
                long long itemp1 = arr_int[idx1] + arr_int[idx2];
                long long itemp2 = itemp1 * (inner + 1);
                long long itemp3 = itemp2 >> 2;
                
                /* Mix integer and FP */
                fp_acc += temp3 * itemp3;
                int_acc_scalar += (long long)(temp3 * 100.0) + itemp3;
                
                /* Vector operations that create virtual registers */
                v4df vec1 = {temp1, temp2, temp3, fp_acc};
                v4df vec2 = {arr_dbl[idx1], arr_dbl[idx2], temp1, temp2};
                v4df vec3 = vec1 + vec2 * 0.25;
                
                /* Shuffle operation often creates virtual registers */
                v4df vec_shuffled = __builtin_shuffle(vec3, 
                    (v4di){2, 1, 0, 3});
                vec_acc += vec_shuffled;
                
                /* More integer vector ops */
                v4di ivec1 = {itemp1, itemp2, itemp3, int_acc_scalar};
                v4di ivec2 = __builtin_shuffle(ivec1, 
                    (v4di){1, 0, 3, 2});
                int_acc += ivec1 * ivec2;
                
                /* Another volatile barrier in the middle */
                if ((inner & 3) == 0) {
                    get_barrier();
                }
            }
            
            /* Additional computation in the if block */
            double complex_expr = 0.0;
            for (int k = 0; k < 4; k++) {
                /* More register pressure with mixed modes */
                complex_expr += (arr_dbl[k] * int_acc[k % 4]) / 
                               (outer + 1.0);
                
                /* Integer arithmetic that might need DImode */
                int_acc_scalar += (arr_int[k] * outer) ^ 
                                 (long long)(complex_expr * 1000.0);
            }
            
            /* Use results to prevent dead code elimination */
            fp_acc += complex_expr;
        } else {
            /* Alternate path to create control flow complexity */
            for (int j = 0; j < 4; j++) {
                int idx = (start + j + outer) & 0xF;
                fp_acc -= arr_dbl[idx] * 0.1;
                int_acc_scalar -= arr_int[idx] >> 1;
            }
        }
        
        /* Cross-block value usage */
        if (outer % 2 == 0) {
            /* Force values to be live across blocks */
            arr_dbl[outer & 0xF] = fp_acc * 0.01;
            arr_int[outer & 0xF] = int_acc_scalar / 3;
        }
    }
    
    /* Final reduction */
    double final_fp = fp_acc;
    for (int i = 0; i < 4; i++) {
        final_fp += vec_acc[i];
    }
    
    long long final_int = int_acc_scalar;
    for (int i = 0; i < 4; i++) {
        final_int += int_acc[i];
    }
    
    /* Return mixed type result */
    return (long long)(final_fp * 100.0) + final_int;
}

int main(void) {
    long long total = 0;
    
    /* Call with different arguments to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i, 10 + (i % 5), 1.0 + i * 0.1);
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 7 == 0) {
            get_barrier();
        }
    }
    
    printf("Result: %lld\n", total);
    printf("Barrier calls: %d\n", barrier_counter);
    
    return 0;
}
