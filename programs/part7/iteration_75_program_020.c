#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile variables to create scheduling hazards */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 0.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) mixed_data {
    char c;
    int i;
    double d;
    float f;
    short s;
};

/* Function pointer type for computed jumps */
typedef void (*compute_func_t)(int, double*, float*, int*);

/* Helper functions that create scheduling boundaries */
static inline void memory_barrier(void) {
    asm volatile("" ::: "memory");
}

static void helper_func1(int *a, int *b, int *c) {
    *a = *b * *c + (*a ^ *b);
    *b = (*a >> 4) | (*c << 4);
    *c = (*a + *b) * (*c - *b);
    memory_barrier();
}

static void helper_func2(double *a, float *b, int *c) {
    *a = sin(*a) * cos(*a) + 1.0;
    *b = (float)(*a * 0.5 + *b * 2.0);
    *c = (int)(*a * 100.0) ^ (int)(*b * 1000.0);
    g_volatile_counter++;
}

static void helper_func3(struct mixed_data *md, int idx) {
    md->c = (char)(idx % 256);
    md->i = md->i * 1103515245 + 12345;
    md->d = md->d * 1.01 + (double)md->i * 0.0001;
    md->f = (float)md->d * 0.5f + md->f * 0.5f;
    md->s = (short)(md->i ^ md->c);
}

/* Different computation kernels for switch statement */
static void kernel_0(int *a, double *b, float *c) {
    for (int j = 0; j < 8; j++) {
        *a = (*a * 3 + j) ^ (*a >> 2);
        *b = *b * 1.1 + sin((double)j * 0.1);
        *c = *c * 0.9f + cosf((float)j * 0.2f);
    }
}

static void kernel_1(int *a, double *b, float *c) {
    *a = (*a << 3) | (*a >> 29);
    *b = sqrt(fabs(*b)) + *b * 0.5;
    *c = *c * *c - *c * 2.0f + 1.0f;
}

static void kernel_2(int *a, double *b, float *c) {
    *a = ~(*a) & 0x7FFFFFFF;
    *b = log(fabs(*b) + 1.0);
    *c = expf(*c * 0.1f);
}

static void kernel_3(int *a, double *b, float *c) {
    *a = (*a % 997) * (*a % 991);
    *b = *b * *b - *b + 1.0;
    *c = tanhf(*c);
}

static void kernel_4(int *a, double *b, float *c) {
    *a = (*a ^ 0xAAAAAAAA) & 0x55555555;
    *b = asin(fmod(fabs(*b), 1.0));
    *c = asinf(fmodf(fabs(*c), 1.0f));
}

/* Array of function pointers for computed jumps */
static compute_func_t compute_funcs[] = {
    (compute_func_t)kernel_0,
    (compute_func_t)kernel_1,
    (compute_func_t)kernel_2,
    (compute_func_t)kernel_3,
    (compute_func_t)kernel_4,
    (compute_func_t)helper_func1,
    (compute_func_t)helper_func2
};

