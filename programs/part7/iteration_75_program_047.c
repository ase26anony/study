/* haifa_sched_trigger.c
 * Program designed to trigger GCC's HAIFA scheduler state save/restore
 * and ensure free_state() is called with non-empty scheduler structures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile and memory barriers to create scheduling hazards */
#define MEMORY_BARRIER() asm volatile("" ::: "memory")
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Various computation kernels to create diverse scheduling patterns */
static int kernel_add_chain(int a, int b) {
    volatile int v1 = a;
    volatile int v2 = b;
    int r = v1 + v2;
    MEMORY_BARRIER();
    r = r * v1 - v2;
    MEMORY_BARRIER();
    r = r / (v1 | 1);
    return r;
}

static int kernel_mul_chain(int a, int b) {
    volatile int v1 = a;
    volatile int v2 = b;
    int r = v1 * v2;
    MEMORY_BARRIER();
    r = r << (v1 & 3);
    MEMORY_BARRIER();
    r = r ^ v2;
    return r;
}

static float kernel_float_ops(float a, float b) {
    volatile float v1 = a;
    volatile float v2 = b;
    float r = v1 * v2;
    MEMORY_BARRIER();
    r = r + v1 / (v2 + 1.0f);
    MEMORY_BARRIER();
    r = sqrtf(fabsf(r));
    return r;
}

static double kernel_double_ops(double a, double b) {
    volatile double v1 = a;
    volatile double v2 = b;
    double r = v1 * v2;
    MEMORY_BARRIER();
    r = r + sin(v1) * cos(v2);
    MEMORY_BARRIER();
    r = r / (fabs(v2) + 1.0);
    return r;
}

/* Non-inlineable function to create scheduling boundaries */
__attribute__((noinline)) 
static int complex_calculation(int seed, int *array, int size) {
    volatile int result = seed;
    struct misaligned_data md;
    
    /* Force misaligned accesses */
    md.c = (char)(seed & 0xFF);
    md.i = seed * 1103515245;
    md.d = (double)seed / 3.14159;
    md.s = (short)(seed >> 16);
    
    /* Pointer chasing with dependencies */
    int *ptr = array;
    for (int i = 0; i < size && i < 8; i++) {
        result = result ^ *ptr;
        ptr = array + (*ptr % size);
        MEMORY_BARRIER();
    }
    
    /* Mixed integer/float operations */
    float f = (float)result;
    f = kernel_float_ops(f, f + 1.0f);
    result = (int)f ^ result;
    
    return result;
}

