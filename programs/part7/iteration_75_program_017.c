/* haifa_scheduler_test.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa_scheduler_test.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile float g_volatile_float = 1.0f;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) PackedData {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that create scheduling boundaries */
static int helper_mul_chain(int a, int b) {
    int t1 = a * b;
    int t2 = t1 * 1103515245;
    int t3 = t2 ^ 12345;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return t3 & 0x7FFFFFFF;
}

static float helper_float_ops(float x, float y) {
    float t1 = x * y;
    float t2 = t1 + x;
    float t3 = t2 - y;
    float t4 = t3 / (y + 1.0f);
    g_volatile_float = t4;  /* Volatile store */
    return t4;
}

static double helper_double_chain(double a, double b) {
    double sum = a;
    for (int i = 0; i < 3; i++) {
        sum = sum * b + i;
        sum = sqrt(fabs(sum) + 1.0);
    }
    return sum;
}

/* Function with many parameters to force register pressure */
static int complex_calculation(int a, int b, int c, int d, int e, 
                               int f, int g, int h, int i, int j) {
    int t1 = a * b + c;
    int t2 = d ^ e ^ f;
    int t3 = g << (h & 3);
    int t4 = i % (j + 1);
    
    if (t1 > t2) {
        t3 = t3 + t4;
    } else {
        t3 = t3 - t4;
    }
    
    switch (t3 & 7) {
        case 0: return t1 + t2;
        case 1: return t1 - t2;
        case 2: return t1 * t2;
        case 3: return t1 ^ t2;
        case 4: return t2 - t1;
        case 5: return helper_mul_chain(t1, t2);
        case 6: return t1 | t2;
        case 7: return t1 & t2;
    }
    return 0;
}

