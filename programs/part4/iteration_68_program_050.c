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

/* Complex data-dependent computation with carried dependencies */
static long long complex_loop_kernel(int* data, int size, int start_idx) {
    long long sum = 0;
    int i;
    
    /* Outer loop with volatile bound */
    for (i = 0; i < volatile_bound; i++) {
        int idx = (start_idx + i) % size;
        int prev_idx = (start_idx + i - 1 + size) % size;
        
        /* Data-dependent computation with carried dependency */
        sum += (long long)data[idx] * data[prev_idx];
        
        /* Mixed-width operations */
        sum = (sum >> 3) + (sum << 5); /* Some bit manipulation */
        
        /* Conditional operation */
        if (data[idx] > 100) {
            sum += data[idx] / 7; /* Non-constant divisor */
        } else {
            sum -= data[idx] * 3;
        }
        
        /* Pointer chasing pattern */
        int next_idx = data[idx] % size;
        sum ^= data[next_idx];
        
        /* Inline assembly to create fixed RTL instruction */
        asm volatile ("" : : "r"(sum) : "memory");
    }
    
    return sum;
}

/* Matrix-vector multiplication style computation */
static float matrix_vector_kernel(float* matrix, float* vector, int n) {
    float result = 0.0f;
    int i, j;
    
    /* Nested loops for scheduling complexity */
    for (i = 0; i < n; i++) {
        float row_sum = 0.0f;
        
        #pragma GCC unroll 4
        for (j = 0; j < n; j++) {
            /* Mixed floating-point operations */
            float prod = matrix[i * n + j] * vector[j];
            
            /* Conditional move via ternary */
            row_sum += (prod > 0.0f) ? prod : -prod * 0.5f;
            
            /* Complex addressing mode */
            int mirror_idx = n - j - 1;
            row_sum += matrix[i * n + mirror_idx] * 0.1f;
        }
        
        /* Data-dependent branching */
        if (row_sum > 100.0f) {
            result += row_sum / (float)(i + 2); /* Non-constant divisor */
        } else {
            result += row_sum * (float)(i + 1);
        }
        
        /* Another inline assembly barrier */
        asm volatile ("" : : "r"(result) : "memory");
    }
    
    return result;
}

/* Switch-based computation with multiple basic blocks */
static int switch_computation(int x, int* data, int size) {
    int result = 0;
    
    switch (x % 5) {
        case 0:
            /* Complex case 0 */
            for (int i = 0; i < size; i += 2) {
                result += data[i] * data[i + 1];
                result ^= (result << 3) | (result >> 29);
            }
            break;
            
        case 1:
            /* Complex case 1 */
            for (int i = 0; i < size; i++) {
                result += data[i] / (x + 1); /* Non-constant divisor */
                if (result < 0) result = -result;
            }
            break;
            
        case 2:
            /* Complex case 2 with nested if-else */
            for (int i = 0; i < size; i++) {
                if (data[i] > 50) {
                    result += data[i] * 2;
                    result |= 0x1;
                } else {
                    result += data[i] / 3;
                    result &= ~0x1;
                }
            }
            break;
            
        case 3:
            /* Case 3 with mixed operations */
            for (int i = 0; i < size; i++) {
                float temp = (float)data[i];
                result += (int)(temp * 1.5f);
                result = (result << 1) ^ (result >> 31);
            }
            break;
            
        default:
            /* Default case with pointer arithmetic */
            int* ptr = data;
            for (int i = 0; i < size; i++) {
                result += *ptr++;
                result = result % 9973; /* Prime modulus */
            }
            break;
    }
    
    return result;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int MATRIX_SIZE = 32;
    
    /* Initialize with pseudo-random data */
    int* int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_matrix = (float*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float));
    float* float_vector = (float*)malloc(MATRIX_SIZE * sizeof(float));
    
    /* Simple PRNG for initialization */
    unsigned int seed = time(NULL) ^ volatile_seed;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        int_data[i] = (int)(seed % 1000);
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        float_matrix[i] = (float)(seed % 100) / 10.0f;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        float_vector[i] = (float)(seed % 100) / 20.0f;
    }
    
    long long total_result = 0;
    
    /* First computation kernel */
    for (int iter = 0; iter < 10; iter++) {
        volatile int start = rand() % ARRAY_SIZE; /* External function call */
        long long kernel_result = complex_loop_kernel(int_data, ARRAY_SIZE, start);
        total_result ^= kernel_result;
        
        /* Additional computation to increase scheduling regions */
        float matrix_result = matrix_vector_kernel(float_matrix, float_vector, MATRIX_SIZE);
        total_result += (long long)matrix_result;
    }
    
    /* Second computation with switch-based control flow */
    for (int i = 0; i < 100; i++) {
        int switch_result = switch_computation(i, int_data, ARRAY_SIZE / 4);
        total_result += switch_result;
        
        /* Volatile access to prevent optimization */
        asm volatile ("" : : "r"(total_result) : "memory");
    }
    
    /* Final reduction with complex dependency chain */
    long long final_check = 0;
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        /* Complex data-dependent chain */
        final_check = (final_check * 6364136223846793005ULL + int_data[i]) ^ int_data[i + 1];
        final_check = (final_check >> 1) | (final_check << 63); /* Rotate */
        
        /* Conditional with both branches having computation */
        if (final_check & 1) {
            final_check += int_data[i] / (i % 7 + 1); /* Non-constant divisor */
        } else {
            final_check -= int_data[i] * (i % 5 + 1);
        }
    }
    
    total_result ^= final_check;
    
    /* Ensure side effect is observable */
    printf("Result: %lld\n", total_result);
    
    free(int_data);
    free(float_matrix);
    free(float_vector);
    
    return 0;
}
