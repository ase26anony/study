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
    /* Mixed-width arithmetic with dependencies */
    int64_t temp1 = (int64_t)a * b;
    double temp2 = (double)temp1 / (d + 1.0);
    float temp3 = c * (float)temp2;
    
    /* Conditional move via ternary */
    int64_t result = (temp2 > 0.0) ? 
                     (int64_t)(temp3 * 100.0) : 
                     -(int64_t)(temp3 * 100.0);
    
    /* Non-constant divisor to prevent optimization */
    if (g_volatile_seed & 1) {
        result /= (g_volatile_seed % 7 + 2);
    }
    
    return result;
}

/* Pointer chasing pattern */
static int64_t pointer_chase_sum(MixedData* array, int size, int stride) {
    int64_t sum = 0;
    volatile int i = 0;  /* Volatile counter */
    
    /* Nested loop with carried dependency */
    for (i = 0; i < size - stride; i++) {
        /* Complex addressing mode */
        MixedData* current = &array[i];
        MixedData* prev = &array[i + stride];
        
        /* Data-dependent computation with cross-iteration dependency */
        sum += data_dependent_compute(
            current->a * prev->a,
            current->b - prev->b,
            current->c + prev->c,
            current->d * prev->d
        );
        
        /* Control flow with multiple basic blocks */
        if (sum & 0x1) {
            /* Branch 1: More complex computation */
            sum = (sum * 3) / 2;
            
            /* Inline assembly to create fixed RTL */
            asm volatile ("" : : : "memory");
        } else {
            /* Branch 2: Different computation */
            sum = (sum << 2) | (sum >> 62);  /* 64-bit rotate */
            
            /* Floating point operation in else branch */
            double fp_temp = (double)sum / (current->d + 1.0);
            sum += (int64_t)(fp_temp * 1000.0);
        }
        
        /* Switch statement for additional control flow */
        switch (i % 4) {
            case 0:
                sum ^= current->a;
                break;
            case 1:
                sum += current->b;
                break;
            case 2:
                sum |= (int64_t)(current->c * 100.0);
                break;
            case 3:
                sum &= ~(int64_t)(current->d);
                break;
        }
    }
    
    return sum;
}

/* Matrix-vector multiplication kernel */
static void matrix_vector_multiply(double matrix[][8], double vector[], double result[], int rows) {
    volatile int row = 0;
    volatile int col = 0;
    
    #pragma GCC unroll 4
    for (row = 0; row < rows; row++) {
        double sum = 0.0;
        
        /* Inner loop with partial unrolling hint */
        #pragma GCC unroll 2
        for (col = 0; col < 8; col++) {
            /* Non-trivial addressing with stride */
            sum += matrix[row][col] * vector[col];
            
            /* Dependency chain */
            vector[col] = vector[col] * 0.99 + 0.01 * sum;
        }
        
        result[row] = sum;
        
        /* Another inline assembly barrier */
        asm volatile ("" : : : "memory");
    }
}

/* Initialize with pseudo-random data */
static void init_data(MixedData* array, int size) {
    unsigned int seed = g_volatile_seed;
    
    for (int i = 0; i < size; i++) {
        /* Simple PRNG to avoid libc calls in init */
        seed = seed * 1103515245 + 12345;
        array[i].a = (int32_t)(seed >> 16) & 0x7FFF;
        
        seed = seed * 1103515245 + 12345;
        array[i].b = (int64_t)seed * 1000;
        
        seed = seed * 1103515245 + 12345;
        array[i].c = (float)(seed % 1000) / 100.0f;
        
        seed = seed * 1103515245 + 12345;
        array[i].d = (double)(seed % 2000) / 200.0;
    }
}

int main(void) {
    const int array_size = g_volatile_bound;
    const int matrix_rows = 64;
    
    /* Allocate and initialize data */
    MixedData* data_array = (MixedData*)malloc(array_size * sizeof(MixedData));
    double matrix[64][8];
    double vector[8];
    double result[64];
    
    if (!data_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with volatile-dependent size */
    init_data(data_array, array_size);
    
    /* Initialize matrix and vector with random data */
    srand(g_volatile_seed);
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = (double)rand() / RAND_MAX;
        }
    }
    for (int j = 0; j < 8; j++) {
        vector[j] = (double)rand() / RAND_MAX;
    }
    
    /* Kernel 1: Pointer chasing with complex dependencies */
    int64_t sum1 = 0;
    for (int iter = 0; iter < 10; iter++) {
        /* Varying stride pattern */
        int stride = (iter % 3) + 1;
        sum1 += pointer_chase_sum(data_array, array_size, stride);
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < array_size; i++) {
            data_array[i].a += (iter & 0xFF);
        }
    }
    
    /* Kernel 2: Matrix-vector multiplication */
    for (int iter = 0; iter < 5; iter++) {
        matrix_vector_multiply(matrix, vector, result, matrix_rows);
        
        /* Update vector for next iteration */
        for (int j = 0; j < 8; j++) {
            vector[j] = result[iter % 8] * 0.5 + vector[j] * 0.5;
        }
    }
    
    /* Final reduction with XOR to prevent optimization */
    int64_t final_result = sum1;
    for (int i = 0; i < 64; i++) {
        final_result ^= (int64_t)(result[i] * 1000000.0);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %ld\n", (long)final_result);
    
    free(data_array);
    return 0;
}
