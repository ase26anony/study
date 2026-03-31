/* haifa_scheduler_test.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa_scheduler_test.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define CHAIN_LENGTH 32

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

/* Small helper functions that create scheduling boundaries */
static int helper_mul_chain(int a, int b) {
    int t1 = a * b;
    int t2 = t1 * 1103515245;
    int t3 = t2 ^ 12345;
    asm volatile("" ::: "memory"); /* Compiler barrier */
    return t3 & 0x7FFFFFFF;
}

static float helper_float_chain(float a, float b) {
    float t1 = a * b;
    float t2 = t1 + g_volatile_float;
    float t3 = t2 / (b + 1.0f);
    asm volatile("" ::: "memory");
    return t3;
}

static double helper_double_chain(double a, double b) {
    double t1 = a * b;
    double t2 = t1 * 1.23456789;
    double t3 = t2 - a;
    double t4 = t3 + b;
    return t4;
}

/* Non-inlineable function to create scheduling boundary */
__attribute__((noinline)) 
static int complex_reduction(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i] * (i & 0xFF);
        sum ^= (sum << 13) | (sum >> 19); /* Simple mixing */
    }
    return sum;
}

/* Another non-inlineable function */
__attribute__((noinline))
static void pointer_chasing(int *base, int steps) {
    int *ptr = base;
    for (int i = 0; i < steps; i++) {
        int idx = (*ptr) & (ARRAY_SIZE - 1);
        ptr = base + idx;
        *ptr = (*ptr * 1103515245 + 12345) & 0x7FFFFFFF;
        g_volatile_counter++;
    }
}

/* Function with switch statement creating multiple basic blocks */
static int switch_computation(int value, int mode) {
    int result = value;
    
    switch (mode % 10) {
        case 0:
            result = helper_mul_chain(result, 3);
            result += g_volatile_counter;
            break;
        case 1:
            result = result * 7 + 11;
            result ^= (result >> 16);
            break;
        case 2:
            result = (result << 5) | (result >> 27);
            result = helper_mul_chain(result, result);
            break;
        case 3:
            result = result + (result * result);
            asm volatile("" ::: "memory");
            break;
        case 4:
            result = result ^ 0xAAAAAAAA;
            result = result * 0x9E3779B9;
            break;
        case 5:
            result = (result & 0x55555555) << 1 | (result & 0xAAAAAAAA) >> 1;
            break;
        case 6:
            result = helper_mul_chain(result, 13);
            result = result - (result / 17);
            break;
        case 7:
            result = result ^ (result << 13);
            result = result ^ (result >> 17);
            result = result ^ (result << 5);
            break;
        case 8:
            result = result + g_volatile_counter * 2;
            asm volatile("" ::: "memory");
            break;
        case 9:
            result = helper_mul_chain(result, 19);
            result = result | 1;
            break;
    }
    
    return result;
}

