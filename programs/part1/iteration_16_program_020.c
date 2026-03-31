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
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read after write */
        float temp = data[i] * 2.0f;
        
        /* WAR hazard: write after read */
        data[i] = temp + 1.0f;
        
        /* WAW hazard: write after write */
        sum += data[i];
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* More complex dependency chain */
        temp = sinf(data[i]) * cosf(sum);
        data[i] = temp * 0.5f;
    }
    
    return sum;
}

/* Cold function with noinline to create scheduling boundaries */
__attribute__((cold, noinline))
static int cold_function(int *arr, int n) {
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
                result -= arr[i] / 3;
                break;
            case 2:
                result ^= arr[i];
                /* Register clobber */
                asm volatile("" ::: "r0", "r1", "memory");
                break;
            case 3:
                result |= arr[i] << 2;
                break;
            case 4:
                result &= ~arr[i];
                break;
            default:
                result = result * 3 + arr[i];
                break;
        }
        
        /* Conditional move vs branch */
        result = (arr[i] > 0) ? (result + 1) : (result - 1);
    }
    
    return result;
}

/* SIMD-friendly function with unrolling pragma */
__attribute__((optimize("O3")))
static void vectorized_function(float *a, float *b, float *c, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Vectorizable operations */
        float t1 = a[i] * b[i];
        float t2 = sinf(a[i]) + cosf(b[i]);
        
        /* Dependency chain */
        c[i] = t1 * t2 + a[i] * 0.3f;
        
        /* Another dependency */
        b[i] = c[i] * 0.7f - t1;
        
        /* Mixed precision */
        a[i] = (float)((int)c[i] ^ (int)b[i]) * 0.1f;
    }
}

/* Function with pointer chasing and complex scheduling */
__attribute__((optimize("O2")))
static double pointer_chasing(int **ptr_array, int steps) {
    double total = 0.0;
    int *current = ptr_array[0];
    
    for (int i = 0; i < steps; i++) {
        /* Load with potential cache miss */
        int value = *current;
        
        /* Floating point operation */
        total += sqrt(fabs((double)value));
        
        /* Pointer arithmetic with dependency */
        current = ptr_array[(value + i) % SIZE];
        
        /* Memory barrier between dependent operations */
        asm volatile("" ::: "memory");
        
        /* More computations */
        total *= 1.0001;
        total -= sin((double)value * 0.01);
    }
    
    return total;
}

/* Nested loops with mixed operations */
__attribute__((optimize("O3", "unroll-loops")))
static void nested_loop_test(float matrix[SIZE][SIZE]) {
    /* Outer loop with multiple early exits */
    for (int i = 0; i < SIZE; i++) {
        if (i % 13 == 0) continue;  /* Skip some iterations */
        
        float row_sum = 0.0f;
        
        /* Inner loop with vectorization hints */
        #pragma GCC ivdep
        for (int j = 0; j < SIZE; j++) {
            /* Mixed operations creating scheduling challenges */
            float val = matrix[i][j];
            
            /* RAW: Read after write hazard */
            float transformed = val * val + sinf(val);
            
            /* WAR: Write after read hazard */
            matrix[i][j] = transformed * 0.5f;
            
            /* Accumulate with dependency */
            row_sum += matrix[i][j];
            
            /* Early exit condition */
            if (row_sum > 1000000.0f) break;
        }
        
        /* Store result with memory barrier */
        asm volatile("" ::: "memory");
        matrix[i][0] = row_sum;
    }
}

/* Function with computed goto (challenges scheduler) */
__attribute__((noinline, optimize("O1")))
static int computed_goto_test(int x) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    int result = x;
    
    /* Jump table */
    goto *labels[x % 5];
    
label0:
    result += x * 2;
    /* Inline asm with specific register constraints */
    asm volatile("add %0, %0, #1" : "+r"(result) ::);
    goto end;
    
label1:
    result -= x / 3;
    asm volatile("" ::: "r0", "r1", "r2", "r3", "memory");
    goto end;
    
label2:
    result ^= x << 1;
    goto end;
    
label3:
    result |= 0xFF;
    asm volatile("" ::: "memory");
    goto end;
    
label4:
    result &= 0x7F;
    goto end;
    
end:
    return result;
}

/* Main test driver */
int main() {
    /* Initialize data */
    float float_data[SIZE];
    int int_data[SIZE];
    float vec_a[SIZE], vec_b[SIZE], vec_c[SIZE];
    float matrix[SIZE][SIZE];
    int *ptr_array[SIZE];
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        int_data[i] = rand() % 1000;
        vec_a[i] = (float)rand() / RAND_MAX;
        vec_b[i] = (float)rand() / RAND_MAX;
        ptr_array[i] = &int_data[(i * 13) % SIZE];
        
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (float)rand() / RAND_MAX;
        }
    }
    
    double total_result = 0.0;
    
    /* Run all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Hot function with scheduling pressure */
        total_result += hot_function(float_data, SIZE);
        
        /* Cold function with complex control flow */
        total_result += cold_function(int_data, SIZE);
        
        /* Vectorized function */
        vectorized_function(vec_a, vec_b, vec_c, SIZE);
        total_result += vec_c[SIZE-1];
        
        /* Pointer chasing */
        total_result += pointer_chasing(ptr_array, 1000);
        
        /* Nested loops */
        nested_loop_test(matrix);
        total_result += matrix[0][0];
        
        /* Computed goto */
        total_result += computed_goto_test(iter % 100);
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
    }
    
    printf("Final result: %f\n", total_result);
    
    /* Prevent dead code elimination */
    volatile float_data[0] = total_result;
    
    return 0;
}
