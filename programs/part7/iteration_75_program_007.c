/* haifa_sched_trigger.c
 * Complex program designed to trigger GCC's HAIFA scheduler state save/restore
 * and exercise the free_state function's uncovered lines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent optimization and create scheduling hazards */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 0.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) packed_data {
    char c;
    int i;
    double d;
    char trailing;
};

/* Function pointer type for computed jumps */
typedef void (*compute_func_t)(int, double*, int*);

/* Small helper functions that create scheduling boundaries */
static void helper_func1(int *a, int *b, int *c) {
    *a = (*b * 1103515245 + 12345) & 0x7fffffff;
    *c = (*a ^ *b) | 0x1;
    g_volatile_counter++;
}

static void helper_func2(double *a, double *b, double *c) {
    *a = sin(*b) * cos(*c);
    *b = *a * *a + *c * *c;
    *c = sqrt(fabs(*a));
    asm volatile("" ::: "memory"); /* Compiler barrier */
}

static void helper_func3(struct packed_data *p) {
    p->i = (p->i * 3 + p->c) & 0xfff;
    p->d = p->d * 1.01 + p->i;
    p->c = (p->c + 1) & 0x7f;
}

/* Different computation kernels for switch statement */
static void kernel_add(int *acc, int x, int y) {
    *acc += x + y + (x & y) + (x | y);
}

static void kernel_mul(int *acc, int x, int y) {
    *acc *= (x * y) ^ (x + y);
}

static void kernel_mem(int *acc, int *mem, int idx) {
    *acc = mem[idx] + mem[idx + 1] - mem[idx + 2];
}

static void kernel_float(double *acc, double x, double y) {
    *acc = x * y + x / (y + 1.0);
}

static void kernel_mixed(int *iacc, double *dacc, int x, double y) {
    *iacc += (int)(x * y);
    *dacc += y * 0.5;
}

