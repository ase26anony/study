/* haifa-sched-trigger.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fno-omit-frame-pointer haifa-sched-trigger.c -o haifa-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define MAX_DEPTH 32

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile float g_volatile_float = 0.0f;

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
static int helper1(int a, int b) {
    volatile int barrier;
    barrier = a * b;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return barrier + (b >> 3);
}

static int helper2(int a, int b) {
    volatile int result = 0;
    for (int i = 0; i < 4; i++) {
        result += (a << i) - (b >> i);
    }
    asm volatile("" ::: "memory");
    return result;
}

static float helper3(float a, float b) {
    volatile float temp = a;
    for (int i = 0; i < 3; i++) {
        temp = temp * b + i;
    }
    return temp;
}

/* Non-inlineable function to create scheduling region boundaries */
__attribute__((noinline)) 
static double process_chunk(double *arr, int start, int end) {
    double sum = 0.0;
    volatile double acc = 0.0;
    
    /* Pointer chasing with mixed strides */
    for (int i = start; i < end; i++) {
        double *ptr = &arr[i];
        for (int j = 0; j < 4; j++) {
            acc += *ptr;
            ptr = &arr[(i + j * 7) % ARRAY_SIZE];  /* Non-linear access */
        }
        sum += acc;
        acc = acc * 0.99;  /* Create dependency chain */
    }
    
    asm volatile("" : "+r"(sum) : : "memory");
    return sum;
}

/* Complex conditional chain */
static int conditional_chain(int x, int *arr, struct mixed_data *md) {
    int result = 0;
    volatile int v = x;
    
    if (v & 1) {
        result = helper1(arr[v % ARRAY_SIZE], v);
        md->i = result;
    } else if (v & 2) {
        result = helper2(arr[(v * 3) % ARRAY_SIZE], v);
        md->f = (float)result;
    } else if (v & 4) {
        result = arr[v % ARRAY_SIZE] * 7;
        md->d = (double)result;
    } else if (v & 8) {
        float ftemp = helper3((float)v, (float)arr[v % ARRAY_SIZE]);
        result = (int)ftemp;
        md->c = (char)(result & 0xFF);
    } else {
        result = v * 11;
        md->s = (short)result;
    }
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    g_volatile_counter++;
    
    return result;
}

/* Switch-based computation kernel */
static int switch_kernel(int idx, int *int_arr, float *float_arr, double *double_arr) {
    int result = 0;
    volatile int local_barrier = idx;
    
    switch (idx % 10) {
        case 0:
            /* Long dependency chain */
            result = int_arr[idx] * 3;
            result = result + int_arr[(idx + 1) % ARRAY_SIZE];
            result = result / ((idx & 0xF) + 1);
            result = result - int_arr[(idx * 2) % ARRAY_SIZE];
            result = result ^ 0x5A5A5A5A;
            break;
            
        case 1:
            /* Mixed float/int operations */
            result = (int)(float_arr[idx] * 100.0f);
            result = result << (idx & 0x7);
            break;
            
        case 2:
            /* Pointer arithmetic chain */
            {
                int *ptr = &int_arr[idx];
                for (int i = 0; i < 8; i++) {
                    result += *ptr;
                    ptr = &int_arr[(idx + i * 13) % ARRAY_SIZE];
                }
            }
            break;
            
        case 3:
            /* Double precision computation */
            result = (int)(double_arr[idx] * 50.0);
            result = result | (idx << 16);
            break;
            
        case 4:
            /* Nested loop with carry dependency */
            for (int i = 0; i < 6; i++) {
                result += int_arr[(idx + i) % ARRAY_SIZE] * int_arr[(idx + i + 1) % ARRAY_SIZE];
            }
            break;
            
        case 5:
            /* Conditional within switch */
            if (idx & 1) {
                result = helper1(int_arr[idx], idx);
            } else {
                result = helper2(int_arr[ARRAY_SIZE - idx - 1], idx);
            }
            break;
            
        case 6:
            /* Memory intensive */
            for (int i = 0; i < 4; i++) {
                int_arr[(idx + i * 7) % ARRAY_SIZE] = result;
                result = int_arr[(idx + i * 11) % ARRAY_SIZE];
            }
            break;
            
        case 7:
            /* Float to int conversion chain */
            {
                float temp = float_arr[idx];
                for (int i = 0; i < 5; i++) {
                    temp = temp * 1.1f + float_arr[(idx + i) % ARRAY_SIZE];
                }
                result = (int)temp;
            }
            break;
            
        case 8:
            /* Bit manipulation */
            result = idx;
            for (int i = 0; i < 12; i++) {
                result = ((result << 3) | (result >> 29)) ^ 0x9E3779B9;
            }
            break;
            
        case 9:
            /* Complex expression */
            result = (int_arr[idx] * 3 + int_arr[(idx + 5) % ARRAY_SIZE] * 7) /
                     ((idx & 0x1F) + 1) - int_arr[(idx * 3) % ARRAY_SIZE];
            break;
    }
    
    local_barrier = result;
    asm volatile("" : "+r"(local_barrier) : : "memory");
    
    return result;
}