/* Function with switch statement creating multiple basic blocks */
static int switch_based_computation(int value, int *data, int data_size) {
    int result = 0;
    
    switch (value % 10) {
        case 0: {
            /* Long dependency chain */
            int a = data[0];
            int b = data[1];
            for (int i = 0; i < 16; i++) {
                a = a * b + i;
                b = b ^ a;
                a = a - b;
            }
            result = a + b;
            break;
        }
        case 1: {
            /* Memory intensive */
            volatile int temp[8];
            for (int i = 0; i < 8; i++) {
                temp[i] = data[i % data_size] * i;
                MEMORY_BARRIER();
            }
            for (int i = 0; i < 8; i++) {
                result += temp[i];
            }
            break;
        }
        case 2: {
            /* Floating point chain */
            double d = (double)value;
            for (int i = 0; i < 12; i++) {
                d = kernel_double_ops(d, d + 1.0);
            }
            result = (int)d;
            break;
        }
        case 3: {
            /* Nested loops with dependencies */
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result += data[(i * j) % data_size];
                }
            }
            break;
        }
        case 4: {
            /* Function pointer call */
            compute_func_t funcs[] = {kernel_add_chain, kernel_mul_chain};
            result = funcs[value & 1](data[0], data[1]);
            break;
        }
        case 5: {
            /* Conditional chain */
            int x = data[0];
            if (x & 1) x = kernel_add_chain(x, data[1]);
            else if (x & 2) x = kernel_mul_chain(x, data[2]);
            else if (x & 4) x = complex_calculation(x, data, data_size);
            else x = x ^ data[3];
            result = x;
            break;
        }
        case 6: {
            /* Mixed operations */
            float f1 = (float)data[0];
            float f2 = (float)data[1];
            for (int i = 0; i < 8; i++) {
                f1 = kernel_float_ops(f1, f2);
                f2 = f2 * 1.1f + (float)i;
            }
            result = (int)(f1 + f2);
            break;
        }
        case 7: {
            /* Large basic block simulation */
            int vals[16];
            for (int i = 0; i < 16; i++) {
                vals[i] = data[i % data_size] + i;
            }
            for (int i = 0; i < 16; i++) {
                result += vals[i] * (i + 1);
            }
            break;
        }
        case 8: {
            /* Goto-based control flow */
            int x = value;
            if (x & 1) goto label1;
            if (x & 2) goto label2;
            goto label3;
            
        label1:
            x = x * 3 + 1;
            goto end;
        label2:
            x = x / 2;
            goto end;
        label3:
            x = x ^ 0x55AA;
        end:
            result = x;
            break;
        }
        case 9: {
            /* Reduction with dependencies */
            int sum = 0;
            for (int i = 0; i < data_size; i++) {
                sum += data[i] * (i + 1);
                if (i > 0) {
                    sum -= data[i - 1];
                }
            }
            result = sum;
            break;
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    /* Parse iteration count */
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    const int array_size = 1024;
    int *int_array = (int*)malloc(array_size * sizeof(int));
    float *float_array = (float*)malloc(array_size * sizeof(float));
    double *double_array = (double*)malloc(array_size * sizeof(double));
    
    if (!int_array || !float_array || !double_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < array_size; i++) {
        int seed = i * 1103515245;
        int_array[i] = seed;
        float_array[i] = (float)seed / 1000.0f;
        double_array[i] = (double)seed / 10000.0;
    }
    
    /* Main computation loop with complex scheduling requirements */
    uint64_t final_result = 0;
    
    for (int iter = 0; iter < N; iter++) {
        volatile int control = iter;
        int local_result = 0;
        
        /* Pointer chasing through array */
        int *ptr = int_array;
        for (int i = 0; i < 32; i++) {
            local_result ^= *ptr;
            ptr = int_array + (*ptr % array_size);
            MEMORY_BARRIER();
        }
        
        /* Chain of dependent arithmetic operations */
        int a = local_result;
        int b = int_array[iter % array_size];
        double c = double_array[iter % array_size];
        float d = float_array[iter % array_size];
        
        a = a * b + iter;
        b = b ^ a;
        c = c * (double)a + sin((double)b);
        d = d * (float)c + cosf((float)a);
        
        /* Mixed type reduction */
        local_result = a + b + (int)c + (int)d;
        
        /* Switch-based computation creating multiple scheduling contexts */
        local_result += switch_based_computation(iter, int_array, array_size);
        
        /* Conditional function call */
        if (iter & 1) {
            local_result = complex_calculation(local_result, int_array, array_size);
        }
        
        /* Nested loop with loop-carried dependency */
        int sum = 0;
        for (int i = 0; i < 64; i++) {
            sum += int_array[(iter + i) % array_size] * 
                   int_array[(iter + i - 1) % array_size];
            MEMORY_BARRIER();
        }
        local_result ^= sum;
        
        /* Update arrays to create data dependencies between iterations */
        int_array[iter % array_size] = local_result;
        float_array[iter % array_size] = (float)local_result / 100.0f;
        double_array[iter % array_size] = (double)local_result / 1000.0;
        
        /* Accumulate final result */
        final_result += (uint64_t)local_result;
        
        /* Periodic memory barrier */
        if (iter % 100 == 0) {
            COMPILER_BARRIER();
        }
    }
    
    /* Final reduction across all arrays */
    int final_int_sum = 0;
    float final_float_sum = 0.0f;
    double final_double_sum = 0.0;
    
    for (int i = 0; i < array_size; i++) {
        final_int_sum ^= int_array[i];
        final_float_sum += float_array[i];
        final_double_sum += double_array[i];
    }
    
    final_result ^= (uint64_t)final_int_sum;
    final_result ^= (uint64_t)final_float_sum;
    final_result ^= (uint64_t)final_double_sum;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %llu\n", (unsigned long long)final_result);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    
    return 0;
}