/* Main computation with complex control flow */
static int complex_computation(int iterations, int array_size) {
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(array_size * sizeof(int));
    double *double_array = (double*)malloc(array_size * sizeof(double));
    float *float_array = (float*)malloc(array_size * sizeof(float));
    struct packed_data *packed_array = 
        (struct packed_data*)malloc(array_size * sizeof(struct packed_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < array_size; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        double_array[i] = (double)int_array[i] / 1000.0;
        float_array[i] = (float)double_array[i];
        packed_array[i].c = i & 0xff;
        packed_array[i].i = int_array[i];
        packed_array[i].d = double_array[i];
        packed_array[i].trailing = (i + 1) & 0xff;
    }
    
    int result = 0;
    double double_result = 0.0;
    
    /* Pointer chasing variable */
    int *chase_ptr = int_array;
    
    /* Main computation loop - designed to create complex scheduling */
    for (int iter = 0; iter < iterations; iter++) {
        /* ====== PART 1: Pointer chasing with dependencies ====== */
        int chase_sum = 0;
        for (int i = 0; i < 100; i++) {
            chase_sum += *chase_ptr;
            /* Complex address calculation with dependencies */
            int next_idx = (*chase_ptr + i) % array_size;
            if (next_idx < 0) next_idx = -next_idx;
            chase_ptr = &int_array[next_idx];
            
            /* Intermix with floating point */
            double_result += sin(double_array[next_idx]) * 0.01;
        }
        result ^= chase_sum;
        
        /* ====== PART 2: Deep conditional chain ====== */
        int chain_val = iter % 256;
        if (chain_val < 64) {
            helper_func1(&int_array[chain_val], 
                        &int_array[chain_val + 1],
                        &int_array[chain_val + 2]);
        } else if (chain_val < 128) {
            helper_func2(&double_array[chain_val % 32],
                        &double_array[(chain_val + 1) % 32],
                        &double_array[(chain_val + 2) % 32]);
        } else if (chain_val < 192) {
            helper_func3(&packed_array[chain_val % 64]);
        } else {
            /* Complex arithmetic chain with dependencies */
            int a = int_array[chain_val];
            int b = a * 3 + 7;
            int c = b ^ a;
            int d = c * 11 - 5;
            int e = d / (a + 1);
            result += e;
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* ====== PART 3: Switch with many cases ====== */
        switch (iter % 10) {
            case 0:
                kernel_add(&result, iter, int_array[iter % array_size]);
                break;
            case 1:
                kernel_mul(&result, iter, int_array[(iter + 1) % array_size]);
                break;
            case 2:
                kernel_mem(&result, int_array, iter % (array_size - 2));
                break;
            case 3:
                kernel_float(&double_result, 
                           double_array[iter % array_size],
                           double_array[(iter + 1) % array_size]);
                break;
            case 4:
                kernel_mixed(&result, &double_result,
                           iter, double_array[iter % array_size]);
                break;
            case 5:
                /* Nested loop with carried dependency */
                for (int j = 0; j < 20; j++) {
                    result = (result * 3 + int_array[j]) & 0xffff;
                }
                break;
            case 6:
                /* Mixed type computations */
                float f1 = float_array[iter % array_size];
                float f2 = float_array[(iter + 10) % array_size];
                double_result += (double)f1 * f2;
                result += (int)(f1 * 100);
                break;
            case 7:
                /* Access packed struct */
                result += packed_array[iter % array_size].i;
                double_result += packed_array[iter % array_size].d;
                break;
            case 8:
                /* Another arithmetic chain */
                for (int k = 0; k < 15; k++) {
                    result = (result << 3) | (result >> 29);
                    result ^= int_array[(iter + k) % array_size];
                }
                break;
            case 9:
                /* Use volatile */
                result += g_volatile_counter;
                g_volatile_double += double_result * 0.001;
                break;
        }
        
        /* ====== PART 4: Large basic block simulation ====== */
        if (iter & 1) {
            /* Sequential independent operations - fills instruction queue */
            int temp[16];
            for (int i = 0; i < 16; i++) {
                temp[i] = int_array[(iter + i) % array_size] * i;
            }
            
            /* Process them with artificial dependencies */
            for (int i = 1; i < 16; i++) {
                temp[i] += temp[i-1];
            }
            
            result += temp[15];
        }
        
        /* ====== PART 5: Computed goto simulation via function pointers ====== */
        compute_func_t funcs[4] = {
            (compute_func_t)helper_func1,
            (compute_func_t)helper_func2,
            (compute_func_t)helper_func3,
            NULL
        };
        
        int func_idx = iter % 3;
        if (funcs[func_idx]) {
            /* This creates non-linear control flow for the scheduler */
            if (func_idx == 0) {
                helper_func1(&int_array[iter % array_size],
                           &int_array[(iter + 1) % array_size],
                           &int_array[(iter + 2) % array_size]);
            } else if (func_idx == 1) {
                helper_func2(&double_array[iter % array_size],
                           &double_array[(iter + 1) % array_size],
                           &double_array[(iter + 2) % array_size]);
            } else {
                helper_func3(&packed_array[iter % array_size]);
            }
        }
    }
    
    /* Final reduction */
    int final_result = result;
    for (int i = 0; i < array_size; i++) {
        final_result ^= int_array[i];
        final_result += (int)(double_array[i] * 1000);
    }
    
    final_result += (int)g_volatile_double;
    final_result &= 0x7fffffff;
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(packed_array);
    
    return final_result;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    int array_size = 1024;
    
    /* Parse command line arguments */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size <= 100) array_size = 1024;
        if (array_size > 100000) array_size = 100000;
    }
    
    printf("Running complex computation with iterations=%d, array_size=%d\n",
           iterations, array_size);
    
    int result = complex_computation(iterations, array_size);
    
    printf("Result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    return result == 0 ? 1 : 0;
}
