#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function with hot attribute and scheduling pressure */
__attribute__((hot, optimize("O3", "sched-pressure"))) 
static float hot_function(float* data, int size) {
    float sum = 0.0f;
    
    /* Complex loop with mixed dependencies */
    #pragma GCC unroll 4
    for (int i = 0; i < size - 1; i++) {
        /* RAW hazard: read after write */
        float temp = data[i] * 2.0f;
        sum += temp;
        
        /* WAR hazard: write after read */
        data[i] = sum * 0.5f;
        
        /* WAW hazard: write after write */
        float old = data[i + 1];
        data[i + 1] = temp + old;
        data[i + 1] = old * 1.1f;  /* Second write */
        
        /* Memory barrier forcing scheduler decisions */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* Cold function with noinline to create scheduling boundaries */
__attribute__((cold, noinline))
static double cold_function(double* array, int n) {
    double result = 0.0;
    
    /* Switch statement with sparse cases */
    for (int i = 0; i < n; i++) {
        int selector = i % 7;
        
        switch (selector) {
            case 0:
                result += array[i] * 1.1;
                /* Inline asm with register clobber */
                asm volatile("" ::: "eax", "ebx", "memory");
                break;
            case 1:
                result -= array[i] * 0.9;
                break;
            case 3:  /* Sparse case value */
                result *= 1.05;
                /* Fall through */
            case 4:
                result += sin(array[i]);
                break;
            case 6:
                /* Conditional move pattern */
                result = (array[i] > 0) ? result * 1.2 : result * 0.8;
                break;
            default:
                result += cos(array[i]);
        }
        
        /* Early exit condition */
        if (result > 1000.0) {
            break;
        }
        
        /* Continue condition */
        if (i % 3 == 0) {
            continue;
        }
        
        result += 0.01;
    }
    
    return result;
}

/* SIMD-friendly function with vectorization hints */
__attribute__((optimize("O3", "tree-vectorize")))
static void vectorized_loop(float* restrict a, float* restrict b, 
                           float* restrict c, int len) {
    /* Compile-time known size helps vectorization */
    #pragma GCC unroll 8
    for (int i = 0; i < len; i++) {
        /* Mixed integer/float operations */
        int idx = i * 2;
        float temp = a[i] + b[i];
        
        /* Pointer chasing pattern */
        float* ptr = &c[idx % len];
        *ptr = temp * (*ptr);
        
        /* Complex dependency chain */
        a[i] = temp * sinf(b[i]);
        b[i] = cosf(a[i]) + (float)i;
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
    }
}

/* Function with computed goto (challenges control flow analysis) */
__attribute__((noinline))
static int computed_goto_test(int x) {
    static void* jump_table[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5, &&label6, &&label7
    };
    
    if (x < 0 || x > 7) goto default_label;
    
    goto *jump_table[x];
    
label0:
    return x * 2;
label1:
    return x + 10;
label2:
    /* Inline asm with specific constraints */
    asm volatile ("movl %1, %%eax\n\t"
                  "addl $5, %%eax\n\t"
                  "movl %%eax, %0"
                  : "=r" (x) : "r" (x) : "%eax");
    return x;
label3:
    return x * x;
label4:
    return x / 2;
label5:
    return x | 0xFF;
label6:
    return x & 0x0F;
label7:
    return ~x;
default_label:
    return -1;
}

/* Main test function with heterogeneous operations */
__attribute__((optimize("O3", "sched-pressure")))
static double scheduling_stress_test(void) {
    /* Allocate arrays with different alignments */
    float* fdata1 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fdata2 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fdata3 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    double* ddata = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fdata1[i] = (float)(i % 100) * 0.1f;
        fdata2[i] = (float)((i + 1) % 100) * 0.2f;
        fdata3[i] = (float)((i + 2) % 100) * 0.3f;
        ddata[i] = (double)(i % 50) * 0.05;
    }
    
    double total_result = 0.0;
    
    /* Multiple iterations with different scheduling patterns */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function (should trigger scheduler) */
        float hot_result = hot_function(fdata1, ARRAY_SIZE);
        
        /* Call cold function */
        double cold_result = cold_function(ddata, ARRAY_SIZE / 2);
        
        /* Vectorized operations */
        vectorized_loop(fdata2, fdata3, fdata1, ARRAY_SIZE);
        
        /* Computed goto test */
        int goto_result = computed_goto_test(iter % 8);
        
        /* Mix results to prevent dead code elimination */
        total_result += hot_result + cold_result + goto_result;
        
        /* Complex conditional with mixed operations */
        if (iter % 10 == 0) {
            /* Nested loop with pointer arithmetic */
            float* ptr = fdata1;
            for (int j = 0; j < ARRAY_SIZE; j += 4) {
                /* Unrolled memory operations */
                ptr[j] = ptr[j] * 1.1f + (float)j;
                ptr[j + 1] = ptr[j + 1] * 0.9f - (float)j;
                ptr[j + 2] = sqrtf(fabsf(ptr[j + 2]));
                ptr[j + 3] = ptr[j + 3] * ptr[j] + ptr[j + 1];
                
                /* Scheduling barrier */
                asm volatile("" ::: "memory");
            }
        }
    }
    
    /* Cleanup */
    free(fdata1);
    free(fdata2);
    free(fdata3);
    free(ddata);
    
    return total_result;
}

/* Additional test with nested loops and complex dependencies */
__attribute__((optimize("O3")))
static int nested_loop_test(int size) {
    int matrix[64][64];
    int sum = 0;
    
    /* Nested loops with data dependencies */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            /* Cross-iteration dependency */
            int prev = (i > 0 && j > 0) ? matrix[i-1][j-1] : 0;
            
            /* Mixed operations */
            matrix[i][j] = i * j + prev;
            
            /* Conditional with side effects */
            sum += (matrix[i][j] % 2 == 0) ? matrix[i][j] * 2 : matrix[i][j] / 2;
            
            /* Early continue */
            if (matrix[i][j] > 1000) continue;
            
            /* Another operation */
            sum += (i ^ j) & 0xFF;
        }
        
        /* Partial unroll pragma */
        #pragma GCC unroll 2
        for (int k = 0; k < 4; k++) {
            sum += matrix[i][k] * k;
        }
    }
    
    return sum;
}

int main(void) {
    clock_t start = clock();
    
    printf("Starting selective scheduling stress test...\n");
    
    /* Accumulate results from all test functions */
    double result1 = scheduling_stress_test();
    printf("Test 1 result: %f\n", result1);
    
    int result2 = nested_loop_test(64);
    printf("Test 2 result: %d\n", result2);
    
    /* Additional mixed workload */
    float array1[256], array2[256];
    for (int i = 0; i < 256; i++) {
        array1[i] = (float)i * 0.25f;
        array2[i] = (float)(255 - i) * 0.5f;
    }
    
    /* Call hot function again with different data */
    float result3 = hot_function(array1, 256);
    printf("Test 3 result: %f\n", result3);
    
    /* Call cold function */
    double darray[128];
    for (int i = 0; i < 128; i++) darray[i] = (double)i * 0.1;
    double result4 = cold_function(darray, 128);
    printf("Test 4 result: %f\n", result4);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total execution time: %f seconds\n", elapsed);
    printf("Final accumulated result: %f\n", 
           result1 + result2 + result3 + result4);
    
    return 0;
}
