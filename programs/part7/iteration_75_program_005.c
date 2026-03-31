/* haifa_sched_trigger.c
 * Program designed to trigger GCC's HAIFA scheduler state save/restore
 * and exercise the free_state function uncovered lines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define MAX_DEPTH 16

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_barrier = 0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) PackedData {
    char c;
    int i;
    double d;
    char c2;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that create scheduling boundaries */
static int helper1(int a, int b) {
    volatile int local_barrier = g_volatile_barrier;
    int result = a * b + (local_barrier & 1);
    g_volatile_counter += result;
    return result;
}

static int helper2(int a, int b) {
    volatile int local_barrier = g_volatile_barrier;
    int result = (a << 3) | (b & 0xFF);
    g_volatile_counter ^= result;
    return result;
}

static int helper3(int a, int b) {
    volatile int local_barrier = g_volatile_barrier;
    int result = a % (b | 1);
    g_volatile_counter -= result;
    return result;
}

/* Function with complex control flow */
static int complex_switch(int value, int* array, double* darray) {
    int result = 0;
    
    /* Deep conditional chain */
    if (value < 10) {
        result = helper1(value, array[value]);
    } else if (value < 20) {
        result = helper2(value, array[value % ARRAY_SIZE]);
    } else if (value < 30) {
        result = helper3(value, array[value % ARRAY_SIZE]);
    } else if (value < 40) {
        result = array[value % ARRAY_SIZE] * 3;
    } else {
        result = (int)darray[value % ARRAY_SIZE] + value;
    }
    
    /* Switch with many cases */
    switch (value % 10) {
        case 0:
            result += array[(value + 1) % ARRAY_SIZE];
            break;
        case 1:
            result -= (int)darray[(value + 2) % ARRAY_SIZE];
            break;
        case 2:
            result *= array[(value + 3) % ARRAY_SIZE] | 1;
            break;
        case 3:
            result /= (array[(value + 4) % ARRAY_SIZE] | 1) + 1;
            break;
        case 4:
            result ^= array[(value + 5) % ARRAY_SIZE];
            break;
        case 5:
            result &= array[(value + 6) % ARRAY_SIZE];
            break;
        case 6:
            result |= array[(value + 7) % ARRAY_SIZE];
            break;
        case 7:
            result <<= (value % 8);
            break;
        case 8:
            result >>= (value % 8);
            break;
        case 9:
            result = ~result;
            break;
    }
    
    return result;
}

/* Pointer chasing simulation */
static int pointer_chase(int* array, int start_idx, int steps) {
    int idx = start_idx;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
        
        sum += array[idx];
        idx = array[idx] % ARRAY_SIZE;
        
        /* Volatile access creates scheduling hazard */
        g_volatile_barrier = i;
    }
    
    return sum;
}

/* Large basic block with many independent operations */
static void large_basic_block(int* results, double* darray, struct PackedData* pdata, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Many independent operations to fill instruction queue */
        int idx = i % ARRAY_SIZE;
        
        /* Operation chain 1 */
        int a = results[idx];
        int b = results[(idx + 1) % ARRAY_SIZE];
        int c = a * b + i;
        int d = c ^ (a << 3);
        int e = d % (b | 1);
        results[idx] = e;
        
        /* Operation chain 2 */
        double x = darray[idx];
        double y = darray[(idx + 2) % ARRAY_SIZE];
        double z = x * y + (double)i;
        double w = z / (y + 1.0);
        darray[idx] = w;
        
        /* Operation chain 3 - mixed types */
        pdata[idx].c = (char)(i & 0xFF);
        pdata[idx].i = pdata[idx].i * 3 + i;
        pdata[idx].d = pdata[idx].d * 1.01;
        pdata[idx].c2 = (char)((i >> 8) & 0xFF);
        
        /* Memory barrier every 8 iterations */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

/* Function with computed goto-like behavior using function pointers */
static int computed_jump_computation(int x, int y, int selector) {
    static compute_func_t funcs[] = {helper1, helper2, helper3};
    int result = x;
    
    /* Non-linear control flow */
    for (int i = 0; i < MAX_DEPTH; i++) {
        int idx = (selector + i) % 3;
        result = funcs[idx](result, y + i);
        
        /* Conditional with both branches having work */
        if (result & 1) {
            result = helper1(result, i);
        } else {
            result = helper2(result, i);
        }
    }
    
    return result;
}

int main(int argc, char** argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    struct PackedData* packed_array = (struct PackedData*)malloc(ARRAY_SIZE * sizeof(struct PackedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFFFFFF;
        double_array[i] = (double)(i * 1103515245) / 1000.0;
        float_array[i] = (float)(i * 1103515245) / 1000.0f;
        packed_array[i].c = (char)i;
        packed_array[i].i = i * 3;
        packed_array[i].d = (double)i * 1.5;
        packed_array[i].c2 = (char)(i >> 1);
    }
    
    int final_result = 0;
    
    /* Primary computation loop with complex control flow */
    for (int i = 0; i < iterations; i++) {
        /* Pointer chasing */
        int chase_result = pointer_chase(int_array, i % ARRAY_SIZE, 8);
        
        /* Complex switch-based computation */
        int switch_result = complex_switch(i, int_array, double_array);
        
        /* Computed jump computation */
        int jump_result = computed_jump_computation(i, chase_result, switch_result % 3);
        
        /* Large basic block every 16 iterations */
        if ((i % 16) == 0) {
            large_basic_block(int_array, double_array, packed_array, 32);
        }
        
        /* Nested loop with loop-carried dependency */
        int nested_sum = 0;
        for (int j = 0; j < 8; j++) {
            int idx = (i + j) % ARRAY_SIZE;
            nested_sum += int_array[idx] * int_array[(idx + ARRAY_SIZE - 1) % ARRAY_SIZE];
            
            /* Mixed floating-point operations */
            double_array[idx] = double_array[idx] * 0.99 + float_array[idx];
            float_array[idx] = float_array[idx] * 0.9f + (float)int_array[idx];
        }
        
        /* Final reduction with volatile access */
        g_volatile_barrier = i;
        final_result ^= chase_result + switch_result + jump_result + nested_sum;
        final_result += g_volatile_counter;
        
        /* Conditional with function call */
        if (i & 1) {
            final_result += helper1(i, final_result);
        } else {
            final_result += helper2(i, final_result);
        }
    }
    
    /* Additional reduction across all arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result ^= int_array[i];
        final_result += (int)double_array[i];
        final_result += (int)float_array[i];
        final_result += packed_array[i].i;
    }
    
    /* Prevent dead code elimination */
    printf("Final result: %d\n", final_result);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(packed_array);
    
    return 0;
}
