/* haifa-sched-coverage.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa-sched-coverage.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define MAX_DEPTH 32

/* Volatile variables to prevent optimization and create scheduling hazards */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 0.0;

/* Packed struct with mixed alignments */
struct __attribute__((packed)) mixed_data {
    char c;
    int i;
    double d;
    short s;
    float f;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int, void*);

/* Helper functions that create scheduling boundaries */
static inline int helper1(int a, int b) {
    asm volatile("" ::: "memory");  /* Compiler barrier */
    int result = (a * b) ^ (a + b);
    g_volatile_counter++;
    return result;
}

static inline double helper2(double x, double y) {
    volatile double temp = x * y;  /* Volatile access creates scheduling hazard */
    asm volatile("" ::: "memory");
    return temp + x - y;
}

static int complex_helper(int base, int offset, void* data) {
    int* arr = (int*)data;
    int sum = 0;
    
    /* Loop with carried dependency */
    for (int i = 0; i < MAX_DEPTH; i++) {
        sum = sum * 1103515245 + arr[(base + i) & (ARRAY_SIZE-1)];
        asm volatile("" ::: "memory");  /* Barrier every iteration */
    }
    
    /* Mixed floating point operations */
    double dsum = (double)sum;
    for (int i = 0; i < 8; i++) {
        dsum = helper2(dsum, 1.61803398875);  /* Golden ratio */
    }
    
    return (int)dsum ^ offset;
}

/* Non-inlineable function (due to recursion) */
__attribute__((noinline)) int recursive_compute(int n, int depth) {
    if (depth <= 0) return n;
    
    /* Create complex expression with many dependencies */
    int a = recursive_compute(n * 2, depth - 1);
    int b = recursive_compute(n / 3, depth - 1);
    
    volatile int v = a ^ b;  /* Volatile access */
    asm volatile("" ::: "memory");
    
    return (a * b + v) ^ (a - b);
}

/* Main computation with complex control flow */
static void perform_computation(int iterations, 
                               int* int_array,
                               double* double_array,
                               float* float_array,
                               struct mixed_data* mixed_array) {
    int i, j, k;
    int state = 0;
    double accumulator = 0.0;
    float f_accum = 0.0f;
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[5] = {
        (compute_func_t)helper1,
        (compute_func_t)complex_helper,
        (compute_func_t)recursive_compute,
        NULL,
        NULL
    };
    
    /* Primary computation loop */
    for (i = 0; i < iterations; i++) {
        /* Pointer chasing through array (simulated linked list) */
        int* current = &int_array[i & (ARRAY_SIZE-1)];
        int sum = 0;
        
        for (j = 0; j < 16; j++) {
            sum = sum * 31 + *current;
            current = &int_array[(*current) & (ARRAY_SIZE-1)];
            asm volatile("" ::: "memory");  /* Barrier in inner loop */
        }
        
        /* Chain of dependent arithmetic operations */
        double d1 = double_array[i & (ARRAY_SIZE-1)];
        double d2 = double_array[(i * 3) & (ARRAY_SIZE-1)];
        double d3 = d1 * d2 + (double)sum;
        double d4 = helper2(d3, d2);
        accumulator += d4 * 0.5 - d1;
        
        /* Complex switch statement with many cases */
        switch (i % 12) {
            case 0: {
                /* Large basic block with independent operations */
                float temp[8];
                for (k = 0; k < 8; k++) {
                    temp[k] = float_array[(i + k) & (ARRAY_SIZE-1)] * 1.1f;
                    f_accum += temp[k];
                }
                /* Dependent operations on the results */
                for (k = 1; k < 8; k++) {
                    temp[0] += temp[k] * (k + 1);
                }
                float_array[i & (ARRAY_SIZE-1)] = temp[0];
                break;
            }
            case 1:
                state = helper1(state, sum);
                break;
            case 2:
                state ^= complex_helper(state, i, int_array);
                break;
            case 3:
                /* Nested conditionals */
                if (i & 1) {
                    if (i & 2) {
                        state = state * 3 - 1;
                    } else {
                        state = state / 2 + 1;
                    }
                    /* Call helper in conditional branch */
                    state = recursive_compute(state, 3);
                } else if (i & 4) {
                    state = ~state;
                } else {
                    state = state ^ 0xAAAAAAAA;
                }
                break;
            case 4:
            case 5:
                /* Mixed integer/float operations */
                f_accum += (float)state * 0.25f;
                state = (int)(f_accum * 1000.0f);
                break;
            case 6:
                /* Access packed struct with misaligned data */
                struct mixed_data* m = &mixed_array[i & (ARRAY_SIZE-1)];
                state += m->i + m->c * 256;
                accumulator += m->d;
                f_accum += m->f;
                break;
            case 7:
                /* Computed goto via function pointer */
                if (funcs[0]) {
                    state = funcs[0](state, i, int_array);
                }
                break;
            case 8:
                /* Loop with complex exit condition */
                for (k = 0; k < (i & 31); k++) {
                    state = (state << 3) | (state >> 29);  /* Rotate */
                    state ^= int_array[(i + k) & (ARRAY_SIZE-1)];
                }
                break;
            case 9:
                /* Memory-intensive operations */
                memcpy(&float_array[(i + 64) & (ARRAY_SIZE-1)],
                       &float_array[i & (ARRAY_SIZE-1)],
                       sizeof(float) * 16);
                break;
            case 10:
                /* More floating point dependencies */
                accumulator = sin(accumulator) * cos(f_accum);
                break;
            case 11:
                /* Everything mixed together */
                state = state * 1103515245 + i;
                accumulator += (double)state * 0.0000001;
                f_accum = (float)accumulator;
                g_volatile_double = accumulator;  /* Volatile store */
                break;
        }
        
        /* Periodic volatile access */
        if ((i & 255) == 0) {
            g_volatile_counter = state;
            asm volatile("" ::: "memory");
        }
        
        /* Data-dependent array update */
        int_array[i & (ARRAY_SIZE-1)] = 
            (int_array[i & (ARRAY_SIZE-1)] * 13 + state) & 0x7FFFFFFF;
    }
    
    /* Final reduction */
    int final_sum = state;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_sum ^= int_array[i];
        final_sum += (int)(double_array[i] * 1000.0);
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d (accum: %f, f_accum: %f, volatile: %d)\n",
           final_sum, accumulator, f_accum, g_volatile_counter);
}

int main(int argc, char** argv) {
    int iterations = 10000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 1000000) iterations = 1000000;
    }
    
    /* Allocate arrays with different alignments and types */
    int* int_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double* double_array = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    float* float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    struct mixed_data* mixed_array = 
        (struct mixed_data*)malloc(ARRAY_SIZE * sizeof(struct mixed_data));
    
    if (!int_array || !double_array || !float_array || !mixed_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 1103515245;
        double_array[i] = (double)i * 1.23456789;
        float_array[i] = (float)i * 0.987654321f;
        
        mixed_array[i].c = (char)(i & 0xFF);
        mixed_array[i].i = int_array[i];
        mixed_array[i].d = double_array[i];
        mixed_array[i].s = (short)(i ^ 0x1234);
        mixed_array[i].f = float_array[i];
    }
    
    /* Perform the main computation */
    perform_computation(iterations, int_array, double_array, 
                       float_array, mixed_array);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(mixed_array);
    
    return 0;
}
