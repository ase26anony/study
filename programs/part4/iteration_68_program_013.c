/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 100;
volatile int volatile_seed = 42;

/* Complex data-dependent computation with carried dependencies */
double complex_kernel(double *data, int size, int stride) {
    double sum = 0.0;
    double prod = 1.0;
    
    /* Nested loops with data dependencies */
    for (int i = 1; i < size; i++) {
        /* Outer loop with volatile bound */
        for (int j = 0; j < (volatile_bound % 8); j++) {
            /* Data-dependent computation with cross-iteration dependency */
            double val1 = data[i];
            double val2 = data[i-1];
            
            /* Mixed-width operations */
            long temp = (long)(val1 * 1000) + (long)(val2 * 1000);
            
            /* Complex arithmetic with division */
            double div_result = val1 / (val2 + 1.0);
            
            /* Conditional operations */
            double cond_result = (val1 > val2) ? 
                (val1 * val1 - val2 * val2) : 
                (val2 * val2 - val1 * val1);
            
            /* Update with dependencies */
            sum += val1 * val2 + div_result + cond_result;
            prod *= (val1 + 0.5) * (val2 + 0.5);
            
            /* Pointer chasing pattern */
            int idx = (i * 13 + j * 7) % size;
            double chase_val = data[idx];
            sum += chase_val * (i % 3);
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : "r"(sum), "r"(prod) : "memory");
        }
        
        /* Branch with substantial computation in both paths */
        if (i % 7 == 0) {
            /* First branch path */
            for (int k = 0; k < 3; k++) {
                double t = data[(i + k) % size];
                sum += t * t * 0.3;
                prod /= (t + 2.0);
            }
        } else if (i % 5 == 0) {
            /* Second branch path */
            double accum = 0.0;
            #pragma GCC unroll 4
            for (int k = 0; k < 4; k++) {
                accum += data[(i * 2 + k) % size] * k;
            }
            sum += accum * 0.1;
            prod *= 1.0 + accum * 0.01;
        } else {
            /* Third branch path - switch statement */
            switch (i % 4) {
                case 0:
                    sum += data[i] * 0.77;
                    break;
                case 1:
                    sum -= data[i] * 0.33;
                    break;
                case 2:
                    sum *= 1.01 + data[i] * 0.01;
                    break;
                case 3:
                    sum /= 1.01 + data[i] * 0.01;
                    break;
            }
        }
    }
    
    return sum + prod * 0.001;
}

/* Matrix-vector multiplication kernel */
void matrix_vector_mult(double *matrix, double *vector, double *result, int n) {
    volatile int vn = n; /* Volatile to prevent optimization */
    
    for (int i = 0; i < vn; i++) {
        double sum = 0.0;
        #pragma GCC unroll 4
        for (int j = 0; j < n; j++) {
            /* Non-trivial addressing with stride */
            double elem = matrix[i * n + j];
            double vec_elem = vector[j];
            
            /* Mixed operations */
            sum += elem * vec_elem;
            
            /* Additional computation to increase pressure */
            if (j % 3 == 0) {
                sum += (elem > 0) ? elem * 0.1 : -elem * 0.1;
            }
            
            /* Memory barrier via inline assembly */
            asm volatile ("" : : "r"(sum) : "memory");
        }
        
        /* Conditional store */
        result[i] = (sum > 0) ? sum : -sum;
        
        /* External function call to prevent optimization */
        if (i % 100 == 0) {
            result[i] += (double)rand() / RAND_MAX * 0.1;
        }
    }
}

/* Main computation driver */
int main() {
    const int SIZE = 1024;
    const int MAT_SIZE = 64;
    
    /* Initialize with pseudo-random data */
    double *data = (double*)malloc(SIZE * sizeof(double));
    double *matrix = (double*)malloc(MAT_SIZE * MAT_SIZE * sizeof(double));
    double *vector = (double*)malloc(MAT_SIZE * sizeof(double));
    double *result = (double*)malloc(MAT_SIZE * sizeof(double));
    
    /* Simple PRNG for initialization */
    unsigned int seed = time(NULL) ^ volatile_seed;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        data[i] = (double)(seed % 1000) / 100.0;
    }
    
    for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        matrix[i] = (double)(seed % 1000) / 100.0 - 5.0;
    }
    
    for (int i = 0; i < MAT_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        vector[i] = (double)(seed % 1000) / 100.0;
    }
    
    /* Run first kernel */
    double result1 = complex_kernel(data, SIZE, 3);
    
    /* Run second kernel */
    matrix_vector_mult(matrix, vector, result, MAT_SIZE);
    
    /* Final reduction to prevent optimization */
    double final_sum = result1;
    for (int i = 0; i < MAT_SIZE; i++) {
        final_sum += result[i];
    }
    
    /* XOR-like reduction using bit manipulation */
    long long int_bits = 0;
    for (int i = 0; i < SIZE; i += 8) {
        int_bits ^= *(long long*)&data[i];
    }
    
    /* Ensure side effects are observable */
    printf("Result: %f (int bits: %llx)\n", final_sum, int_bits);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
