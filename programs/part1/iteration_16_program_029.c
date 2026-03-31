/* Selective Scheduling Stress Test for GCC sel-sched-dump.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, noinline, optimize("O3")))
static float hot_loop_scheduler(float *data, int n) {
    volatile float sum = 0.0f;
    float temp[SIZE] __attribute__((aligned(16)));
    
    /* Mixed data dependencies with RAW/WAR/WAW hazards */
    for (int i = 0; i < n - 1; i++) {
        /* RAW hazard: data[i+1] depends on data[i] calculation */
        float t1 = data[i] * 2.5f;
        
        /* Memory barrier forcing scheduler decisions */
        asm volatile("" ::: "memory");
        
        /* WAR hazard: reusing same variable */
        t1 = t1 + data[i + 1] * 1.5f;
        
        /* WAW hazard: multiple writes to same location */
        temp[i] = t1;
        temp[i] = sqrtf(fabsf(t1)) + 1.0f;
        
        /* Inline assembly with register clobbers */
        asm volatile("" : "=r"(t1) : "0"(t1) : "r0", "r1", "cc");
        
        sum += temp[i];
    }
    return sum;
}

__attribute__((cold, optimize("sched-pressure")))
static int cold_path_scheduler(int *arr, int n) {
    int result = 0;
    
    /* Complex control flow with switch statement */
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 7) {
            case 0:
                result += arr[i] * 2;
                /* Fall through */
            case 1:
                result -= arr[i] / 3;
                break;
            case 2:
            case 3:
                result ^= arr[i];
                /* Conditional move */
                result = (arr[i] > 0) ? result : -result;
                break;
            default:
                result = (result << 3) | (arr[i] & 0x7);
                /* Multiple early exits */
                if (result > 1000000) return result;
                if (i % 13 == 0) continue;
        }
        
        /* Pointer chasing pattern */
        int *ptr = &arr[i];
        for (int j = 0; j < 3; j++) {
            if (ptr) {
                *ptr += j;
                ptr = (j < 2) ? ptr + 1 : NULL;
            }
        }
    }
    return result;
}

__attribute__((optimize("O3"), noinline))
static void vectorized_unrolled_scheduler(float *a, float *b, float *c, int n) {
    int i;
    
    /* SIMD-friendly loop with unroll pragma */
    #pragma GCC unroll 4
    for (i = 0; i < n - 3; i += 4) {
        /* Mixed integer/float operations */
        float f1 = a[i] * b[i] + 1.0f;
        float f2 = a[i+1] * b[i+1] - 2.0f;
        float f3 = a[i+2] * b[i+2] * 3.0f;
        float f4 = a[i+3] * b[i+3] / 4.0f;
        
        /* Create scheduling dependencies */
        c[i] = f1 + (float)((int)f2 % 256);
        c[i+1] = f2 + fabsf(f3);
        c[i+2] = f3 * ((f4 > 0) ? f4 : -f4);
        c[i+3] = f4 + sinf(f1 * 0.1f);
        
        /* Assembly barrier splitting scheduling regions */
        asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
    }
    
    /* Remainder loop with different pattern */
    for (; i < n; i++) {
        c[i] = a[i] + b[i] * ((i % 2) ? 0.5f : 2.0f);
    }
}

__attribute__((optimize("O2"), noinline))
static double nested_loop_scheduler(double *matrix, int size) {
    double total = 0.0;
    
    /* Nested loops with mixed dependencies */
    for (int i = 0; i < size; i++) {
        double row_sum = 0.0;
        #pragma GCC unroll 2
        for (int j = 0; j < size; j++) {
            /* WAW hazard on row_sum */
            row_sum = matrix[i * size + j];
            row_sum = row_sum * row_sum + 1.0;
            
            /* RAW hazard through memory */
            matrix[i * size + j] = row_sum + (double)(i * j);
            
            /* Conditional execution */
            if ((i + j) % 3 == 0) {
                row_sum += sin(matrix[i * size + j] * 0.01);
            } else if ((i + j) % 3 == 1) {
                row_sum -= cos(matrix[i * size + j] * 0.01);
            } else {
                row_sum *= 0.99;
            }
        }
        total += row_sum;
        
        /* Memory barrier between outer loop iterations */
        asm volatile("" ::: "memory");
    }
    return total;
}

__attribute__((optimize("O3")))
static int computed_goto_scheduler(int *arr, int n) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int idx = arr[i] % 6;
        goto *labels[idx];
        
    L0:
        sum += arr[i] * 2;
        continue;
    L1:
        sum -= arr[i] / 2;
        /* Insert scheduling barrier */
        asm volatile("" ::: "r0", "r1");
        continue;
    L2:
        sum ^= (arr[i] << 3);
        continue;
    L3:
        sum |= arr[i];
        /* Another barrier with different clobbers */
        asm volatile("" ::: "r2", "r3", "r4");
        continue;
    L4:
        sum = (sum * 1103515245 + 12345) & 0x7fffffff;
        continue;
    L5:
        sum = ~sum;
        continue;
    }
    return sum;
}

int main(void) {
    /* Initialize data */
    float float_data[SIZE];
    int int_data[SIZE];
    float a[SIZE], b[SIZE], c[SIZE];
    double matrix[64 * 64];
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        int_data[i] = rand() % 1000;
        a[i] = (float)rand() / RAND_MAX;
        b[i] = (float)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < 64 * 64; i++) {
        matrix[i] = (double)rand() / RAND_MAX;
    }
    
    /* Accumulate results to prevent dead code elimination */
    float total_float = 0.0f;
    int total_int = 0;
    double total_double = 0.0;
    
    /* Run multiple iterations to stress the scheduler */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call each test function with different scheduling characteristics */
        total_float += hot_loop_scheduler(float_data, SIZE);
        total_int += cold_path_scheduler(int_data, SIZE);
        
        vectorized_unrolled_scheduler(a, b, c, SIZE);
        for (int i = 0; i < SIZE; i++) {
            total_float += c[i];
        }
        
        total_double += nested_loop_scheduler(matrix, 64);
        total_int += computed_goto_scheduler(int_data, SIZE);
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            float_data[i] *= 0.99f;
            int_data[i] = (int_data[i] * 13 + 7) % 1000;
        }
    }
    
    /* Print results to ensure all code executes */
    printf("Results: float=%f int=%d double=%f\n", 
           total_float, total_int, total_double);
    
    return 0;
}
