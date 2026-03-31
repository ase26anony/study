/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20
#define OUTER_LOOP 10

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

/* Function with complex reduction pattern and carried dependency */
__attribute__((optimize("O2,unroll-loops")))
double complex_reduction(const double* array, int size) {
    volatile int cond = g_volatile_counter % 7;
    double acc = 0.0;
    double prev = array[0];
    
    /* Loop with carried dependency */
    for (int i = 1; i < size; i++) {
        /* Data-dependent branching */
        if (cond > 3) {
            acc = acc + array[i] * prev;
        } else {
            acc = acc - array[i] * prev;
        }
        
        /* Mixed operations */
        prev = array[i] + (i % 5);
        
        /* Volatile access to prevent optimization */
        if (g_volatile_counter++ % 1000 == 0) {
            acc *= 0.99;
        }
    }
    
    return acc;
}

/* Function with nested loops and non-contiguous access */
__attribute__((optimize("O3")))
float nested_loop_processing(struct MixedData* data, int count) {
    float total = 0.0f;
    volatile int skip = g_volatile_counter % 4;
    
    /* Triple nested loop */
    for (int o = 0; o < OUTER_LOOP; o++) {
        for (int m = 0; m < MIDDLE_LOOP; m++) {
            for (int i = 0; i < INNER_LOOP; i++) {
                int idx = (o * 100 + m * 5 + i) % count;
                
                /* Non-contiguous access pattern */
                if (idx % 3 == skip) {
                    /* Mixed type operations */
                    total += data[idx].value * data[idx].weight;
                    
                    /* Conditional store */
                    if (data[idx].id % 2 == 0) {
                        data[idx].data[0] = (int)(total * 100);
                    }
                }
                
                /* Complex condition with volatile */
                if (g_volatile_float > 2.0f || (o + m) % 7 == skip) {
                    total -= data[idx].weight * 0.5f;
                }
            }
            
            /* Data-dependent branch in middle loop */
            if (m % (skip + 2) == 0) {
                total *= 1.01f;
            }
        }
        
        /* Outer loop reduction */
        total = total / (o + 1);
    }
    
    return total;
}

/* Function with deeply nested control flow */
__attribute__((hot, optimize("O2")))
int complex_control_flow(int* arr, int size) {
    int result = 0;
    volatile int v1 = g_volatile_counter;
    volatile int v2 = g_volatile_counter + 1;
    
    /* Multiple nested loops with data-dependent conditions */
    for (int i = 0; i < size; i += 2) {
        for (int j = i; j < size && j < i + 10; j++) {
            /* Complex condition using volatile */
            if ((v1++ % 5) > (v2 % 3)) {
                for (int k = 0; k < 5; k++) {
                    /* Reduction with mixed operations */
                    result += arr[j] * k;
                    
                    /* Floating point in integer loop */
                    if (k % 2 == 0) {
                        float temp = (float)arr[j] / (k + 1);
                        result += (int)temp;
                    }
                }
                
                /* Conditional break */
                if (result > 1000000) {
                    result /= 2;
                }
            } else {
                /* Alternative path */
                result -= arr[j] * 2;
            }
            
            /* Memory access with stride */
            if (j % 4 == 0) {
                arr[j] = result % 100;
            }
        }
        
        /* Loop-carried dependency */
        arr[i] = result;
    }
    
    return result;
}

/* Function with software pipelining candidate */
__attribute__((optimize("O3,unroll-loops")))
double pipeline_candidate(double* a, double* b, double* c, int n) {
    double sum = 0.0;
    volatile int mod = g_volatile_counter % 8;
    
    /* Loop designed for software pipelining */
    for (int i = 0; i < n - 4; i += 2) {
        /* Multiple independent operations that can be pipelined */
        double t1 = a[i] * b[i];
        double t2 = a[i+1] * b[i+1];
        double t3 = a[i+2] * b[i+2];
        
        /* Reduction with staggered dependencies */
        sum += t1 + t2 * 0.5;
        c[i] = t1 + sum;
        
        /* Conditional that depends on volatile */
        if ((i % (mod + 1)) == 0) {
            sum -= t3 * 0.25;
            c[i+1] = t2 - sum;
        } else {
            sum += t3 * 0.75;
            c[i+1] = t2 + sum;
        }
        
        /* Cross-iteration dependency */
        a[i] = sum * 0.1;
    }
    
    return sum;
}

/* Initialize data with pseudo-random values */
void initialize_data(double* array, struct MixedData* mixed, int size) {
    unsigned int seed = 42;
    
    for (int i = 0; i < size; i++) {
        /* Simple LCG for reproducibility */
        seed = (1103515245 * seed + 12345) % 2147483648;
        array[i] = (double)(seed % 1000) / 10.0;
        
        mixed[i].id = i;
        mixed[i].value = (float)(seed % 500) / 5.0f;
        mixed[i].weight = (double)(seed % 1000) / 100.0;
        mixed[i].tag = (char)('A' + (seed % 26));
        
        for (int j = 0; j < 3; j++) {
            seed = (1103515245 * seed + 12345) % 2147483648;
            mixed[i].data[j] = seed % 100;
        }
    }
}

int main() {
    /* Allocate and initialize data */
    double* array = (double*)malloc(SIZE * sizeof(double));
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* array_b = (double*)malloc(SIZE * sizeof(double));
    double* array_c = (double*)malloc(SIZE * sizeof(double));
    struct MixedData* mixed = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    
    if (!array || !int_array || !array_b || !array_c || !mixed) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    initialize_data(array, mixed, SIZE);
    
    /* Initialize other arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i * 3 % 100;
        array_b[i] = (double)(i % 50) / 3.0;
        array_c[i] = 0.0;
    }
    
    double total_result = 0.0;
    
    /* Call various computation functions to exercise scheduler */
    total_result += complex_reduction(array, SIZE);
    
    float nested_result = nested_loop_processing(mixed, SIZE);
    total_result += nested_result;
    
    int control_result = complex_control_flow(int_array, SIZE);
    total_result += control_result;
    
    double pipeline_result = pipeline_candidate(array, array_b, array_c, SIZE);
    total_result += pipeline_result;
    
    /* Additional mixed workload */
    for (int iter = 0; iter < 5; iter++) {
        g_volatile_counter++;
        total_result += complex_reduction(array_c, SIZE / 2);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", total_result);
    printf("Mixed data sample: id=%d, value=%f, weight=%lf\n", 
           mixed[100].id, mixed[100].value, mixed[100].weight);
    
    /* Cleanup */
    free(array);
    free(int_array);
    free(array_b);
    free(array_c);
    free(mixed);
    
    return 0;
}
