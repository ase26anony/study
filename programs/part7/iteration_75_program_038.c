/* haifa-sched-trigger.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa-sched-trigger.c -o haifa-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define CHASE_STEPS 50
#define MAX_SWITCH_CASES 10

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile float g_volatile_float = 1.0f;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) mixed_data {
    char c;
    int i;
    double d;
    float f;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Helper functions with different computation patterns */
static int helper1(int a, int b) {
    volatile int barrier;
    asm volatile("" : "=r"(barrier) : : "memory");
    return (a * b) + (a >> 3) - (b << 2);
}

static int helper2(int a, int b) {
    int result = a;
    for (int i = 0; i < 5; i++) {
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        if (result & 1) {
            result ^= b;
        }
    }
    return result;
}

static float helper3(float a, float b) {
    float temp = a;
    for (int i = 0; i < 3; i++) {
        temp = temp * b - temp / (b + 1.0f);
        g_volatile_float = temp; /* Memory side effect */
    }
    return temp;
}

/* Non-inlineable function (due to complexity) */
__attribute__((noinline)) 
int complex_chain(int start, int *array, int size) {
    int a = start;
    int b = array[start % size];
    int c = array[(start * 3) % size];
    
    /* Long dependency chain */
    a = a * b + c;
    b = b ^ a;
    c = c * 2 - a;
    a = (a << 3) | (b >> 2);
    b = b + c * 7;
    c = a ^ b ^ c;
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    a = helper1(a, b);
    b = helper2(b, c);
    
    return a + b + c;
}

/* Function with many basic blocks for switch statement */
static int switch_computation(int value, struct mixed_data *data, int idx) {
    int result = value;
    
    switch (value % MAX_SWITCH_CASES) {
        case 0:
            result = data[idx].i * 3 + result;
            result ^= data[idx].s;
            break;
        case 1:
            result = (result << 4) | (result >> 28);
            result += helper1(result, idx);
            break;
        case 2:
            result = complex_chain(result, &data[0].i, ARRAY_SIZE);
            break;
        case 3:
            result = result * 7 - idx * 13;
            g_volatile_counter = result; /* Volatile store */
            break;
        case 4:
            result = helper2(result, data[idx].i);
            result = (result & 0x5555) | ((result & 0xAAAA) >> 1);
            break;
        case 5:
            result = result + data[idx].i - data[idx].s;
            result = result * result / (idx + 1);
            break;
        case 6:
            for (int i = 0; i < 3; i++) {
                result = (result * 1103515245) ^ data[idx].i;
            }
            break;
        case 7:
            result = (result << 1) | (result >> 31);
            result = helper1(result, data[idx].i);
            break;
        case 8:
            result = result ^ ~data[idx].i;
            result = result + (idx * 17);
            break;
        case 9:
            result = complex_chain(result, &data[0].i, ARRAY_SIZE / 2);
            result = helper2(result, idx);
            break;
        default:
            result = 0;
    }
    
    return result;
}

/* Main computation with complex control flow */
static uint64_t compute_kernel(int iterations, struct mixed_data *data, 
                               int *int_array, float *float_array) {
    uint64_t accumulator = 0;
    int chase_ptr = 0;
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {helper1, helper2};
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Pointer chasing through int_array */
        int chase_sum = 0;
        chase_ptr = iter % ARRAY_SIZE;
        for (int step = 0; step < CHASE_STEPS; step++) {
            chase_ptr = int_array[chase_ptr] % ARRAY_SIZE;
            chase_sum ^= int_array[chase_ptr];
            /* Memory barrier every 8 steps */
            if ((step & 7) == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Mixed data type computations */
        float f_temp = float_array[iter % ARRAY_SIZE];
        double d_temp = data[iter % ARRAY_SIZE].d;
        
        for (int j = 0; j < 8; j++) {
            f_temp = f_temp * 1.01f - d_temp;
            d_temp = d_temp * 0.99 + f_temp;
            
            /* Volatile access */
            if (j == 3) {
                f_temp += g_volatile_float;
            }
        }
        
        data[iter % ARRAY_SIZE].f = f_temp;
        data[iter % ARRAY_SIZE].d = d_temp;
        
        /* Deeply nested conditionals */
        int cond_result = chase_sum;
        if (iter & 1) {
            cond_result = helper1(cond_result, iter);
            if (iter & 2) {
                cond_result = complex_chain(cond_result, int_array, ARRAY_SIZE);
                if (iter & 4) {
                    cond_result = switch_computation(cond_result, data, iter % ARRAY_SIZE);
                    if (iter & 8) {
                        /* Computed jump */
                        compute_func_t f = funcs[iter & 1];
                        cond_result = f(cond_result, int_array[iter % ARRAY_SIZE]);
                    }
                }
            }
        } else {
            cond_result = helper2(cond_result, iter);
        }
        
        /* Large basic block with many independent operations */
        int temp_array[16];
        for (int i = 0; i < 16; i++) {
            temp_array[i] = cond_result * i + int_array[(iter + i) % ARRAY_SIZE];
            temp_array[i] ^= (temp_array[i] << 3);
            temp_array[i] += data[(iter + i) % ARRAY_SIZE].i;
            temp_array[i] = (temp_array[i] * 1103515245) & 0x7fff;
        }
        
        /* Reduction across temp_array */
        int block_result = 0;
        for (int i = 0; i < 16; i++) {
            block_result += temp_array[i];
            block_result ^= (block_result >> 16);
        }
        
        /* Final accumulation with data-dependent operation */
        if (block_result & 1) {
            accumulator += (uint64_t)block_result * cond_result;
        } else {
            accumulator ^= (uint64_t)block_result << (cond_result & 31);
        }
        
        /* Update arrays with data-dependent stores */
        int idx = iter % ARRAY_SIZE;
        int_array[idx] = (int_array[idx] * 3 + cond_result) & 0x7fffffff;
        float_array[idx] = helper3(float_array[idx], cond_result * 0.01f);
    }
    
    return accumulator;
}

int main(int argc, char **argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Allocate and initialize arrays with pseudo-random values */
    struct mixed_data *data = (struct mixed_data*)malloc(ARRAY_SIZE * sizeof(struct mixed_data));
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!data || !int_array || !float_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int seed = i * 1103515245 + 12345;
        
        data[i].c = (char)(seed & 0xFF);
        data[i].i = seed;
        data[i].d = (double)seed / 1000.0;
        data[i].f = (float)seed / 500.0f;
        data[i].s = (short)(seed & 0xFFFF);
        
        int_array[i] = (seed * 1664525 + 1013904223) & 0x7fffffff;
        float_array[i] = (float)int_array[i] / 1000000.0f;
    }
    
    printf("Starting computation with %d iterations...\n", iterations);
    
    /* Perform the main computation */
    uint64_t result = compute_kernel(iterations, data, int_array, float_array);
    
    /* Final reduction across all data structures */
    uint64_t final_check = result;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_check ^= (uint64_t)data[i].i;
        final_check += (uint64_t)(float_array[i] * 1000.0f);
        final_check = (final_check << 13) | (final_check >> 51);
    }
    
    /* Mix in volatile variable */
    final_check ^= g_volatile_counter;
    
    printf("Result: 0x%016llx\n", (unsigned long long)final_check);
    
    /* Cleanup */
    free(data);
    free(int_array);
    free(float_array);
    
    return 0;
}
