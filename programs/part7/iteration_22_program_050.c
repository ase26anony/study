/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int data[3];
};

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2")))
double reduction_with_carry(struct MixedData* arr, int n) {
    double acc = 0.0;
    double prev = arr[0].weight;
    
    /* Complex loop with data-dependent control flow */
    for (int i = 1; i < n; i++) {
        /* Volatile read to prevent optimization */
        volatile int cond = g_volatile_counter % (i + 1);
        
        /* Carried dependency: uses previous iteration's value */
        double current = arr[i].weight;
        acc += prev * current;
        
        /* Data-dependent branching */
        if (cond > (i % 100)) {
            acc += arr[i].value * 2.0;
            prev = current * 1.1;
        } else {
            acc -= arr[i].value * 0.5;
            prev = current * 0.9;
        }
        
        /* Non-contiguous memory access */
        if (i % 3 == 0) {
            acc += arr[i].data[0] * 0.01;
        }
        
        /* Mix integer and floating point operations */
        g_volatile_counter += (int)(acc * 0.001);
    }
    
    return acc;
}

/* Function 2: Deeply nested loops with volatile conditionals */
__attribute__((optimize("O3")))
float nested_loops_volatile(float* data, int size) {
    float result = 0.0f;
    
    /* Three-level nested loop */
    for (int i = 0; i < size / INNER_LOOP; i++) {
        volatile int outer_cond = g_volatile_counter % (i + 2);
        
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            volatile int middle_cond = (i * j) % 7;
            
            for (int k = 0; k < INNER_LOOP; k++) {
                int idx = i * INNER_LOOP * MIDDLE_LOOP + j * INNER_LOOP + k;
                if (idx >= size) break;
                
                /* Complex data-dependent condition */
                volatile float fcond = g_volatile_float * (i + j + k);
                
                if ((outer_cond + middle_cond) % 3 == 0) {
                    result += data[idx] * fcond;
                } else if (fcond > 10.0f) {
                    result -= data[idx] / fcond;
                } else {
                    result *= 1.001f;
                }
                
                /* Mixed operations */
                data[idx] = result * 0.99f;
                
                /* Prevent complete optimization */
                g_volatile_float += 0.01f;
            }
            
            /* Conditional store with stride */
            if (j % 4 == 0) {
                data[j * 10 % size] = result;
            }
        }
    }
    
    return result;
}

/* Function 3: Mixed data types with pointer arithmetic */
__attribute__((optimize("O2")))
double mixed_types_complex(struct MixedData* arr, int n) {
    double total = 0.0;
    int* int_ptr = &arr[0].id;
    float* float_ptr = &arr[0].value;
    
    /* Process with non-unit stride */
    for (int i = 0; i < n; i += 2) {
        /* Access through different type pointers */
        int int_val = *(int_ptr + i * sizeof(struct MixedData)/sizeof(int));
        float float_val = *(float_ptr + i * sizeof(struct MixedData)/sizeof(float));
        
        /* Complex floating point computation */
        double temp = (double)int_val * float_val;
        
        /* Data-dependent branching with volatile */
        volatile double v = g_volatile_float * i;
        
        if (v > temp) {
            total += temp * arr[i].weight;
            
            /* Nested conditional */
            if (int_val % 5 == 0) {
                total -= arr[i].data[1] * 0.5;
            }
        } else {
            total /= 1.1;
            
            /* Memory store with computation */
            arr[i].value = (float)(total * 0.01);
        }
        
        /* Cross-iteration dependency */
        if (i > 0) {
            total += arr[i-1].weight * 0.3;
        }
        
        /* Prevent dead code elimination */
        g_volatile_counter += int_val % 100;
    }
    
    return total;
}

/* Function 4: Reduction with multiple accumulators */
__attribute__((noinline))
void multi_reduction(double* data, int n, double* results) {
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0;
    
    /* Loop with multiple carried dependencies */
    for (int i = 1; i < n; i++) {
        volatile int mod = g_volatile_counter % (i + 3);
        
        /* Multiple inter-dependent computations */
        double val1 = data[i];
        double val2 = data[i-1];
        
        acc1 = acc1 + val1 * val2;
        acc2 = acc2 - val1 / (val2 + 1.0);
        acc3 = acc3 * 0.999 + val1 - val2;
        
        /* Complex conditional updates */
        if (mod == 0) {
            acc1 *= 1.01;
            data[i] = acc1;
        } else if (mod == 1) {
            acc2 += acc3 * 0.5;
        } else {
            acc3 = acc2 * 0.8;
        }
        
        /* Cross-accumulator dependencies */
        if (i % 10 == 0) {
            double temp = acc1 + acc2;
            acc3 += temp * 0.1;
            acc1 -= temp * 0.05;
        }
    }
    
    results[0] = acc1;
    results[1] = acc2;
    results[2] = acc3;
}

/* Simple PRNG for initialization (no library dependency) */
static unsigned int seed = 123456789;
unsigned int simple_rand() {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Initialize array with pseudo-random data */
void init_data(struct MixedData* arr, float* float_arr, double* double_arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i].id = simple_rand() % 1000;
        arr[i].value = (simple_rand() % 10000) / 100.0f;
        arr[i].weight = (simple_rand() % 10000) / 50.0;
        arr[i].tag = 'A' + (simple_rand() % 26);
        for (int j = 0; j < 3; j++) {
            arr[i].data[j] = simple_rand() % 100;
        }
        
        float_arr[i] = arr[i].value;
        double_arr[i] = arr[i].weight;
    }
}

int main() {
    /* Allocate and initialize data */
    struct MixedData* mixed_arr = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    float* float_arr = (float*)malloc(SIZE * sizeof(float));
    double* double_arr = (double*)malloc(SIZE * sizeof(double));
    double reduction_results[3];
    
    if (!mixed_arr || !float_arr || !double_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_data(mixed_arr, float_arr, double_arr, SIZE);
    
    printf("Starting complex computations...\n");
    
    /* Call all functions to trigger various scheduling scenarios */
    double result1 = reduction_with_carry(mixed_arr, SIZE);
    printf("Result 1: %f\n", result1);
    
    float result2 = nested_loops_volatile(float_arr, SIZE);
    printf("Result 2: %f\n", result2);
    
    double result3 = mixed_types_complex(mixed_arr, SIZE);
    printf("Result 3: %f\n", result3);
    
    multi_reduction(double_arr, SIZE, reduction_results);
    printf("Results 4: %f, %f, %f\n", 
           reduction_results[0], reduction_results[1], reduction_results[2]);
    
    /* Combine results to ensure all computations are used */
    double final_result = result1 + result2 + result3 + 
                         reduction_results[0] + reduction_results[1] + reduction_results[2];
    
    printf("Final combined result: %f\n", final_result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    printf("Volatile float: %f\n", g_volatile_float);
    
    /* Cleanup */
    free(mixed_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}
