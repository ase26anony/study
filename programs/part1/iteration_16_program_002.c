#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with hot attribute and scheduling pressure */
__attribute__((hot, optimize("O3", "sched-pressure"))) 
static float hot_function(float* restrict a, float* restrict b, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read a[i], write to temp, read temp */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: write to a[i] after reading it */
        a[i] = temp + b[i];
        
        /* WAW hazard: multiple writes to sum */
        sum += a[i];
        sum = sum * 0.99f;  /* Another write to sum */
        
        /* Pointer chasing pattern */
        float* ptr = &a[i];
        *ptr = *ptr * (*ptr + 1.0f);
    }
    
    /* Memory barrier inline assembly */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Cold function with noinline to create scheduling boundaries */
__attribute__((cold, noinline))
static double cold_function(double* arr, int n) {
    double result = 0.0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        int mod = i % 7;
        
        switch (mod) {
            case 0:
                arr[i] = sin(arr[i]);
                break;
            case 1:
                arr[i] = cos(arr[i]);
                break;
            case 2:
                arr[i] = sqrt(fabs(arr[i]));
                break;
            case 3:
                arr[i] = arr[i] * arr[i];
                break;
            case 4:
                /* Conditional move pattern */
                arr[i] = (arr[i] > 0) ? arr[i] : -arr[i];
                break;
            case 5:
                /* Early continue */
                if (arr[i] < 0.001) continue;
                arr[i] = 1.0 / arr[i];
                break;
            default:
                /* Fallthrough with computation */
                arr[i] = arr[i] + mod;
                /* Another memory barrier */
                asm volatile("" ::: "memory");
        }
        
        /* Multiple exit points */
        if (result > 1000.0) break;
        if (i > n/2 && arr[i] < 0.0) continue;
        
        result += arr[i];
    }
    
    return result;
}

/* SIMD-friendly function with unroll pragma */
__attribute__((optimize("O3")))
static void vectorized_loop(float* restrict in1, 
                           float* restrict in2, 
                           float* restrict out, 
                           int n) {
    int i;
    
    /* Manual unrolling with pragma hint */
    #pragma GCC unroll 4
    for (i = 0; i < n; i++) {
        /* Mixed operations that should vectorize */
        float t1 = in1[i] * in2[i];
        float t2 = in1[i] + in2[i];
        float t3 = t1 - t2;
        
        /* Dependency chain */
        out[i] = t3 * 0.5f;
        out[i] = out[i] + (i % 3);  /* Integer operation mixed in */
        
        /* Assembly with register clobber */
        if (i % 8 == 0) {
            asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        }
    }
    
    /* Handle remainder */
    for (; i < n; i++) {
        out[i] = in1[i] + in2[i];
    }
}

/* Function with nested loops and complex dependencies */
__attribute__((optimize("O3")))
static double nested_loop_scheduler_test(int size) {
    double matrix[SIZE][SIZE];
    double result = 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = (i * 1.5) + (j * 0.7);
        }
    }
    
    /* Nested loops with mixed access patterns */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (int i = 1; i < size - 1; i++) {
            for (int j = 1; j < size - 1; j++) {
                /* Stencil computation with multiple dependencies */
                double north = matrix[i-1][j];
                double south = matrix[i+1][j];
                double east = matrix[i][j+1];
                double west = matrix[i][j-1];
                double center = matrix[i][j];
                
                /* Complex dependency chain */
                double avg = (north + south + east + west) / 4.0;
                double diff = center - avg;
                
                /* Conditional update with branch */
                if (diff > 0.0) {
                    matrix[i][j] = avg + diff * 0.5;
                } else {
                    matrix[i][j] = avg - diff * 0.3;
                }
                
                /* Accumulate result with mixed operations */
                result += matrix[i][j] * (i + j);
                
                /* Early exit condition */
                if (result > 1e6) goto early_exit;
            }
            
            /* Memory barrier every few iterations */
            if (i % 16 == 0) {
                asm volatile("" ::: "memory");
            }
        }
    }
    
early_exit:
    return result;
}

/* Function with computed goto (challenges scheduler) */
__attribute__((noinline, optimize("O2")))
static int computed_goto_test(int x) {
    static void* jump_table[] = {
        &&case0, &&case1, &&case2, &&case3, 
        &&case4, &&case5, &&default_case
    };
    
    int result = 0;
    int index = x % 7;
    
    goto *jump_table[index];
    
case0:
    result = x * 2;
    /* Fall through */
case1:
    result += x / 3;
    asm volatile("" ::: "memory");
    goto end;
    
case2:
    result = x << 2;
    goto end;
    
case3:
    result = x | 0xFF;
    goto end;
    
case4:
    result = x & 0x0F;
    goto end;
    
case5:
    result = x ^ 0x55;
    goto end;
    
default_case:
    result = ~x;
    /* Another scheduling barrier */
    asm volatile("" : : : "r0", "r1", "r2", "r3");
    
end:
    return result;
}

/* Main test driver */
int main() {
    float array1[SIZE], array2[SIZE], array3[SIZE];
    double darray[SIZE];
    double total_result = 0.0;
    
    /* Initialize arrays */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (float)rand() / RAND_MAX;
        array2[i] = (float)rand() / RAND_MAX;
        darray[i] = (double)rand() / RAND_MAX;
    }
    
    printf("Starting selective scheduling stress tests...\n");
    
    /* Test 1: Hot function with scheduling pressure */
    total_result += hot_function(array1, array2, SIZE);
    
    /* Test 2: Cold function with complex control flow */
    total_result += cold_function(darray, SIZE);
    
    /* Test 3: Vectorized loop with unrolling */
    vectorized_loop(array1, array2, array3, SIZE);
    for (int i = 0; i < SIZE; i++) {
        total_result += array3[i];
    }
    
    /* Test 4: Nested loops (reduced size for speed) */
    total_result += nested_loop_scheduler_test(64);
    
    /* Test 5: Computed goto */
    for (int i = 0; i < 1000; i++) {
        total_result += computed_goto_test(i);
    }
    
    /* Additional mixed tests */
    for (int iter = 0; iter < 10; iter++) {
        /* Mix all patterns */
        total_result += hot_function(array1, array2, SIZE/2);
        total_result += cold_function(darray, SIZE/2);
        
        /* Inline assembly with explicit constraints */
        asm volatile(
            "mov r0, %0\n\t"
            "add r0, r0, #1\n\t"
            : 
            : "r"((int)total_result)
            : "r0", "cc"
        );
    }
    
    printf("Final result: %f\n", total_result);
    printf("(This value is meaningless - used to prevent optimization)\n");
    
    return 0;
}
