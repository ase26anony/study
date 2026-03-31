/* haifa_sched_trigger.c
 * Program designed to trigger GCC's HAIFA scheduler state save/restore
 * and exercise the free_state function uncovered lines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile int g_memory_barrier = 0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*ComputeFunc)(int, int);

/* Small helper functions that create scheduling boundaries */
static int helper1(int a, int b) {
    volatile int barrier = g_memory_barrier;
    int result = a * b + (barrier & 1);
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return result ^ (result >> 16);
}

static int helper2(int a, int b) {
    volatile int barrier = g_memory_barrier;
    int result = a + b * 3 - (barrier & 2);
    asm volatile("" ::: "memory");
    return result | (result << 8);
}

static float helper3(float a, float b) {
    volatile int barrier = g_memory_barrier;
    float result = a * b + (float)(barrier & 3);
    asm volatile("" ::: "memory");
    return result / 2.0f;
}

/* Non-inlineable function to force scheduling boundary */
__attribute__((noinline)) 
static double complex_calculation(double *arr, int idx) {
    double sum = 0.0;
    for (int j = 0; j < 4; j++) {
        sum += arr[(idx + j) % 256] * arr[(idx - j + 256) % 256];
        asm volatile("" ::: "memory");  /* Prevent optimization */
    }
    return sum;
}

/* Main computation with complex control flow */
static int compute_kernel(int *int_arr, float *float_arr, double *double_arr, 
                         struct MixedData *mixed_arr, int iterations) {
    int result = 0;
    int *chase_ptr = int_arr;
    
    /* Pointer chasing with loop-carried dependency */
    for (int i = 0; i < iterations; i++) {
        int chase_index = 0;
        
        /* Pointer chasing through array (simulated linked list) */
        for (int j = 0; j < 8; j++) {
            chase_index = *chase_ptr;
            chase_ptr = &int_arr[chase_index % 256];
            g_volatile_counter++;  /* Volatile access creates hazard */
        }
        
        /* Deeply nested conditional chain */
        if (i & 1) {
            if (i & 2) {
                if (i & 4) {
                    /* Chain of dependent arithmetic operations */
                    int a = int_arr[i % 256];
                    int b = a * 3 + chase_index;
                    int c = b / 2 - int_arr[(i + 1) % 256];
                    int d = c ^ (c << 3);
                    result += d;
                    
                    /* Mixed floating point operations */
                    float f1 = float_arr[i % 256];
                    float f2 = f1 * 2.5f + (float)(i % 100);
                    float_arr[(i + 128) % 256] = f2;
                } else {
                    /* Different computation path */
                    double d1 = double_arr[i % 256];
                    double d2 = complex_calculation(double_arr, i);
                    double_arr[(i + 64) % 256] = d1 * 0.5 + d2;
                }
            } else {
                /* Call helper function */
                result += helper1(i, result);
            }
        } else {
            /* Switch statement with many cases */
            switch (i % 10) {
                case 0: {
                    int temp = int_arr[i % 256] * 7;
                    int_arr[(i + 32) % 256] = temp;
                    result ^= temp;
                    break;
                }
                case 1: {
                    float temp = float_arr[i % 256] / 3.0f;
                    float_arr[(i + 96) % 256] = temp;
                    result += (int)temp;
                    break;
                }
                case 2: {
                    double temp = double_arr[i % 256] * 1.5;
                    double_arr[(i + 192) % 256] = temp;
                    result -= (int)temp;
                    break;
                }
                case 3: {
                    struct MixedData *m = &mixed_arr[i % 128];
                    m->i = result;
                    m->d = (double)result / 256.0;
                    result = m->i ^ (int)m->d;
                    break;
                }
                case 4:
                case 5:
                case 6: {
                    /* Multiple instructions with dependencies */
                    int a = result * 2;
                    int b = a + int_arr[(i + 16) % 256];
                    int c = b - helper2(a, b);
                    result = c;
                    break;
                }
                case 7: {
                    /* Memory-intensive operations */
                    for (int k = 0; k < 16; k++) {
                        int_arr[(i + k) % 256] += k;
                        float_arr[(i + k) % 256] *= 1.01f;
                    }
                    break;
                }
                case 8: {
                    /* Computed jump via function pointer */
                    ComputeFunc funcs[] = {helper1, helper2};
                    int idx = (i >> 2) & 1;
                    result = funcs[idx](result, i);
                    break;
                }
                case 9: {
                    /* Large basic block with independent operations */
                    int_arr[0] = i; int_arr[1] = i*2; int_arr[2] = i*3;
                    int_arr[3] = i*4; int_arr[4] = i*5; int_arr[5] = i*6;
                    int_arr[6] = i*7; int_arr[7] = i*8; int_arr[8] = i*9;
                    int_arr[9] = i*10; int_arr[10] = i*11; int_arr[11] = i*12;
                    float_arr[0] = i*0.1f; float_arr[1] = i*0.2f;
                    float_arr[2] = i*0.3f; float_arr[3] = i*0.4f;
                    result += int_arr[i % 12];
                    break;
                }
            }
        }
        
        /* Loop-carried dependency with data-dependent condition */
        if (result > 1000000) {
            result >>= 1;
        } else if (result < -1000000) {
            result <<= 1;
        } else {
            result += (i % 256);
        }
        
        /* Periodic memory barrier */
        if ((i % 64) == 0) {
            asm volatile("" ::: "memory");
            g_memory_barrier = result;
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_arr = (int*)aligned_alloc(64, 256 * sizeof(int));
    float *float_arr = (float*)aligned_alloc(32, 256 * sizeof(float));
    double *double_arr = (double*)aligned_alloc(64, 256 * sizeof(double));
    struct MixedData *mixed_arr = (struct MixedData*)malloc(128 * sizeof(struct MixedData));
    
    if (!int_arr || !float_arr || !double_arr || !mixed_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        int_arr[i] = (i * 1103515245) & 0x7FFFFFFF;
        float_arr[i] = (float)(int_arr[i] % 1000) / 3.0f;
        double_arr[i] = (double)(int_arr[i] % 2000) / 7.0;
        if (i < 128) {
            mixed_arr[i].c = (char)(i % 256);
            mixed_arr[i].i = int_arr[i];
            mixed_arr[i].d = double_arr[i];
            mixed_arr[i].s = (short)(i * 3);
        }
    }
    
    /* Set up pointer chasing pattern */
    for (int i = 0; i < 256; i++) {
        int_arr[i] = (i + 37) % 256;  /* Simple chase pattern */
    }
    
    /* Main computation with complex scheduling requirements */
    int final_result = compute_kernel(int_arr, float_arr, double_arr, 
                                     mixed_arr, iterations);
    
    /* Reduction across arrays to prevent dead code elimination */
    int reduction = 0;
    for (int i = 0; i < 256; i++) {
        reduction ^= int_arr[i];
        reduction += (int)float_arr[i];
        reduction ^= (int)double_arr[i];
        if (i < 128) {
            reduction += mixed_arr[i].i;
            reduction ^= (int)mixed_arr[i].d;
        }
    }
    
    final_result ^= reduction;
    
    /* Print result to prevent optimization */
    printf("Result: %d (iterations: %d)\n", final_result, iterations);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(mixed_arr);
    
    return 0;
}
