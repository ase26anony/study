/* haifa-sched-coverage.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa-sched-coverage.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define MAX_DEPTH 32

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile float g_volatile_float = 0.0f;

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

/* Small helper functions with different computation patterns */
static int helper1(int a, int b) {
    int result = a * b;
    result += (a << 3);
    result ^= (b >> 2);
    /* Memory barrier */
    asm volatile("" ::: "memory");
    return result;
}

static int helper2(int a, int b) {
    float temp = (float)a * 1.5f;
    int result = (int)temp + b;
    result = result * 7 - 3;
    g_volatile_counter++;
    return result;
}

static int helper3(int a, int b) {
    double d = (double)a / (b + 1);
    int result = (int)(d * 100.0);
    /* Force memory dependency */
    g_volatile_float = (float)d;
    return result + g_volatile_counter;
}

/* Non-inlineable function to create scheduling boundary */
__attribute__((noinline)) static int complex_chain(int start) {
    int a = start * 1103515245 + 12345;
    int b = (a >> 16) & 32767;
    int c = b * 16807;
    int d = c % 2147483647;
    
    /* Chain of dependent operations */
    int e = d * 3;
    int f = e + a;
    int g = f ^ b;
    int h = g << 2;
    int i = h / (c + 1);
    int j = i - d;
    int k = j * 7;
    int l = k & 0xFFF;
    
    return l;
}

/* Function with switch statement creating multiple basic blocks */
static int switch_computation(int value, int* array, int idx) {
    int result = 0;
    
    switch (value % 10) {
        case 0:
            result = array[idx] * 3 + array[idx + 1];
            result ^= 0x55AA55AA;
            break;
        case 1:
            result = (array[idx] << 4) | (array[idx] >> 28);
            result += helper1(array[idx], idx);
            break;
        case 2:
            result = array[idx] * array[idx - 1];
            result -= complex_chain(result);
            break;
        case 3:
            result = array[idx] + array[idx + 2] * 2;
            /* Memory barrier */
            asm volatile("" ::: "memory");
            result ^= result >> 16;
            break;
        case 4:
            result = helper2(array[idx], idx);
            result = result * 11 % 1024;
            break;
        case 5:
            result = array[idx] * 5 - array[idx + 3];
            result = (result << 1) | (result >> 31);
            break;
        case 6:
            result = helper3(array[idx], idx);
            result = result & 0xFFFF;
            break;
        case 7:
            result = array[idx] ^ array[idx + 1] ^ array[idx + 2];
            result = result * 13 + 7;
            break;
        case 8:
            result = (array[idx] * array[idx]) / (idx + 1);
            result += g_volatile_counter;
            break;
        case 9:
            result = array[idx] % 97;
            result = result * result * result;
            break;
    }
    
    return result;
}

/* Pointer chasing through array */
static int pointer_chase(int* array, int start_idx, int steps) {
    int idx = start_idx;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        sum += array[idx];
        /* Next index depends on current value (creates dependency) */
        idx = (array[idx] + i) % ARRAY_SIZE;
        /* Small computation at each step */
        sum ^= (sum << 3) | (sum >> 29);
    }
    
    return sum;
}

/* Nested loops with loop-carried dependencies */
static int nested_dependent_compute(int* arr1, float* arr2, double* arr3, int size) {
    int total = 0;
    float ftotal = 0.0f;
    double dtotal = 0.0;
    
    for (int i = 1; i < size - 1; i++) {
        /* Outer loop dependency */
        int base = arr1[i] + total % 256;
        
        for (int j = 0; j < 8; j++) {
            /* Mixed type computations */
            ftotal += (float)arr1[i + j] * 0.5f;
            dtotal += (double)arr2[i - j] * 1.5;
            
            /* Dependent chain */
            int temp = base * j;
            temp += (int)(ftotal * 10.0f);
            temp ^= (int)dtotal;
            
            /* Conditional store */
            if (temp & 1) {
                arr3[i] += (double)temp * 0.01;
            } else {
                arr3[i] -= (double)temp * 0.005;
            }
            
            /* Update volatile */
            g_volatile_float = ftotal;
        }
        
        /* Loop-carried dependency */
        total += (int)arr3[i] + (int)ftotal;
        total = total * 3 - 7;
    }
    
    return total;
}