/* Main computation with dense instruction mix */
static uint64_t complex_computation(int iterations, int *int_arr, 
                                   float *float_arr, double *double_arr,
                                   struct mixed_data *md_arr) {
    uint64_t final_result = 0;
    volatile uint64_t accumulator = 0;
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {helper1, helper2};
    
    for (int iter = 0; iter < iterations; iter++) {
        int idx = iter % ARRAY_SIZE;
        int temp_result = 0;
        
        /* 1. Pointer chasing simulation */
        {
            int *chase_ptr = &int_arr[idx];
            int chase_sum = 0;
            for (int depth = 0; depth < MAX_DEPTH; depth++) {
                chase_sum += *chase_ptr;
                /* Non-linear pointer update */
                int next_idx = (*chase_ptr * 13 + depth * 7) % ARRAY_SIZE;
                chase_ptr = &int_arr[next_idx];
            }
            temp_result = chase_sum;
        }
        
        /* 2. Dependent arithmetic chain */
        {
            int a = temp_result;
            int b = int_arr[(idx + 1) % ARRAY_SIZE];
            int c = int_arr[(idx + 2) % ARRAY_SIZE];
            int d = int_arr[(idx + 3) % ARRAY_SIZE];
            
            /* Long dependency chain */
            a = a * b + c;
            b = a / ((d & 0xFF) + 1);
            c = b - a;
            d = c * 3 + b;
            a = d ^ a;
            b = a << (iter & 0x7);
            c = b >> 2;
            d = c | 0x55555555;
            
            temp_result = d;
            asm volatile("" : "+r"(temp_result) : : "memory");
        }
        
        /* 3. Switch statement with many cases */
        temp_result += switch_kernel(iter, int_arr, float_arr, double_arr);
        
        /* 4. Conditional with function call */
        if (iter & 1) {
            /* Use function pointer for computed jump */
            compute_func_t f = funcs[iter & 1];
            temp_result = f(temp_result, int_arr[idx]);
        } else {
            /* Inline complex conditional chain */
            temp_result = conditional_chain(temp_result, int_arr, &md_arr[idx % ARRAY_SIZE]);
        }
        
        /* 5. Mixed floating point operations */
        if (iter % 3 == 0) {
            float ftemp = float_arr[idx] * 2.5f;
            ftemp = ftemp + (float)int_arr[idx] * 0.1f;
            ftemp = helper3(ftemp, float_arr[(idx + 5) % ARRAY_SIZE]);
            temp_result += (int)ftemp;
            
            /* Update volatile float */
            g_volatile_float = ftemp;
        }
        
        /* 6. Double precision computation */
        if (iter % 4 == 0) {
            double dtemp = process_chunk(double_arr, 
                                        idx % (ARRAY_SIZE / 2),
                                        (idx % (ARRAY_SIZE / 2)) + 16);
            temp_result += (int)(dtemp * 100.0);
        }
        
        /* 7. Packed struct access */
        {
            struct mixed_data *md = &md_arr[idx];
            md->i = temp_result;
            md->f = (float)temp_result * 0.5f;
            md->d = (double)temp_result * 1.5;
            temp_result = md->i + (int)md->f + (int)md->d;
        }
        
        /* Accumulate results with memory barrier */
        accumulator += temp_result;
        asm volatile("" : "+r"(accumulator) : : "memory");
        
        /* Modify arrays to create loop-carried dependencies */
        int_arr[idx] = (int_arr[idx] * 1103515245 + 12345) & 0x7FFFFFFF;
        float_arr[idx] = (float)int_arr[idx] / 1000.0f;
        double_arr[idx] = (double)int_arr[idx] / 10000.0;
        
        /* Update volatile counter */
        g_volatile_counter = iter;
    }
    
    final_result = accumulator;
    
    /* Final reduction across arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result ^= (uint64_t)int_arr[i];
        final_result += (uint64_t)(float_arr[i] * 1000.0f);
        final_result ^= (uint64_t)(double_arr[i] * 10000.0);
        
        /* Access packed struct with potential misalignment */
        struct mixed_data *md = &md_arr[i];
        final_result += md->i + (uint64_t)md->d;
    }
    
    return final_result;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    printf("Running HAIFA scheduler test with %d iterations\n", iterations);
    
    /* Allocate and initialize arrays with different alignments */
    int *int_array = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)aligned_alloc(16, ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    struct mixed_data *mixed_array = 
        (struct mixed_data*)malloc(ARRAY_SIZE * sizeof(struct mixed_data));
    
    if (!int_array || !float_array || !double_array || !mixed_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        float_array[i] = (float)int_array[i] / 1000.0f;
        double_array[i] = (double)int_array[i] / 10000.0;
        
        mixed_array[i].c = (char)(int_array[i] & 0xFF);
        mixed_array[i].i = int_array[i];
        mixed_array[i].f = float_array[i];
        mixed_array[i].d = double_array[i];
        mixed_array[i].s = (short)(int_array[i] & 0xFFFF);
    }
    
    /* Perform complex computation */
    uint64_t result = complex_computation(iterations, 
                                         int_array, float_array, 
                                         double_array, mixed_array);
    
    /* Print result to prevent dead code elimination */
    printf("Final result: 0x%016llX\n", (unsigned long long)result);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(mixed_array);
    
    return 0;
}
