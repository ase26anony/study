/* Test program to trigger early rematerialization in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int volatile_barrier(int x) {
    return x;
}

/* Non-inlineable function with __attribute__((noinline)) */
static __attribute__((noinline, noipa))
long long test_remat(int iterations, int seed, int threshold) {
    /* Large vectors to create virtual registers */
    typedef int v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_double[16];
    long long arr_long[16];
    int arr_int[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_double[i] = (seed + i) * 1.5;
        arr_long[i] = (seed + i) * 3LL;
        arr_int[i] = seed + i;
    }
    
    /* Loop-invariant variable for control flow */
    int loop_invariant = volatile_barrier(threshold);
    
    /* Accumulators of different types */
    double double_acc = 0.0;
    long long long_acc = 0;
    int int_acc = 0;
    
    /* Vector accumulators */
    v4df vdf_acc = {0.0, 0.0, 0.0, 0.0};
    v4di vdi_acc = {0, 0, 0, 0};
    v8si vsi_acc = {0, 0, 0, 0, 0, 0, 0, 0};
    
    /* Outer loop to create multiple basic blocks */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex control flow with if condition */
        if (iter % loop_invariant != 0) {
            /* 
             * Complex expression with many intermediate values
             * Mixing integer and floating-point operations
             * Using volatile function to prevent code motion
             */
            
            /* Vector operations that create virtual registers */
            v4df v1 = {arr_double[iter & 0xF], arr_double[(iter + 1) & 0xF], 
                       arr_double[(iter + 2) & 0xF], arr_double[(iter + 3) & 0xF]};
            v4df v2 = {arr_double[(iter + 4) & 0xF], arr_double[(iter + 5) & 0xF],
                       arr_double[(iter + 6) & 0xF], arr_double[(iter + 7) & 0xF]};
            
            /* Shuffle operations that need virtual registers */
            v4df v_shuffled = __builtin_shuffle(v1, v2, 
                (v4di){0, 1, 4, 5});
            
            /* Mixed integer/float computation chain */
            double temp1 = v_shuffled[0] * v_shuffled[1];
            temp1 += volatile_barrier(iter);  /* Code motion barrier */
            
            long long temp2 = arr_long[iter & 0xF] * arr_long[(iter + 1) & 0xF];
            temp2 += volatile_barrier(iter);  /* Another barrier */
            
            /* Complex expression with many temporaries */
            double temp3 = temp1 * arr_double[iter & 0xF];
            long long temp4 = temp2 * arr_long[(iter + 2) & 0xF];
            int temp5 = arr_int[iter & 0xF] * arr_int[(iter + 1) & 0xF];
            
            /* More operations mixing types */
            temp3 = temp3 + (double)temp4 + (double)temp5;
            temp4 = (long long)temp3 * temp2;
            temp5 = (int)temp4 ^ arr_int[(iter + 3) & 0xF];
            
            /* Vector operations continued */
            v4di vi1 = {temp4, temp2, arr_long[iter & 0xF], arr_long[(iter + 1) & 0xF]};
            v4di vi2 = {arr_long[(iter + 2) & 0xF], arr_long[(iter + 3) & 0xF],
                       arr_long[(iter + 4) & 0xF], arr_long[(iter + 5) & 0xF]};
            
            /* Another shuffle */
            v4di vi_shuffled = __builtin_shuffle(vi1, vi2,
                (v4di){0, 2, 4, 6});
            
            /* Update accumulators */
            double_acc += temp3;
            long_acc += temp4;
            int_acc += temp5;
            
            /* Vector accumulator updates */
            vdf_acc += v_shuffled;
            vdi_acc += vi_shuffled;
            
            /* Integer vector operations */
            v8si vsi1 = {arr_int[0], arr_int[1], arr_int[2], arr_int[3],
                        arr_int[4], arr_int[5], arr_int[6], arr_int[7]};
            v8si vsi2 = {arr_int[8], arr_int[9], arr_int[10], arr_int[11],
                        arr_int[12], arr_int[13], arr_int[14], arr_int[15]};
            
            /* Permute vector elements */
            vsi_acc += __builtin_shuffle(vsi1, vsi2,
                (v8si){0, 2, 4, 6, 8, 10, 12, 14});
        } else {
            /* Alternative path to create more control flow complexity */
            double temp = arr_double[iter & 0xF] * 2.0;
            temp += volatile_barrier(iter);
            double_acc += temp;
        }
        
        /* Additional computation outside if block */
        if (iter % 7 == 0) {
            /* More register pressure */
            double t1 = arr_double[(iter + 8) & 0xF];
            double t2 = arr_double[(iter + 9) & 0xF];
            double t3 = t1 * t2 + volatile_barrier(iter);
            double_acc += t3;
            
            long long lt1 = arr_long[(iter + 8) & 0xF];
            long long lt2 = arr_long[(iter + 9) & 0xF];
            long long lt3 = lt1 ^ lt2 * volatile_barrier(iter);
            long_acc += lt3;
        }
    }
    
    /* Combine results from different types */
    long long result = (long long)double_acc + long_acc + int_acc;
    
    /* Add vector results */
    for (int i = 0; i < 4; i++) {
        result += (long long)vdf_acc[i];
        result += vdi_acc[i];
    }
    
    for (int i = 0; i < 8; i++) {
        result += vsi_acc[i];
    }
    
    return result;
}

int main(int argc, char **argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    long long total = 0;
    
    /* Call test function multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        long long result = test_remat(iterations, i * 100, 3 + (i % 5));
        total += result;
        
        /* Print progress to prevent dead code elimination */
        if (i % 3 == 0) {
            printf("Iteration %d: result = %lld\n", i, result);
        }
    }
    
    printf("Final checksum: %lld\n", total);
    return 0;
}
