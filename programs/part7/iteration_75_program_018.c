/* haifa_sched_trigger.c
 * Designed to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fno-omit-frame-pointer haifa_sched_trigger.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define MAX_DEPTH 32

/* Volatile variables to prevent optimization and create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_barrier = 0;

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
    int t2 = b * 1664525 + 1013904223;
    int t3 = t1 ^ t2;
    int t4 = t3 * 3 + t1;
    return t4 ^ (t2 >> 16);
}

static int compute_chain_b(int a, int b) {
    float f1 = (float)a * 0.5f;
    float f2 = (float)b * 1.5f;
    int i1 = (int)(f1 * f2);
    int i2 = i1 + (a & 0xFF) - (b & 0xFF);
    return i2 * 7 - 13;
}

static double compute_chain_c(double a, int b) {
    double d1 = a * 1.6180339887;
    double d2 = d1 + (double)b * 0.3141592653;
    double d3 = d2 * d2 - d1 * d1;
    return d3 * 0.5;
}

/* Non-inlineable function (due to complexity) to create scheduling boundaries */
__attribute__((noinline)) 
static int complex_transform(int x, int y) {
    struct mixed_data md;
    md.c = (char)(x & 0xFF);
    md.i = x * y - 123;
    md.d = (double)x / (y + 1);
    md.s = (short)(y & 0xFFFF);
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
    
    int result = md.i + (int)md.d + md.s + md.c;
    g_volatile_counter++;
    
    return result ^ 0x55AA55AA;
}

/* Another non-inlineable function with loop-carried dependencies */
__attribute__((noinline))
static int pointer_chase(int *array, int size, int start) {
    int idx = start;
    int sum = 0;
    
    for (int i = 0; i < MAX_DEPTH; i++) {
        /* Pointer chasing with data dependency */
        idx = array[idx % size];
        sum += idx * i;
        
        /* Volatile access creates scheduling hazard */
        sum ^= g_volatile_barrier;
        
        /* Conditional with data-dependent branch */
        if (idx & 1) {
            sum = compute_chain_a(sum, i);
        } else {
            sum = compute_chain_b(sum, idx);
        }
    }
    
    return sum;
}

/* Function with switch statement creating multiple basic blocks */
static int switch_computation(int value, int mod) {
    int result = value;
    
    switch (mod) {
        case 0:
            result = result * 3 + 1;
            /* Fall through */
        case 1:
            result ^= 0xAAAAAAAA;
            result = result >> 1;
            break;
        case 2:
            result = complex_transform(result, mod);
            break;
        case 3:
            result = result + (result << 2) + (result << 3);
            break;
        case 4:
            result = (int)compute_chain_c((double)result, mod);
            break;
        case 5:
            result = result | 0x55555555;
            result = result & 0x33333333;
            break;
        case 6:
            result = result - (result / 7) * 7;
            break;
        case 7:
            result = (result << 4) | (result >> 28);
            break;
        case 8:
            result = result ^ (result << 1);
            result = result ^ (result << 2);
            break;
        case 9:
            result = ~result;
            break;
        default:
            result = 0;
    }
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    return result;
}

