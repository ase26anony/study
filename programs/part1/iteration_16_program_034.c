#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with scheduling pressure attribute */
__attribute__((hot, optimize("O3"))) 
static float hot_function(float* restrict a, float* restrict b, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read after write */
        float temp = a[i] * 2.0f;
        sum += temp;
        
        /* WAR hazard: write after read */
        a[i] = sum * 0.5f;
        
        /* WAW hazard: write after write */
        b[i] = a[i] + b[i];
        b[i] = b[i] * 1.5f;  // Second write to b[i]
    }
    
    /* Inline assembly barrier */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Cold function with different optimization */
__attribute__((cold, noinline, optimize("sched-pressure")))
static int cold_function(int* restrict arr, int n) {
    int result = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 7) {
            case 0:
                result += arr[i] * 2;
                break;
            case 1:
                result -= arr[i];
                /* Fall through */
            case 2:
                result ^= arr[i];
                break;
            case 3:
                result |= arr[i] << 2;
                break;
            case 4:
                /* Conditional move */
                result = (arr[i] > 0) ? result + arr[i] : result - arr[i];
                break;
            case 5:
                result = result * 3 + arr[i];
                break;
            default:
                result = ~result;
                break;
        }
        
        /* Early exit condition */
        if (result > 1000000) {
            break;
        }
        
        /* Continue condition */
        if (arr[i] % 3 == 0) {
            continue;
        }
        
        result += i;
    }
    
    return result;
}

/* Vectorization-friendly function with unrolling */
__attribute__((optimize("O3")))
static void vectorized_loop(float* restrict src, float* restrict dst, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* SIMD-friendly operations */
        dst[i] = src[i] * src[i] + sqrtf(fabsf(src[i]));
        
        /* Mixed precision */
        if (i % 2 == 0) {
            dst[i] = (float)((int)dst[i] * 2);
        }
    }
    
    /* Assembly with register clobber */
    asm volatile(
        "mov $0, %%eax\n\t"
        "cpuid\n\t"
        : 
        : 
        : "eax", "ebx", "ecx", "edx", "memory"
    );
}

/* Pointer chasing function */
__attribute__((noinline))
static float pointer_chase(float** ptr_array, int steps) {
    float* current = ptr_array[0];
    float sum = 0.0f;
    
    for (int i = 0; i < steps; i++) {
        /* Load/store sequence with varying latencies */
        float val = *current;
        sum += val * val;
        
        /* Pointer arithmetic with dependency */
        current = ptr_array[(int)val % steps];
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Store with dependency */
        *current = sum * 0.1f;
    }
    
    return sum;
}

/* Function with nested loops and mixed operations */
__attribute__((optimize("O3")))
static double nested_loop_computation(double* matrix, int size) {
    double total = 0.0;
    
    /* Nested loops with data dependencies */
    for (int i = 0; i < size; i++) {
        double row_sum = 0.0;
        
        #pragma GCC unroll 2
        for (int j = 0; j < size; j++) {
            /* Complex addressing */
            double val = matrix[i * size + j];
            
            /* Mixed operations */
            row_sum += (j % 3 == 0) ? sin(val) : cos(val);
            
            /* Write with dependency */
            matrix[i * size + j] = row_sum * 0.01;
        }
        
        total += row_sum;
        
        /* Early exit from outer loop */
        if (total > 1000.0) {
            break;
        }
    }
    
    return total;
}

/* Computed goto pattern */
__attribute__((optimize("O2")))
static int computed_goto_test(int x) {
    static void* jump_table[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5, &&label6, &&label7
    };
    
    int result = x;
    
    if (x >= 0 && x < 8) {
        goto *jump_table[x];
    }
    
    goto *jump_table[0];
    
label0:
    result += 10;
    goto end;
label1:
    result *= 2;
    goto end;
label2:
    result -= 5;
    goto end;
label3:
    result ^= 0xFF;
    goto end;
label4:
    result = result << 2;
    goto end;
label5:
    result = result >> 1;
    goto end;
label6:
    result = ~result;
    goto end;
label7:
    result = result % 17;
    goto end;
    
end:
    return result;
}

/* Main test driver */
int main(void) {
    /* Initialize data */
    float* fdata1 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* fdata2 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float** ptr_array = (float**)malloc(SIZE * sizeof(float*));
    int* idata = (int*)malloc(SIZE * sizeof(int));
    double* matrix = (double*)malloc(SIZE * SIZE * sizeof(double));
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        fdata1[i] = (float)rand() / RAND_MAX * 100.0f;
        fdata2[i] = (float)rand() / RAND_MAX * 100.0f;
        idata[i] = rand() % 1000;
        ptr_array[i] = &fdata1[rand() % SIZE];
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = (double)rand() / RAND_MAX * 10.0;
    }
    
    /* Accumulator for results */
    float total = 0.0f;
    
    /* Run multiple iterations to stress the scheduler */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with scheduling pressure */
        total += hot_function(fdata1, fdata2, SIZE);
        
        /* Call cold function with complex control flow */
        total += (float)cold_function(idata, SIZE);
        
        /* Vectorized operations */
        vectorized_loop(fdata1, fdata2, SIZE);
        total += fdata2[SIZE-1];
        
        /* Pointer chasing */
        total += pointer_chase(ptr_array, 64);
        
        /* Nested loop computation */
        total += (float)nested_loop_computation(matrix, 32);
        
        /* Computed goto test */
        total += (float)computed_goto_test(iter % 8);
        
        /* Modify data to prevent optimization */
        for (int i = 0; i < SIZE; i++) {
            fdata1[i] += 0.001f * total;
            idata[i] = (idata[i] + iter) % 1000;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", total);
    
    /* Cleanup */
    free(fdata1);
    free(fdata2);
    free(ptr_array);
    free(idata);
    free(matrix);
    
    return 0;
}