int main(int argc, char** argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct mixed_data* mixed_array = (struct mixed_data*)malloc(ARRAY_SIZE * sizeof(struct mixed_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        float_array[i] = (float)(int_array[i] % 1000) * 0.001f;
        double_array[i] = (double)(int_array[i] % 2000) * 0.0005;
        
        mixed_array[i].c = (char)(i & 0xFF);
        mixed_array[i].i = int_array[i];
        mixed_array[i].d = double_array[i];
        mixed_array[i].f = float_array[i];
        mixed_array[i].s = (short)(i * 3);
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {helper1, helper2, helper3};
    
    int final_result = 0;
    
    /* Main computation loop */
    for (int iter = 0; iter < iterations; iter++) {
        int base_idx = iter % (ARRAY_SIZE - MAX_DEPTH);
        
        /* 1. Pointer chasing (creates memory dependencies) */
        int chase_result = pointer_chase(int_array, base_idx, MAX_DEPTH);
        
        /* 2. Nested loops with dependencies */
        int nested_result = nested_dependent_compute(int_array, float_array, double_array, ARRAY_SIZE / 2);
        
        /* 3. Switch-based computation with multiple basic blocks */
        int switch_result = switch_computation(iter, int_array, base_idx);
        
        /* 4. Deep conditional chain */
        int cond_result = 0;
        if (iter & 1) {
            cond_result = helper1(chase_result, nested_result);
            if (iter & 2) {
                cond_result += helper2(switch_result, iter);
                if (iter & 4) {
                    cond_result ^= helper3(cond_result, switch_result);
                    if (iter & 8) {
                        cond_result = complex_chain(cond_result);
                    }
                }
            }
        } else {
            cond_result = nested_result * 3 - switch_result;
        }
        
        /* 5. Computed jump (function pointer call) */
        int func_idx = iter % 3;
        int func_result = funcs[func_idx](cond_result, chase_result);
        
        /* 6. Mixed data type accesses with packed struct */
        struct mixed_data* mixed_ptr = &mixed_array[base_idx];
        int mixed_calc = mixed_ptr->i * (int)mixed_ptr->c;
        mixed_calc += (int)(mixed_ptr->d * 100.0);
        mixed_calc ^= (int)(mixed_ptr->f * 1000.0f);
        mixed_calc += mixed_ptr->s;
        
        /* 7. Large sequential basic block (fills instruction queue) */
        int seq_result = 0;
        for (int i = 0; i < 64; i++) {
            /* Independent operations within loop - scheduler can reorder */
            int idx = (base_idx + i) % ARRAY_SIZE;
            int_array[idx] = int_array[idx] * 3 + i;
            float_array[idx] = float_array[idx] * 1.1f + (float)i;
            double_array[idx] = double_array[idx] * 0.9 - (double)i;
            
            /* But with occasional dependencies */
            if (i % 8 == 0) {
                seq_result += int_array[idx];
                seq_result ^= (int)(float_array[idx] * 100.0f);
            }
        }
        
        /* Combine all results with complex dependency chain */
        int combined = chase_result + nested_result;
        combined = combined * switch_result - cond_result;
        combined = combined ^ func_result;
        combined = combined + mixed_calc * 2;
        combined = combined - seq_result / 4;
        
        /* Final reduction with memory barrier */
        asm volatile("" ::: "memory");
        final_result ^= combined;
        final_result = final_result * 31 + 17;
        
        /* Update volatile variables */
        g_volatile_counter += iter & 0xFF;
        g_volatile_float = (float)final_result * 0.001f;
    }
    
    /* Final reduction across arrays */
    int array_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_sum ^= int_array[i];
        array_sum += (int)(float_array[i] * 1000.0f);
        array_sum ^= (int)(double_array[i] * 10000.0);
    }
    
    final_result ^= array_sum;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d (iterations: %d)\n", final_result, iterations);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(mixed_array);
    
    return 0;
}
