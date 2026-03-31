/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Complex data-dependent computation with carried dependencies */
double complex_kernel(double *data, int size, int offset) {
    double sum = 0.0;
    double prod = 1.0;
    
    /* Nested loops with data dependencies */
    for (int i = 1; i < size - offset; i++) {
        /* Volatile loop bound to prevent full unrolling */
        int limit = volatile_bound % 128;
        
        for (int j = 0; j < limit; j++) {
            /* Mixed-width operations creating register pressure */
            long long temp64 = (long long)data[i] * 1000000LL;
            int temp32 = (int)(temp64 >> 16);
            
            /* Data-dependent computation with carried dependency */
            double dep_value = data[i] * data[i-1] + (double)temp32 / 1000.0;
            
            /* Conditional operations using ternary operator */
            double cond_result = (dep_value > 0.5) ? 
                dep_value * 2.0 : dep_value / 2.0;
            
            /* Complex addressing modes */
            sum += cond_result * data[(i * 7 + j) % size];
            
            /* Floating-point division with non-constant divisor */
            prod /= (1.0 + fabs(data[(i + j * 3) % size]));
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : : "memory");
            
            /* Switch statement creating multiple basic blocks */
            switch (j % 4) {
                case 0:
                    sum += data[i] * 0.25;
                    break;
                case 1:
                    sum -= data[i] * 0.125;
                    break;
                case 2:
                    sum *= 1.01;
                    break;
                case 3:
                    sum = fmod(sum, 1000.0);
                    break;
            }
        }
        
        /* External function call preventing optimization */
        if (i % 37 == 0) {
            sum += (double)rand() / RAND_MAX;
        }
    }
    
    return sum + prod;
}

/* Matrix-vector multiplication for additional scheduling regions */
void matrix_vector_mult(double *matrix, double *vector, double *result, int n) {
    /* Loop unrolling hint */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        
        /* Strided memory access pattern */
        for (int j = 0; j < n; j++) {
            /* Complex addressing with mixed operations */
            double val = matrix[i * n + j] * vector[j];
            
            /* Conditional move simulation */
            sum = (val > 0) ? sum + val : sum - fabs(val);
            
            /* Pointer chasing pattern */
            if (j > 0) {
                val += matrix[i * n + j - 1] * 0.1;
            }
            
            /* Another inline assembly barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Branch with substantial computation on both sides */
        if (sum > 100.0) {
            result[i] = sum / (1.0 + (double)(i % 8));
        } else {
            result[i] = sum * (1.0 + sin((double)i * 0.1));
        }
    }
}

/* Pointer chasing computation */
double pointer_chase(double *data, int size) {
    double total = 0.0;
    int idx = 0;
    
    for (int i = 0; i < size * 2; i++) {
        /* True pointer chasing with data dependency */
        idx = (int)data[idx] % size;
        
        /* Mixed integer/floating operations */
        total += (double)idx * 0.5 + data[idx] * 2.0;
        
        /* Division with volatile divisor */
        volatile double div = (double)(volatile_seed % 100 + 1);
        total /= div;
        
        /* Complex conditional */
        if (idx % 3 == 0) {
            total = sqrt(fabs(total)) + 1.0;
        } else if (idx % 3 == 1) {
            total = total * total * 0.01;
        } else {
            total = log(fabs(total) + 1.0);
        }
    }
    
    return total;
}

int main() {
    const int SIZE = 1024;
    const int MATRIX_SIZE = 64;
    
    /* Initialize with pseudo-random data */
    double *data = (double*)malloc(SIZE * sizeof(double));
    double *matrix = (double*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double *vector = (double*)malloc(MATRIX_SIZE * sizeof(double));
    double *result = (double*)malloc(MATRIX_SIZE * sizeof(double));
    
    srand(time(NULL));
    
    /* Fill arrays with random data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (double)rand() / RAND_MAX * 100.0 - 50.0;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (double)rand() / RAND_MAX * 2.0 - 1.0;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        vector[i] = (double)rand() / RAND_MAX * 10.0 - 5.0;
    }
    
    /* Execute multiple computation kernels */
    double result1 = 0.0;
    double result2 = 0.0;
    double result3 = 0.0;
    
    /* Run computations multiple times to increase scheduling opportunities */
    for (int iter = 0; iter < 10; iter++) {
        /* Vary parameters slightly each iteration */
        int offset = iter % 7 + 1;
        
        /* First kernel: complex data-dependent computation */
        result1 += complex_kernel(data, SIZE, offset);
        
        /* Second kernel: matrix-vector multiplication */
        matrix_vector_mult(matrix, vector, result, MATRIX_SIZE);
        
        /* Reduce matrix result */
        for (int i = 0; i < MATRIX_SIZE; i++) {
            result2 += result[i];
        }
        
        /* Third kernel: pointer chasing */
        result3 += pointer_chase(data, SIZE);
        
        /* Modify data slightly for next iteration */
        for (int i = 0; i < SIZE; i += 8) {
            data[i] *= 1.001;
        }
        
        /* Volatile update to prevent loop optimization */
        volatile_seed++;
    }
    
    /* Final reduction with XOR-like operation */
    long long final_hash = 0;
    final_hash ^= *(long long*)&result1;
    final_hash ^= *(long long*)&result2;
    final_hash ^= *(long long*)&result3;
    
    /* Print result to ensure side effects */
    printf("Final hash: %llx\n", final_hash);
    printf("Results: %f %f %f\n", result1, result2, result3);
    
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
