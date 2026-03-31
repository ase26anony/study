/* haifa-sched-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define MAX_DEPTH 32

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_barrier = 0;

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

/* Helper functions with different characteristics */
static int helper1(int a, int b) {
    volatile int local_barrier = g_volatile_barrier;
    int result = (a * b) + (a >> 3) - (b << 2);
    g_volatile_counter++;
    return result ^ local_barrier;
}

static int helper2(int a, int b) {
    float temp = (float)a * 1.5f + (float)b * 0.75f;
    int result = (int)temp;
    result = result * 7 - 13;
    g_volatile_counter += 2;
    return result;
}

static int helper3(int a, int b) {
    double d1 = (double)a * 2.71828;
    double d2 = (double)b * 3.14159;
    int result = (int)(d1 * d2);
    result = (result & 0xFF) | ((result >> 8) << 16);
    g_volatile_counter += 3;
    return result;
}

/* Non-inlineable function (prevents early optimization) */
__attribute__((noinline)) 
int complex_switch_kernel(int index, int *data, struct mixed_data *md) {
    int result = 0;
    
    /* Large switch with many cases */
    switch (index % 12) {
        case 0: {
            /* Pointer chasing through array */
            int *ptr = data;
            for (int i = 0; i < 8; i++) {
                result += *ptr;
                ptr = data + (*ptr % ARRAY_SIZE);
                asm volatile("" ::: "memory"); /* Compiler barrier */
            }
            break;
        }
        case 1: {
            /* Mixed integer/float operations */
            float f1 = md->f * 2.0f;
            double d1 = md->d * 1.5;
            result = (int)(f1 * d1) + md->i;
            break;
        }
        case 2: {
            /* Chain of dependent operations */
            int a = md->i * 3;
            int b = a + md->s;
            int c = b * 7;
            int d = c - (md->c * 2);
            result = d / 4;
            break;
        }
        case 3: {
            /* Memory-intensive operations */
            for (int i = 0; i < 16; i++) {
                data[(index + i) % ARRAY_SIZE] += i * result;
                result ^= data[i];
            }
            break;
        }
        case 4: {
            /* Function pointer call */
            compute_func_t funcs[] = {helper1, helper2, helper3};
            result = funcs[index % 3](md->i, md->s);
            break;
        }
        case 5: {
            /* Nested conditionals */
            if (md->i > 1000) {
                result = md->s * 2;
            } else if (md->i > 500) {
                result = md->s * 3;
            } else if (md->i > 100) {
                result = md->s * 4;
            } else {
                result = md->s * 5;
            }
            if (md->c > 'A') result += 10;
            if (md->f > 0.5f) result *= 2;
            break;
        }
        case 6: {
            /* Loop with carried dependency */
            int acc = md->i;
            for (int i = 0; i < 12; i++) {
                acc = acc * 1103515245 + 12345;
                result += acc % 100;
            }
            break;
        }
        case 7: {
            /* Mixed computations */
            result = (int)(sin((double)md->i) * 1000.0) +
                     (int)(cos((double)md->s) * 1000.0);
            break;
        }
        case 8: {
            /* Unpredictable branching */
            for (int i = 0; i < 10; i++) {
                if ((md->i * i) & 1) {
                    result += data[i] * 2;
                } else {
                    result += data[i] / 2;
                }
            }
            break;
        }
        case 9: {
            /* Deep expression */
            result = (((md->i * 3 + md->s) >> 2) * 
                     (md->c + 5) - (md->i % 7)) / 
                     ((md->s % 5) + 1);
            break;
        }
        case 10: {
            /* Memory barrier intensive */
            for (int i = 0; i < 6; i++) {
                asm volatile("" ::: "memory");
                result += md->i * i;
                asm volatile("" ::: "memory");
                result -= md->s * i;
                asm volatile("" ::: "memory");
            }
            break;
        }
        case 11: {
            /* Goto-based control flow (challenges scheduler) */
            int x = md->i;
            if (x > 1000) goto label1;
            if (x > 500) goto label2;
            if (x > 100) goto label3;
            goto label4;
            
        label1:
            result = x * 10;
            goto end;
        label2:
            result = x * 20;
            goto end;
        label3:
            result = x * 30;
            goto end;
        label4:
            result = x * 40;
        end:
            break;
        }
    }
    
    return result;
}

