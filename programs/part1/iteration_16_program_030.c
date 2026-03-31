#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with hot attribute and scheduling pressure */
__attribute__((hot, optimize("O3", "sched-pressure"))) 
static float hot_function(float *data, int n) {
    float sum = 0.0f;
    float temp[SIZE];
    
    /* Complex loop with mixed dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read after write to sum */
        float t = sum + data[i];
        
        /* WAR hazard: write after read to data[i] */
        data[i] = t * 0.5f;
        
        /* WAW hazard: multiple writes to temp */
        temp[i] = data[i];
        temp[i] = sqrtf(fabsf(temp[i])) + 1.0f;
        
        /* Pointer chasing pattern */
        float *ptr = &temp[i];
        for (int j = 0; j < 3; j++) {
            *ptr = *ptr * *ptr + 1.0f;
            ptr = &temp[(i + j) % n];
        }
        
        sum += temp[i];
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
    }
    return sum;
}

/* Cold function with noinline */
__attribute__((cold, noinline))
static double cold_function(double *arr, int size) {
    double result = 0.0;
    
    /* Switch statement with sparse cases */
    for (int i = 0; i < size; i++) {
        switch (i % 7) {
            case 0: result += arr[i] * 2.0; break;
            case 1: result -= arr[i] / 3.0; break;
            case 3: result = fmod(result, arr[i] + 1.0); break;
            case 5: result = (result > 0) ? result : -result; break;
            default: result += sin(arr[i]); break;
        }
        
        /* Conditional move mixed with branch */
        if (i % 2 == 0) {
            result = (arr[i] > 0.5) ? result * 1.1 : result * 0.9;
        } else {
            result += cos(arr[i]);
        }
    }
    return result;
}

/* Vectorization-friendly function with unroll pragma */
__attribute__((optimize("O3")))
static void vectorized_loop(float *a, float *b, float *c, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* SIMD-friendly operations */
        a[i] = b[i] * c[i] + sinf(b[i]);
        c[i] = a[i] / (fabsf(c[i]) + 1.0f);
        b[i] = sqrtf(a[i] * a[i] + c[i] * c[i]);
        
        /* Mixed integer/floating point */
        int idx = (int)(a[i] * 10) % n;
        b[idx] += 0.1f;
    }
}

/* Function with complex control flow */
__attribute__((noinline))
static int complex_control(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
    /* Loop with multiple exit points */
    while (i < n) {
        if (arr[i] < 0) {
            /* Early continue */
            i++;
            continue;
        }
        
        /* Nested if-else chain */
        if (arr[i] % 2 == 0) {
            sum += arr[i] * 2;
            if (sum > 1000) {
                /* Early break */
                break;
            }
        } else if (arr[i] % 3 == 0) {
            sum -= arr[i];
        } else {
            sum = sum ^ arr[i];
        }
        
        /* Inline assembly with register clobber */
        asm volatile(
            "movl %0, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0"
            : "+r" (arr[i])
            :
            : "eax", "cc"
        );
        
        i++;
    }
    return sum;
}

/* Main test function with scheduling challenges */
__attribute__((optimize("O3", "sched-pressure")))
static float scheduling_stress_test(void) {
    static float data[SIZE];
    static double darr[SIZE];
    static int iarr[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)(i % 100) * 0.01f;
        darr[i] = (double)(i % 50) * 0.02;
        iarr[i] = i * 3 - SIZE/2;
    }
    
    float total = 0.0f;
    
    /* Multiple iterations with different scheduling patterns */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function */
        total += hot_function(data, SIZE);
        
        /* Call cold function */
        total += (float)cold_function(darr, SIZE);
        
        /* Vectorized operations */
        float a[SIZE], b[SIZE], c[SIZE];
        for (int i = 0; i < SIZE; i++) {
            a[i] = (float)i * 0.1f;
            b[i] = (float)(i + 1) * 0.2f;
            c[i] = (float)(i + 2) * 0.3f;
        }
        vectorized_loop(a, b, c, SIZE);
        
        /* Accumulate results */
        for (int i = 0; i < SIZE; i++) {
            total += a[i] + b[i] + c[i];
        }
        
        /* Complex control flow */
        total += (float)complex_control(iarr, SIZE);
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
        
        /* Modify arrays for next iteration */
        for (int i = 0; i < SIZE; i++) {
            data[i] += 0.001f * total;
            darr[i] += 0.0005 * total;
            iarr[i] = (iarr[i] + 1) % 1000;
        }
    }
    
    return total;
}

/* Additional test with nested loops and dependencies */
__attribute__((optimize("O3")))
static double nested_loop_test(void) {
    double matrix[64][64];
    double result = 0.0;
    
    /* Initialize matrix */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = (double)(i * j) * 0.01;
        }
    }
    
    /* Complex nested loop with dependencies */
    for (int k = 0; k < 10; k++) {
        for (int i = 1; i < 63; i++) {
            for (int j = 1; j < 63; j++) {
                /* Stencil computation with multiple dependencies */
                double avg = (matrix[i-1][j] + matrix[i+1][j] +
                            matrix[i][j-1] + matrix[i][j+1]) * 0.25;
                
                /* Mixed operations */
                matrix[i][j] = avg * sin(avg) + cos(matrix[i][j]);
                
                /* Conditional update */
                result += (matrix[i][j] > 0.5) ? matrix[i][j] : -matrix[i][j];
            }
        }
        
        /* Barrier every few iterations */
        if (k % 3 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return result;
}

int main(void) {
    clock_t start = clock();
    
    printf("Starting selective scheduling stress test...\n");
    
    /* Run scheduling stress test */
    float result1 = scheduling_stress_test();
    printf("Result 1: %f\n", result1);
    
    /* Run nested loop test */
    double result2 = nested_loop_test();
    printf("Result 2: %f\n", result2);
    
    /* Additional mixed workload */
    float data[SIZE];
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.01f;
    }
    
    float final_result = hot_function(data, SIZE);
    printf("Final result: %f\n", final_result);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Execution time: %f seconds\n", elapsed);
    
    /* Return meaningful value to prevent optimization */
    return (int)(result1 + result2 + final_result) % 256;
}
