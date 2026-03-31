#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Mixed data types with different alignments */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    float f;
    short s;
};

/* Volatile variables to create scheduling hazards */
volatile int vol_counter = 0;
volatile double vol_double = 1.0;

/* Function pointer for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Non-inlineable function to create scheduling boundaries */
__attribute__((noinline)) 
static int complex_helper(int a, int b, int c) {
    int result = a * b + c;
    result ^= (result >> 16);
    result *= 1103515245;
    return result & 0x7FFFFFFF;
}

/* Another helper with memory operations */
__attribute__((noinline))
static void memory_intensive(int* arr, int size, int seed) {
    for (int i = 0; i < size; i++) {
        arr[i] = (arr[i] * seed + i) ^ (seed >> (i & 7));
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
}

/* Different computation kernels */
static int kernel_add(int a, int b) {
    for (int i = 0; i < 5; i++) {
        a = (a + b) * 3 - 1;
        b = (b - a) ^ 0x5555;
    }
    return a + b;
}

static int kernel_mul(int a, int b) {
    double temp = (double)a * (double)b;
    int result = (int)(temp * 1.5);
    for (int i = 0; i < 3; i++) {
        result = (result << 3) | (result >> 29);
        result ^= 0xAAAAAAAA;
    }
    return result;
}

static int kernel_mem(int a, int b) {
    volatile int local[8];
    for (int i = 0; i < 8; i++) {
        local[i] = a + i * b;
        a = local[i] ^ b;
    }
    return local[7];
}

/* Array of function pointers for switch-like dispatch */
static compute_func_t kernels[] = {
    kernel_add,
    kernel_mul,
    kernel_mem,
    kernel_add,
    kernel_mul,
    kernel_mem,
    kernel_add,
    kernel_mul,
    kernel_mem,
    kernel_add
};

int main(int argc, char** argv) {
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int* int_array = (int*)malloc(N * sizeof(int));
    double* double_array = (double*)malloc(N * sizeof(double));
    float* float_array = (float*)malloc(N * sizeof(float));
    struct MixedData* mixed_array = (struct MixedData*)malloc(N * sizeof(struct MixedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        int_array[i] = i * 1103515245;
        double_array[i] = sin((double)i * 0.1) * 100.0;
        float_array[i] = (float)cos((double)i * 0.05) * 50.0f;
        mixed_array[i].c = (char)(i & 0xFF);
        mixed_array[i].i = int_array[i] ^ 0x12345678;
        mixed_array[i].d = double_array[i] * 2.0;
        mixed_array[i].f = float_array[i] * 1.5f;
        mixed_array[i].s = (short)(i * 3);
    }
    
    /* Main computation with complex control flow */
    uint64_t final_result = 0;
    
    for (int outer = 0; outer < 100; outer++) {
        int local_sum = 0;
        double local_double = 1.0;
        float local_float = 0.5f;
        
        /* Pointer chasing through array (simulated linked list) */
        int chase_index = outer % N;
        for (int chase = 0; chase < 50; chase++) {
            chase_index = int_array[chase_index] % N;
            if (chase_index < 0) chase_index = -chase_index;
            
            /* Dependent arithmetic chain */
            int a = int_array[chase_index];
            int b = (chase_index * 3) ^ 0x5555;
            int c = a * b + chase;
            int d = c ^ (c >> 16);
            int e = d * 1103515245;
            local_sum += e & 0xFFF;
            
            /* Mixed floating point operations */
            local_double *= double_array[chase_index] * 0.99;
            local_float += float_array[chase_index] * 0.01f;
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Deeply nested conditionals */
        if (outer & 1) {
            if (local_sum > 1000) {
                if (local_double > 50.0) {
                    if (local_float < 25.0f) {
                        /* Call helper function in deepest branch */
                        local_sum = complex_helper(local_sum, (int)local_double, (int)local_float);
                    } else {
                        local_sum = kernel_mem(local_sum, (int)local_float);
                    }
                } else {
                    local_sum = kernel_mul(local_sum, (int)local_double);
                }
            } else {
                local_sum = kernel_add(local_sum, (int)local_double);
            }
        } else {
            /* Switch statement with many cases */
            switch (outer % 10) {
                case 0: {
                    /* Large basic block with independent operations */
                    int temp[16];
                    for (int i = 0; i < 16; i++) {
                        temp[i] = local_sum + i * 3;
                        temp[i] ^= (temp[i] << 3);
                        temp[i] *= 0x9E3779B9;
                    }
                    for (int i = 0; i < 16; i++) {
                        local_sum += temp[i];
                    }
                    break;
                }
                case 1:
                    local_sum = kernels[1](local_sum, (int)local_double);
                    break;
                case 2:
                    local_sum = complex_helper(local_sum, outer, (int)local_float);
                    break;
                case 3:
                    memory_intensive(int_array, N > 100 ? 100 : N, local_sum);
                    break;
                case 4:
                    local_sum = (local_sum << 4) | (local_sum >> 28);
                    local_sum ^= 0xDEADBEEF;
                    break;
                case 5:
                    for (int i = 0; i < 20; i++) {
                        local_sum = (local_sum * 3 + i) ^ (local_sum >> 1);
                    }
                    break;
                case 6:
                    local_sum = kernel_mem(local_sum, outer);
                    break;
                case 7:
                    local_sum = kernels[7](local_sum, (int)local_double);
                    break;
                case 8:
                    /* Another large basic block */
                    for (int i = 0; i < 32; i++) {
                        float_array[i % N] = (float)local_sum * 0.1f + i;
                        int_array[i % N] ^= local_sum + i;
                    }
                    break;
                case 9:
                    local_sum = complex_helper(local_sum, (int)local_double, (int)local_float);
                    break;
            }
        }
        
        /* Update volatile variables */
        vol_counter += local_sum;
        vol_double *= 1.0 + local_double * 0.00001;
        
        /* Final reduction */
        final_result ^= (uint64_t)local_sum;
        final_result += (uint64_t)(local_double * 1000.0);
        final_result ^= (uint64_t)(local_float * 100.0f);
        
        /* Non-linear control flow with goto */
        if (outer % 7 == 0) {
            goto special_case;
        }
        continue;
        
    special_case:
        /* Special processing */
        for (int i = 0; i < 10; i++) {
            mixed_array[i].d = sqrt(mixed_array[i].d);
            mixed_array[i].f = mixed_array[i].f * 0.9f + 0.1f;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Additional reduction across arrays */
    for (int i = 0; i < N; i++) {
        final_result ^= (uint64_t)int_array[i];
        final_result += (uint64_t)(double_array[i] * 100.0);
        if (i % 3 == 0) {
            final_result ^= (uint64_t)(float_array[i] * 50.0f);
        }
    }
    
    /* Prevent dead code elimination */
    printf("Final result: %llu\n", (unsigned long long)final_result);
    printf("Volatile counter: %d\n", vol_counter);
    printf("Volatile double: %f\n", vol_double);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(mixed_array);
    
    return (final_result > 0) ? 0 : 1;
}