/* Main computation function */
static int process_data(int iterations, int *int_data, float *float_data, 
                       double *double_data, struct mixed_data *mixed_array) {
    int total_result = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        int base_idx = iter % ARRAY_SIZE;
        
        /* Create complex data dependencies */
        int_data[base_idx] = (int_data[base_idx] * 3 + 7) ^ 
                            (int_data[(base_idx + 1) % ARRAY_SIZE]);
        
        float_data[base_idx] = float_data[base_idx] * 1.1f + 
                              sinf((float)iter * 0.01f);
        
        double_data[base_idx] = double_data[base_idx] * 0.99 + 
                               cos((double)iter * 0.005);
        
        /* Update mixed data struct */
        mixed_array[base_idx].i = int_data[base_idx];
        mixed_array[base_idx].f = float_data[base_idx];
        mixed_array[base_idx].d = double_data[base_idx];
        mixed_array[base_idx].c = (char)((iter + base_idx) & 0xFF);
        mixed_array[base_idx].s = (short)(iter * 3);
        
        /* Force scheduling complexity with conditional */
        if (iter & 1) {
            /* Call helper function in one branch */
            total_result += helper1(int_data[base_idx], iter);
        } else {
            /* Inline computation in other branch */
            int temp = int_data[base_idx] * 5;
            temp = (temp >> 3) + (temp << 2);
            total_result ^= temp;
        }
        
        /* Complex switch-based kernel */
        total_result += complex_switch_kernel(iter, int_data, &mixed_array[base_idx]);
        
        /* Pointer chasing simulation */
        int *chase_ptr = &int_data[base_idx];
        for (int depth = 0; depth < MAX_DEPTH; depth++) {
            int next_idx = (*chase_ptr) % ARRAY_SIZE;
            chase_ptr = &int_data[next_idx];
            *chase_ptr += depth + iter;
            total_result ^= *chase_ptr;
            
            /* Memory barrier every few iterations */
            if (depth % 8 == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Large basic block with many independent operations */
        for (int i = 0; i < 64; i++) {
            int idx = (base_idx + i) % ARRAY_SIZE;
            float_data[idx] = float_data[idx] * 0.95f + (float)i * 0.1f;
            double_data[idx] = double_data[idx] * 1.01 - (double)i * 0.05;
            int_data[idx] = int_data[idx] + i * 3 - (iter % 7);
            
            /* Conditional store */
            if ((i + iter) & 3) {
                mixed_array[idx].i = int_data[idx];
                mixed_array[idx].f = float_data[idx];
            }
        }
        
        /* Update volatile barrier */
        g_volatile_barrier = total_result & 0xFF;
    }
    
    return total_result;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Allocate and initialize arrays with different types */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct mixed_data *mixed_array = 
        (struct mixed_data*)malloc(ARRAY_SIZE * sizeof(struct mixed_data));
    
    if (!int_data || !float_data || !double_data || !mixed_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        float_data[i] = (float)int_data[i] / 1000.0f;
        double_data[i] = (double)int_data[i] / 500.0;
        mixed_array[i].c = (char)(i & 0xFF);
        mixed_array[i].i = int_data[i];
        mixed_array[i].f = float_data[i];
        mixed_array[i].d = double_data[i];
        mixed_array[i].s = (short)(i * 3);
    }
    
    /* Perform main computation */
    int final_result = process_data(iterations, int_data, float_data, 
                                   double_data, mixed_array);
    
    /* Reduction across all data */
    int reduction = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        reduction ^= int_data[i];
        reduction += (int)float_data[i];
        reduction ^= (int)double_data[i];
        reduction += mixed_array[i].i;
        reduction ^= mixed_array[i].c;
        reduction += mixed_array[i].s;
    }
    
    final_result ^= reduction;
    final_result += g_volatile_counter;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (iterations: %d)\n", final_result, iterations);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    free(mixed_array);
    
    return 0;
}
