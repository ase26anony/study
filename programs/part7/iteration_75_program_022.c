/* haifa_sched_trigger.c - Complex program to trigger HAIFA scheduler state save/restore */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile variables to create scheduling hazards */
volatile int vol_counter = 0;
volatile double vol_double = 0.0;

/* Packed struct with mixed types to force alignment issues */
struct __attribute__((packed)) mixed_data {
    char c;
    int i;
    double d;
    short s;
    float f;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions with different computation patterns */
static int helper1(int a, int b) {
    asm volatile("" ::: "memory");  /* Compiler barrier */
    int result = a * b + (a >> 3) - (b << 2);
    vol_counter++;  /* Volatile access creates scheduling boundary */
    return result ^ (result >> 16);
}

static int helper2(int a, int b) {
    float temp = (float)a * 1.5f + (float)b * 0.75f;
    asm volatile("" ::: "memory");
    int result = (int)temp * 7 - a / 3;
    vol_double += 0.001;
    return result & 0xFFF;
}

static int helper3(int a, int b) {
    double d = (double)a * 3.14159 + (double)b * 2.71828;
    asm volatile("" ::: "memory");
    int result = (int)(d * 1000.0) % 1000;
    vol_counter += 2;
    return result | 0x8000;
}

/* Array of function pointers for computed jumps */
static compute_func_t func_table[] = {helper1, helper2, helper3, helper1, helper2};

/* Main computation with complex control flow and dependencies */
static int complex_computation(int *int_array, double *double_array, 
                               float *float_array, struct mixed_data *mixed,
                               int size, int iterations) {
    int result = 0;
    int i, j;
    
    /* Pointer chasing simulation */
    int *chase_ptr = int_array;
    for (i = 0; i < iterations; i++) {
        /* Deeply nested conditional chain */
        if (i & 1) {
            if (i & 2) {
                if (i & 4) {
                    if (i & 8) {
                        /* Branch 1: Pointer arithmetic with dependencies */
                        int idx = *chase_ptr % size;
                        chase_ptr = &int_array[idx];
                        result += *chase_ptr * 3;
                        result -= int_array[(idx + 1) % size];
                    } else {
                        /* Branch 2: Floating point chain */
                        double temp = double_array[i % size];
                        temp = temp * 1.2345 + sin(temp * 0.01);
                        float_array[i % size] = (float)temp;
                        result += (int)(temp * 1000.0);
                    }
                } else {
                    /* Branch 3: Mixed type operations */
                    mixed[i % size].i = result;
                    mixed[i % size].f = (float)result * 0.5f;
                    result = mixed[i % size].i ^ (int)mixed[i % size].f;
                }
            } else {
                /* Branch 4: Function pointer call */
                int func_idx = i % (sizeof(func_table)/sizeof(func_table[0]));
                result = func_table[func_idx](result, i);
            }
        } else {
            /* Branch 5: Long dependency chain */
            int a = result;
            int b = a * 2 + 1;
            int c = b / 3 - a;
            int d = c << 4;
            int e = d ^ 0xAAAA;
            int f = e * 7;
            int g = f % 100;
            result = g + i;
        }
        
        /* Switch statement with many cases */
        switch (i % 10) {
            case 0: {
                /* Case 0: Memory intensive */
                for (j = 0; j < 8; j++) {
                    int_array[(i + j) % size] += result;
                    result ^= int_array[(i + j) % size];
                }
                break;
            }
            case 1: {
                /* Case 1: Floating point intensive */
                double sum = 0.0;
                for (j = 0; j < 4; j++) {
                    sum += double_array[(i + j) % size] * j;
                }
                result += (int)(sum * 100.0);
                break;
            }
            case 2: {
                /* Case 2: Mixed operations */
                result = helper1(result, i);
                result = helper2(result, i + 1);
                break;
            }
            case 3: {
                /* Case 3: Volatile access pattern */
                result += vol_counter;
                vol_double += 0.0001 * i;
                break;
            }
            case 4: {
                /* Case 4: Pointer chasing with arithmetic */
                int idx = result % size;
                chase_ptr = &int_array[idx];
                *chase_ptr = result;
                result = *chase_ptr + i;
                break;
            }
            case 5: {
                /* Case 5: Nested loop with dependencies */
                int temp = result;
                for (j = 0; j < 3; j++) {
                    temp = temp * (j + 2) - int_array[(i + j) % size];
                }
                result = temp;
                break;
            }
            case 6: {
                /* Case 6: Floating point chain */
                float f1 = float_array[i % size];
                float f2 = f1 * 2.5f;
                float f3 = f2 - 1.0f;
                float f4 = f3 / 0.75f;
                result += (int)(f4 * 100.0f);
                break;
            }
            case 7: {
                /* Case 7: Memory barrier intensive */
                asm volatile("" ::: "memory");
                result = result * 3 + 1;
                asm volatile("" ::: "memory");
                result = result / 2 - 5;
                asm volatile("" ::: "memory");
                break;
            }
            case 8: {
                /* Case 8: Struct operations */
                mixed[i % size].c = (char)(result & 0xFF);
                mixed[i % size].s = (short)(result >> 8);
                result = mixed[i % size].c + mixed[i % size].s;
                break;
            }
            case 9: {
                /* Case 9: Complex arithmetic chain */
                int x = result;
                int y = x * x + 2 * x + 1;
                int z = y % 1000;
                int w = z * 7 - 3;
                result = (w ^ 0x5555) & 0xFFFF;
                break;
            }
        }
        
        /* Loop-carried dependency */
        if (i > 0) {
            result += int_array[(i - 1) % size] % 100;
        }
        
        /* Periodic volatile update */
        if ((i % 100) == 0) {
            vol_counter = result % 1000;
        }
    }
    
    return result;
}

/* Large basic block generator */
static void fill_arrays(int *int_arr, double *double_arr, float *float_arr,
                       struct mixed_data *mixed_arr, int size) {
    int i;
    /* Independent operations to fill instruction queue */
    for (i = 0; i < size; i++) {
        /* No dependencies between iterations */
        int_arr[i] = i * 1103515245;
        double_arr[i] = sin((double)i * 0.01) * 1000.0;
        float_arr[i] = (float)i * 1.5f;
        mixed_arr[i].c = (char)(i & 0xFF);
        mixed_arr[i].i = i * 3;
        mixed_arr[i].d = (double)i * 2.5;
        mixed_arr[i].s = (short)(i * 2);
        mixed_arr[i].f = (float)i * 0.75f;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    const int array_size = 1024;
    int final_result = 0;
    
    /* Parse iteration count from command line */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 100000) iterations = 100000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)aligned_alloc(16, array_size * sizeof(int));
    double *double_array = (double*)aligned_alloc(32, array_size * sizeof(double));
    float *float_array = (float*)malloc(array_size * sizeof(float));
    struct mixed_data *mixed_array = (struct mixed_data*)malloc(array_size * sizeof(struct mixed_data));
    
    if (!int_array || !double_array || !float_array || !mixed_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    fill_arrays(int_array, double_array, float_array, mixed_array, array_size);
    
    /* Perform complex computation that should trigger scheduler state save/restore */
    final_result = complex_computation(int_array, double_array, float_array,
                                      mixed_array, array_size, iterations);
    
    /* Reduction across arrays to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < array_size; i++) {
        sum += int_array[i];
        sum ^= (int)(double_array[i] * 100.0);
        sum += (int)(float_array[i] * 10.0f);
        sum += mixed_array[i].i;
        sum += mixed_array[i].c;
        sum += mixed_array[i].s;
    }
    
    final_result ^= sum;
    
    /* Print result to prevent optimization */
    printf("Final result: %d (iterations: %d)\n", final_result, iterations);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(mixed_array);
    
    return 0;
}
