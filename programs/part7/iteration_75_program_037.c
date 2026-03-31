/* haifa_sched_trigger.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -funroll-loops haifa_sched_trigger.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define LINKED_LIST_SIZE 512

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile float g_volatile_float = 0.0f;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    double d;
    char c2;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that create scheduling boundaries */
static int helper1(int a, int b) {
    volatile int barrier;
    barrier = a * b;
    return barrier + (a ^ b);
}

static int helper2(int a, int b) {
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return (a << 3) | (b >> 2);
}

static float helper3(float a, float b) {
    volatile float temp = a * b;
    return temp / (a + b + 1.0f);
}

/* Non-inlineable function (due to complexity) */
__attribute__((noinline)) 
int complex_calculation(int *arr, int idx, float *farr, double *darr) {
    int result = 0;
    double acc = 0.0;
    
    /* Mixed data type operations */
    for (int i = 0; i < 8; i++) {
        result += arr[(idx + i) % ARRAY_SIZE];
        acc += darr[(idx + i) % ARRAY_SIZE];
        farr[(idx + i) % ARRAY_SIZE] = (float)acc * 0.5f;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Chain of dependent operations */
    double t1 = acc * 1.234567;
    float t2 = (float)t1 * 2.71828f;
    int t3 = (int)t2 ^ result;
    t3 = t3 * 1103515245 + 12345;
    
    g_volatile_counter += t3 & 0xFF;
    return t3;
}

/* Function with switch statement creating multiple basic blocks */
static int switch_computation(int val, int *arr, float *farr) {
    int result = val;
    
    switch (val % 10) {
        case 0:
            result = arr[val % ARRAY_SIZE] * 3;
            farr[val % ARRAY_SIZE] = result * 0.25f;
            break;
        case 1:
            result = helper1(val, arr[val % ARRAY_SIZE]);
            result ^= 0xAAAAAAAA;
            break;
        case 2:
            result = (result << 4) | (result >> 28);
            result += arr[(val + 1) % ARRAY_SIZE];
            break;
        case 3:
            result = complex_calculation(arr, val, farr, NULL);
            result = result * 7 - 13;
            break;
        case 4:
            result = arr[val % ARRAY_SIZE] + arr[(val + 2) % ARRAY_SIZE];
            result = result * result;
            break;
        case 5:
            result = helper2(val, arr[val % ARRAY_SIZE]);
            result = ~result;
            break;
        case 6:
            result = (int)(sin(val * 0.01) * 1000.0);
            result += arr[val % ARRAY_SIZE];
            break;
        case 7:
            result = val * val * val;
            result = result % 10007;
            break;
        case 8:
            result = arr[val % ARRAY_SIZE] ^ arr[(val + 3) % ARRAY_SIZE];
            result = result << 1;
            break;
        case 9:
            result = helper1(val, helper2(val, arr[val % ARRAY_SIZE]));
            result = abs(result);
            break;
        default:
            result = 1;
    }
    
    /* Another memory barrier */
    asm volatile("" ::: "memory");
    return result;
}

/* Pointer chasing through array simulating linked list */
static int pointer_chase(int *next_indices, int *values, int start_idx) {
    int idx = start_idx;
    int sum = 0;
    int steps = 0;
    
    while (steps < 50 && idx >= 0 && idx < LINKED_LIST_SIZE) {
        sum += values[idx];
        idx = next_indices[idx];
        steps++;
        
        /* Loop-carried dependency */
        sum = sum * 6364136223846793005ULL + 1442695040888963407ULL;
        
        /* Volatile access to prevent optimization */
        g_volatile_float = sum * 0.001f;
    }
    
    return sum;
}

/* Main computation with nested loops and complex control flow */
static uint64_t compute_kernel(int iterations, int *int_arr, float *float_arr, 
                               double *double_arr, struct misaligned_data *mdata) {
    uint64_t total = 0;
    int next_indices[LINKED_LIST_SIZE];
    int list_values[LINKED_LIST_SIZE];
    
    /* Initialize linked list structure */
    for (int i = 0; i < LINKED_LIST_SIZE; i++) {
        next_indices[i] = (i * 1103515245 + 12345) % LINKED_LIST_SIZE;
        list_values[i] = (i * 6364136223846793005ULL) & 0x7FFFFFFF;
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {helper1, helper2, NULL};
    
    /* Main computation loop */
    for (int iter = 0; iter < iterations; iter++) {
        int base = iter % ARRAY_SIZE;
        
        /* Deeply nested conditional chain */
        if (iter & 1) {
            if (iter & 2) {
                if (iter & 4) {
                    if (iter & 8) {
                        /* Complex arithmetic chain with dependencies */
                        int a = int_arr[base];
                        int b = int_arr[(base + 1) % ARRAY_SIZE];
                        int c = a * b + iter;
                        int d = c ^ (a << 3);
                        int e = d * 7 - 13;
                        int f = e / (abs(b) + 1);
                        total += f;
                        
                        /* Mixed floating point operations */
                        float fa = float_arr[base];
                        float fb = float_arr[(base + 2) % ARRAY_SIZE];
                        float fc = fa * fb * 1.41421356f;
                        float_arr[base] = helper3(fa, fc);
                    } else {
                        /* Pointer chasing */
                        total += pointer_chase(next_indices, list_values, iter % LINKED_LIST_SIZE);
                    }
                } else {
                    /* Switch statement with many cases */
                    total += switch_computation(iter, int_arr, float_arr);
                }
            } else {
                /* Computed jump via function pointer */
                if (funcs[iter % 2]) {
                    total += funcs[iter % 2](int_arr[base], iter);
                }
            }
        } else {
            /* Large basic block with many independent operations */
            for (int j = 0; j < 32; j++) {
                int idx = (base + j) % ARRAY_SIZE;
                int_arr[idx] = int_arr[idx] * 3 + j;
                float_arr[idx] = float_arr[idx] * 1.5f - 0.25f;
                double_arr[idx] = double_arr[idx] * 0.999 + sin(j * 0.1);
                
                /* Access misaligned struct */
                mdata[idx % 100].i = int_arr[idx];
                mdata[idx % 100].d = double_arr[idx];
            }
        }
        
        /* Loop-carried dependency with volatile */
        g_volatile_counter++;
        total = total ^ (g_volatile_counter * 0x5DEECE66DULL);
        
        /* Conditional function call */
        if (iter % 7 == 0) {
            total += complex_calculation(int_arr, iter % ARRAY_SIZE, float_arr, double_arr);
        }
        
        /* Another level of nesting */
        if (iter % 13 == 0) {
            for (int k = 0; k < 5; k++) {
                int t = int_arr[(iter + k) % ARRAY_SIZE];
                for (int m = 0; m < 3; m++) {
                    t = t * 1664525 + 1013904223;
                    total += t & 0xFF;
                }
                float_arr[(iter + k) % ARRAY_SIZE] = t * 0.001f;
            }
        }
    }
    
    return total;
}

int main(int argc, char **argv) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Allocate and initialize arrays with different data types */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct misaligned_data *mdata = (struct misaligned_data*)malloc(100 * sizeof(struct misaligned_data));
    
    if (!int_array || !float_array || !double_array || !mdata) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        float_array[i] = (float)(i * 6364136223846793005ULL) * 0.0001f;
        double_array[i] = (double)(i * 1664525 + 1013904223) * 0.000001;
    }
    
    for (int i = 0; i < 100; i++) {
        mdata[i].c = i & 0xFF;
        mdata[i].i = int_array[i % ARRAY_SIZE];
        mdata[i].d = double_array[i % ARRAY_SIZE];
        mdata[i].c2 = (i * 3) & 0xFF;
    }
    
    printf("Starting computation with %d iterations...\n", iterations);
    
    /* Perform the main computation */
    uint64_t result = compute_kernel(iterations, int_array, float_array, double_array, mdata);
    
    /* Final reduction across arrays to prevent dead code elimination */
    uint64_t final_check = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_check ^= int_array[i];
        final_check += (uint64_t)(float_array[i] * 1000.0f);
        final_check ^= *(uint64_t*)&double_array[i];
    }
    
    for (int i = 0; i < 100; i++) {
        final_check += mdata[i].i;
        final_check ^= *(uint64_t*)&mdata[i].d;
    }
    
    final_check += result;
    final_check += g_volatile_counter;
    final_check ^= (uint64_t)(g_volatile_float * 1000000.0f);
    
    printf("Result: 0x%016llX\n", (unsigned long long)final_check);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(mdata);
    
    return 0;
}
