/* Complex scheduling test for GCC HAIFA scheduler state save/restore */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define CHASE_SIZE 256

/* Volatile and memory barriers to create scheduling hazards */
static volatile int g_volatile_sink;
#define MEMORY_BARRIER() asm volatile("" ::: "memory")

/* Packed struct with mixed types to force alignment issues */
struct __attribute__((packed)) mixed_data {
    char c;
    int i;
    double d;
    short s;
};

/* Non-inlineable function to create scheduling boundaries */
static __attribute__((noinline)) 
int helper_function(int a, int b, int *arr) {
    int result = a * b;
    for (int i = 0; i < 8; i++) {
        result += arr[i] * (i + 1);
        MEMORY_BARRIER();
    }
    g_volatile_sink = result;
    return result ^ (b << 3);
}

/* Another helper with floating point */
static __attribute__((noinline))
double fp_helper(double *darr, int idx) {
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += darr[(idx + i) % CHASE_SIZE] * (i + 1);
        /* Create loop-carried dependency */
        darr[(idx + i) % CHASE_SIZE] = sum * 0.5;
    }
    return sum;
}

/* Function pointer for computed jumps */
typedef int (*compute_func_t)(int, int, int*);
static compute_func_t func_table[5];

static int func1(int a, int b, int *arr) { return a + b + arr[0]; }
static int func2(int a, int b, int *arr) { return a * b - arr[1]; }
static int func3(int a, int b, int *arr) { return (a << 3) | (b & 0xFF); }
static int func4(int a, int b, int *arr) { return arr[a % 8] ^ arr[b % 8]; }
static int func5(int a, int b, int *arr) { 
    int r = 1;
    for (int i = 0; i < 4; i++) r *= arr[i] + 1;
    return r;
}

