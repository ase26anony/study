/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* Mixed-width operations to create register pressure */
typedef struct {
    int32_t a;
    int64_t b;
    float c;
    double d;
} MixedData;

/* Complex computation with data dependencies */
static inline int64_t complex_op(int64_t acc, int32_t a, int64_t b, float c, double d) {
    /* Mixed-width arithmetic with dependencies */
    int64_t t1 = acc * a;          /* 64x32 -> 64-bit */
    double t2 = (double)t1 * d;    /* Convert and multiply */
    float t3 = c * (float)t2;      /* Float operations */
    int64_t t4 = b + (int64_t)t3;  /* Convert back */
    
    /* Conditional move via ternary */
    return (t4 > 0) ? t4 : -t4;
}

/* Pointer chasing pattern */
static double chase_pointer(double *data, int steps, volatile int *control) {
    double sum = 0.0;
    double *ptr = data;
    
    for (int i = 0; i < steps; i++) {
        /* Volatile read to prevent reordering */
        int offset = *control % 16;
        
        /* Non-trivial addressing mode */
        sum += ptr[offset * 2] * ptr[offset * 2 + 1];
        
        /* Pointer chasing with stride */
        ptr = data + ((ptr - data + offset) % steps);
        
        /* Inline assembly to create fixed RTL */
        asm volatile ("" ::: "memory");
    }
    return sum;
}

/* Main computation kernel with nested loops */
static uint64_t compute_kernel(MixedData *data, int size, volatile int bound) {
    uint64_t total = 0;
    int32_t local_sum = 0;
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < bound; outer++) {
        float fp_acc = 1.0f;
        double dp_acc = 1.0;
        
        /* Inner loop with carried dependency */
        #pragma GCC unroll 4
        for (int i = 1; i < size; i++) {
            /* Data-dependent computation */
            int64_t acc = data[i].b + local_sum;
            
            /* Complex operation with mixed types */
            acc = complex_op(acc, data[i].a, data[i-1].b, 
                           data[i].c, data[i-1].d);
            
            /* Conditional branch with computation in both paths */
            if (acc % 3 == 0) {
                fp_acc *= data[i].c;
                dp_acc /= (data[i].d + 1.0);
                local_sum += (int32_t)(fp_acc * 100);
            } else {
                fp_acc /= data[i].c;
                dp_acc *= (data[i].d + 1.0);
                local_sum -= (int32_t)(dp_acc * 100);
            }
            
            /* Memory access with stride */
            total ^= (uint64_t)(data[i * 2 % size].a * acc);
            
            /* Another inline assembly barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Switch statement creating multiple basic blocks */
        switch (outer % 4) {
            case 0:
                total += (uint64_t)(fp_acc * dp_acc * 1000);
                break;
            case 1:
                total ^= (uint64_t)(local_sum * dp_acc);
                break;
            case 2:
                total *= (uint64_t)(fp_acc * local_sum);
                break;
            default:
                total = (total >> 3) | (total << 61); /* Rotate */
                break;
        }
    }
    
    return total;
}

/* Second computation kernel - matrix-vector style */
static double matrix_vector_kernel(double *matrix, double *vector, int n, 
                                   volatile int iter) {
    double result = 0.0;
    
    for (int it = 0; it < iter; it++) {
        double sum = 0.0;
        
        /* Nested loops for matrix access */
        for (int i = 0; i < n; i++) {
            double row_sum = 0.0;
            for (int j = 0; j < n; j++) {
                /* Non-contiguous access pattern */
                int idx = (i * n + j * 3) % (n * n);
                row_sum += matrix[idx] * vector[j];
                
                /* Division with non-constant divisor */
                if (j > 0) {
                    row_sum /= (vector[j-1] + 1.5);
                }
            }
            
            /* Conditional update */
            sum += (row_sum > 0) ? row_sum : -row_sum;
            
            /* Function call to external function */
            vector[i] = (double)rand() / RAND_MAX;
        }
        
        result += sum / n;
        
        /* Volatile write */
        *(volatile double *)&result = result;
    }
    
    return result;
}

int main(void) {
    const int DATA_SIZE = 1024;
    const int MATRIX_SIZE = 64;
    
    /* Initialize with pseudo-random data */
    srand(g_volatile_seed);
    
    /* Allocate and initialize data */
    MixedData *data = (MixedData *)malloc(DATA_SIZE * sizeof(MixedData));
    double *matrix = (double *)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *vector = (double *)malloc(MATRIX_SIZE * sizeof(double));
    
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i].a = rand() % 1000;
        data[i].b = (int64_t)rand() * rand();
        data[i].c = (float)rand() / RAND_MAX;
        data[i].d = (double)rand() / RAND_MAX * 10.0;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (double)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        vector[i] = (double)rand() / RAND_MAX;
    }
    
    /* Volatile control variable */
    volatile int control = rand() % 100;
    
    printf("Starting complex computations...\n");
    
    /* First kernel - mixed data type computations */
    uint64_t result1 = compute_kernel(data, DATA_SIZE, g_volatile_bound);
    
    /* Pointer chasing pattern */
    double result2 = chase_pointer((double *)data, DATA_SIZE * 2, &control);
    
    /* Second kernel - matrix operations */
    double result3 = matrix_vector_kernel(matrix, vector, MATRIX_SIZE, 
                                         g_volatile_bound % 10);
    
    /* Final reduction to prevent optimization */
    uint64_t final_result = result1 ^ (uint64_t)result2 ^ (uint64_t)result3;
    
    printf("Result: 0x%016lx\n", final_result);
    printf("Control: %d\n", control);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    
    return (final_result > 0) ? 0 : 1;
}
