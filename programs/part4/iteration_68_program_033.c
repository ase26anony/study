/* sel_sched_trigger.c - Program to trigger selective scheduler debug dumping */
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
static inline int64_t data_dependent_compute(int32_t a, int64_t b, float c, double d) {
    /* Mixed operations that create scheduling complexity */
    double temp1 = (double)a * d + (double)b / (c + 1.0f);
    float temp2 = c * (float)d - (float)a / (float)(b & 0xFF);
    
    /* Conditional move via ternary */
    double result = (temp1 > temp2) ? temp1 * 0.5 : temp2 * 2.0;
    
    /* Non-constant divisor */
    int divisor = (a % 7) + 1;
    return (int64_t)(result / divisor);
}

/* Pointer chasing pattern */
static int64_t pointer_chase(MixedData* array, int size, int start) {
    int idx = start;
    int64_t sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent index calculation */
        idx = (idx * 13 + array[idx].a) % size;
        
        /* Complex computation with dependencies */
        sum += data_dependent_compute(
            array[idx].a,
            array[idx].b,
            array[idx].c,
            array[idx].d
        );
        
        /* Inline assembly to create fixed RTL instructions */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* Nested loop with control flow */
static double nested_loop_compute(MixedData* data, int rows, int cols) {
    double total = 0.0;
    volatile int outer_bound = rows; /* Prevent optimization */
    
    /* Outer loop with volatile bound */
    for (int i = 0; i < outer_bound; i++) {
        double row_sum = 0.0;
        
        /* Inner loop with partial unrolling hint */
        #pragma GCC unroll 4
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Complex addressing mode */
            MixedData* elem = &data[idx];
            
            /* Branch with substantial computation in both paths */
            if (elem->a > 0) {
                /* Path 1: Floating-point intensive */
                row_sum += elem->c * elem->d + 
                          (elem->b % 256) * 0.01;
            } else {
                /* Path 2: Integer intensive */
                row_sum += (double)(elem->a * elem->b) / 
                          (abs(elem->a) + 1);
            }
            
            /* Cross-iteration dependency */
            if (j > 0) {
                row_sum += data[idx - 1].c * 0.1;
            }
        }
        
        total += row_sum;
        
        /* External function call prevents optimization */
        if (i % 100 == 0) {
            total *= (1.0 + (rand() % 100) * 0.0001);
        }
    }
    
    return total;
}

/* Switch statement with multiple cases */
static int switch_based_compute(int x, MixedData* data) {
    int result = 0;
    
    switch (x % 5) {
        case 0:
            result = data[x].a * 2 + data[x].b / 3;
            /* Memory barrier */
            asm volatile("" ::: "memory");
            break;
        case 1:
            result = (int)(data[x].c * 100.0f) - data[x].a;
            break;
        case 2:
            result = data[x].a * data[x].a - data[x].b;
            break;
        case 3:
            result = (data[x].a << 3) | (data[x].b & 0xFF);
            break;
        case 4:
            result = (int)(data[x].d * 10.0) ^ data[x].a;
            break;
    }
    
    return result;
}

/* Matrix-vector multiplication kernel */
static void matrix_vector_multiply(double* result, MixedData* matrix, 
                                   double* vector, int n) {
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        
        /* Strided access pattern */
        for (int j = 0; j < n; j += 2) {
            int idx = i * n + j;
            
            /* Mixed operations */
            sum += matrix[idx].c * vector[j] +
                   matrix[idx].d * vector[j + 1];
            
            /* Dependency chain */
            if (j > 0) {
                sum -= matrix[idx - 1].c * 0.5;
            }
        }
        
        result[i] = sum;
        
        /* Volatile operation */
        volatile double* vptr = &result[i];
        *vptr = *vptr * (1.0 + (i % 10) * 0.01);
    }
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int MATRIX_SIZE = 64;
    
    /* Initialize with pseudo-random data */
    srand(g_volatile_seed);
    
    MixedData* data = (MixedData*)malloc(ARRAY_SIZE * sizeof(MixedData));
    double* vector = (double*)malloc(MATRIX_SIZE * sizeof(double));
    double* matrix_result = (double*)malloc(MATRIX_SIZE * sizeof(double));
    
    /* Fill arrays with varied data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i].a = (rand() % 1000) - 500;
        data[i].b = (int64_t)rand() * rand();
        data[i].c = (rand() % 1000) / 10.0f;
        data[i].d = (rand() % 10000) / 100.0;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        vector[i] = (rand() % 100) / 10.0;
    }
    
    int64_t final_result = 0;
    
    /* Kernel 1: Pointer chasing with complex dependencies */
    final_result ^= pointer_chase(data, ARRAY_SIZE, 0);
    
    /* Kernel 2: Nested loops with control flow */
    double nested_result = nested_loop_compute(data, 32, 32);
    final_result ^= (int64_t)(nested_result * 1000.0);
    
    /* Kernel 3: Switch-based computation */
    int switch_sum = 0;
    for (int i = 0; i < g_volatile_bound && i < ARRAY_SIZE; i++) {
        switch_sum += switch_based_compute(i, data);
    }
    final_result ^= switch_sum;
    
    /* Kernel 4: Matrix-vector multiplication */
    matrix_vector_multiply(matrix_result, data, vector, MATRIX_SIZE);
    
    /* Final reduction to prevent optimization */
    double final_reduction = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        final_reduction += matrix_result[i];
    }
    final_result ^= (int64_t)(final_reduction * 100.0);
    
    /* Ensure side effect */
    printf("Final result: %ld\n", (long)final_result);
    
    free(data);
    free(vector);
    free(matrix_result);
    
    return 0;
}
