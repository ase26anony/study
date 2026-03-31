/* sel_sched_trigger.c
 * Designed to trigger GCC selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel_sched_trigger.c -o sel_sched_trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* External function to create dependencies */
extern int rand(void);

/* Complex data-dependent computation with multiple basic blocks */
static long long complex_kernel(int *data, int size, volatile int *v_counter) {
    long long sum = 0;
    long long product = 1;
    double fp_sum = 0.0;
    
    /* Nested loops with carried dependencies */
    for (int i = 1; i < size; i++) {
        /* Data-dependent computation with cross-iteration dependency */
        int diff = data[i] - data[i-1];
        
        /* Mixed-width operations */
        long long wide_diff = (long long)diff * diff;
        
        /* Conditional branch creating multiple basic blocks */
        if (diff > 0) {
            /* Branch 1: Integer operations */
            sum += wide_diff;
            product *= (diff & 0xFF) + 1;
            
            /* Floating point operations */
            fp_sum += (double)diff * 0.5;
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : : "memory");
        } else {
            /* Branch 2: Different operations */
            sum -= wide_diff / 2;
            product /= (abs(diff) % 16) + 1;
            
            /* More floating point */
            fp_sum -= (double)diff * 0.25;
            
            /* Another inline assembly barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Switch statement for additional control flow */
        switch (data[i] % 4) {
            case 0:
                sum += i * 2;
                break;
            case 1:
                sum -= i;
                break;
            case 2:
                product ^= data[i];
                break;
            case 3:
                /* Complex addressing mode */
                sum += data[(i * 3) % size] * 2;
                break;
        }
        
        /* Pointer chasing pattern */
        int *ptr = &data[i];
        for (int j = 0; j < 2; j++) {
            if (ptr > data) {
                sum += *ptr;
                ptr--;
            }
        }
        
        /* Volatile counter update */
        (*v_counter)++;
    }
    
    /* Final reduction with mixed types */
    return sum + (long long)fp_sum + product;
}

/* Matrix-vector multiplication kernel */
static void matrix_vector_multiply(int matrix[4][4], int vector[4], int result[4]) {
    #pragma GCC unroll 4
    for (int i = 0; i < 4; i++) {
        int row_sum = 0;
        
        #pragma GCC unroll 2
        for (int j = 0; j < 4; j++) {
            /* Non-trivial addressing with stride */
            row_sum += matrix[i][j] * vector[(i + j) % 4];
            
            /* Data-dependent operation */
            if (matrix[i][j] > vector[j]) {
                row_sum -= matrix[i][j] / (vector[j] + 1);
            } else {
                row_sum += vector[j] % 7;
            }
        }
        
        /* Division with non-constant divisor */
        result[i] = row_sum / (abs(vector[i]) + 1);
        
        /* Conditional move via ternary */
        result[i] = (result[i] < 0) ? -result[i] : result[i] * 2;
    }
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    int *data = malloc(ARRAY_SIZE * sizeof(int));
    volatile int v_counter = 0;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Use volatile seed to prevent compile-time computation */
        data[i] = rand() % 100 + volatile_seed % 50;
    }
    
    long long total_result = 0;
    
    /* Outer loop with volatile bound */
    for (int outer = 0; outer < volatile_bound % 10; outer++) {
        /* First computation kernel */
        long long kernel1_result = complex_kernel(data, ARRAY_SIZE, &v_counter);
        
        /* Second distinct kernel: matrix operations */
        int matrix[4][4];
        int vector[4];
        int result[4];
        
        /* Initialize matrix and vector with data-dependent values */
        for (int i = 0; i < 4; i++) {
            vector[i] = data[(outer * 4 + i) % ARRAY_SIZE];
            for (int j = 0; j < 4; j++) {
                matrix[i][j] = data[(outer * 4 + i + j) % ARRAY_SIZE];
            }
        }
        
        matrix_vector_multiply(matrix, vector, result);
        
        /* Combine results */
        for (int i = 0; i < 4; i++) {
            total_result ^= result[i];
        }
        total_result += kernel1_result;
        
        /* Modify data for next iteration to prevent dead code elimination */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            data[i] ^= outer;
        }
    }
    
    /* Final reduction to ensure side effects */
    long long final_hash = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_hash ^= (long long)data[i] << (i % 48);
    }
    final_hash += total_result;
    
    printf("Result: %lld (volatile counter: %d)\n", final_hash, v_counter);
    
    free(data);
    return 0;
}
