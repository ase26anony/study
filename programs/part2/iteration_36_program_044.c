/* sel-sched-coverage.c
 * Designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Function with potential aliasing to create memory dependencies */
static inline void process_chunk(double *restrict dest, 
                                 double *src1, 
                                 double *src2, 
                                 int *counter,
                                 int start, 
                                 int end) {
    volatile int *volatile_counter = counter; /* Prevent optimization */
    double local_acc = 0.0;
    int int_acc = 0;
    float float_acc = 0.0f;
    
    /* Hot loop with multiple dependencies and operations */
    for (int i = start; i < end; i++) {
        /* Memory loads with potential aliasing */
        double val1 = src1[i];
        double val2 = src2[i];
        
        /* Integer operations with carried dependency */
        int_acc += i * (*volatile_counter);
        int_acc ^= (i << 3) | (i >> 5);
        
        /* Floating-point operations mixing precision */
        double temp = val1 * val2 + (double)float_acc;
        float_acc = (float)(temp * 0.5);
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Division operation - expensive and hard to schedule */
            local_acc += val1 / (val2 + 1.0);
            int_acc /= 2; /* Integer division creates dependency chain */
        } else if (i % 13 == 0) {
            /* Different execution path */
            local_acc -= val1 * 0.25;
            int_acc *= 3;
        } else {
            /* Default path */
            local_acc += val1 + val2;
            int_acc -= 1;
        }
        
        /* Memory store with potential dependency */
        dest[i] = local_acc + (double)int_acc * 0.01;
        
        /* More arithmetic diversity */
        float_acc = float_acc * 0.9f + (float)local_acc * 0.1f;
        
        /* Inline assembly to prevent optimization and create barriers */
        asm volatile("" : "+r" (int_acc), "+m" (dest[i]) : : "memory");
    }
    
    /* Update counter with dependency */
    *volatile_counter += int_acc;
}

/* Main computational function that will be inlined */
static inline double compute_loop(double *arr1, double *arr2, int *counter) {
    double result = 0.0;
    
    /* Multiple calls to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Process in chunks to create more scheduling opportunities */
        process_chunk(arr1, arr1, arr2, counter, 0, SIZE/2);
        process_chunk(arr2, arr2, arr1, counter, SIZE/2, SIZE);
        
        /* Cross-iteration dependency */
        result += arr1[iter % SIZE] * arr2[(iter * 7) % SIZE];
        
        /* Additional operations to increase instruction mix */
        if (iter % 100 == 0) {
            /* Complex operation that might generate interesting RTL */
            double temp = result;
            for (int j = 0; j < 4; j++) {
                temp = temp * 1.01 - 0.5;
            }
            result = temp;
        }
    }
    
    return result;
}

int main() {
    /* Initialize data with non-trivial patterns */
    double *array1 = (double*)malloc(SIZE * sizeof(double));
    double *array2 = (double*)malloc(SIZE * sizeof(double));
    int counter = 1;
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (double)(i + 1) * 0.1;
        array2[i] = (double)(rand() % 100) * 0.01;
    }
    
    /* Perform computation - this should trigger selective scheduling */
    double final_result = compute_loop(array1, array2, &counter);
    
    /* Additional computations to ensure scheduler sees multiple regions */
    for (int phase = 0; phase < 3; phase++) {
        final_result += compute_loop(array2, array1, &counter);
    }
    
    /* Create checksum to prevent optimization */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + array2[i];
    }
    checksum += final_result + counter;
    
    /* Print result to ensure computation isn't optimized away */
    printf("Result: checksum = %f, counter = %d\n", checksum, counter);
    
    free(array1);
    free(array2);
    
    return 0;
}