/* Main computation with complex control flow */
static uint64_t run_computation(int iterations, int *int_array, 
                               double *double_array, float *float_array,
                               struct mixed_data *struct_array) {
    uint64_t total = 0;
    int idx = 0;
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {
        compute_chain_a,
        compute_chain_b,
        NULL,
        NULL,
        NULL
    };
    
    for (int i = 0; i < iterations; i++) {
        int local_sum = 0;
        
        /* 1. Pointer chasing through integer array */
        idx = pointer_chase(int_array, ARRAY_SIZE, i);
        local_sum ^= idx;
        
        /* 2. Chain of dependent arithmetic operations */
        double d1 = double_array[i % ARRAY_SIZE];
        double d2 = double_array[(i + 1) % ARRAY_SIZE];
        double d3 = d1 * d2 + 1.41421356;
        double d4 = d3 / (d2 + 0.000001);
        int di = (int)(d4 * 1000.0);
        local_sum += di;
        
        /* 3. Switch statement with multiple computation paths */
        local_sum = switch_computation(local_sum, i % 10);
        
        /* 4. Conditional with function call */
        if (i & 1) {
            local_sum = complex_transform(local_sum, i);
        } else if (i & 2) {
            /* Computed jump */
            if (funcs[i % 2]) {
                local_sum = funcs[i % 2](local_sum, i);
            }
        }
        
        /* 5. Large basic block with many independent operations */
        /* This fills the instruction queue */
        float f_acc = float_array[0];
        for (int j = 0; j < 64; j++) {
            /* Independent operations - scheduler can reorder these */
            float_array[(i + j) % ARRAY_SIZE] = 
                float_array[(i + j) % ARRAY_SIZE] * 1.1f + 0.5f;
            f_acc += float_array[j % ARRAY_SIZE];
            
            /* Mixed data type access */
            struct_array[j % ARRAY_SIZE].i = 
                struct_array[j % ARRAY_SIZE].i + j;
            struct_array[j % ARRAY_SIZE].d = 
                struct_array[j % ARRAY_SIZE].d * 0.99;
        }
        local_sum += (int)f_acc;
        
        /* 6. Nested loop with loop-carried dependency */
        int carry = local_sum;
        for (int k = 0; k < 8; k++) {
            carry = carry * 3 + int_array[k];
            for (int m = 0; m < 4; m++) {
                carry ^= (carry >> 1);
                /* Volatile access in inner loop */
                carry += g_volatile_counter;
            }
        }
        local_sum = carry;
        
        /* 7. Memory operations with varying alignments */
        char *byte_ptr = (char *)&struct_array[i % ARRAY_SIZE];
        for (int b = 0; b < 16; b++) {
            byte_ptr[b] ^= (i + b) & 0xFF;
        }
        
        /* Accumulate result */
        total += (uint64_t)local_sum;
        
        /* Update volatile for next iteration */
        g_volatile_barrier = i & 0xFF;
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    printf("Running HAIFA scheduler test with %d iterations\n", iterations);
    
    /* Allocate and initialize arrays with different data types */
    int *int_array = (int *)malloc(ARRAY_SIZE * sizeof(int));
    double *double_array = (double *)malloc(ARRAY_SIZE * sizeof(double));
    float *float_array = (float *)malloc(ARRAY_SIZE * sizeof(float));
    struct mixed_data *struct_array = 
        (struct mixed_data *)malloc(ARRAY_SIZE * sizeof(struct mixed_data));
    
    if (!int_array || !double_array || !float_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        double_array[i] = (double)(i * 1664525 + 1013904223) / 1000000.0;
        float_array[i] = (float)(i * 1103515245) / 1000000000.0f;
        
        struct_array[i].c = (char)(i & 0xFF);
        struct_array[i].i = i * 3 - 7;
        struct_array[i].d = (double)i * 1.23456789;
        struct_array[i].s = (short)(i & 0x7FFF);
    }
    
    /* Create linked-list-like structure in int_array */
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        int_array[i] = (i + 1) % ARRAY_SIZE;
    }
    int_array[ARRAY_SIZE - 1] = 0;
    
    /* Run the main computation */
    clock_t start = clock();
    uint64_t result = run_computation(iterations, int_array, 
                                     double_array, float_array, 
                                     struct_array);
    clock_t end = clock();
    
    /* Perform final reduction across all arrays */
    uint64_t final_check = result;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_check ^= (uint64_t)int_array[i];
        final_check += (uint64_t)(double_array[i] * 1000.0);
        final_check ^= (uint64_t)(float_array[i] * 1000.0f);
        final_check += struct_array[i].i + struct_array[i].c + struct_array[i].s;
    }
    
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Result: 0x%016llX\n", (unsigned long long)final_check);
    printf("Time: %.3f seconds\n", elapsed);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(struct_array);
    
    return 0;
}
