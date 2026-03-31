/* sel_sched_trigger.c - Program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* External function to create dependencies */
extern int rand_range(int min, int max);

/* Simple PRNG to avoid libc rand() inlining */
static uint32_t prng_state = 123456789;
static uint32_t prng() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Complex data-dependent computation with mixed operations */
static int64_t compute_kernel(int32_t* data, int size, volatile int* bound_ptr) {
    int64_t sum = 0;
    int64_t product = 1;
    double fp_acc = 1.0;
    
    /* Force selective scheduling with complex loop */
    for (int i = 1; i < size; i++) {
        /* Data-dependent carried dependency */
        int32_t prev = data[i-1];
        int32_t curr = data[i];
        
        /* Mixed-width arithmetic creating register pressure */
        int64_t wide_op = (int64_t)prev * (int64_t)curr;
        int32_t narrow_op = prev + curr;
        
        /* Conditional operation with ternary */
        int32_t cond_val = (prev > curr) ? (prev - curr) : (curr - prev);
        
        /* Floating-point operation mixed with integer */
        fp_acc *= (cond_val != 0) ? (1.0 + (double)cond_val * 0.001) : 1.001;
        
        /* Complex addressing mode with stride */
        int idx = (i * 13) % size;
        int32_t mem_val = data[idx];
        
        /* Division with non-constant divisor (prevents optimization) */
        if (mem_val != 0) {
            wide_op /= (mem_val > 0 ? mem_val : 1);
        }
        
        /* Inline assembly to create fixed RTL instructions */
        asm volatile ("" : "+r" (wide_op) : : "memory");
        
        /* Update accumulators with dependencies */
        sum += wide_op + cond_val;
        product *= (narrow_op != 0) ? narrow_op : 1;
        
        /* Volatile access to prevent loop unrolling */
        if (i % (*bound_ptr) == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Final reduction with mixed types */
    return sum + (int64_t)fp_acc + product;
}

/* Matrix-vector multiplication kernel */
static void matvec_kernel(double matrix[4][4], double vector[4], double result[4]) {
    #pragma GCC unroll 4
    for (int i = 0; i < 4; i++) {
        double acc = 0.0;
        
        #pragma GCC unroll 2
        for (int j = 0; j < 4; j++) {
            /* Complex FP operation chain */
            double elem = matrix[i][j];
            double vec_elem = vector[j];
            
            /* Conditional FP operation */
            acc += (elem > 0.0) ? (elem * vec_elem) : (vec_elem / (elem - 1.0));
            
            /* Dependency chain */
            vector[j] = vec_elem * 0.99;
        }
        
        /* Store with potential aliasing */
        result[i] = acc;
        
        /* Inline assembly barrier */
        asm volatile ("" : : : "memory");
    }
}

/* Pointer chasing pattern */
static int64_t pointer_chase(int32_t* data, int size, int steps) {
    int64_t hash = 0;
    int pos = 0;
    
    for (int i = 0; i < steps; i++) {
        /* Chase through array with data-dependent stride */
        int32_t val = data[pos];
        int stride = (val % 7) + 1;
        
        /* Mixed operations on loaded value */
        hash ^= (int64_t)val << (i % 32);
        hash += (hash >> 32) | (hash << 32);
        
        /* Complex conditional update */
        pos = (pos + stride) % size;
        if (pos < 0) pos += size;
        
        /* Volatile check every 8 iterations */
        if ((i & 7) == 0) {
            volatile int check = g_volatile_bound;
            pos = pos % (check > 0 ? check : size);
        }
    }
    
    return hash;
}

/* Branch-heavy computation */
static double branchy_computation(float* array, int size) {
    double result = 0.0;
    
    for (int i = 0; i < size; i++) {
        float val = array[i];
        
        /* Switch-like branching with different operations */
        int category = ((int)val) % 5;
        
        switch (category) {
            case 0:
                result += val * 1.5;
                break;
            case 1:
                result -= val / 2.0;
                break;
            case 2:
                result *= (1.0 + val * 0.01);
                break;
            case 3:
                result = (result > 0) ? result + val : result - val;
                break;
            case 4:
                /* Complex FP operation */
                result = result * cos(val * 0.01);
                break;
            default:
                result += 1.0;
        }
        
        /* Data-dependent array update */
        array[i] = (float)(result * 0.1);
    }
    
    return result;
}

int main() {
    const int DATA_SIZE = 1024;
    const int ITERATIONS = 100;
    
    /* Initialize with pseudo-random data */
    int32_t* int_data = (int32_t*)malloc(DATA_SIZE * sizeof(int32_t));
    float* float_data = (float*)malloc(DATA_SIZE * sizeof(float));
    
    /* Use volatile seed */
    prng_state = g_volatile_seed;
    
    for (int i = 0; i < DATA_SIZE; i++) {
        int_data[i] = (int32_t)(prng() % 1000) - 500;
        float_data[i] = (float)(prng() % 1000) / 100.0f - 5.0f;
    }
    
    double matrix[4][4];
    double vector[4] = {1.0, 2.0, 3.0, 4.0};
    double result[4];
    
    /* Initialize matrix with PRNG data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = (double)(prng() % 100) / 10.0;
        }
    }
    
    int64_t total_result = 0;
    double fp_result = 0.0;
    
    /* Main computation loop with multiple kernels */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Update volatile bound occasionally */
        if (iter % 17 == 0) {
            g_volatile_bound = 500 + (iter % 500);
        }
        
        /* Kernel 1: Data-dependent integer computation */
        int64_t kernel1 = compute_kernel(int_data, DATA_SIZE, &g_volatile_bound);
        
        /* Kernel 2: Pointer chasing */
        int64_t kernel2 = pointer_chase(int_data, DATA_SIZE, 1000);
        
        /* Kernel 3: Matrix-vector (creates FP scheduling pressure) */
        matvec_kernel(matrix, vector, result);
        
        /* Kernel 4: Branch-heavy computation */
        fp_result += branchy_computation(float_data, DATA_SIZE);
        
        /* Combine results with non-linear operation */
        total_result ^= kernel1 + kernel2 + (int64_t)fp_result;
        
        /* Modify data for next iteration to prevent dead code elimination */
        for (int i = 0; i < DATA_SIZE; i += 8) {
            int_data[i] += (iter % 3);
            float_data[i] *= 0.99f;
        }
        
        /* External function call to prevent optimization */
        if (iter % 100 == 0) {
            int r = rand_range(1, 100);
            total_result += r;
        }
    }
    
    /* Final reduction to observable output */
    printf("Result: %ld (fp: %.6f)\n", 
           (long)total_result, fp_result);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    
    return 0;
}

/* External function definition */
int rand_range(int min, int max) {
    return min + (prng() % (max - min + 1));
}
