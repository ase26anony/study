#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define CHASE_DEPTH 128

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_barrier = 0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    short s;
    float f;
};

/* Function pointer type for computed jumps */
typedef int (*ComputeFunc)(int, int);

/* Helper functions with different characteristics */
static inline int helper1(int a, int b) {
    return (a * b) + (a >> 3) - (b & 0xFF);
}

static int helper2(int a, int b) {
    int result = a;
    for (int i = 0; i < 4; i++) {
        result = (result ^ b) + (result << 2);
    }
    return result;
}

static double helper3(double a, double b) {
    volatile double temp = a * b;  /* Prevent optimization */
    asm volatile("" ::: "memory"); /* Compiler barrier */
    return temp + sin(a) * cos(b);
}

/* Non-inlineable function to create scheduling boundaries */
__attribute__((noinline)) 
int complex_calculation(int *arr, double *darr, float *farr, int idx) {
    int result = arr[idx];
    double dtemp = darr[idx];
    float ftemp = farr[idx];
    
    /* Chain of dependent operations */
    result = result * 1103515245 + 12345;
    dtemp = dtemp * 1.234567 - sqrt(fabs(dtemp));
    ftemp = ftemp * 2.718281f + ftemp * ftemp;
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    arr[idx] = result ^ (int)dtemp;
    darr[idx] = dtemp + ftemp;
    farr[idx] = ftemp - result;
    
    return result;
}

/* Function with switch statement creating multiple basic blocks */
static int switch_computation(int value, int *counter) {
    int result = value;
    
    switch (value % 10) {
        case 0:
            result = result * 3 + 1;
            for (int i = 0; i < 3; i++) {
                result ^= (result << i);
            }
            break;
        case 1:
            result = (result >> 2) | (result << 30);
            result += *counter * 7;
            break;
        case 2:
            result = helper1(result, *counter);
            result = helper2(result, result ^ 0x55AA55AA);
            break;
        case 3:
            result = result * result - result;
            asm volatile("" ::: "memory");
            result = result % 1023;
            break;
        case 4:
            for (int i = 0; i < 5; i++) {
                result = (result + i) * (result - i);
            }
            break;
        case 5:
            result = ~result;
            result = result & 0x0F0F0F0F;
            break;
        case 6:
            result = result << 4 | result >> 28;
            result = result ^ 0xCCCCCCCC;
            break;
        case 7:
            result = abs(result);
            result = result * 13 + 17;
            break;
        case 8:
            result = result / (value % 7 + 1);
            result = result * 2 - 1;
            break;
        case 9:
            result = (result & 0x55555555) << 1 | (result & 0xAAAAAAAA) >> 1;
            break;
    }
    
    (*counter)++;
    return result;
}

