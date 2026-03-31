/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* External function to create dependencies */
extern int external_rand(void);

/* Complex data-dependent computation with pointer chasing */
double pointer_chase_sum(double* data, int size, int stride) {
    double sum = 0.0;
    volatile int vol_idx = 0;  /* Prevent optimization */
    
    #pragma GCC unroll 4
    for (int i = 1; i < size - 1; i += stride) {
        /* Data-dependent computation with carried dependency */
        double temp = data[i] * data[i-1];
        
        /* Mixed-width operations */
        int int_temp = (int)temp;
        long long_temp = (long)temp * 3LL;
        
        /* Conditional move via ternary */
        double cond_val = (temp > 0.5) ? temp * 2.0 : temp / 2.0;
        
        /* Non-trivial addressing with stride */
        sum += cond_val + data[(i * 7) % size] * 0.3;
        
        /* Memory barrier to force ordering */
        asm volatile("" ::: "memory");
        
        /* Complex integer arithmetic */
        vol_idx += (int_temp % 17) + (long_temp % 23);
    }
    
    return sum;
}

/* Matrix-vector multiplication with control flow */
void matrix_vector_mult(float matrix[4][4], float vector[4], float result[4]) {
    for (int i = 0; i < 4; i++) {
        float sum = 0.0f;
        
        /* Switch with different operations per case */
        switch (i % 3) {
            case 0:
                for (int j = 0; j < 4; j++) {
                    sum += matrix[i][j] * vector[j];
                    /* Division with non-constant divisor */
                    if (vector[j] != 0.0f) {
                        sum /= (vector[j] + 0.001f);
                    }
                }
                break;
                
            case 1:
                for (int j = 0; j < 4; j++) {
                    /* Different addressing pattern */
                    sum += matrix[j][i] * vector[(j + 1) % 4];
                    /* Square root approximation */
                    sum += (sum > 0) ? sum * 0.5f : -sum * 0.3f;
                }
                break;
                
            default:
                for (int j = 0; j < 4; j++) {
                    /* Mixed operations */
                    float prod = matrix[i][j] * vector[j];
                    sum += prod * prod;
                    /* Integer conversion */
                    sum += (float)((int)prod % 256);
                }
                break;
        }
        
        result[i] = sum;
        
        /* Inline assembly to create fixed RTL */
        asm volatile("/* Fixed instruction */" ::: "memory");
    }
}

/* Nested loop with complex dependencies */
long nested_loop_computation(int* data, int size) {
    long total = 0;
    volatile int outer_bound = g_volatile_bound / 2;
    
    for (int i = 0; i < outer_bound; i++) {
        int inner_bound = (external_rand() % 50) + 10;
        
        for (int j = 0; j < inner_bound; j++) {
            /* Complex index calculation */
            int idx = (i * 31 + j * 7) % size;
            int idx2 = (i * 17 + j * 13) % size;
            
            /* Data-dependent branching */
            if (data[idx] > data[idx2]) {
                /* Floating point in integer loop */
                double ratio = (double)data[idx] / (data[idx2] + 1);
                total += (long)(ratio * 100.0);
                
                /* More arithmetic */
                total += (data[idx] % 17) * (data[idx2] % 23);
            } else {
                /* Different computation path */
                int diff = data[idx2] - data[idx];
                total += diff * diff;
                
                /* Division with variable divisor */
                if (diff != 0) {
                    total += 1000 / (abs(diff) + 1);
                }
            }
            
            /* Pointer arithmetic */
            int* ptr = &data[(i + j) % size];
            total += *ptr * (j + 1);
            
            /* Prevent loop unrolling */
            if (j % 8 == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Volatile function call */
        total += rand() % 100;
    }
    
    return total;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    double double_data[ARRAY_SIZE];
    int int_data[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    srand(g_volatile_seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double_data[i] = (double)rand() / RAND_MAX * 100.0;
        int_data[i] = rand() % 1000;
    }
    
    /* First computation kernel */
    double sum1 = 0.0;
    for (int iter = 0; iter < 100; iter++) {
        int stride = (iter % 5) + 1;
        sum1 += pointer_chase_sum(double_data, ARRAY_SIZE, stride);
    }
    
    /* Second computation kernel */
    float matrix[4][4], vector[4], result[4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = (float)rand() / RAND_MAX;
        }
        vector[i] = (float)rand() / RAND_MAX;
    }
    
    matrix_vector_mult(matrix, vector, result);
    
    /* Third computation kernel */
    long total = nested_loop_computation(int_data, ARRAY_SIZE);
    
    /* Final reduction with XOR to prevent optimization */
    long final_result = (long)sum1 ^ total;
    for (int i = 0; i < 4; i++) {
        final_result ^= (long)result[i];
    }
    
    printf("Final result: %ld\n", final_result);
    
    return 0;
}

/* Simple external function implementation */
int external_rand(void) {
    static unsigned int state = 123456789;
    state = state * 1103515245 + 12345;
    return (int)(state >> 16) & 32767;
}
