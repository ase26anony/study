/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o trigger
 * Or for more aggressive: gcc -O3 -fsel-sched-pipelining -funroll-loops -march=native -fdump-rtl-all sel-sched-trigger.c -o trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Mixed-width operations to create register pressure */
typedef struct {
    int data32;
    long data64;
    float f32;
    double f64;
} MixedData;

/* Simple PRNG to avoid library call overhead in tight loops */
static inline unsigned int simple_rand(unsigned int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Function with complex control flow */
static long process_chunk(MixedData *arr, int start, int end, volatile int *v_counter) {
    long acc64 = 0;
    int acc32 = 0;
    float f_acc = 0.0f;
    double d_acc = 0.0;
    
    /* Nested loops with data dependencies */
    for (int i = start; i < end; i++) {
        /* Volatile access to prevent optimization */
        (*v_counter)++;
        
        /* Complex addressing with mixed operations */
        MixedData *curr = &arr[i];
        MixedData *prev = (i > 0) ? &arr[i-1] : &arr[end-1];
        
        /* Data-dependent computation with carried dependency */
        int temp32 = curr->data32 * prev->data32;
        long temp64 = curr->data64 / (prev->data64 ? prev->data64 : 1);
        
        /* Conditional operations creating multiple basic blocks */
        if (temp32 > 1000) {
            /* Branch 1: Floating point intensive */
            f_acc += curr->f32 * prev->f32;
            d_acc += curr->f64 / (prev->f64 ? prev->f64 : 1.0);
            
            /* Mixed width operations */
            acc32 += temp32 >> 2;
            acc64 += temp64 * 3;
        } else if (temp32 > 500) {
            /* Branch 2: Integer intensive */
            acc32 += temp32 * 2;
            acc64 += temp64 / 2;
            
            /* Floating point with conversion */
            f_acc += (float)temp32 * 0.5f;
            d_acc += (double)temp64 * 0.25;
        } else {
            /* Branch 3: Bit manipulation */
            acc32 ^= temp32;
            acc64 |= temp64;
            
            /* Minimal inline assembly to create fixed RTL */
            asm volatile ("" : : : "memory");
        }
        
        /* Switch statement for additional control flow complexity */
        switch (temp32 & 0x3) {
            case 0:
                acc32 += curr->data32 & 0xFF;
                break;
            case 1:
                acc64 ^= curr->data64 & 0xFFFF;
                break;
            case 2:
                f_acc += (curr->data32 & 0xF) * 0.1f;
                break;
            case 3:
                d_acc += (curr->data64 & 0xF) * 0.01;
                /* Another inline assembly barrier */
                asm volatile ("" : : : "memory");
                break;
        }
        
        /* Pointer chasing pattern */
        if (i % 4 == 0) {
            MixedData *chase = curr;
            for (int j = 0; j < 3; j++) {
                chase = &arr[chase->data32 % end];
                acc32 += chase->data32;
            }
        }
    }
    
    /* Final reduction mixing all accumulators */
    return acc64 + acc32 + (long)f_acc + (long)d_acc;
}

/* Matrix-vector like computation for additional scheduling regions */
static void matrix_vector_op(MixedData *matrix, MixedData *vector, MixedData *result, int size) {
    volatile int v_ctr = 0;
    
    #pragma GCC unroll 4
    for (int i = 0; i < size; i++) {
        long sum64 = 0;
        int sum32 = 0;
        float sum_f = 0.0f;
        double sum_d = 0.0;
        
        /* Inner loop with stride access pattern */
        for (int j = 0; j < size; j++) {
            v_ctr++;
            
            /* Complex addressing with mixed operations */
            int idx = (i * size + j) % (size * size);
            MixedData *m = &matrix[idx];
            MixedData *v = &vector[j];
            
            /* Mixed computations creating instruction diversity */
            sum32 += m->data32 * v->data32;
            sum64 += m->data64 * v->data64;
            sum_f += m->f32 * v->f32;
            sum_d += m->f64 * v->f64;
            
            /* Data-dependent conditional */
            if ((sum32 & 0xFF) > 128) {
                sum32 >>= 1;
                sum64 >>= 1;
            }
        }
        
        /* Store results with mixed operations */
        result[i].data32 = sum32;
        result[i].data64 = sum64;
        result[i].f32 = sum_f;
        result[i].f64 = sum_d;
        
        /* External function call to prevent optimization */
        if (i % 100 == 0) {
            result[i].data32 = rand() % 256;
        }
    }
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int MATRIX_SIZE = 64;
    
    /* Initialize with pseudo-random data */
    unsigned int seed = (unsigned int)time(NULL) ^ volatile_seed;
    MixedData *array = (MixedData*)malloc(ARRAY_SIZE * sizeof(MixedData));
    MixedData *matrix = (MixedData*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(MixedData));
    MixedData *vector = (MixedData*)malloc(MATRIX_SIZE * sizeof(MixedData));
    MixedData *result = (MixedData*)malloc(MATRIX_SIZE * sizeof(MixedData));
    
    if (!array || !matrix || !vector || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with varied data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        unsigned int r = simple_rand(&seed);
        array[i].data32 = r;
        array[i].data64 = (long)r * r;
        array[i].f32 = (float)r / 1000.0f;
        array[i].f64 = (double)r / 10000.0;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        unsigned int r = simple_rand(&seed);
        matrix[i].data32 = r % 100;
        matrix[i].data64 = (long)r * (i % 10 + 1);
        matrix[i].f32 = (float)(r % 1000) / 10.0f;
        matrix[i].f64 = (double)(r % 10000) / 100.0;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        unsigned int r = simple_rand(&seed);
        vector[i].data32 = r % 50;
        vector[i].data64 = (long)r * 2;
        vector[i].f32 = (float)(r % 500) / 5.0f;
        vector[i].f64 = (double)(r % 5000) / 50.0;
    }
    
    volatile int v_counter = 0;
    long total_result = 0;
    
    /* First computation kernel: Complex loop with dependencies */
    int iterations = volatile_bound % ARRAY_SIZE;
    if (iterations < 100) iterations = 1000;
    
    for (int chunk = 0; chunk < iterations; chunk += 128) {
        int end = (chunk + 128 < iterations) ? chunk + 128 : iterations;
        total_result ^= process_chunk(array, chunk, end, &v_counter);
        
        /* Volatile bound check to prevent optimization */
        if (v_counter > 1000000) {
            v_counter = 0;
        }
    }
    
    /* Second computation kernel: Matrix operations */
    matrix_vector_op(matrix, vector, result, MATRIX_SIZE);
    
    /* Final reduction across both computations */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        total_result += result[i].data64;
        total_result ^= result[i].data32;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %ld (volatile counter: %d)\n", total_result, v_counter);
    
    free(array);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
