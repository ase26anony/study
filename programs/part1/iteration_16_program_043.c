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
        /* RAW hazard: read after write to same location */
        float temp = data[i] * 2.0f;
        sum += temp;
        
        /* WAR hazard: write after read */
        data[i] = sum * 0.5f;
        
        /* WAW hazard: multiple writes to sum */
        sum = sum * 0.99f + temp * 0.01f;
    }
    
    /* Memory barrier forcing scheduler decisions */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Cold function with noinline to create scheduling boundaries */
__attribute__((cold, noinline))
static int cold_function(int *arr, int n) {
    int result = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 8) {
            case 0: result += arr[i] * 2; break;
            case 1: result -= arr[i]; break;
            case 2: result ^= arr[i]; break;
            case 3: result |= arr[i]; break;
            case 4: result &= arr[i]; break;
            case 5: result = result << (arr[i] & 3); break;
            case 6: result = result >> (arr[i] & 3); break;
            default: result = ~result; break;
        }
        
        /* Conditional move vs branch */
        result = (arr[i] > 0) ? (result + 1) : (result - 1);
    }
    
    return result;
}

/* SIMD-friendly function with unroll pragma */
__attribute__((optimize("O3")))
static void vectorized_loop(float *a, float *b, float *c, int n) {
    int i;
    
    /* Pointer chasing with mixed dependencies */
    float *ptr_a = a;
    float *ptr_b = b;
    float *ptr_c = c;
    
    #pragma GCC unroll 4
    for (i = 0; i < n; i++) {
        /* Load/store sequences with varying latencies */
        float val1 = *ptr_a++;
        float val2 = *ptr_b++;
        
        /* Mixed FP operations */
        val1 = val1 * val2 + sinf(val1) * cosf(val2);
        val1 = sqrtf(fabsf(val1)) + expf(val2 * 0.01f);
        
        /* Store with dependency chain */
        *ptr_c++ = val1;
        
        /* Inline asm with register clobber */
        if (i % 16 == 0) {
            asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        }
    }
}

/* Function with nested loops and early exits */
__attribute__((noinline))
static double complex_control_flow(int *matrix, int size) {
    double total = 0.0;
    
    for (int i = 0; i < size; i++) {
        /* Multiple early exit points */
        if (matrix[i * size] == 0) {
            continue;
        }
        
        for (int j = 0; j < size; j++) {
            int idx = i * size + j;
            
            /* Complex condition with side effects */
            if (matrix[idx] > 100 && j % 3 == 0) {
                total += sqrt(matrix[idx]);
                
                /* Another early exit */
                if (total > 1000000.0) {
                    goto early_exit;
                }
            } else if (matrix[idx] < -50) {
                total -= fabs(matrix[idx] * 0.5);
            } else {
                total += matrix[idx] * 0.25;
            }
            
            /* Memory barrier splitting scheduling regions */
            if (j % 8 == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Continue condition with computation */
        if (i % 2 == 0) {
            total *= 0.95;
        }
    }
    
early_exit:
    return total;
}

/* Main test function with all patterns */
__attribute__((optimize("O3", "unroll-loops")))
static void run_scheduler_stress_test(void) {
    static float float_data[SIZE];
    static int int_data[SIZE];
    static float vec_a[SIZE], vec_b[SIZE], vec_c[SIZE];
    static int matrix[64][64];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        float_data[i] = (float)i / SIZE;
        int_data[i] = i * 3 - SIZE/2;
        vec_a[i] = sinf(i * 0.1f);
        vec_b[i] = cosf(i * 0.1f);
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = (i * j - 512) % 1000;
        }
    }
    
    float hot_result = 0.0f;
    int cold_result = 0;
    double flow_result = 0.0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with mixed operations */
        hot_result += hot_function(float_data, SIZE);
        
        /* Call cold function with complex control flow */
        cold_result ^= cold_function(int_data, SIZE);
        
        /* Vectorized operations */
        vectorized_loop(vec_a, vec_b, vec_c, SIZE);
        
        /* Complex control flow with matrix */
        flow_result += complex_control_flow((int *)matrix, 64);
        
        /* Modify data for next iteration */
        for (int i = 0; i < SIZE; i++) {
            float_data[i] += 0.001f;
            int_data[i] = (int_data[i] * 13 + 7) & 0xFFF;
            vec_a[i] = vec_c[i] * 0.9f;
            vec_b[i] = vec_a[i] * 1.1f;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Hot result: %f\n", hot_result);
    printf("Cold result: %d\n", cold_result);
    printf("Flow result: %f\n", flow_result);
    
    /* Final vector result check */
    float vec_sum = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        vec_sum += vec_c[i];
    }
    printf("Vector sum: %f\n", vec_sum);
}

/* Additional test with computed goto */
__attribute__((noinline, optimize("O2")))
static int computed_goto_test(int x) {
    static void *jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3, 
        &&case_4, &&case_5, &&case_6, &&case_7
    };
    
    int result = x;
    
    /* Computed goto creates interesting control flow */
    if (x >= 0 && x < 8) {
        goto *jump_table[x];
    } else {
        goto default_case;
    }
    
case_0:
    result *= 2;
    /* Fall through */
case_1:
    result += 3;
    goto end;
case_2:
    result -= 5;
    goto end;
case_3:
    result ^= 0xFF;
    goto end;
case_4:
    result |= 0xAA;
    goto end;
case_5:
    result &= 0x55;
    goto end;
case_6:
    result <<= 2;
    goto end;
case_7:
    result >>= 1;
    goto end;
default_case:
    result = ~result;
    goto end;
    
end:
    return result;
}

int main(void) {
    clock_t start = clock();
    
    printf("Starting selective scheduler stress test...\n");
    
    /* Run main stress test */
    run_scheduler_stress_test();
    
    /* Additional tests with different characteristics */
    int goto_result = 0;
    for (int i = 0; i < 1000; i++) {
        goto_result += computed_goto_test(i % 10);
    }
    printf("Computed goto result: %d\n", goto_result);
    
    /* Mixed precision arithmetic */
    double mixed_result = 0.0;
    for (int i = 0; i < SIZE; i++) {
        float f = i * 0.01f;
        double d = i * 0.01;
        int n = i;
        
        /* Heterogeneous operation mix */
        mixed_result += f * d + n / (f + 1.0f) - sin(d) * cos(f);
        
        /* Scheduling barrier every 32 iterations */
        if (i % 32 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    printf("Mixed precision result: %f\n", mixed_result);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Total time: %f seconds\n", elapsed);
    
    return 0;
}
