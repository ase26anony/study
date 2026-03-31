/* haifa_sched_coverage.c
 * Designed to trigger GCC's HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fno-omit-frame-pointer haifa_sched_coverage.c -o haifa_test
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
struct __attribute__((packed)) PackedData {
    char c;
    int i;
    double d;
    char padding[3];
};

/* Function pointer type for computed jumps */
typedef int (*ComputeFunc)(int, int);

/* Small helper functions that won't be inlined easily */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    asm volatile("" : : : "memory");  /* Compiler barrier */
    return a * b + g_volatile_counter;
}

static int __attribute__((noinline)) helper_add(int a, int b) {
    asm volatile("" : : : "memory");
    return a + b - g_volatile_counter;
}

static double __attribute__((noinline)) helper_fp(double a, double b) {
    asm volatile("" : : : "memory");
    return a * b + (double)g_volatile_counter;
}

/* Complex computation kernel with many dependencies */
static int complex_kernel(int *arr, double *darr, float *farr, 
                         struct PackedData *pdata, int idx) {
    int result = 0;
    int temp1, temp2, temp3;
    double dtemp1, dtemp2;
    float ftemp;
    
    /* Create a chain of dependent operations */
    temp1 = arr[idx] * 1103515245;
    temp2 = helper_add(temp1, idx);
    temp3 = helper_mul(temp2, arr[(idx + 1) % ARRAY_SIZE]);
    
    /* Mixed integer/float operations */
    dtemp1 = (double)temp3 * 1.234567;
    ftemp = (float)dtemp1 + farr[idx];
    dtemp2 = helper_fp(dtemp1, (double)ftemp);
    
    /* Pointer chasing through array */
    int next_idx = arr[idx] % ARRAY_SIZE;
    if (next_idx < 0) next_idx = -next_idx;
    
    /* Access packed struct (misaligned) */
    result = temp3 + pdata[idx % MAX_DEPTH].i + (int)dtemp2;
    
    /* Volatile access creates scheduling barrier */
    g_volatile_counter++;
    
    return result;
}

/* Different computation patterns for switch statement */
static int compute_pattern_0(int a, int b) { return a * b + a - b; }
static int compute_pattern_1(int a, int b) { return (a << 3) | (b & 0xFF); }
static int compute_pattern_2(int a, int b) { return a ^ b ^ 0xDEADBEEF; }
static int compute_pattern_3(int a, int b) { return a % (b + 1) + a; }
static int compute_pattern_4(int a, int b) { return (a + b) * (a - b); }
static int compute_pattern_5(int a, int b) { return a & ~b; }
static int compute_pattern_6(int a, int b) { return a | b; }
static int compute_pattern_7(int a, int b) { return a + (b << 1); }
static int compute_pattern_8(int a, int b) { return a * 3 + b * 7; }
static int compute_pattern_9(int a, int b) { return a ^ (b << 16); }

/* Array of function pointers for computed jumps */
static ComputeFunc pattern_funcs[10] = {
    compute_pattern_0,
    compute_pattern_1,
    compute_pattern_2,
    compute_pattern_3,
    compute_pattern_4,
    compute_pattern_5,
    compute_pattern_6,
    compute_pattern_7,
    compute_pattern_8,
    compute_pattern_9
};

