/* Complex loop patterns to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 5000
#define INNER_ITERS 100
#define MIDDLE_ITERS 50
#define OUTER_ITERS 20

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int data[3];
};

/* Function with complex reduction pattern and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double complex_reduction(struct MixedData* arr, int n) {
    double acc = 0.0;
    volatile int v_cond = g_volatile_counter;
    
    /* Outer loop with data-dependent condition */
    for (int i = 1; i < n; i++) {
        /* Middle loop with volatile condition */
        for (int j = 0; j < MIDDLE_ITERS; j++) {
            if (v_cond > (j % 3)) {
                /* Innermost loop with reduction and carried dependency */
                for (int k = 0; k < INNER_ITERS; k++) {
                    /* Critical carried dependency across iterations */
                    acc = acc + arr[i].value * arr[i-1].weight;
                    
                    /* Mixed operations to generate diverse RTL */
                    arr[i].data[k % 3] = (int)(acc * 0.1);
                    arr[i].weight = arr[i].weight * 0.99 + acc * 0.01;
                    
                    /* Volatile access to prevent optimization */
                    if (g_volatile_float > 0.5f) {
                        arr[i].value = arr[i].value * 1.1f - 0.1f;
                    }
                }
            } else {
                /* Alternative path with different operations */
                for (int k = 0; k < INNER_ITERS/2; k++) {
                    acc = acc - arr[i].value * 0.5;
                    arr[i].data[(k*2) % 3] = (int)(acc * 0.2);
                }
            }
            
            /* Update volatile condition */
            v_cond = (v_cond + j) % 7;
        }
        
        /* Non-contiguous access pattern */
        if (i % 3 == 0) {
            arr[i].tag = (char)((int)acc % 26 + 'A');
        }
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed data types */
__attribute__((optimize("O3", "funroll-loops")))
float nested_mixed_operations(struct MixedData* arr, int n) {
    float result = 0.0f;
    volatile float vf = g_volatile_float;
    
    /* Triple nested loop with data-dependent branches */
    for (int i = 0; i < OUTER_ITERS; i++) {
        for (int j = 0; j < MIDDLE_ITERS; j++) {
            /* Volatile condition prevents branch prediction */
            if (vf > (float)((i + j) % 5)) {
                for (int k = 0; k < INNER_ITERS; k++) {
                    /* Complex mixed-type operations */
                    int idx = (i * 7 + j * 3 + k) % n;
                    result += (float)arr[idx].weight * arr[idx].value;
                    
                    /* Conditional store with stride */
                    if (k % 4 == 0) {
                        arr[idx].data[0] = (int)(result * 100.0f);
                    }
                    
                    /* More mixed operations */
                    double temp = arr[idx].weight;
                    arr[idx].weight = temp * 0.95 + (double)result * 0.05;
                    
                    /* Update volatile */
                    vf = vf * 0.9f + 0.1f;
                }
            }
        }
        
        /* Function call with side effect to prevent optimization */
        g_volatile_counter = (g_volatile_counter + i) % 100;
    }
    
    return result;
}

/* Function with pointer chasing and complex addressing */
__attribute__((optimize("O2")))
int pointer_chasing_reduction(struct MixedData* arr, int n) {
    int sum = 0;
    struct MixedData* current = &arr[0];
    volatile int skip = g_volatile_counter % 5;
    
    /* Loop with pointer chasing pattern */
    for (int i = 0; i < n * 2; i++) {
        /* Non-linear access pattern */
        int next_idx = (current->id + i * 3) % n;
        current = &arr[next_idx];
        
        /* Reduction with mixed operations */
        sum += current->data[i % 3];
        sum -= (int)(current->value * 10.0f);
        
        /* Data-dependent conditional */
        if (skip > 2) {
            current->weight = current->weight * 0.8 + (double)sum * 0.001;
            skip = (skip * 3) % 7;
        }
        
        /* Nested loop with small iteration count */
        for (int j = 0; j < 5; j++) {
            sum = sum ^ (current->data[j % 3] * j);
            if (j % 2 == 0) {
                current->value = current->value * 1.05f - 0.02f;
            }
        }
    }
    
    return sum;
}

/* Initialize array with pseudo-random but deterministic values */
void initialize_array(struct MixedData* arr, int n) {
    unsigned int seed = 42;
    for (int i = 0; i < n; i++) {
        arr[i].id = i;
        arr[i].value = (float)((seed = seed * 1103515245 + 12345) % 1000) / 100.0f;
        arr[i].weight = (double)((seed = seed * 1103515245 + 12345) % 2000) / 200.0;
        arr[i].tag = (char)('A' + i % 26);
        for (int j = 0; j < 3; j++) {
            arr[i].data[j] = (seed = seed * 1103515245 + 12345) % 10000;
        }
    }
}

int main() {
    /* Allocate and initialize data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_array(data, SIZE);
    
    printf("Starting complex computations...\n");
    
    /* Call all computation functions to trigger various scheduling scenarios */
    double result1 = complex_reduction(data, SIZE);
    printf("Result 1: %f\n", result1);
    
    float result2 = nested_mixed_operations(data, SIZE);
    printf("Result 2: %f\n", result2);
    
    int result3 = pointer_chasing_reduction(data, SIZE);
    printf("Result 3: %d\n", result3);
    
    /* Final combination to ensure all results are used */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    /* Additional volatile operations to prevent dead code elimination */
    g_volatile_counter = (int)final_result % 1000;
    g_volatile_float = (float)(final_result * 0.001);
    
    free(data);
    return 0;
}
