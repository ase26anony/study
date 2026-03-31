#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with complex scheduling patterns */
__attribute__((hot, optimize("O3")))
static float hot_function(float* restrict a, float* restrict b, float* restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read a[i], then write to c[i] */
        float temp = a[i] * 2.5f;
        
        /* WAR hazard: read b[i] after potential write */
        asm volatile("" ::: "memory");  /* Scheduling barrier */
        
        /* Complex dependency chain */
        c[i] = temp + b[i] * 1.5f;
        
        /* WAW hazard: multiple writes to sum */
        sum += c[i];
        
        /* Pointer chasing simulation */
        if (i > 0) {
            c[i] += c[i-1] * 0.1f;  /* Anti-dependency */
        }
    }
    
    return sum;
}

/* Cold function with different scheduling needs */
__attribute__((cold, noinline))
static int cold_function(int* restrict arr, int n) {
    int result = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 8) {
            case 0: result += arr[i] * 2; break;
            case 1: result += arr[i] >> 1; break;
            case 2: result ^= arr[i]; break;
            case 3: result |= arr[i]; break;
            case 4: result &= arr[i]; break;
            case 5: result -= arr[i]; break;
            case 6: result = (result * arr[i]) & 0xFF; break;
            default: result = (result + arr[i]) % 256; break;
        }
        
        /* Conditional move vs branch */
        result = (arr[i] > 0) ? (result + 1) : (result - 1);
        
        /* Early exit conditions */
        if (result > 1000000) break;
        if (i % 32 == 0) continue;
        
        /* Memory barrier with clobbered registers */
        asm volatile("" ::: "rax", "rbx", "memory");
    }
    
    return result;
}

/* Function with vectorization opportunities */
__attribute__((optimize("O3")))
static void vectorized_loop(double* restrict a, double* restrict b, 
                           double* restrict c, int n) {
    /* SIMD-friendly loop with mixed operations */
    #pragma GCC unroll 2
    for (int i = 0; i < n; i++) {
        /* Multiple dependent floating-point operations */
        double t1 = sin(a[i] * 0.5);
        double t2 = cos(b[i] * 0.3);
        
        /* Cross-iteration dependency */
        c[i] = t1 * t2 + (i > 0 ? c[i-1] * 0.1 : 0.0);
        
        /* Complex expression with multiple uses */
        a[i] = t1 * t1 + t2 * t2;
        b[i] = sqrt(fabs(a[i])) + 1.0;
    }
}

/* Function with nested loops and mixed hazards */
__attribute__((optimize("sched-pressure")))
static float nested_loop_scheduler(float* restrict mat, int rows, int cols) {
    float total = 0.0f;
    
    /* Outer loop with pointer arithmetic */
    for (int i = 0; i < rows; i++) {
        float* row = &mat[i * cols];
        
        /* Inner loop with unrolling */
        #pragma GCC unroll 8
        for (int j = 0; j < cols; j++) {
            /* RAW: read row[j], write to temp */
            float temp = row[j] * 1.1f;
            
            /* Memory barrier splitting scheduling region */
            asm volatile("" ::: "memory");
            
            /* WAR: read temp, write to row[j] */
            row[j] = temp + (j > 0 ? row[j-1] * 0.2f : 0.0f);
            
            /* WAW: multiple accumulations to total */
            total += row[j];
            
            /* Complex conditional */
            if ((i + j) % 3 == 0) {
                total *= 0.99f;
            } else if ((i + j) % 5 == 0) {
                total += 0.01f;
            }
        }
        
        /* Early continue with computation */
        if (i % 7 == 0) {
            total = fmodf(total, 1000.0f);
            continue;
        }
    }
    
    return total;
}

/* Function with computed goto (challenges scheduler) */
__attribute__((noinline))
static int computed_goto_pattern(int x) {
    static void* jump_table[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5, &&label6, &&label7
    };
    
    int result = x;
    
    if (x < 0 || x > 7) goto default_label;
    
    goto *jump_table[x];
    
label0:
    result = result * 2 + 1;
    asm volatile("" ::: "rcx", "rdx", "memory");
    goto end;
    
label1:
    result = result >> 1;
    /* Fall through */
    
label2:
    result = result ^ 0x55;
    goto end;
    
label3:
    result = result | 0xAA;
    /* Fall through */
    
label4:
    result = result & 0xF0;
    goto end;
    
label5:
    result = result - 17;
    /* Fall through */
    
label6:
    result = result * 3;
    goto end;
    
label7:
    result = result + 42;
    goto end;
    
default_label:
    result = -result;
    
end:
    return result;
}

/* Main test driver */
int main(void) {
    /* Allocate aligned memory for better vectorization */
    float* farr1 = aligned_alloc(64, SIZE * sizeof(float));
    float* farr2 = aligned_alloc(64, SIZE * sizeof(float));
    float* farr3 = aligned_alloc(64, SIZE * sizeof(float));
    double* darr1 = aligned_alloc(64, SIZE * sizeof(double));
    double* darr2 = aligned_alloc(64, SIZE * sizeof(double));
    double* darr3 = aligned_alloc(64, SIZE * sizeof(double));
    int* iarr = aligned_alloc(64, SIZE * sizeof(int));
    float* matrix = aligned_alloc(64, SIZE * SIZE * sizeof(float));
    
    /* Initialize with pattern */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
        darr1[i] = (double)rand() / RAND_MAX;
        darr2[i] = (double)rand() / RAND_MAX;
        iarr[i] = rand() % 256;
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = (float)rand() / RAND_MAX;
    }
    
    float total_sum = 0.0f;
    int int_result = 0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function (should trigger selective scheduling) */
        total_sum += hot_function(farr1, farr2, farr3, SIZE);
        
        /* Call cold function with different patterns */
        int_result += cold_function(iarr, SIZE);
        
        /* Vectorized operations */
        vectorized_loop(darr1, darr2, darr3, SIZE);
        
        /* Nested loop with scheduling pressure */
        total_sum += nested_loop_scheduler(matrix, 32, 32);
        
        /* Computed goto pattern */
        int_result += computed_goto_pattern(iter % 10);
        
        /* Modify data to prevent optimization */
        farr1[iter % SIZE] += 0.1f;
        iarr[iter % SIZE] ^= 0xFF;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %f\n", total_sum);
    printf("Integer result: %d\n", int_result);
    printf("Sample values: farr3[0]=%f, darr3[0]=%f\n", 
           farr3[0], darr3[0]);
    
    /* Cleanup */
    free(farr1);
    free(farr2);
    free(farr3);
    free(darr1);
    free(darr2);
    free(darr3);
    free(iarr);
    free(matrix);
    
    return 0;
}
