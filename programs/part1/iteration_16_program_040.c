#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3"))) 
static float hot_function(float *data, int n) {
    float sum = 0.0f;
    float temp[SIZE];
    
    /* Complex loop with mixed dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read after write */
        float x = data[i];
        float y = x * 2.0f;
        data[i] = y;  /* WAW hazard potential */
        
        /* WAR hazard: write after read */
        float z = data[(i + 1) % n];
        data[i] = x + z;
        
        /* Mixed integer/float operations */
        int idx = (int)(x * 100) % n;
        sum += data[idx] * (float)i;
        
        /* Store for later use */
        temp[i] = sum;
    }
    
    /* Pointer chasing pattern */
    float *ptr = data;
    for (int i = 0; i < n/4; i++) {
        ptr = data + (int)(*ptr * 10) % n;
        sum += *ptr;
    }
    
    return sum;
}

__attribute__((cold, noinline))
static float cold_function(float *a, float *b, int n) {
    float result = 0.0f;
    
    /* Switch statement with sparse cases */
    for (int i = 0; i < n; i++) {
        int val = (int)a[i] % 10;
        
        switch (val) {
            case 0:
                result += a[i] * b[i];
                break;
            case 3:
                result -= a[i] / (b[i] + 1.0f);
                break;
            case 7:
                result *= 1.01f;
                break;
            default:
                /* Conditional move pattern */
                result = (val > 5) ? result + 1.0f : result - 1.0f;
                break;
        }
        
        /* Early exit condition */
        if (result > 1000.0f) break;
        
        /* Continue with another condition */
        if (i % 2 == 0) continue;
        
        result += sinf(a[i]) * cosf(b[i]);
    }
    
    return result;
}

__attribute__((optimize("sched-pressure")))
static void vectorized_loop(float * __restrict a, 
                           float * __restrict b,
                           float * __restrict c,
                           int n) {
    /* SIMD-friendly loop with unrolling hint */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Independent operations for vectorization */
        float t1 = a[i] * b[i];
        float t2 = sinf(a[i]) + cosf(b[i]);
        float t3 = sqrtf(fabsf(a[i] - b[i]));
        
        /* Memory barrier to force scheduling decisions */
        asm volatile("" ::: "memory");
        
        /* Complex dependency chain */
        c[i] = t1 * t2 + t3;
        
        /* Another barrier with register clobber */
        asm volatile("" ::: "r0", "r1", "r2", "r3", "memory");
        
        /* More mixed operations */
        int idx = i * 2 % n;
        a[idx] = c[i] * 0.5f;
    }
}

__attribute__((optimize("O3"), noinline))
static double nested_loop_scheduler_test(int n) {
    double matrix[SIZE][SIZE];
    double sum = 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * 1.5 + j * 0.7) / (n + 1);
        }
    }
    
    /* Complex nested loop with dependencies */
    for (int iter = 0; iter < n; iter++) {
        for (int i = 1; i < SIZE - 1; i++) {
            for (int j = 1; j < SIZE - 1; j++) {
                /* Stencil computation with multiple dependencies */
                double north = matrix[i-1][j];
                double south = matrix[i+1][j];
                double east = matrix[i][j+1];
                double west = matrix[i][j-1];
                double center = matrix[i][j];
                
                /* RAW hazard chain */
                double temp1 = north + south;
                double temp2 = east + west;
                double temp3 = temp1 * temp2;
                
                /* Assembly barrier splitting scheduling region */
                asm volatile("" ::: "memory");
                
                /* WAW hazard */
                matrix[i][j] = (center + temp3) * 0.25;
                
                /* Mixed precision computation */
                sum += matrix[i][j] * (double)(i + j);
            }
        }
        
        /* Control flow variation */
        if (iter % 10 == 0) {
            /* Additional computation on boundaries */
            for (int i = 0; i < SIZE; i++) {
                matrix[i][0] = matrix[i][SIZE-1] = sin(sum);
                matrix[0][i] = matrix[SIZE-1][i] = cos(sum);
            }
        }
    }
    
    return sum;
}

__attribute__((optimize("O3")))
static int integer_pointer_chasing(int *data, int n) {
    int sum = 0;
    int *ptr = data;
    
    /* Pointer chasing with computed goto simulation */
    for (int i = 0; i < n * 2; i++) {
        int offset = (*ptr) % 16;
        
        /* Computed goto-like switch */
        switch (offset) {
            case 0: case 1: case 2: case 3:
                ptr = data + (offset * 4) % n;
                sum += *ptr * 2;
                break;
            case 4: case 5: case 6: case 7:
                ptr = data + (offset * 8) % n;
                sum += *ptr / 2;
                break;
            default:
                ptr = data + (offset * 3) % n;
                sum += *ptr + offset;
                break;
        }
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Additional computation */
        if (i % 3 == 0) {
            sum ^= (sum << 3) | (sum >> 29);
        }
    }
    
    return sum;
}

int main() {
    float *data1 = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *data2 = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *data3 = (float*)aligned_alloc(64, SIZE * sizeof(float));
    int *int_data = (int*)aligned_alloc(64, SIZE * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data1[i] = (float)i / SIZE;
        data2[i] = (float)sin(i * 0.01);
        data3[i] = (float)cos(i * 0.02);
        int_data[i] = (i * 13 + 7) % 97;
    }
    
    double total = 0.0;
    
    /* Execute all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Hot path with vectorization */
        total += hot_function(data1, SIZE);
        
        /* Cold path with complex control flow */
        total += cold_function(data2, data3, SIZE);
        
        /* Vectorized loop with unrolling */
        vectorized_loop(data1, data2, data3, SIZE);
        total += data3[SIZE/2];
        
        /* Nested loop scheduler stress */
        total += nested_loop_scheduler_test(5);
        
        /* Integer pointer chasing */
        total += integer_pointer_chasing(int_data, SIZE);
        
        /* Modify data for next iteration */
        for (int i = 0; i < SIZE; i++) {
            data1[i] = data1[i] * 0.99f + 0.01f;
            data2[i] = data2[i] * 1.01f - 0.01f;
        }
    }
    
    /* Print result to prevent optimization */
    printf("Result: %f\n", total);
    
    free(data1);
    free(data2);
    free(data3);
    free(int_data);
    
    return 0;
}
