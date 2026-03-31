#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Complex struct with mixed types and packed attribute */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    float f;
    char arr[3];
};

/* Volatile variables to create scheduling hazards */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 0.0;

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that won't be inlined easily */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return a * b + (a ^ b);
}

static float __attribute__((noinline)) helper_float(float a, float b) {
    volatile float temp = a;  /* Prevent optimization */
    return temp * b - (temp / (b + 1.0f));
}

static double __attribute__((noinline)) helper_double(double a, double b) {
    asm volatile("" ::: "memory");
    return (a * b) + sin(a) * cos(b);
}

/* Function pointer array for computed jumps */
static compute_func_t func_table[] = {
    helper_mul,
    NULL,
    NULL,
    NULL,
    NULL
};

/* Main computation with complex control flow */
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
    struct MixedData *struct_array = (struct MixedData*)malloc(N * sizeof(struct MixedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFFFFFF;
        float_array[i] = (float)(int_array[i] % 1000) / 3.14159f;
        double_array[i] = (double)(int_array[i] % 2000) / 2.71828;
        struct_array[i].c = (char)(i % 256);
        struct_array[i].i = int_array[i];
        struct_array[i].d = double_array[i];
        struct_array[i].f = float_array[i];
        struct_array[i].arr[0] = (char)((i + 1) % 256);
        struct_array[i].arr[1] = (char)((i + 2) % 256);
        struct_array[i].arr[2] = (char)((i + 3) % 256);
    }
    
    /* Complex pointer chasing setup */
    int *chase_ptr = int_array;
    float *chase_float = float_array;
    double *chase_double = double_array;
    
    /* Accumulators for different types */
    int int_acc = 0;
    float float_acc = 0.0f;
    double double_acc = 0.0;
    long long ll_acc = 0;
    
    /* Main computation loop with complex dependencies */
    for (int i = 0; i < N; i++) {
        /* Deeply nested conditional chain */
        if (i & 1) {
            if (i & 2) {
                if (i & 4) {
                    if (i & 8) {
                        /* Complex arithmetic chain with dependencies */
                        int t1 = int_array[i] * 3;
                        int t2 = t1 + int_array[(i + 1) % N];
                        int t3 = t2 ^ (t1 << 3);
                        int t4 = t3 - helper_mul(t2, t3);
                        int_acc += t4;
                        
                        /* Memory barrier */
                        asm volatile("" ::: "memory");
                    } else {
                        /* Floating point chain */
                        float f1 = float_array[i] * 2.5f;
                        float f2 = f1 + helper_float(float_array[(i + 2) % N], f1);
                        float f3 = f2 * 3.14159f;
                        float_acc += f3;
                    }
                } else {
                    /* Double precision chain */
                    double d1 = double_array[i] * 1.234567;
                    double d2 = helper_double(d1, double_array[(i + 3) % N]);
                    double d3 = d2 * d2 - d1;
                    double_acc += d3;
                }
            } else {
                /* Mixed type operations */
                int_acc += (int)(float_array[i] * 100.0f);
                float_acc += (float)(int_array[i] % 100);
            }
        } else {
            /* Switch statement with many cases */
            switch (i % 10) {
                case 0: {
                    /* Kernel 0: Integer operations */
                    int a = int_array[i];
                    int b = int_array[(i + 1) % N];
                    int c = a * b + (a ^ b);
                    int d = c << (a % 8);
                    int e = d - (b >> 3);
                    int_acc += e;
                    break;
                }
                case 1: {
                    /* Kernel 1: Float operations */
                    float a = float_array[i];
                    float b = float_array[(i + 2) % N];
                    float c = a * b - a / (b + 1.0f);
                    float d = c * 3.14159f;
                    float_acc += d;
                    break;
                }
                case 2: {
                    /* Kernel 2: Double operations */
                    double a = double_array[i];
                    double b = double_array[(i + 3) % N];
                    double c = a * b + sin(a) * cos(b);
                    double d = c * 2.71828;
                    double_acc += d;
                    break;
                }
                case 3: {
                    /* Kernel 3: Pointer chasing */
                    chase_ptr = &int_array[*chase_ptr % N];
                    int_acc += *chase_ptr;
                    chase_float = &float_array[*chase_ptr % N];
                    float_acc += *chase_float;
                    break;
                }
                case 4: {
                    /* Kernel 4: Struct operations */
                    struct_array[i].i = struct_array[i].i * 2 + struct_array[(i + 1) % N].i;
                    struct_array[i].d = struct_array[i].d * 1.5;
                    int_acc += struct_array[i].i;
                    break;
                }
                case 5: {
                    /* Kernel 5: Volatile operations */
                    g_volatile_counter++;
                    g_volatile_double += (double)i / 1000.0;
                    int_acc += g_volatile_counter;
                    break;
                }
                case 6: {
                    /* Kernel 6: Memory intensive */
                    for (int j = 0; j < 10; j++) {
                        int idx = (i + j) % N;
                        int_array[idx] = int_array[idx] * 2 + 1;
                        float_array[idx] = float_array[idx] * 1.1f;
                    }
                    break;
                }
                case 7: {
                    /* Kernel 7: Function pointer call */
                    if (func_table[0]) {
                        int_acc += func_table[0](int_array[i], int_array[(i + 1) % N]);
                    }
                    break;
                }
                case 8: {
                    /* Kernel 8: Mixed computations */
                    int_acc += (int)(sin(double_array[i]) * 1000.0);
                    float_acc += (float)(cos(double_array[(i + 1) % N]) * 100.0f);
                    break;
                }
                case 9: {
                    /* Kernel 9: Reduction-like operations */
                    ll_acc += (long long)int_array[i] * int_array[(i + 2) % N];
                    double_acc += (double)float_array[i] * float_array[(i + 3) % N];
                    break;
                }
            }
        }
        
        /* Loop-carried dependency */
        if (i > 0) {
            int_array[i] += int_array[i - 1] % 100;
            float_array[i] += float_array[i - 1] * 0.01f;
        }
        
        /* Periodic complex operation every 16 iterations */
        if ((i & 15) == 0) {
            /* Large basic block with many independent operations */
            for (int j = 0; j < 32; j++) {
                int idx = (i + j) % N;
                /* Independent operations that can be scheduled in parallel */
                int_array[idx] = int_array[idx] ^ (j * 0x5A5A5A5A);
                float_array[idx] = float_array[idx] + (float)j * 0.1f;
                double_array[idx] = double_array[idx] * (1.0 + (double)j * 0.01);
                struct_array[idx].i = struct_array[idx].i + j;
            }
            /* Memory barrier */
            asm volatile("" ::: "memory");
        }
    }
    
    /* Final reduction across all accumulators and arrays */
    int final_sum = int_acc;
    final_sum += (int)float_acc;
    final_sum += (int)double_acc;
    final_sum += (int)ll_acc;
    
    for (int i = 0; i < N; i++) {
        final_sum ^= int_array[i];
        final_sum += (int)float_array[i];
        final_sum += (int)(double_array[i] * 100.0);
        final_sum += struct_array[i].i;
    }
    
    /* Use volatile to prevent optimization */
    volatile int output = final_sum % 1000000;
    printf("Result: %d\n", output);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return output != 0 ? 0 : 1;
}
