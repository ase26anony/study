/* haifa-sched-test.c - Complex program to trigger HAIFA scheduler state save/restore */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define LINKED_LIST_SIZE 256
#define MAX_SWITCH_CASES 10

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile double g_volatile_double = 0.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    double d;
    char c2;
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
    int i1 = (int)f4;
    return i1 + (a & b);
}

/* Non-inlineable function (due to complexity) to create scheduling boundaries */
static __attribute__((noinline)) int complex_helper(int x, int y, int z) {
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (x >> i) & 1;
        sum -= (y >> i) & 1;
        sum *= (z >> i) & 1 ? 2 : 1;
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Another helper with loop-carried dependencies */
static int helper_with_deps(int *arr, int size) {
    int result = arr[0];
    for (int i = 1; i < size; i++) {
        /* Loop-carried dependency */
        result = result * 3 + arr[i];
        
        /* Volatile access to force scheduler to handle side effects */
        g_volatile_counter++;
    }
    return result;
}

/* Linked list traversal simulation */
static int pointer_chasing(int *base, int steps) {
    int index = 0;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        sum += base[index];
        /* Non-linear access pattern */
        index = (index * 13 + 17) & (ARRAY_SIZE - 1);
        
        /* Memory barrier every 8 steps */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    return sum;
}

/* Main computation with complex control flow */
static int compute_with_switches(int seed, int *int_arr, float *float_arr, 
                                 double *double_arr, struct misaligned_data *struct_arr) {
    int result = seed;
    
    /* Deep conditional chain */
    if (seed & 1) {
        result = kernel_add_chain(result, int_arr[0]);
        if (seed & 2) {
            result = kernel_mul_chain(result, int_arr[1]);
            if (seed & 4) {
                result = complex_helper(result, int_arr[2], int_arr[3]);
            } else {
                result = helper_with_deps(int_arr, 16);
            }
        }
    } else if (seed & 8) {
        result = kernel_mixed(result, int_arr[4]);
    } else {
        result = pointer_chasing(int_arr, 32);
    }
    
    /* Switch with many cases */
    switch (seed % MAX_SWITCH_CASES) {
        case 0: {
            /* Large basic block with independent operations */
            float f1 = float_arr[0];
            float f2 = float_arr[1];
            float f3 = float_arr[2];
            float f4 = float_arr[3];
            
            f1 = f1 * 1.1f + f2;
            f2 = f2 * 2.2f - f3;
            f3 = f3 * 3.3f / f4;
            f4 = f4 * 4.4f + f1;
            
            float_arr[0] = f1;
            float_arr[1] = f2;
            float_arr[3] = f4;
            
            result += (int)(f1 + f2 + f3 + f4);
            break;
        }
        case 1:
            result ^= int_arr[result & (ARRAY_SIZE - 1)];
            break;
        case 2: {
            double d = double_arr[0];
            for (int i = 1; i < 8; i++) {
                d = d * 1.01 + double_arr[i];
            }
            g_volatile_double = d;
            result += (int)d;
            break;
        }
        case 3:
            result = result * 7 - 13;
            break;
        case 4: {
            /* Access packed struct */
            struct misaligned_data md = struct_arr[seed % 16];
            result += md.i + (int)md.d + md.c;
            break;
        }
        case 5:
            /* Function pointer call */
            compute_func_t funcs[] = {kernel_add_chain, kernel_mul_chain, kernel_mixed};
            result = funcs[seed % 3](result, int_arr[5]);
            break;
        case 6:
            /* Nested loop with dependencies */
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result += int_arr[i * 16 + j] * int_arr[(i + 1) * 16 + j];
                }
            }
            break;
        case 7:
            /* Mixed float/int operations */
            result += (int)(sinf((float)result) * 100.0f);
            break;
        case 8:
            /* Chain of dependent operations */
            {
                int t1 = result * 3;
                int t2 = t1 >> 2;
                int t3 = t2 | 0xABCD;
                int t4 = t3 & 0x7FFF;
                int t5 = t4 ^ 0x1234;
                result = t5;
            }
            break;
        case 9:
            /* Memory-intensive operations */
            for (int i = 0; i < 32; i++) {
                int idx = (result + i) & (ARRAY_SIZE - 1);
                float_arr[idx] = float_arr[idx] * 0.99f + 0.01f;
                result += (int)float_arr[idx];
            }
            break;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct misaligned_data *struct_array = 
        (struct misaligned_data*)malloc(ARRAY_SIZE * sizeof(struct misaligned_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFFFFFF;
        float_array[i] = (float)(int_array[i] % 1000) * 0.001f;
        double_array[i] = (double)(int_array[i] % 2000) * 0.0005;
        struct_array[i].c = (char)(i & 0xFF);
        struct_array[i].i = int_array[i];
        struct_array[i].d = double_array[i];
        struct_array[i].c2 = (char)((i >> 8) & 0xFF);
    }
    
    int final_result = 0;
    
    /* Main computation loop with complex scheduling requirements */
    for (int iter = 0; iter < iterations; iter++) {
        int seed = iter ^ final_result;
        
        /* Pointer chasing through array */
        int chase_result = pointer_chasing(int_array, 64);
        
        /* Chain of dependent arithmetic operations */
        int a = chase_result;
        int b = int_array[iter & (ARRAY_SIZE - 1)];
        int c = a * b;
        int d = c + (iter * 7);
        int e = d / (b ? b : 1);
        int f = e ^ 0x5A5A5A5A;
        
        /* Conditional with function call */
        if (iter & 1) {
            f = complex_helper(f, a, b);
        }
        
        /* Switch-based computation */
        int switch_result = compute_with_switches(
            f, int_array, float_array, double_array, struct_array);
        
        /* Reduction across multiple arrays */
        int array_sum = 0;
        for (int i = 0; i < 128; i++) {
            int idx = (switch_result + i) & (ARRAY_SIZE - 1);
            array_sum += int_array[idx];
            float_array[idx] = float_array[idx] * 0.95f + 0.05f;
        }
        
        /* Update final result with all computations */
        final_result = (final_result * 31 + switch_result + array_sum) & 0x7FFFFFFF;
        
        /* Periodically update volatile variables */
        if ((iter & 31) == 0) {
            g_volatile_counter = final_result & 0xFF;
            g_volatile_double = (double)final_result * 0.001;
        }
    }
    
    /* Final reduction across all data */
    int xor_result = 0;
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        xor_result ^= int_array[i];
        xor_result ^= (int)float_array[i];
        xor_result ^= (int)double_array[i];
        xor_result ^= struct_array[i].i;
    }
    
    final_result ^= xor_result;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d (volatile counter: %d, volatile double: %f)\n",
           final_result, g_volatile_counter, g_volatile_double);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return final_result & 1;
}
