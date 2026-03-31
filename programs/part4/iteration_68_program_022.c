/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Simple PRNG to avoid libc rand() overhead in analysis */
static inline uint32_t simple_prng(uint32_t *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Complex data-dependent computation with mixed operations */
static int64_t compute_kernel(int32_t *data, int size, volatile int *vbound) {
    int64_t sum = 0;
    int32_t prev = data[0];
    uint32_t prng_state = (uint32_t)time(NULL);
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < *vbound; outer++) {
        int inner_bound = size - 1;
        int32_t local_prev = prev;
        
        /* Inner loop with carried dependency */
        #pragma GCC unroll 4
        for (int i = 1; i < inner_bound; i++) {
            /* Data-dependent computation with mixed-width operations */
            int32_t current = data[i];
            int64_t product = (int64_t)current * (int64_t)local_prev;
            
            /* Conditional operations creating control flow */
            if (current > 0) {
                /* Integer division with non-constant divisor */
                product /= (simple_prng(&prng_state) % 256 + 1);
                
                /* Floating point operations to create FPU pressure */
                float fp_val = (float)product * 0.5f;
                int32_t int_val = (int32_t)fp_val;
                
                /* Ternary operator for conditional move pattern */
                sum += (product % 2 == 0) ? int_val : -int_val;
            } else {
                /* Different computation path */
                product *= 3;
                sum += product >> 2;
            }
            
            /* Pointer chasing pattern */
            int32_t *ptr = &data[i];
            asm volatile("" : "+r"(ptr) : : "memory");
            
            /* Mixed memory access with stride */
            int32_t next = data[i + (simple_prng(&prng_state) % 3)];
            local_prev = next;
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile("nop" : : : "memory");
        }
        
        /* Switch statement creating multiple basic blocks */
        switch (outer % 4) {
            case 0:
                sum += data[simple_prng(&prng_state) % size] * 2;
                break;
            case 1:
                sum -= data[simple_prng(&prng_state) % size] / 3;
                break;
            case 2:
                sum ^= data[simple_prng(&prng_state) % size];
                break;
            case 3:
                sum |= data[simple_prng(&prng_state) % size];
                break;
        }
        
        /* Additional volatile operation */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    return sum;
}

/* Second computation kernel with different pattern */
static int64_t matrix_vector_kernel(int32_t *matrix, int32_t *vector, int n) {
    int64_t result = 0;
    volatile int vn = n;
    
    for (int i = 0; i < vn; i++) {
        int32_t row_sum = 0;
        
        /* Unrolled inner loop with complex addressing */
        #pragma GCC unroll 2
        for (int j = 0; j < n; j++) {
            /* Non-trivial addressing: matrix[i*n + j] */
            int32_t elem = matrix[i * n + j];
            int32_t vec_elem = vector[j];
            
            /* Mixed operations */
            int64_t prod = (int64_t)elem * (int64_t)vec_elem;
            
            /* Branch with both paths having computation */
            if (prod > 1000000) {
                prod = prod >> 3;
                row_sum += (int32_t)(prod % 65536);
            } else {
                prod = prod * 3;
                row_sum += (int32_t)(prod & 0xFFFF);
            }
            
            /* Memory barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Floating-point conversion and back */
        float fp_row = (float)row_sum;
        result += (int64_t)(fp_row * 1.5f);
    }
    
    return result;
}

int main(void) {
    const int size = 1024;
    int32_t *data = malloc(size * sizeof(int32_t));
    int32_t *matrix = malloc(size * size * sizeof(int32_t));
    int32_t *vector = malloc(size * sizeof(int32_t));
    
    if (!data || !matrix || !vector) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    uint32_t seed = (uint32_t)time(NULL);
    for (int i = 0; i < size; i++) {
        data[i] = (int32_t)simple_prng(&seed);
        vector[i] = (int32_t)simple_prng(&seed);
    }
    
    for (int i = 0; i < size * size; i++) {
        matrix[i] = (int32_t)simple_prng(&seed);
    }
    
    /* Run first kernel */
    volatile int bound = volatile_bound + (simple_prng(&seed) % 100);
    int64_t result1 = compute_kernel(data, size, &bound);
    
    /* Run second kernel */
    volatile int matrix_size = 32;  /* Smaller for reasonable runtime */
    int64_t result2 = matrix_vector_kernel(matrix, vector, matrix_size);
    
    /* Final reduction to prevent optimization */
    int64_t final_result = result1 ^ result2;
    
    /* Use result to prevent dead code elimination */
    printf("Result: %ld\n", (long)final_result);
    
    free(data);
    free(matrix);
    free(vector);
    
    return 0;
}