/* Pointer chasing simulation */
static int pointer_chase(int *array, int start_idx) {
    int idx = start_idx;
    int sum = 0;
    
    for (int i = 0; i < CHASE_DEPTH; i++) {
        idx = array[idx] % ARRAY_SIZE;
        sum += array[idx];
        /* Create loop-carried dependency */
        array[idx] = (array[idx] + sum) & 0x7FFFFFFF;
        
        /* Memory barrier every 8 iterations */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return sum;
}

/* Main computation kernel */
static uint64_t compute_kernel(int iterations, int *int_arr, 
                               double *double_arr, float *float_arr,
                               struct MixedData *mixed_arr) {
    uint64_t total = 0;
    int local_counter = 0;
    
    /* Array of function pointers for computed jumps */
    ComputeFunc funcs[] = {helper1, helper2};
    
    for (int iter = 0; iter < iterations; iter++) {
        int base_idx = iter % (ARRAY_SIZE - CHASE_DEPTH - 1);
        
        /* 1. Pointer chasing with dependencies */
        int chase_result = pointer_chase(int_arr, base_idx);
        
        /* 2. Mixed data type operations */
        double dtemp = double_arr[base_idx];
        float ftemp = float_arr[base_idx];
        
        /* Chain of floating point operations with dependencies */
        for (int j = 0; j < 8; j++) {
            dtemp = dtemp * 1.1 + sin(dtemp * 0.01);
            ftemp = ftemp * 1.05f + cos(ftemp * 0.02f);
            
            /* Dependent integer operation */
            int_arr[base_idx + j] = (int)(dtemp * 1000) ^ (int)(ftemp * 1000);
        }
        
        /* 3. Switch-based computation */
        int switch_result = switch_computation(chase_result, &local_counter);
        
        /* 4. Computed jump (function pointer call) */
        int fp_result = funcs[iter & 1](switch_result, chase_result);
        
        /* 5. Packed struct operations */
        mixed_arr[base_idx].i = fp_result;
        mixed_arr[base_idx].d = dtemp;
        mixed_arr[base_idx].f = ftemp;
        mixed_arr[base_idx].c = (char)(fp_result & 0xFF);
        mixed_arr[base_idx].s = (short)((fp_result >> 8) & 0xFFFF);
        
        /* 6. Complex conditional chain */
        if (iter & 1) {
            /* Call non-inlineable function */
            int complex_result = complex_calculation(int_arr, double_arr, 
                                                    float_arr, base_idx);
            total += complex_result;
        } else if (iter & 2) {
            /* Deep arithmetic chain */
            int chain = fp_result;
            for (int k = 0; k < 12; k++) {
                chain = (chain * 3) + (chain >> 1) - (chain % 19);
                chain = chain ^ (chain << 3);
            }
            total += chain;
        } else {
            /* Memory-intensive pattern */
            for (int k = 0; k < 16; k++) {
                int_arr[(base_idx + k) % ARRAY_SIZE] = 
                    int_arr[(base_idx + k + 1) % ARRAY_SIZE] * 3 -
                    int_arr[(base_idx + k + 2) % ARRAY_SIZE];
            }
            total += int_arr[base_idx];
        }
        
        /* 7. Volatile access and barrier */
        g_volatile_counter++;
        if ((iter % 32) == 0) {
            g_volatile_barrier = total & 0xFFFF;
            asm volatile("" ::: "memory");
        }
        
        /* 8. Large basic block with independent operations */
        for (int k = 0; k < 64; k++) {
            int idx = (base_idx + k * 3) % ARRAY_SIZE;
            /* Independent operations to fill instruction queue */
            double_arr[idx] = double_arr[idx] * 0.99 + k * 0.01;
            float_arr[idx] = float_arr[idx] * 1.01f - k * 0.005f;
            if ((k & 3) == 0) {
                int_arr[idx] = int_arr[idx] ^ (k << 8);
            }
        }
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Allocate and initialize arrays with different data types */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    struct MixedData *mixed_array = 
        (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        double_array[i] = sin(i * 0.01) * 1000.0;
        float_array[i] = cos(i * 0.02f) * 500.0f;
        mixed_array[i].c = (char)(i & 0xFF);
        mixed_array[i].i = i * 3;
        mixed_array[i].d = i * 0.5;
        mixed_array[i].s = (short)(i * 7);
        mixed_array[i].f = i * 0.25f;
    }
    
    /* Create linked-list like structure in int_array */
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        int_array[i] = (int_array[i] + i + 1) % ARRAY_SIZE;
    }
    int_array[ARRAY_SIZE - 1] = 0;
    
    /* Perform main computation */
    uint64_t result = compute_kernel(iterations, int_array, double_array, 
                                    float_array, mixed_array);
    
    /* Final reduction to prevent dead code elimination */
    uint64_t final_sum = result;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum ^= int_array[i];
        final_sum += (uint64_t)(double_array[i] * 1000);
        final_sum += (uint64_t)(float_array[i] * 1000);
        final_sum += mixed_array[i].i;
    }
    
    /* Mix in volatile variables */
    final_sum ^= g_volatile_counter;
    final_sum += g_volatile_barrier;
    
    printf("Result: %lu\n", (unsigned long)final_sum);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(mixed_array);
    
    return (int)(final_sum & 0x7FFFFFFF);
}
