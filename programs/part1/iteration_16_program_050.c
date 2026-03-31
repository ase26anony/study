/* sel-sched-test.c - Comprehensive test for GCC selective scheduler dump logic */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, noinline, optimize("O3", "unroll-loops")))
static float hot_loop_scheduler(float *data, int size) {
    volatile float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < size; i++) {
        /* RAW hazard: read after write */
        float temp = data[i] * 2.0f;
        sum += temp;
        
        /* WAR hazard: write after read */
        data[i] = sum * 0.5f;
        
        /* WAW hazard: write after write */
        temp = data[i] + 1.0f;
        data[i] = temp * 0.8f;
    }
    
    /* Inline assembly barrier */
    asm volatile("" ::: "memory");
    
    return sum;
}

__attribute__((cold, noinline, optimize("sched-pressure")))
static int cold_control_flow(int *arr, int n) {
    int result = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 7) {
            case 0:
                result += arr[i] * 2;
                /* Fall through */
            case 1:
                result -= arr[i];
                break;
            case 2:
                result ^= arr[i];
                /* Conditional move */
                result = (arr[i] > 0) ? result : -result;
                break;
            case 3:
                /* Nested if-else */
                if (arr[i] % 3 == 0) {
                    result += arr[i] << 2;
                } else if (arr[i] % 3 == 1) {
                    result -= arr[i] >> 1;
                } else {
                    result |= arr[i];
                }
                break;
            case 4:
                /* Early exit point */
                if (arr[i] > 1000) {
                    return result;
                }
                result *= arr[i];
                break;
            case 5:
                /* Continue with computation */
                result += arr[i] * arr[i];
                continue;
            default:
                result = ~result;
                break;
        }
        
        /* Another assembly barrier with register clobber */
        asm volatile("" ::: "eax", "memory");
    }
    
    return result;
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vectorized_pointer_chasing(float **ptrs, float *output, int count) {
    /* Pointer chasing with mixed access patterns */
    #pragma GCC unroll 4
    for (int i = 0; i < count; i++) {
        float *current = ptrs[i];
        float accum = 0.0f;
        
        /* Load/store sequence with varying latencies */
        for (int j = 0; j < 8; j++) {
            float val = *current;
            accum += val * val;
            
            /* Store with dependency */
            *current = accum * 0.1f;
            
            /* Pointer arithmetic */
            current += (j % 3) + 1;
            
            /* Memory barrier */
            if (j % 4 == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        output[i] = accum;
    }
}

__attribute__((noinline, optimize("O3")))
static double mixed_operations_test(double *a, double *b, int len) {
    double sum = 0.0;
    
    /* SIMD-friendly loop with mixed operations */
    #pragma GCC unroll 8
    for (int i = 0; i < len; i++) {
        /* Floating point operations */
        double temp1 = sin(a[i]) * cos(b[i]);
        double temp2 = exp(a[i] * 0.1) + log(fabs(b[i]) + 1.0);
        
        /* Integer operations mixed in */
        int idx = i & 0xFF;
        double temp3 = temp1 * idx + temp2 / (idx + 1);
        
        /* Conditional operation */
        sum += (temp3 > 0) ? temp3 : -temp3;
        
        /* Write to both arrays */
        a[i] = temp1 * 0.5;
        b[i] = temp2 * 2.0;
        
        /* Periodic scheduling barrier */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory", "xmm0", "xmm1");
        }
    }
    
    return sum;
}

__attribute__((optimize("O3", "unroll-loops")))
static void nested_loop_hazards(int size, float *out) {
    /* Triple nested loop with complex dependencies */
    for (int i = 0; i < size; i++) {
        float acc_i = 0.0f;
        
        #pragma GCC unroll 2
        for (int j = 0; j < size; j++) {
            float acc_j = 0.0f;
            
            for (int k = 0; k < size; k++) {
                /* Multiple hazards in inner loop */
                float val = out[k];
                
                /* RAW: read after write to acc_j */
                acc_j += val * (i + j + k);
                
                /* WAR: write after read of val */
                val = acc_j * 0.3f;
                
                /* WAW: write after write to out[k] */
                out[k] = val + out[(k + 1) % size];
                
                /* Complex dependency chain */
                acc_j = acc_j * 0.9f + sin(val);
            }
            
            acc_i += acc_j;
            
            /* Control flow affecting scheduling */
            if (j % 3 == 0) {
                asm volatile("" ::: "memory");
                acc_i *= 0.5f;
            } else if (j % 3 == 1) {
                acc_i += 1.0f;
            }
        }
        
        out[i] = acc_i;
    }
}

/* Computed goto for complex control flow */
__attribute__((noinline))
static int computed_goto_test(int x) {
    static void *jumptable[] = {
        &&case0, &&case1, &&case2, &&case3, 
        &&case4, &&case5, &&default_case
    };
    
    int result = x;
    int index = x % 7;
    
    goto *jumptable[index];
    
case0:
    result += x * 2;
    /* Fall through */
case1:
    result -= x / 2;
    goto end;
case2:
    result ^= 0xAAAA;
    result *= 3;
    goto end;
case3:
    result = (result > 0) ? result : -result;
    result <<= 2;
    goto end;
case4:
    result |= 0xFF00;
    result += x * x;
    goto end;
case5:
    result = ~result;
    result += 1;
    goto end;
default_case:
    result = 0;
    goto end;
    
end:
    /* Scheduling barrier */
    asm volatile("" ::: "memory", "eax", "ebx");
    return result;
}

int main(void) {
    /* Initialize data */
    float float_data[ARRAY_SIZE];
    int int_data[ARRAY_SIZE];
    float *pointers[ARRAY_SIZE/8];
    float output[ARRAY_SIZE/8];
    double double_a[ARRAY_SIZE];
    double double_b[ARRAY_SIZE];
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        int_data[i] = rand() % 1000;
    }
    
    for (int i = 0; i < ARRAY_SIZE/8; i++) {
        pointers[i] = &float_data[i * 8];
    }
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double_a[i] = (double)rand() / RAND_MAX * 50.0;
        double_b[i] = (double)rand() / RAND_MAX * 50.0;
    }
    
    /* Accumulator to prevent dead code elimination */
    volatile float total = 0.0f;
    volatile int int_total = 0;
    volatile double double_total = 0.0;
    
    /* Execute all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Hot loop with scheduling challenges */
        total += hot_loop_scheduler(float_data, ARRAY_SIZE);
        
        /* Cold function with complex control flow */
        int_total += cold_control_flow(int_data, ARRAY_SIZE);
        
        /* Vectorized pointer chasing */
        vectorized_pointer_chasing(pointers, output, ARRAY_SIZE/8);
        for (int i = 0; i < ARRAY_SIZE/8; i++) {
            total += output[i];
        }
        
        /* Mixed operations test */
        double_total += mixed_operations_test(double_a, double_b, ARRAY_SIZE);
        
        /* Nested loop hazards */
        nested_loop_hazards(32, float_data);
        for (int i = 0; i < 32; i++) {
            total += float_data[i];
        }
        
        /* Computed goto test */
        int_total += computed_goto_test(iter);
    }
    
    /* Print results to ensure execution */
    printf("Results: float_total = %f, int_total = %d, double_total = %f\n",
           (double)total, int_total, double_total);
    
    return 0;
}