/* Main computation with complex control flow */
static uint64_t compute_kernel(int iterations) {
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct mixed_data *mixed_array = (struct mixed_data*)malloc(ARRAY_SIZE * sizeof(struct mixed_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        float_array[i] = (float)(i * 0.001953125); /* 1/512 */
        double_array[i] = (double)(i * 0.0009765625); /* 1/1024 */
        mixed_array[i].c = (char)(i & 0xFF);
        mixed_array[i].i = i * 3;
        mixed_array[i].d = (double)i * 0.00390625; /* 1/256 */
        mixed_array[i].f = (float)i * 0.0078125; /* 1/128 */
        mixed_array[i].s = (short)(i & 0x7FFF);
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {
        helper_mul_chain,
        (compute_func_t)helper_float_chain,
        (compute_func_t)helper_double_chain
    };
    
    uint64_t accumulator = 0;
    
    /* Main computation loop with complex dependencies */
    for (int iter = 0; iter < iterations; iter++) {
        /* Chain of dependent arithmetic operations */
        int a = int_array[iter & (ARRAY_SIZE - 1)];
        int b = a * 3 + 7;
        int c = b ^ (a << 3);
        int d = helper_mul_chain(c, b);
        int e = d - a + g_volatile_counter;
        
        /* Mixed floating point operations */
        float f1 = float_array[iter & (ARRAY_SIZE - 1)];
        float f2 = helper_float_chain(f1, (float)e);
        float_array[iter & (ARRAY_SIZE - 1)] = f2;
        
        /* Double precision chain */
        double d1 = double_array[iter & (ARRAY_SIZE - 1)];
        double d2 = helper_double_chain(d1, (double)(iter + 1));
        double_array[iter & (ARRAY_SIZE - 1)] = d2;
        
        /* Pointer chasing with loop-carried dependency */
        if (iter & 1) {
            pointer_chasing(int_array, CHAIN_LENGTH);
        }
        
        /* Switch statement creating multiple control flow paths */
        int switch_result = switch_computation(e, iter);
        
        /* Computed jump (function pointer call) */
        int fp_result;
        if (iter & 2) {
            fp_result = funcs[iter % 3](switch_result, iter);
        } else {
            fp_result = switch_result;
        }
        
        /* Nested loop with data-dependent bounds */
        int inner_limit = (iter & 0xF) + 4;
        int inner_sum = 0;
        for (int j = 0; j < inner_limit; j++) {
            /* Loop-carried dependency */
            inner_sum += int_array[(iter + j) & (ARRAY_SIZE - 1)] * 
                        int_array[(iter + j + 1) & (ARRAY_SIZE - 1)];
            inner_sum ^= (inner_sum << 7) | (inner_sum >> 25);
        }
        
        /* Conditional with different computation in each branch */
        if (iter & 4) {
            /* Branch 1: More arithmetic */
            int t1 = fp_result * inner_sum;
            int t2 = t1 + (t1 >> 16);
            int t3 = t2 ^ (t2 << 9);
            accumulator += t3;
        } else {
            /* Branch 2: Memory operations */
            mixed_array[iter & (ARRAY_SIZE - 1)].i = fp_result;
            mixed_array[iter & (ARRAY_SIZE - 1)].f = (float)inner_sum;
            accumulator += mixed_array[iter & (ARRAY_SIZE - 1)].i;
        }
        
        /* Volatile access to prevent reordering */
        g_volatile_counter += (iter & 0xFF);
        asm volatile("" ::: "memory");
        
        /* Deeply nested conditional chain */
        if (iter % 3 == 0) {
            int x = accumulator & 0xFF;
            if (x < 64) {
                accumulator += x * 2;
            } else if (x < 128) {
                accumulator += x * 3;
            } else if (x < 192) {
                accumulator += x * 5;
            } else {
                accumulator += x * 7;
            }
        } else if (iter % 3 == 1) {
            accumulator = (accumulator << 3) | (accumulator >> 61);
        } else {
            accumulator ^= 0xDEADBEEFCAFEBABEULL;
        }
    }
    
    /* Final reduction across all arrays */
    int int_sum = complex_reduction(int_array, ARRAY_SIZE);
    
    double double_sum = 0.0;
    float float_sum = 0.0f;
    int mixed_sum = 0;
    
    /* Independent operations that can be scheduled in parallel */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double_sum += double_array[i];
        float_sum += float_array[i];
        mixed_sum += mixed_array[i].i + mixed_array[i].s;
        
        /* Create some dependencies to prevent vectorization */
        if (i > 0) {
            double_array[i] += double_array[i-1] * 0.1;
            float_array[i] += float_array[i-1] * 0.1f;
        }
    }
    
    /* Combine all results */
    accumulator += int_sum;
    accumulator += (uint64_t)(double_sum * 1000.0);
    accumulator += (uint64_t)(float_sum * 1000.0f);
    accumulator += mixed_sum;
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(mixed_array);
    
    return accumulator;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 1000000) iterations = 1000000;
    }
    
    printf("Running HAIFA scheduler test with %d iterations...\n", iterations);
    
    uint64_t result = compute_kernel(iterations);
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%016llX\n", (unsigned long long)result);
    
    /* Additional volatile operations to create more scheduling opportunities */
    for (int i = 0; i < 100; i++) {
        g_volatile_counter += i;
        g_volatile_float *= 1.0001f;
        asm volatile("" ::: "memory");
    }
    
    printf("Volatile counter: %d\n", g_volatile_counter);
    printf("Volatile float: %f\n", g_volatile_float);
    
    return 0;
}
