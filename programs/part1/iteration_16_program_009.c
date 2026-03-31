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
    
    /* Mixed integer and floating point operations */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read after write */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: write after read */
        a[i] = temp + b[i];
        
        /* WAW hazard: write after write */
        sum += a[i];
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* More complex dependency chain */
        b[i] = sinf(temp) * cosf(a[i]);
    }
    
    return sum;
}

/* Cold function with noinline */
__attribute__((cold, noinline))
static int cold_function(int* restrict arr, int n) {
    int result = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 7) {
            case 0:
                result += arr[i] * 2;
                /* Memory barrier */
                asm volatile("" ::: "memory");
                break;
            case 1:
                result -= arr[i];
                break;
            case 2:
                result ^= arr[i];
                /* Register clobber */
                asm volatile("" ::: "r0", "r1", "r2", "r3");
                break;
            case 3:
                result |= arr[i] << 2;
                break;
            case 4:
                result &= ~arr[i];
                break;
            default:
                result = (result > 0) ? result : arr[i];
                break;
        }
        
        /* Multiple early exit points */
        if (result > 1000000) return result;
        if (i % 13 == 0) continue;
        
        /* Conditional move pattern */
        int x = (i & 1) ? arr[i] : result;
        result = (x > result) ? x : result;
    }
    
    return result;
}

/* Vectorization-friendly function with unroll pragma */
__attribute__((optimize("O3")))
static void vectorized_function(float* restrict out, 
                                const float* restrict in1,
                                const float* restrict in2,
                                int n) {
    int i;
    
    /* SIMD-friendly loop with unrolling */
    #pragma GCC unroll 4
    for (i = 0; i < n; i++) {
        /* Multiple dependent operations */
        float x = in1[i] * 3.14159f;
        float y = in2[i] * 2.71828f;
        
        /* Mixed operations creating scheduling challenges */
        out[i] = x * y + sinf(x) - cosf(y);
        
        /* Pointer chasing pattern */
        if (i > 0) {
            out[i] += out[i-1] * 0.1f;
        }
    }
    
    /* Residual loop for non-multiple-of-4 cases */
    for (; i < n + 3; i++) {
        if (i < n) {
            out[i] = in1[i] + in2[i];
        }
    }
}

/* Function with nested loops and mixed hazards */
__attribute__((optimize("O2")))
static double nested_loop_scheduler(int size) {
    double matrix[SIZE][SIZE];
    double sum = 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = (i * j) / (double)(i + j + 1);
        }
    }
    
    /* Complex nested loop with dependencies */
    for (int i = 1; i < size - 1; i++) {
        for (int j = 1; j < size - 1; j++) {
            /* Multiple RAW hazards */
            double a = matrix[i-1][j];
            double b = matrix[i][j-1];
            double c = matrix[i+1][j];
            double d = matrix[i][j+1];
            
            /* WAR hazard */
            matrix[i][j] = (a + b + c + d) * 0.25;
            
            /* WAW hazard avoided by accumulation */
            sum += matrix[i][j];
            
            /* Inline assembly with specific constraints */
            if ((i + j) % 32 == 0) {
                asm volatile("" : "=r"(sum) : "0"(sum) : "cc", "memory");
            }
        }
        
        /* Loop with multiple exit conditions */
        if (sum > 1e6) break;
        if (i % 64 == 0) continue;
    }
    
    return sum;
}

/* Function with computed goto (challenges scheduler) */
__attribute__((noinline, optimize("O1")))
static int computed_goto_pattern(int* arr, int n) {
    static void* labels[] = { &&label0, &&label1, &&label2, 
                             &&label3, &&label4, &&label5 };
    
    int result = 0;
    int i = 0;
    
    while (i < n) {
        int idx = arr[i] % 6;
        goto *labels[idx];
        
    label0:
        result += arr[i] * 2;
        i++;
        continue;
        
    label1:
        result -= arr[i];
        /* Scheduling barrier */
        asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        i++;
        continue;
        
    label2:
        result ^= arr[i] << 1;
        i += 2;
        continue;
        
    label3:
        result |= arr[i];
        i++;
        continue;
        
    label4:
        result &= arr[i];
        i++;
        continue;
        
    label5:
        result = (result < 0) ? -result : result;
        i++;
        continue;
    }
    
    return result;
}

/* Main test driver */
int main() {
    float array1[SIZE];
    float array2[SIZE];
    float array3[SIZE];
    int int_array[SIZE];
    
    /* Initialize arrays with meaningful data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (float)rand() / RAND_MAX * 100.0f;
        array2[i] = (float)rand() / RAND_MAX * 100.0f;
        int_array[i] = rand() % 1000;
    }
    
    double total_result = 0.0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function (should trigger selective scheduling) */
        float hot_result = hot_function(array1, array2, SIZE);
        total_result += hot_result;
        
        /* Call cold function */
        int cold_result = cold_function(int_array, SIZE);
        total_result += cold_result;
        
        /* Vectorized function */
        vectorized_function(array3, array1, array2, SIZE);
        for (int i = 0; i < SIZE; i++) {
            total_result += array3[i];
        }
        
        /* Nested loop scheduler */
        double nested_result = nested_loop_scheduler(64);  /* Smaller size for speed */
        total_result += nested_result;
        
        /* Computed goto pattern */
        int goto_result = computed_goto_pattern(int_array, SIZE);
        total_result += goto_result;
        
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            array1[i] += 0.01f;
            int_array[i] = (int_array[i] + 1) % 1000;
        }
    }
    
    printf("Final accumulated result: %f\n", total_result);
    
    /* Additional test with profile feedback */
    if (total_result > 0) {
        /* One more complex scheduling scenario */
        float* dyn_array = malloc(SIZE * sizeof(float));
        if (dyn_array) {
            for (int i = 0; i < SIZE; i++) {
                dyn_array[i] = sinf(i * 0.1f);
            }
            
            /* Complex pointer arithmetic with scheduling challenges */
            float* ptr = dyn_array;
            float sum = 0.0f;
            for (int i = 0; i < SIZE; i++) {
                /* Pointer chasing with offset */
                sum += *ptr;
                ptr = dyn_array + ((ptr - dyn_array + 7) % SIZE);
                
                /* Memory barrier every 16 iterations */
                if (i % 16 == 0) {
                    asm volatile("" ::: "memory");
                }
            }
            
            printf("Dynamic array sum: %f\n", sum);
            free(dyn_array);
        }
    }
    
    return 0;
}
