#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile variables to create scheduling hazards */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 0.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) PackedData {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef void (*compute_func_t)(int, double*, float*, int*);

/* Static helper functions that won't be inlined easily */
static __attribute__((noinline)) 
void helper_func1(int *a, double *b, float *c) {
    *a = (*a * 1103515245 + 12345) & 0x7fffffff;
    *b = sin(*b) * cos(*b);
    *c = (*c > 0) ? sqrtf(*c) : -sqrtf(-*c);
    asm volatile("" ::: "memory"); /* Compiler barrier */
}

static __attribute__((noinline))
void helper_func2(struct PackedData *pd, int idx) {
    pd->i = (pd->i << 3) | (pd->c & 0x7);
    pd->d = pd->d * 1.0001 + (idx * 0.001);
    g_volatile_counter++;
}

/* Different computation kernels */
static void kernel_add(int i, double *d, float *f, int *a) {
    for (int j = 0; j < 8; j++) {
        d[j] = d[j] + f[j] + i;
        a[j] = a[j] * 2 - j;
    }
}

static void kernel_mul(int i, double *d, float *f, int *a) {
    for (int j = 0; j < 8; j++) {
        d[j] = d[j] * f[j] * (i % 7 + 1);
        a[j] = (a[j] << 1) | (a[j] >> 31);
    }
}

static void kernel_mixed(int i, double *d, float *f, int *a) {
    for (int j = 0; j < 8; j++) {
        double temp = sin(d[j]) + cos(f[j]);
        d[j] = temp * temp - 0.5;
        a[j] = a[j] ^ (a[j] << 13);
        a[j] = a[j] ^ (a[j] >> 17);
        a[j] = a[j] ^ (a[j] << 5);
    }
}

/* Array of function pointers for computed jumps */
static compute_func_t kernels[] = {
    kernel_add,
    kernel_mul,
    kernel_mixed,
    kernel_add,
    kernel_mul,
    kernel_mixed,
    kernel_add,
    kernel_mul,
    kernel_mixed,
    kernel_add
};

int main(int argc, char *argv[]) {
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(N * sizeof(int));
    float *float_array = (float*)malloc(N * sizeof(float));
    double *double_array = (double*)malloc(N * sizeof(double));
    struct PackedData *packed_array = (struct PackedData*)malloc(N * sizeof(struct PackedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        int_array[i] = i * 1103515245;
        float_array[i] = (i * 0.12345f) - 0.5f;
        double_array[i] = sin(i * 0.01) * 100.0;
        packed_array[i].c = (char)(i & 0xFF);
        packed_array[i].i = i * 3;
        packed_array[i].d = i * 0.001;
        packed_array[i].s = (short)(i * 5);
    }
    
    /* Main computation loop with complex control flow */
    uint64_t final_sum = 0;
    
    for (int iter = 0; iter < N; iter++) {
        /* Pointer chasing through int_array */
        int chase_idx = iter % N;
        int chase_sum = 0;
        for (int chase = 0; chase < 50; chase++) {
            chase_idx = int_array[chase_idx] % N;
            chase_sum += chase_idx;
            if (chase_idx == 0) chase_idx = iter % N;
        }
        int_array[iter % N] = chase_sum;
        
        /* Deeply nested conditional chain */
        double temp_double = double_array[iter % N];
        float temp_float = float_array[iter % N];
        int temp_int = int_array[iter % N];
        
        if (iter & 1) {
            helper_func1(&temp_int, &temp_double, &temp_float);
            if (iter & 2) {
                for (int j = 0; j < 4; j++) {
                    temp_double = temp_double * 1.1 - j * 0.01;
                    temp_float = temp_float + temp_double * 0.5f;
                    temp_int = (temp_int << j) | (temp_int >> (32 - j));
                }
                if (iter & 4) {
                    helper_func2(&packed_array[iter % N], iter);
                    temp_int ^= packed_array[iter % N].i;
                }
            }
        } else if (iter & 8) {
            /* Different computation path */
            for (int j = 0; j < 3; j++) {
                temp_double = cos(temp_double) * 2.0;
                temp_int = temp_int * 3 + j;
            }
        }
        
        /* Switch statement with many cases */
        switch (iter % 10) {
            case 0: {
                /* Large basic block with independent operations */
                double d1 = double_array[(iter + 0) % N];
                double d2 = double_array[(iter + 1) % N];
                double d3 = double_array[(iter + 2) % N];
                double d4 = double_array[(iter + 3) % N];
                double d5 = double_array[(iter + 4) % N];
                double d6 = double_array[(iter + 5) % N];
                double d7 = double_array[(iter + 6) % N];
                double d8 = double_array[(iter + 7) % N];
                
                d1 = d1 * d2 + d3;
                d2 = d2 - d4 * d5;
                d3 = d3 / (d6 + 1.0);
                d4 = d4 + d7 * d8;
                d5 = d5 - d1 / d2;
                d6 = d6 * d3 + d4;
                d7 = d7 / d5 - d6;
                d8 = d8 + d7 * d1;
                
                double_array[(iter + 0) % N] = d1;
                double_array[(iter + 1) % N] = d2;
                double_array[(iter + 2) % N] = d3;
                double_array[(iter + 3) % N] = d4;
                double_array[(iter + 4) % N] = d5;
                double_array[(iter + 6) % N] = d7;
                break;
            }
            case 1:
            case 2:
            case 3:
                /* Use function pointer for computed jump */
                kernels[iter % 10](iter, 
                    &double_array[iter % N], 
                    &float_array[iter % N], 
                    &int_array[iter % N]);
                break;
            case 4: {
                /* Loop with carried dependency */
                float f_prev = float_array[iter % N];
                for (int j = 1; j < 20; j++) {
                    float f_curr = float_array[(iter + j) % N];
                    float_array[(iter + j) % N] = f_prev * 0.9f + f_curr * 0.1f;
                    f_prev = f_curr;
                }
                break;
            }
            case 5:
            case 6:
                /* Mixed integer/float operations */
                for (int j = 0; j < 10; j++) {
                    int idx = (iter + j) % N;
                    double_array[idx] = int_array[idx] * 0.001 + float_array[idx];
                    int_array[idx] = (int)(double_array[idx] * 1000.0);
                    float_array[idx] = (float)(int_array[idx] % 1000) * 0.001f;
                }
                break;
            case 7:
            case 8:
            case 9:
                /* Chain of dependent operations */
                temp_double = double_array[iter % N];
                for (int j = 0; j < 15; j++) {
                    temp_double = sin(temp_double) * 1.1;
                    temp_double = cos(temp_double) * 0.9;
                    temp_double = temp_double * temp_double - 0.5;
                    g_volatile_double = temp_double; /* Volatile store */
                }
                double_array[iter % N] = temp_double;
                break;
        }
        
        /* Update volatile counter */
        g_volatile_counter += (iter & 0xFF);
        
        /* Final reduction accumulation */
        final_sum += temp_int;
        final_sum ^= (uint64_t)(temp_double * 1000.0);
        final_sum += (uint64_t)(temp_float * 100.0f);
    }
    
    /* Additional reduction across all arrays */
    uint64_t array_sum = 0;
    for (int i = 0; i < N; i++) {
        array_sum += int_array[i];
        array_sum ^= (uint64_t)(float_array[i] * 1000.0f);
        array_sum += (uint64_t)(double_array[i] * 100.0);
        array_sum += packed_array[i].i;
    }
    
    final_sum ^= array_sum;
    final_sum += g_volatile_counter;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %lu\n", (unsigned long)final_sum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(packed_array);
    
    return 0;
}