int main(int argc, char *argv[]) {
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(N * sizeof(int));
    double *double_array = (double*)malloc(N * sizeof(double));
    float *float_array = (float*)malloc(N * sizeof(float));
    struct mixed_data *mixed_array = (struct mixed_data*)malloc(N * sizeof(struct mixed_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        int_array[i] = i * 1103515245;
        double_array[i] = (double)i * 1.23456789;
        float_array[i] = (float)i * 0.987654321f;
        mixed_array[i].c = (char)i;
        mixed_array[i].i = i * 123456789;
        mixed_array[i].d = (double)i * 3.1415926535;
        mixed_array[i].f = (float)i * 2.718281828f;
        mixed_array[i].s = (short)i;
    }
    
    /* Complex pointer chasing through arrays */
    int *chase_ptr = int_array;
    double *d_chase_ptr = double_array;
    float *f_chase_ptr = float_array;
    
    /* Main computation loop with complex control flow */
    uint64_t final_result = 0;
    
    for (int i = 0; i < N; i++) {
        /* Deeply nested conditional chain */
        if (i & 1) {
            if (i & 2) {
                if (i & 4) {
                    if (i & 8) {
                        /* Pointer chasing with arithmetic */
                        int idx = *chase_ptr % N;
                        chase_ptr = &int_array[idx];
                        *chase_ptr = *chase_ptr * 3 + i;
                        
                        idx = (int)(*d_chase_ptr) % N;
                        d_chase_ptr = &double_array[idx];
                        *d_chase_ptr = *d_chase_ptr * 1.01 + sin((double)i * 0.01);
                        
                        idx = (int)(*f_chase_ptr) % N;
                        f_chase_ptr = &float_array[idx];
                        *f_chase_ptr = *f_chase_ptr * 0.99f + cosf((float)i * 0.02f);
                    } else {
                        /* Chain of dependent arithmetic operations */
                        int a = int_array[i];
                        double b = double_array[i];
                        float c = float_array[i];
                        
                        a = a * a - a + 1;
                        b = b * b - b + 2.0;
                        c = c * c - c + 3.0f;
                        
                        a = (a ^ (a >> 16)) * 0x45d9f3b;
                        b = sin(b) * cos(b) * tan(b);
                        c = sqrtf(fabsf(c)) * c;
                        
                        a = a % 10007 * a % 10009;
                        b = fmod(b, 100.0) * b;
                        c = fmodf(c, 10.0f) * c;
                        
                        int_array[i] = a;
                        double_array[i] = b;
                        float_array[i] = c;
                    }
                } else {
                    /* Call helper function in one branch */
                    helper_func1(&int_array[i], &int_array[(i + 1) % N], 
                                 &int_array[(i + 2) % N]);
                }
            } else {
                /* Mixed data type operations */
                helper_func2(&double_array[i], &float_array[i], &int_array[i]);
            }
        } else {
            /* Packed struct operations */
            helper_func3(&mixed_array[i], i);
        }
        
        /* Switch statement with many cases */
        switch (i % 10) {
            case 0: kernel_0(&int_array[i], &double_array[i], &float_array[i]); break;
            case 1: kernel_1(&int_array[i], &double_array[i], &float_array[i]); break;
            case 2: kernel_2(&int_array[i], &double_array[i], &float_array[i]); break;
            case 3: kernel_3(&int_array[i], &double_array[i], &float_array[i]); break;
            case 4: kernel_4(&int_array[i], &double_array[i], &float_array[i]); break;
            case 5: 
                /* Computed jump via function pointer */
                compute_funcs[i % 7](i, &double_array[i], &float_array[i], &int_array[i]);
                break;
            case 6: {
                /* Small loop with loop-carried dependency */
                int sum = 0;
                for (int j = 0; j < 16; j++) {
                    sum += int_array[(i + j) % N] * int_array[(i + j - 1 + N) % N];
                }
                int_array[i] = sum;
                break;
            }
            case 7: {
                /* Memory-intensive operations */
                for (int j = 0; j < 8; j++) {
                    int idx = (i + j * 13) % N;
                    double_array[idx] = double_array[idx] * 0.99 + double_array[(idx + 1) % N] * 0.01;
                    float_array[idx] = float_array[idx] * 0.99f + float_array[(idx + 1) % N] * 0.01f;
                }
                break;
            }
            case 8: {
                /* Volatile operations that create scheduling barriers */
                g_volatile_double += double_array[i];
                int_array[i] ^= g_volatile_counter;
                memory_barrier();
                float_array[i] += (float)g_volatile_double;
                break;
            }
            case 9: {
                /* Complex arithmetic chain */
                double temp = double_array[i];
                for (int j = 0; j < 4; j++) {
                    temp = sin(temp) * cos(temp) + tan(temp);
                    temp = sqrt(fabs(temp)) * 1.1;
                }
                double_array[i] = temp;
                int_array[i] = (int)(temp * 1000000.0);
                break;
            }
        }
        
        /* Update volatile variable periodically */
        if (i % 100 == 0) {
            g_volatile_counter++;
            memory_barrier();
        }
    }
    
    /* Reduction across all results */
    for (int i = 0; i < N; i++) {
        final_result ^= (uint64_t)int_array[i];
        final_result ^= (uint64_t)(double_array[i] * 1000.0);
        final_result ^= (uint64_t)(float_array[i] * 1000.0f);
        final_result ^= (uint64_t)mixed_array[i].i;
        final_result ^= (uint64_t)mixed_array[i].c;
        final_result ^= (uint64_t)mixed_array[i].s;
    }
    
    /* Final volatile operation */
    g_volatile_counter += (int)(final_result & 0x7FFFFFFF);
    
    printf("Result: %lu\n", final_result);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(mixed_array);
    
    return (int)(final_result % 256);
}