/* Main computation kernel with dense operations */
static void compute_kernel(int *int_array, float *float_array, 
                          double *double_array, struct PackedData *packed,
                          int size, int iteration) {
    /* Pointer chasing simulation */
    int idx = iteration % size;
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    
    /* Dense sequential operations with dependencies */
    for (int i = 0; i < size; i++) {
        /* Loop-carried dependency */
        idx = (idx * 1103515245 + 12345) % size;
        
        /* Mixed type computations */
        int_array[idx] = int_array[idx] * 3 + i;
        float_array[idx] = float_array[idx] * 1.5f - float_array[(idx + 1) % size];
        double_array[idx] = double_array[idx] / 2.0 + sin((double)i * 0.01);
        
        /* Packed struct access (misaligned) */
        packed[idx].i = int_array[idx];
        packed[idx].d = double_array[idx];
        packed[idx].s = (short)(int_array[idx] & 0xFFFF);
        
        /* Volatile read creates scheduling barrier */
        g_volatile_counter++;
        
        /* Deeply nested conditional chain */
        if (i & 1) {
            if (i & 2) {
                if (i & 4) {
                    int_array[idx] = helper_mul_chain(int_array[idx], i);
                } else {
                    float_array[idx] = helper_float_ops(float_array[idx], i);
                }
            } else {
                if (i & 8) {
                    double_array[idx] = helper_double_chain(double_array[idx], i);
                }
            }
        }
        
        /* Switch with many cases - each creates basic block boundaries */
        switch (i % 10) {
            case 0: {
                /* Kernel 0: Integer arithmetic chain */
                int t = int_array[idx];
                t = t * 3 + 7;
                t = t ^ 0xABCD;
                t = t >> 2;
                int_array[idx] = t;
                break;
            }
            case 1: {
                /* Kernel 1: Floating point chain */
                float f = float_array[idx];
                f = f * 1.1f + 0.5f;
                f = f - floorf(f);
                f = f * 2.0f;
                float_array[idx] = f;
                break;
            }
            case 2: {
                /* Kernel 2: Mixed operations */
                double d = double_array[idx];
                d = d * d + 1.0;
                d = sqrt(d);
                d = d * 0.5;
                double_array[idx] = d;
                break;
            }
            case 3: {
                /* Kernel 3: Memory intensive */
                for (int j = 0; j < 4; j++) {
                    int_array[(idx + j) % size] += j;
                }
                break;
            }
            case 4: {
                /* Kernel 4: Function call chain */
                int_array[idx] = complex_calculation(
                    int_array[idx], i, idx, 
                    int_array[(idx + 1) % size],
                    int_array[(idx + 2) % size],
                    iteration, g_volatile_counter,
                    packed[idx].s, idx * 3, idx * 7
                );
                break;
            }
            case 5: {
                /* Kernel 5: Trigonometric */
                double_array[idx] = sin(double_array[idx]) + cos(i * 0.1);
                break;
            }
            case 6: {
                /* Kernel 6: Bit manipulation */
                int val = int_array[idx];
                val = (val << 3) | (val >> 29);
                val = val ^ (val << 7);
                val = val & 0x7FFFFFFF;
                int_array[idx] = val;
                break;
            }
            case 7: {
                /* Kernel 7: Reduction */
                sum_int += int_array[idx];
                sum_float += float_array[idx];
                sum_double += double_array[idx];
                break;
            }
            case 8: {
                /* Kernel 8: Conditional store */
                if (int_array[idx] > 1000) {
                    float_array[idx] = float_array[idx] * 0.9f;
                } else {
                    float_array[idx] = float_array[idx] * 1.1f;
                }
                break;
            }
            case 9: {
                /* Kernel 9: Computed goto simulation via function pointer */
                compute_func_t funcs[3] = {helper_mul_chain, NULL, NULL};
                if (funcs[0]) {
                    int_array[idx] = funcs[0](int_array[idx], i);
                }
                break;
            }
        }
        
        /* Data-dependent loop continuation */
        if (int_array[idx] & 1) {
            i++;  /* Skip next iteration occasionally */
        }
    }
    
    /* Final reduction with memory barrier */
    asm volatile("" ::: "memory");
    int_array[0] += sum_int;
    float_array[0] += sum_float;
    double_array[0] += sum_double;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    const int array_size = 1024;
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)aligned_alloc(64, array_size * sizeof(int));
    float *float_array = (float*)aligned_alloc(32, array_size * sizeof(float));
    double *double_array = (double*)aligned_alloc(64, array_size * sizeof(double));
    struct PackedData *packed_array = (struct PackedData*)malloc(array_size * sizeof(struct PackedData));
    
    if (!int_array || !float_array || !double_array || !packed_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < array_size; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        float_array[i] = (float)(i * 0.001);
        double_array[i] = (double)(i * 0.0001);
        packed_array[i].c = (char)(i & 0xFF);
        packed_array[i].i = int_array[i];
        packed_array[i].d = double_array[i];
        packed_array[i].s = (short)(i & 0xFFFF);
    }
    
    /* Main computation loop - creates complex scheduling graph */
    long long total_sum = 0;
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary computation pattern based on iteration */
        if (iter & 1) {
            /* Heavy computation path */
            compute_kernel(int_array, float_array, double_array, 
                          packed_array, array_size, iter);
        } else {
            /* Lighter but still complex path */
            for (int i = 0; i < array_size; i += 8) {
                /* Unrolled block with independent operations */
                int_array[i] = int_array[i] * 2 - int_array[i+1];
                int_array[i+1] = int_array[i+1] ^ int_array[i];
                float_array[i] = float_array[i] + float_array[i+1];
                float_array[i+1] = float_array[i+1] * 0.5f;
                double_array[i] = double_array[i] / 1.5;
                double_array[i+1] = sqrt(fabs(double_array[i+1]));
                
                /* Memory barrier every 8 iterations */
                if ((i & 63) == 0) {
                    asm volatile("" ::: "memory");
                }
            }
        }
        
        /* Occasionally call helper functions to create scheduling boundaries */
        if (iter % 37 == 0) {
            g_volatile_float = helper_float_ops(g_volatile_float, iter);
        }
        
        /* Progressively modify array sizes to prevent pattern recognition */
        int effective_size = array_size - (iter % 16);
        if (effective_size < 100) effective_size = 100;
        
        /* Final reduction across arrays */
        int block_sum = 0;
        for (int i = 0; i < effective_size; i++) {
            block_sum += int_array[i];
            block_sum ^= packed_array[i].i;
        }
        total_sum += block_sum;
        
        /* Volatile operation prevents dead code elimination */
        g_volatile_counter += (iter & 255);
    }
    
    /* Final computation to use all results */
    double final_result = 0.0;
    for (int i = 0; i < array_size; i++) {
        final_result += int_array[i] * 0.001;
        final_result += float_array[i];
        final_result += double_array[i];
        final_result += packed_array[i].d;
    }
    
    /* Print result to prevent optimization */
    printf("Result: %lld, Final: %.6f, Volatile: %d\n", 
           total_sum, final_result, g_volatile_counter);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(packed_array);
    
    return 0;
}