/* Main computation with complex control flow */
static uint64_t compute(int iterations) {
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    struct PackedData *packed_array = (struct PackedData*)malloc(MAX_DEPTH * sizeof(struct PackedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245) % 1000;
        double_array[i] = (double)(i * 1103515245) / 1000000.0;
        float_array[i] = (float)(i * 1103515245) / 500000.0f;
    }
    
    for (int i = 0; i < MAX_DEPTH; i++) {
        packed_array[i].c = (char)(i % 256);
        packed_array[i].i = i * 1234567;
        packed_array[i].d = (double)i / 3.14159;
    }
    
    uint64_t accumulator = 0;
    int branch_counter = 0;
    
    /* Main computation loop with complex scheduling requirements */
    for (int iter = 0; iter < iterations; iter++) {
        int idx = iter % ARRAY_SIZE;
        
        /* Deeply nested conditional chain */
        if (iter & 1) {
            if (iter & 2) {
                if (iter & 4) {
                    if (iter & 8) {
                        /* Complex computation in deepest branch */
                        int result = complex_kernel(int_array, double_array, 
                                                  float_array, packed_array, idx);
                        accumulator += result;
                    } else {
                        /* Alternative computation path */
                        accumulator ^= int_array[idx] * 3;
                    }
                } else {
                    /* Call helper function */
                    accumulator += helper_add(int_array[idx], idx);
                }
            } else {
                /* Mixed operations */
                double temp = double_array[idx] * 2.71828;
                accumulator += (uint64_t)temp;
            }
        } else {
            /* Different path */
            accumulator -= int_array[idx];
        }
        
        /* Switch statement with many cases - each creates scheduling boundaries */
        switch (iter % 10) {
            case 0: {
                int a = int_array[(idx + 0) % ARRAY_SIZE];
                int b = int_array[(idx + 1) % ARRAY_SIZE];
                accumulator += pattern_funcs[0](a, b);
                /* Memory barrier */
                asm volatile("" : : : "memory");
                break;
            }
            case 1: {
                int a = int_array[(idx + 2) % ARRAY_SIZE];
                int b = int_array[(idx + 3) % ARRAY_SIZE];
                accumulator += pattern_funcs[1](a, b);
                g_volatile_barrier = a;
                break;
            }
            case 2: {
                int a = int_array[(idx + 4) % ARRAY_SIZE];
                int b = int_array[(idx + 5) % ARRAY_SIZE];
                accumulator += pattern_funcs[2](a, b);
                break;
            }
            case 3: {
                int a = int_array[(idx + 6) % ARRAY_SIZE];
                int b = int_array[(idx + 7) % ARRAY_SIZE];
                accumulator += pattern_funcs[3](a, b);
                asm volatile("" : : : "memory");
                break;
            }
            case 4: {
                int a = int_array[(idx + 8) % ARRAY_SIZE];
                int b = int_array[(idx + 9) % ARRAY_SIZE];
                accumulator += pattern_funcs[4](a, b);
                break;
            }
            case 5: {
                int a = int_array[(idx + 10) % ARRAY_SIZE];
                int b = int_array[(idx + 11) % ARRAY_SIZE];
                accumulator += pattern_funcs[5](a, b);
                g_volatile_barrier = b;
                break;
            }
            case 6: {
                int a = int_array[(idx + 12) % ARRAY_SIZE];
                int b = int_array[(idx + 13) % ARRAY_SIZE];
                accumulator += pattern_funcs[6](a, b);
                break;
            }
            case 7: {
                int a = int_array[(idx + 14) % ARRAY_SIZE];
                int b = int_array[(idx + 15) % ARRAY_SIZE];
                accumulator += pattern_funcs[7](a, b);
                asm volatile("" : : : "memory");
                break;
            }
            case 8: {
                int a = int_array[(idx + 16) % ARRAY_SIZE];
                int b = int_array[(idx + 17) % ARRAY_SIZE];
                accumulator += pattern_funcs[8](a, b);
                break;
            }
            case 9: {
                int a = int_array[(idx + 18) % ARRAY_SIZE];
                int b = int_array[(idx + 19) % ARRAY_SIZE];
                accumulator += pattern_funcs[9](a, b);
                g_volatile_barrier = a + b;
                break;
            }
        }
        
        /* Large basic block with many independent instructions */
        /* This fills the instruction queue */
        {
            int temp_arr[16];
            for (int i = 0; i < 16; i++) {
                temp_arr[i] = int_array[(idx + i) % ARRAY_SIZE] + iter;
            }
            
            /* Chain of operations with loop-carried dependency */
            int chain_result = temp_arr[0];
            for (int i = 1; i < 16; i++) {
                chain_result = chain_result * 3 + temp_arr[i];
            }
            accumulator += chain_result;
        }
        
        /* Computed goto simulation via function pointer */
        if (iter % 7 == 0) {
            ComputeFunc f = pattern_funcs[iter % 10];
            int a = int_array[idx];
            int b = int_array[(idx + 5) % ARRAY_SIZE];
            accumulator += f(a, b);
        }
        
        /* Update volatile variable - creates scheduling hazard */
        if (iter % 13 == 0) {
            g_volatile_counter = (g_volatile_counter + 1) % 100;
        }
        
        branch_counter++;
    }
    
    /* Reduction across arrays */
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        accumulator ^= int_array[i];
        accumulator += (uint64_t)(double_array[i] * 1000.0);
        accumulator ^= (uint64_t)(float_array[i] * 100.0f);
    }
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(packed_array);
    
    return accumulator;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000;
        }
    }
    
    printf("Running HAIFA scheduler test with %d iterations...\n", iterations);
    
    uint64_t result = compute(iterations);
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%016llx\n", (unsigned long long)result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    printf("Volatile barrier: %d\n", g_volatile_barrier);
    
    return 0;
}
