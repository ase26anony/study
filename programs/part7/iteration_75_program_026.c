/* haifa-sched-coverage.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa-sched-coverage.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define LINKED_LIST_SIZE 256

/* Volatile variables to create scheduling hazards */
static volatile int vol_counter = 0;
static volatile double vol_double = 0.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) mixed_data {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Helper functions with different computation patterns */
static int compute_chain_a(int a, int b) {
    int t1 = a * 1103515245 + 12345;
    int t2 = b ^ t1;
    int t3 = t2 * 1664525 + 1013904223;
    asm volatile("" ::: "memory"); /* Memory barrier */
    return t3 & 0x7FFFFFFF;
}

static int compute_chain_b(int a, int b) {
    double d1 = (double)a * 3.14159;
    double d2 = (double)b * 2.71828;
    int t1 = (int)(d1 * d2);
    int t2 = t1 >> 4;
    int t3 = t2 | (a & b);
    return t3;
}

static float compute_chain_c(float a, float b) {
    float f1 = a * 1.2345f;
    float f2 = b * 0.9876f;
    float f3 = f1 + f2;
    float f4 = f3 * f3 - f1 * f2;
    vol_double = f4; /* Volatile store creates scheduling boundary */
    return f4;
}

/* Non-inlineable function (due to complexity) */
__attribute__((noinline)) 
static double complex_calculation(double *arr, int idx, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        /* Loop-carried dependency */
        sum = sum * 1.01 + arr[(idx + i) % ARRAY_SIZE];
        
        /* Memory barrier every 8 iterations */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    return sum;
}

/* Function with switch statement creating multiple basic blocks */
static int switch_computation(int val, int *array, struct mixed_data *md) {
    int result = val;
    
    switch (val % 10) {
        case 0:
            result = array[val % ARRAY_SIZE] * 3;
            md->i = result;
            break;
        case 1:
            result = (int)(md->d * 100.0) + array[val % ARRAY_SIZE];
            break;
        case 2:
            result = compute_chain_a(val, array[val % ARRAY_SIZE]);
            md->s = (short)result;
            break;
        case 3:
            result = (val << 4) | (array[val % ARRAY_SIZE] & 0xF);
            md->c = (char)result;
            break;
        case 4:
            result = (int)compute_chain_c((float)val, (float)array[val % ARRAY_SIZE]);
            break;
        case 5:
            result = val * val - array[val % ARRAY_SIZE];
            md->i = result ^ 0x55555555;
            break;
        case 6:
            result = compute_chain_b(val, array[val % ARRAY_SIZE]);
            md->d = (double)result;
            break;
        case 7:
            result = (val % 77) * array[val % ARRAY_SIZE];
            md->s = (short)(result >> 8);
            break;
        case 8:
            result = ~val & array[val % ARRAY_SIZE];
            md->c = (char)(result & 0xFF);
            break;
        case 9:
            result = val + array[val % ARRAY_SIZE] * 2;
            md->i = result | 0x80000000;
            break;
    }
    
    vol_counter++; /* Volatile operation in each case */
    return result;
}

/* Linked list traversal with pointer chasing */
static int pointer_chase(int *next_indices, int *values, int start_idx) {
    int sum = 0;
    int idx = start_idx;
    int steps = 0;
    
    while (steps < LINKED_LIST_SIZE && idx >= 0 && idx < ARRAY_SIZE) {
        /* Mixed operations with dependencies */
        int val = values[idx];
        sum = sum * 31 + val;
        
        /* Floating-point operation in the middle */
        double dval = (double)val * 0.01;
        if (dval > 100.0) {
            sum += (int)(dval);
        }
        
        idx = next_indices[idx];
        steps++;
        
        /* Memory barrier every 16 steps */
        if ((steps & 15) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return sum;
}

/* Main computation with complex control flow */
static uint64_t run_computation(int iterations, int *array_int, 
                               float *array_float, double *array_double,
                               struct mixed_data *mixed_array) {
    uint64_t total_result = 0;
    int next_indices[ARRAY_SIZE];
    
    /* Initialize linked list structure */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        next_indices[i] = (i * 1103515245 + 12345) % ARRAY_SIZE;
        array_int[i] = (i * 1664525 + 1013904223) & 0x7FFFFFFF;
        array_float[i] = (float)i * 1.2345f;
        array_double[i] = (double)i * 3.14159;
        
        mixed_array[i].c = (char)(i & 0xFF);
        mixed_array[i].i = i * i - i;
        mixed_array[i].d = sqrt((double)i + 1.0);
        mixed_array[i].s = (short)(i * 3);
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {
        compute_chain_a,
        compute_chain_b,
        (compute_func_t)compute_chain_c
    };
    
    for (int iter = 0; iter < iterations; iter++) {
        int base_val = iter & 0xFFF;
        
        /* Pattern 1: Deep conditional chain */
        int cond_result = 0;
        if (iter & 0x01) {
            cond_result = array_int[base_val % ARRAY_SIZE] * 2;
            if (iter & 0x02) {
                cond_result += array_int[(base_val + 1) % ARRAY_SIZE];
                if (iter & 0x04) {
                    cond_result ^= 0xAAAAAAAA;
                    if (iter & 0x08) {
                        cond_result = ~cond_result;
                        if (iter & 0x10) {
                            cond_result *= 3;
                        }
                    }
                }
            }
        } else {
            cond_result = array_int[base_val % ARRAY_SIZE] / 2;
        }
        total_result ^= (uint64_t)cond_result << 32;
        
        /* Pattern 2: Switch statement with many cases */
        int switch_result = switch_computation(iter, array_int, &mixed_array[iter % ARRAY_SIZE]);
        total_result += switch_result;
        
        /* Pattern 3: Pointer chasing through linked list */
        int chase_result = pointer_chase(next_indices, array_int, iter % ARRAY_SIZE);
        total_result ^= (uint64_t)chase_result;
        
        /* Pattern 4: Large basic block with independent operations */
        double block_sum = 0.0;
        for (int j = 0; j < 128; j++) {
            /* Independent operations that can be reordered */
            int idx = (iter + j) % ARRAY_SIZE;
            double d1 = array_double[idx] * 1.01;
            double d2 = array_double[(idx + 1) % ARRAY_SIZE] * 0.99;
            float f1 = array_float[idx] * 2.0f;
            float f2 = array_float[(idx + 2) % ARRAY_SIZE] * 0.5f;
            
            block_sum += d1 * d2 + (double)(f1 * f2);
            
            /* Store results back to different arrays */
            array_int[idx] = (int)(d1 + d2);
            array_float[(idx + 3) % ARRAY_SIZE] = (float)(block_sum * 0.1);
        }
        total_result += (uint64_t)block_sum;
        
        /* Pattern 5: Computed function call */
        int func_idx = iter % 3;
        int func_result = funcs[func_idx](iter, array_int[iter % ARRAY_SIZE]);
        total_result = total_result * 31 + func_result;
        
        /* Pattern 6: Complex calculation with function call */
        if (iter & 0x20) {
            double complex_result = complex_calculation(array_double, iter % ARRAY_SIZE, 16);
            total_result ^= *(uint64_t*)&complex_result;
        }
        
        /* Pattern 7: Nested loops with loop-carried dependency */
        int nested_sum = 0;
        for (int k = 0; k < 8; k++) {
            int inner_sum = 0;
            for (int l = 0; l < 8; l++) {
                /* Loop-carried dependency */
                inner_sum = inner_sum * 7 + array_int[(iter + k + l) % ARRAY_SIZE];
            }
            nested_sum += inner_sum;
        }
        total_result += nested_sum;
        
        /* Pattern 8: Mixed data type operations */
        struct mixed_data *md = &mixed_array[iter % ARRAY_SIZE];
        double mixed_calc = md->d * (double)md->i + (double)md->s * 0.01;
        total_result ^= *(uint64_t*)&mixed_calc;
        
        /* Update volatile variable */
        vol_counter = iter;
    }
    
    return total_result;
}

int main(int argc, char **argv) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000;
        }
    }
    
    /* Allocate arrays with different alignments */
    int *array_int = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    float *array_float = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    double *array_double = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    struct mixed_data *mixed_array = (struct mixed_data*)malloc(ARRAY_SIZE * sizeof(struct mixed_data));
    
    if (!array_int || !array_float || !array_double || !mixed_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Run the complex computation */
    uint64_t result = run_computation(iterations, array_int, array_float, 
                                     array_double, mixed_array);
    
    /* Final reduction to prevent dead code elimination */
    uint64_t final_check = result;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_check ^= (uint64_t)array_int[i];
        final_check += *(uint64_t*)&array_double[i];
        final_check ^= (uint64_t)mixed_array[i].i;
    }
    
    /* Print result to prevent optimization */
    printf("Result: 0x%016llx\n", (unsigned long long)final_check);
    printf("Volatile counter: %d\n", vol_counter);
    
    /* Cleanup */
    free(array_int);
    free(array_float);
    free(array_double);
    free(mixed_array);
    
    return 0;
}
