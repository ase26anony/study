/* haifa-sched-test.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fno-omit-frame-pointer haifa-sched-test.c -o haifa-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define MAX_DEPTH 32

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_barrier = 0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    double d;
    char pad[3];
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Various computation kernels */
static int kernel_add_chain(int a, int b) {
    int t1 = a + b;
    int t2 = t1 * 3;
    int t3 = t2 - b;
    int t4 = t3 / 2;
    int t5 = t4 ^ a;
    return t5;
}

static int kernel_mul_chain(int a, int b) {
    int t1 = a * b;
    int t2 = t1 << 2;
    int t3 = t2 >> 1;
    int t4 = t3 | 0xFF;
    int t5 = t4 & 0x7F;
    return t5;
}

static int kernel_mixed(int a, int b) {
    float f1 = (float)a * 1.5f;
    float f2 = (float)b * 2.5f;
    float f3 = f1 + f2;
    float f4 = f3 * 0.75f;
    return (int)f4;
}

/* Helper function with many dependencies */
static int complex_helper(int *arr, int idx, int depth) {
    if (depth >= MAX_DEPTH) return arr[idx];
    
    int a = arr[idx];
    int b = arr[(idx * 13 + 17) % ARRAY_SIZE];
    int c = arr[(idx * 7 + 23) % ARRAY_SIZE];
    
    /* Chain of dependent operations */
    int r1 = a * b + c;
    int r2 = r1 ^ (a << 3);
    int r3 = r2 | (b >> 2);
    int r4 = r3 & 0x3FFFFFFF;
    int r5 = r4 * 3 - 7;
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    /* Recursive call with different index */
    int next_idx = (idx + r5) % ARRAY_SIZE;
    int rec_result = complex_helper(arr, next_idx, depth + 1);
    
    return r5 + rec_result;
}

/* Another helper with switch statement */
static int switch_helper(int val, int *arr) {
    int result = val;
    
    switch (val % 8) {
        case 0:
            result = arr[val % ARRAY_SIZE] * 2;
            break;
        case 1:
            result = arr[(val + 1) % ARRAY_SIZE] + arr[(val + 2) % ARRAY_SIZE];
            break;
        case 2:
            result = arr[(val * 3) % ARRAY_SIZE] ^ arr[(val * 5) % ARRAY_SIZE];
            break;
        case 3:
            result = (arr[val % ARRAY_SIZE] << 4) | (arr[(val + 3) % ARRAY_SIZE] & 0xF);
            break;
        case 4:
            result = complex_helper(arr, val % ARRAY_SIZE, 0);
            break;
        case 5:
            result = kernel_add_chain(val, arr[val % ARRAY_SIZE]);
            break;
        case 6:
            result = kernel_mul_chain(val, arr[(val + 7) % ARRAY_SIZE]);
            break;
        case 7:
            result = kernel_mixed(val, arr[(val * 11) % ARRAY_SIZE]);
            break;
    }
    
    return result;
}

/* Main computation with complex control flow */
static uint64_t compute_heavy(int iterations, int *int_arr, float *float_arr, 
                              double *double_arr, struct misaligned_data *struct_arr) {
    uint64_t accumulator = 0;
    int i, j;
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {
        kernel_add_chain,
        kernel_mul_chain,
        kernel_mixed
    };
    
    for (i = 0; i < iterations; i++) {
        /* Pointer chasing through int array */
        int idx = i % ARRAY_SIZE;
        int steps = 0;
        int chase_sum = 0;
        
        while (steps < 16) {
            chase_sum += int_arr[idx];
            idx = (idx * 17 + int_arr[idx]) % ARRAY_SIZE;
            steps++;
            
            /* Volatile access creates scheduling barrier */
            g_volatile_counter++;
        }
        
        /* Complex chain of dependent arithmetic */
        int a = chase_sum;
        int b = int_arr[i % ARRAY_SIZE];
        float c = float_arr[(i * 3) % ARRAY_SIZE];
        double d = double_arr[(i * 5) % ARRAY_SIZE];
        
        /* Mixed type computations */
        float f1 = (float)a * c;
        double d1 = (double)b * d;
        int t1 = (int)(f1 * 100.0f);
        int t2 = (int)(d1 * 1000.0);
        int t3 = t1 + t2;
        int t4 = t3 * 3 - 7;
        int t5 = t4 / 2 + (t4 % 13);
        
        /* Deep conditional chain */
        if (i & 1) {
            t5 = complex_helper(int_arr, i % ARRAY_SIZE, 0);
            if (i & 2) {
                t5 ^= switch_helper(t5, int_arr);
                if (i & 4) {
                    t5 += kernel_mixed(t5, int_arr[(i + 1) % ARRAY_SIZE]);
                    if (i & 8) {
                        t5 *= kernel_add_chain(t5, int_arr[(i + 2) % ARRAY_SIZE]);
                    }
                }
            }
        } else if (i & 16) {
            /* Computed goto via function pointer */
            compute_func_t f = funcs[i % 3];
            t5 = f(t5, int_arr[(i * 7) % ARRAY_SIZE]);
        } else {
            t5 = switch_helper(t5, int_arr);
        }
        
        /* Access misaligned struct */
        struct misaligned_data *s = &struct_arr[i % ARRAY_SIZE];
        t5 += s->i;
        t5 ^= (int)s->d;
        
        /* Large basic block with many independent operations */
        for (j = 0; j < 32; j++) {
            int offset = (i + j) % ARRAY_SIZE;
            float_arr[offset] = float_arr[offset] * 1.01f + (float)j;
            double_arr[offset] = double_arr[offset] * 0.99 - (double)j;
            
            /* Independent but mixed operations */
            if (j & 1) {
                int_arr[offset] = int_arr[offset] + t5;
            } else {
                int_arr[offset] = int_arr[offset] ^ t5;
            }
        }
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Update accumulator */
        accumulator = (accumulator * 6364136223846793005ULL) + (uint64_t)t5;
        accumulator ^= (uint64_t)chase_sum;
        
        /* Another volatile access */
        g_volatile_barrier = i;
    }
    
    return accumulator;
}

int main(int argc, char **argv) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Allocate and initialize arrays with different types */
    int *int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct misaligned_data *struct_arr = 
        (struct misaligned_data*)malloc(ARRAY_SIZE * sizeof(struct misaligned_data));
    
    if (!int_arr || !float_arr || !double_arr || !struct_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Simple pseudo-random sequence */
        int seed = i * 1103515245 + 12345;
        
        int_arr[i] = seed;
        float_arr[i] = (float)seed / 1000.0f;
        double_arr[i] = (double)seed / 10000.0;
        
        struct_arr[i].c = (char)(seed & 0xFF);
        struct_arr[i].i = seed ^ 0xAAAAAAAA;
        struct_arr[i].d = (double)seed * 1.23456789;
    }
    
    printf("Starting complex computation with %d iterations...\n", iterations);
    
    /* Perform the heavy computation */
    uint64_t result = compute_heavy(iterations, int_arr, float_arr, double_arr, struct_arr);
    
    /* Final reduction across arrays */
    uint64_t final_check = result;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_check ^= (uint64_t)int_arr[i];
        final_check += (uint64_t)(float_arr[i] * 1000.0f);
        final_check ^= (uint64_t)(double_arr[i] * 10000.0);
        final_check += struct_arr[i].i;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: 0x%016llx\n", (unsigned long long)final_check);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    
    return 0;
}
