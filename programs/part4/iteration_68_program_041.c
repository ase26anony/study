/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o trigger
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining -funroll-loops -march=native -fdump-rtl-all sel-sched-trigger.c -o trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Simple PRNG to create data dependencies */
static inline uint32_t prng(uint32_t *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Function with mixed-width operations to create register pressure */
static uint64_t mixed_width_computation(uint32_t a, uint64_t b, uint32_t c) {
    uint64_t result = 0;
    
    /* Mixed 32-bit and 64-bit operations */
    result = (uint64_t)a * b;          /* 32x64 multiplication */
    result += (uint64_t)c << 16;       /* 32-bit shift extended to 64-bit */
    result /= (b & 0xFFFF) + 1;        /* Division with non-constant divisor */
    
    /* Conditional move via ternary operator */
    result = (result > 1000000) ? result / 2 : result * 3;
    
    return result;
}

/* Matrix-vector multiplication kernel */
void matrix_vector_multiply(int size, double *matrix, double *vector, double *result) {
    volatile int v_size = size;  /* Volatile to prevent optimization */
    
    for (int i = 0; i < v_size; i++) {
        double sum = 0.0;
        
        /* Inner loop with data dependencies */
        for (int j = 0; j < size; j++) {
            /* Complex addressing with stride */
            double element = matrix[i * size + j];
            
            /* Data-dependent computation with carried dependency */
            if (j > 0) {
                element *= vector[j - 1] * 0.5;
            }
            
            sum += element * vector[j];
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : : "memory");
            
            /* Conditional operation inside loop */
            if (sum > 1000.0) {
                sum = sum * 0.99;
            } else {
                sum = sum * 1.01;
            }
        }
        
        /* Floating-point division (expensive operation) */
        result[i] = sum / (vector[i] + 1.0);
    }
}

/* Main computation kernel with nested loops */
uint64_t compute_kernel(int *data, int size) {
    uint64_t total = 0;
    uint32_t prng_state = volatile_seed;
    volatile int v_size = size;
    
    /* Outer loop */
    for (int i = 0; i < v_size; i++) {
        int local_sum = 0;
        
        /* Inner loop with carried dependency */
        #pragma GCC unroll 4
        for (int j = 0; j < volatile_bound; j++) {
            /* Data-dependent computation across iterations */
            if (j > 0) {
                local_sum += data[j] * data[j - 1];
            } else {
                local_sum += data[j] * prng(&prng_state);
            }
            
            /* Mixed operations */
            local_sum -= (data[j] / ((j & 0xF) + 1));
            
            /* Pointer chasing pattern */
            int idx = (local_sum & 0xFF);
            if (idx < size) {
                local_sum ^= data[idx];
            }
            
            /* Switch statement with multiple cases */
            switch (j & 0x3) {
                case 0:
                    local_sum = (local_sum << 1) | (local_sum >> 31);
                    break;
                case 1:
                    local_sum = local_sum * 3 + 1;
                    break;
                case 2:
                    local_sum = local_sum ^ prng(&prng_state);
                    break;
                case 3:
                    local_sum = (local_sum > 1000000) ? local_sum / 2 : local_sum * 2;
                    break;
            }
            
            /* Another inline assembly barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Mixed-width computation */
        total += mixed_width_computation(local_sum, total, i);
        
        /* Volatile function call to prevent optimization */
        if (rand() % 100 == 0) {
            total >>= 1;
        }
    }
    
    return total;
}

/* Second distinct computation kernel */
double second_kernel(double *array, int size) {
    double result = 1.0;
    volatile int v_size = size;
    
    for (int i = 0; i < v_size; i++) {
        /* Complex floating-point operations */
        double x = array[i];
        
        /* Taylor series approximation for exp(x) */
        double term = 1.0;
        double sum = 1.0;
        
        #pragma GCC unroll 2
        for (int k = 1; k < 6; k++) {
            term *= x / k;
            sum += term;
            
            /* Conditional inside inner loop */
            if (sum > 100.0) {
                sum = 100.0;
            }
        }
        
        /* Data-dependent update */
        if (i > 0) {
            result *= sum + array[i - 1] * 0.1;
        } else {
            result *= sum;
        }
        
        /* More inline assembly */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

int main() {
    const int ARRAY_SIZE = 1024;
    const int MATRIX_SIZE = 64;
    
    /* Initialize with pseudo-random data */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double *matrix = (double*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *vector = (double*)malloc(MATRIX_SIZE * sizeof(double));
    double *result = (double*)malloc(MATRIX_SIZE * sizeof(double));
    
    srand(time(NULL));
    
    /* Fill arrays with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        double_data[i] = (double)rand() / RAND_MAX * 2.0 - 1.0;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (double)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        vector[i] = (double)rand() / RAND_MAX;
    }
    
    printf("Starting computations...\n");
    
    /* First kernel - integer heavy */
    uint64_t result1 = compute_kernel(int_data, ARRAY_SIZE);
    printf("Result 1: %lu\n", (unsigned long)result1);
    
    /* Second kernel - floating point */
    double result2 = second_kernel(double_data, ARRAY_SIZE);
    printf("Result 2: %f\n", result2);
    
    /* Third kernel - matrix operations */
    matrix_vector_multiply(MATRIX_SIZE, matrix, vector, result);
    
    /* Final reduction to prevent optimization */
    double final_sum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        final_sum += result[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    
    /* XOR all results together */
    uint64_t final_xor = result1 ^ *(uint64_t*)&result2 ^ *(uint64_t*)&final_sum;
    printf("Final XOR: %lx\n", (unsigned long)final_xor);
    
    /* Cleanup */
    free(int_data);
    free(double_data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
