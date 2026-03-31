/* sel_sched_trigger.c
 * Designed to trigger selective scheduler debug dumping in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel_sched_trigger.c -o sel_sched_trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Complex data-dependent computation with carried dependencies */
static inline int complex_op(int a, int b, int c) {
    /* Mixed-width operations to create register pressure */
    long long wide = (long long)a * b;
    int narrow = (int)(wide >> 16);
    
    /* Conditional move via ternary */
    int result = (c > 0) ? (narrow + c) : (narrow - c);
    
    /* Division with non-constant divisor (prevoves constant propagation) */
    if (b != 0) {
        result /= (b & 0xFF) + 1;  /* Non-zero divisor */
    }
    
    return result;
}

/* Pointer chasing pattern */
int chase_pointer(int *data, int start, int steps) {
    int idx = start;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        /* Data-dependent memory access */
        sum += data[idx];
        
        /* Next index depends on current value */
        idx = (data[idx] * 13 + 7) & 0x3FF;
        
        /* Inline assembly to create fixed RTL instruction */
        asm volatile ("" : : : "memory");
    }
    
    return sum;
}

/* Matrix-vector multiplication kernel */
void matvec_multiply(float *matrix, float *vector, float *result, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        float sum = 0.0f;
        
        /* Inner loop with stride access */
        for (int j = 0; j < n; j++) {
            /* Mixed floating-point operations */
            float prod = matrix[i * n + j] * vector[j];
            
            /* Conditional operation */
            if (prod > 0.0f) {
                sum += prod * 0.5f;
            } else {
                sum -= prod * 0.25f;
            }
            
            /* Dependency across iterations */
            if (j > 0) {
                sum += matrix[i * n + j - 1] * 0.1f;
            }
        }
        
        /* Division with volatile denominator */
        volatile float denom = 3.14159f;
        result[i] = sum / denom;
    }
}

/* Main computation with nested loops and control flow */
int main() {
    const int SIZE = 1024;
    int *data = malloc(SIZE * sizeof(int));
    float *matrix = malloc(SIZE * SIZE * sizeof(float));
    float *vector = malloc(SIZE * sizeof(float));
    float *result = malloc(SIZE * sizeof(float));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        data[i] = rand() % 256;
        vector[i] = (float)(rand() % 100) / 10.0f;
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = (float)(rand() % 100) / 10.0f;
    }
    
    int total = 0;
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < volatile_bound; outer++) {
        int running_sum = 0;
        
        /* First nested loop with data-dependent computation */
        for (int i = 1; i < SIZE - 1; i++) {
            /* Carried dependency: uses previous iteration's value */
            int temp = complex_op(data[i], data[i-1], running_sum);
            
            /* Switch statement creating multiple basic blocks */
            switch (data[i] & 0x3) {
                case 0:
                    running_sum += temp * 2;
                    /* Inline assembly barrier */
                    asm volatile ("" : : : "memory");
                    break;
                case 1:
                    running_sum -= temp / 2;
                    break;
                case 2:
                    running_sum ^= temp;
                    /* Memory clobber to prevent reordering */
                    asm volatile ("" : : : "memory");
                    break;
                default:
                    running_sum = (running_sum * 3) / (temp + 1);
                    break;
            }
            
            /* Additional conditional with both branches having computation */
            if (data[i] > 128) {
                running_sum += chase_pointer(data, i, 5);
            } else {
                running_sum -= (data[i] * data[i+1]) & 0xFF;
            }
        }
        
        total ^= running_sum;
        
        /* Second computation kernel */
        if (outer % 10 == 0) {
            matvec_multiply(matrix, vector, result, 32);
            
            /* Use result to prevent dead code elimination */
            float checksum = 0.0f;
            for (int i = 0; i < 32; i++) {
                checksum += result[i];
            }
            total += (int)checksum;
        }
        
        /* Modify data to prevent loop-invariant code motion */
        data[outer % SIZE] = total & 0xFF;
    }
    
    /* Final reduction with volatile */
    volatile int final_result = total;
    printf("Result: %d\n", final_result);
    
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