int main(int argc, char **argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Allocate and initialize arrays with different types and alignments */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    struct mixed_data *mixed_array = (struct mixed_data*)malloc(CHASE_SIZE * sizeof(struct mixed_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFFFFFF;
        double_array[i] = (double)(i * 1103515245) / 1000000.0;
        float_array[i] = (float)(i * 1103515245) / 1000000.0f;
    }
    
    for (int i = 0; i < CHASE_SIZE; i++) {
        mixed_array[i].c = (char)(i ^ 0x55);
        mixed_array[i].i = i * 3;
        mixed_array[i].d = sqrt((double)i + 1.0);
        mixed_array[i].s = (short)(i << 2);
    }
    
    /* Initialize function pointer table */
    func_table[0] = func1;
    func_table[1] = func2;
    func_table[2] = func3;
    func_table[3] = func4;
    func_table[4] = func5;
    
    /* Create linked-list-like pointer chasing indices */
    int chase_indices[CHASE_SIZE];
    for (int i = 0; i < CHASE_SIZE; i++) {
        chase_indices[i] = (i * 97) % CHASE_SIZE;  /* Pseudo-random permutation */
    }
    
    /* Main computation loop with complex control flow */
    int result_acc = 0;
    double fp_acc = 0.0;
    
    for (int iter = 0; iter < N; iter++) {
        /* Pointer chasing through mixed array */
        int chase_ptr = iter % CHASE_SIZE;
        for (int j = 0; j < 32; j++) {
            struct mixed_data *curr = &mixed_array[chase_ptr];
            result_acc ^= curr->i;
            fp_acc += curr->d;
            chase_ptr = chase_indices[chase_ptr];
            MEMORY_BARRIER();
        }
        
        /* Large basic block with many independent operations */
        /* This should fill the instruction queue */
        int temp[16];
        for (int i = 0; i < 16; i++) {
            temp[i] = int_array[(iter + i) % ARRAY_SIZE];
        }
        
        /* Chain of dependent arithmetic operations */
        int a = temp[0], b = temp[1], c = temp[2], d = temp[3];
        for (int i = 0; i < 8; i++) {
            a = b * c + d;
            b = c ^ a;
            c = d + (a << 2);
            d = (b * 3) - c;
            MEMORY_BARRIER();
        }
        result_acc += a + b + c + d;
        
        /* Complex switch statement with different computation kernels */
        switch (iter % 10) {
            case 0: {
                /* Integer arithmetic chain */
                int x = int_array[iter % ARRAY_SIZE];
                for (int i = 0; i < 12; i++) {
                    x = (x * 3 + 7) ^ (x >> 1);
                    int_array[(iter + i) % ARRAY_SIZE] ^= x;
                }
                result_acc ^= x;
                break;
            }
            case 1: {
                /* Floating point intensive */
                double sum = 0.0;
                for (int i = 0; i < 8; i++) {
                    sum += double_array[(iter + i) % ARRAY_SIZE] * 
                           float_array[(iter + i * 3) % ARRAY_SIZE];
                }
                fp_acc += sum;
                double_array[iter % ARRAY_SIZE] = fp_acc * 0.99;
                break;
            }
            case 2: {
                /* Memory intensive with barriers */
                for (int i = 0; i < 64; i += 4) {
                    int idx = (iter + i) % ARRAY_SIZE;
                    int_array[idx] = int_array[idx] * 2 + 1;
                    MEMORY_BARRIER();
                    float_array[idx] = int_array[idx] * 0.5f;
                    MEMORY_BARRIER();
                }
                break;
            }
            case 3: {
                /* Function call in one branch */
                if (iter & 1) {
                    result_acc += helper_function(iter, result_acc, int_array);
                }
                break;
            }
            case 4: {
                /* Computed jump via function pointer */
                int idx = iter % 5;
                result_acc = func_table[idx](result_acc, iter, int_array);
                break;
            }
            case 5: {
                /* Nested loops with loop-carried dependency */
                int sum = 0;
                for (int i = 0; i < 16; i++) {
                    sum += int_array[(iter + i) % ARRAY_SIZE] * 
                           int_array[(iter + i + 1) % ARRAY_SIZE];
                    int_array[(iter + i) % ARRAY_SIZE] = sum;
                }
                result_acc += sum;
                break;
            }
            case 6: {
                /* Mixed integer/float operations */
                for (int i = 0; i < 8; i++) {
                    int idx = (iter + i * 7) % ARRAY_SIZE;
                    double dval = double_array[idx];
                    int ival = (int)(dval * 1000.0);
                    float_array[idx] = (float)ival / 1000.0f;
                    result_acc += ival;
                }
                break;
            }
            case 7: {
                /* Deep conditional chain */
                int val = iter;
                if (val & 0x01) val ^= 0xAAAAAAAA;
                else if (val & 0x02) val ^= 0x55555555;
                else if (val & 0x04) val ^= 0x33333333;
                else if (val & 0x08) val ^= 0xCCCCCCCC;
                else if (val & 0x10) val ^= 0x0F0F0F0F;
                else val ^= 0xF0F0F0F0;
                result_acc ^= val;
                break;
            }
            case 8: {
                /* Another floating point helper */
                fp_acc += fp_helper(double_array, iter % CHASE_SIZE);
                break;
            }
            case 9: {
                /* Goto-based non-linear control flow */
                int x = iter;
                target1:
                    x = (x * 3) & 0xFF;
                    if (x & 1) goto target2;
                    x ^= 0x55;
                target2:
                    x = (x << 4) | (x >> 4);
                    if (x > 100) goto target3;
                    x += 77;
                target3:
                    result_acc += x;
                break;
            }
        }
        
        /* Additional memory operations with volatile */
        g_volatile_sink = result_acc;
        
        /* Array initialization in large basic block */
        /* Many independent stores to fill instruction queue */
        if ((iter % 100) == 0) {
            for (int i = 0; i < 128; i++) {
                int idx = (iter + i) % ARRAY_SIZE;
                int_array[idx] = i * iter;
                double_array[idx] = (double)i / (iter + 1.0);
                MEMORY_BARRIER();
            }
        }
    }
    
    /* Final reduction across all arrays */
    int final_result = result_acc;
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        final_result ^= int_array[i];
        fp_acc += double_array[i];
    }
    
    /* Mix in floating point result */
    final_result ^= (int)fp_acc;
    final_result ^= (int)(fp_acc * 1000000.0);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (N=%d)\n", final_result, N);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(mixed_array);
    
    return final_result & 0xFF;
}
