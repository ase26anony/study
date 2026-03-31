/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Simple PRNG using volatile seed */
static inline int pseudo_rand(void) {
    volatile_seed = (volatile_seed * 1103515245 + 12345) & 0x7fffffff;
    return volatile_seed;
}

/* Complex data-dependent computation with multiple operation types */
static long long complex_kernel(int *data, int size, int stride) {
    long long sum = 0;
    volatile int vol_counter = 0;
    
    /* Outer loop with volatile bound */
    for (int i = 1; i < volatile_bound && i < size; i++) {
        int local_sum = 0;
        
        /* Inner loop with carried dependency */
        #pragma GCC unroll 4
        for (int j = 0; j < 8; j++) {
            /* Data-dependent computation with multiple operation types */
            int idx = (i * stride + j) % size;
            int prev_idx = (idx - 1 + size) % size;
            
            /* Mixed-width operations */
            long long temp = (long long)data[idx] * data[prev_idx];
            
            /* Conditional operations */
            int cond = data[idx] & 0x1;
            temp = cond ? temp + data[prev_idx] : temp - data[prev_idx];
            
            /* Floating point to create FPU pressure */
            float ftemp = (float)temp / (data[idx] != 0 ? data[idx] : 1);
            
            /* Integer division with non-constant divisor */
            local_sum += (int)ftemp + (temp % (data[prev_idx] != 0 ? data[prev_idx] : 1));
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : "r"(local_sum) : "memory");
        }
        
        /* Complex addressing mode */
        sum += local_sum * data[(i * 7) % size];
        
        /* Pointer chasing pattern */
        int *ptr = &data[i % size];
        for (int k = 0; k < 3; k++) {
            sum += *ptr;
            ptr = &data[(*ptr) % size];
        }
        
        vol_counter++;
    }
    
    return sum;
}

/* Second computation kernel with different pattern */
static double matrix_vector_kernel(int *matrix, int *vector, int n) {
    double result = 0.0;
    volatile int vol_n = n;
    
    /* Nested loops for matrix operations */
    for (int i = 0; i < vol_n; i++) {
        double row_sum = 0.0;
        
        /* Inner loop with stride access */
        for (int j = 0; j < vol_n; j++) {
            /* Mixed computations */
            int idx = i * vol_n + j;
            int val = matrix[idx] * vector[j];
            
            /* Conditional with both branches having computation */
            if (val > 0) {
                row_sum += (double)val / (pseudo_rand() % 100 + 1);
            } else {
                row_sum -= (double)(-val) / (pseudo_rand() % 100 + 1);
            }
            
            /* Switch statement to create control flow */
            switch (j % 4) {
                case 0:
                    row_sum += 0.1;
                    break;
                case 1:
                    row_sum -= 0.2;
                    break;
                case 2:
                    row_sum *= 1.01;
                    break;
                case 3:
                    row_sum /= 1.01;
                    break;
            }
        }
        
        result += row_sum;
        
        /* Another inline assembly barrier */
        asm volatile ("" : : "r"(result) : "memory");
    }
    
    return result;
}

/* Third kernel with pointer arithmetic and complex conditions */
static unsigned long long pointer_chase_kernel(int *data, int size) {
    unsigned long long hash = 0;
    int *ptr = data;
    volatile int steps = size / 2;
    
    for (int i = 0; i < steps; i++) {
        /* Complex pointer arithmetic */
        int offset = (*ptr) % 16;
        ptr += offset;
        if (ptr >= data + size) ptr = data;
        
        /* Bit manipulation operations */
        hash ^= (unsigned long long)*ptr << (i % 32);
        hash = (hash >> 31) | (hash << 33);  /* Rotate */
        
        /* Division with external function call */
        hash /= (rand() % 256 + 1);
        
        /* Memory barrier via inline assembly */
        asm volatile ("" : : "r"(hash), "r"(ptr) : "memory");
    }
    
    return hash;
}

int main(void) {
    const int SIZE = 1024;
    const int MATRIX_SIZE = 32;
    
    /* Initialize with pseudo-random data */
    int *data = (int*)malloc(SIZE * sizeof(int));
    int *matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int *vector = (int*)malloc(MATRIX_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Fill arrays with random data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = rand() % 1000 - 500;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = rand() % 200 - 100;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        vector[i] = rand() % 100 - 50;
    }
    
    /* Volatile to prevent compile-time computation */
    volatile int stride = rand() % 10 + 1;
    
    printf("Starting complex computations...\n");
    
    /* Execute first kernel */
    long long result1 = complex_kernel(data, SIZE, stride);
    printf("Kernel 1 result: %lld\n", result1);
    
    /* Execute second kernel */
    double result2 = matrix_vector_kernel(matrix, vector, MATRIX_SIZE);
    printf("Kernel 2 result: %f\n", result2);
    
    /* Execute third kernel */
    unsigned long long result3 = pointer_chase_kernel(data, SIZE);
    printf("Kernel 3 result: %llu\n", result3);
    
    /* Final reduction to ensure side effects */
    unsigned long long final_hash = result1 ^ (unsigned long long)result2 ^ result3;
    printf("Final hash: %llu\n", final_hash);
    
    free(data);
    free(matrix);
    free(vector);
    
    return 0;
}
