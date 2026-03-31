#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with complex scheduling patterns */
__attribute__((hot, optimize("O3", "unroll-loops")))
static float hot_function(float* restrict a, float* restrict b, float* restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read a[i], then write to it */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: write to b[i] after reading in previous iteration */
        b[i] = temp + (i > 0 ? b[i-1] : 0.0f);
        
        /* WAW hazard: multiple writes to c[i] */
        c[i] = temp * 3.0f;
        asm volatile("" ::: "memory");  /* Scheduling barrier */
        c[i] = c[i] + b[i];  /* Second write to c[i] */
        
        /* Pointer chasing pattern */
        float* ptr = &c[i];
        *ptr = *ptr * 1.5f;
        
        sum += c[i];
    }
    
    return sum;
}

/* Cold function with different scheduling characteristics */
__attribute__((cold, noinline, optimize("sched-pressure")))
static double cold_function(int* restrict arr, int n) {
    double result = 0.0;
    int* ptr = arr;
    
    /* Complex control flow with scheduling challenges */
    for (int i = 0; i < n; i++) {
        switch (i % 7) {
            case 0:
                ptr[i] = i * 2;
                asm volatile("" ::: "r8", "r9", "memory");  /* Register clobber */
                break;
            case 1:
                ptr[i] = i + ptr[i-1];  /* RAW dependency */
                break;
            case 2:
                ptr[i] = ptr[i] * 3;  /* Self-dependency */
                break;
            case 3:
                /* Conditional move pattern */
                ptr[i] = (i > n/2) ? ptr[i-1] : ptr[i+1];
                break;
            case 4:
                ptr[i] = ~ptr[i];  /* Bitwise operation */
                break;
            case 5:
                /* Mixed float/int */
                ptr[i] = (int)(sinf(i) * 100.0f);
                break;
            default:
                ptr[i] = i;
                /* Multiple exit points */
                if (ptr[i] > 1000) return result;
                if (ptr[i] < 0) continue;
        }
        
        /* Early exit conditions */
        if (ptr[i] > 500 && i < n/2) {
            result += ptr[i] * 0.5;
            continue;
        }
        
        result += (ptr[i] % 2 == 0) ? ptr[i] * 2.0 : ptr[i] * 0.5;
    }
    
    return result;
}

/* Function with vectorization opportunities */
__attribute__((optimize("O3", "tree-vectorize")))
static void vectorized_loop(float* restrict a, float* restrict b, 
                           float* restrict c, int n) {
    /* SIMD-friendly loop with mixed operations */
    #pragma GCC unroll 8
    for (int i = 0; i < n; i++) {
        /* Create complex dependency chain */
        float x = a[i] * b[i];
        float y = x + sinf(i * 0.01f);
        
        /* WAW hazard */
        c[i] = x * y;
        asm volatile("" ::: "memory");
        c[i] = c[i] + a[i] * 0.3f;
        
        /* Cross-iteration dependency */
        if (i > 0) {
            a[i] = a[i] + c[i-1] * 0.1f;
        }
        
        /* Branch with predictable pattern */
        b[i] = (i % 3 == 0) ? y * 2.0f : y * 0.5f;
    }
}

/* Function with nested loops and pointer arithmetic */
__attribute__((noinline))
static double nested_loop_scheduling(int size) {
    double matrix[32][32];
    double sum = 0.0;
    
    /* Nested loops with mixed access patterns */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            /* Complex addressing */
            matrix[i][j] = (i * j) / (double)(size + 1);
            
            /* Multiple uses with hazards */
            double temp = matrix[i][j];
            matrix[i][j] = temp * temp;
            
            /* Scheduling barrier with register constraints */
            asm volatile("" : "=r"(temp) : "0"(temp) : "r10", "r11");
            
            if (j > 0) {
                matrix[i][j] += matrix[i][j-1] * 0.5;
            }
            
            sum += matrix[i][j];
        }
        
        /* Early continue with computation */
        if (i % 4 == 0) {
            sum *= 1.1;
            continue;
        }
    }
    
    return sum;
}

/* Main test driver */
int main(void) {
    float a[SIZE], b[SIZE], c[SIZE];
    int int_arr[SIZE];
    double total = 0.0;
    
    /* Initialize arrays */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        a[i] = (float)rand() / RAND_MAX;
        b[i] = (float)rand() / RAND_MAX;
        c[i] = 0.0f;
        int_arr[i] = rand() % 1000;
    }
    
    /* Execute multiple scheduling patterns */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with vectorization */
        total += hot_function(a, b, c, SIZE);
        
        /* Call cold function with complex control flow */
        total += cold_function(int_arr, SIZE);
        
        /* Vectorized operations */
        vectorized_loop(a, b, c, SIZE);
        
        /* Nested loop scheduling */
        total += nested_loop_scheduling(SIZE);
        
        /* Modify arrays to create different patterns */
        for (int i = 0; i < SIZE; i++) {
            a[i] = sinf(a[i] + iter * 0.01f);
            b[i] = cosf(b[i] + iter * 0.02f);
            int_arr[i] = (int_arr[i] + iter) % 997;
        }
    }
    
    printf("Result: %f\n", total);
    return 0;
}
