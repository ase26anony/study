/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2048
#define INNER_LOOP 128
#define MIDDLE_LOOP 64
#define OUTER_LOOP 32

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

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double complex_reduction(const double* data, int size) {
    volatile int cond = g_volatile_counter;
    double acc = 0.0;
    double prev = data[0];
    
    /* Outer loop */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Middle loop with volatile condition */
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            /* Inner loop with data-dependent control flow */
            for (int k = 0; k < INNER_LOOP; k++) {
                /* Data-dependent branch using volatile */
                if ((cond + i + j + k) % 7 == 0) {
                    /* Carried dependency across iterations */
                    double current = data[(i * MIDDLE_LOOP * INNER_LOOP + 
                                          j * INNER_LOOP + k) % size];
                    acc += current * prev;
                    prev = current;
                    
                    /* Mixed operations */
                    acc = acc * 1.0001 - 0.00005;
                } else if ((cond + i + j) % 5 == 0) {
                    /* Alternative path with different operations */
                    float temp = (float)acc * 0.99f;
                    acc = (double)temp + 0.01;
                }
                
                /* Volatile read to prevent optimization */
                cond = g_volatile_counter % 13;
            }
            
            /* Non-contiguous memory access */
            if (j % 3 == 0) {
                double temp = data[(j * 7) % size];
                acc += temp * 0.5;
            }
        }
        
        /* Additional computation between middle loops */
        acc = acc * (1.0 + (i % 10) * 0.001);
    }
    
    return acc;
}

/* Function processing mixed data types with non-contiguous access */
__attribute__((optimize("O3", "funroll-loops")))
float process_mixed_types(struct MixedData* array, int count) {
    volatile float fcond = g_volatile_float;
    float total = 0.0f;
    double running_sum = 0.0;
    
    /* Deeply nested loops */
    for (int a = 0; a < 8; a++) {
        for (int b = 0; b < 16; b++) {
            /* Data-dependent loop with volatile */
            int limit = (int)(fcond * 10) + b;
            limit = limit > 32 ? 32 : limit;
            
            for (int c = 0; c < limit; c++) {
                /* Non-unit stride access */
                int idx = (a * 97 + b * 23 + c * 7) % count;
                
                if (idx >= 0 && idx < count) {
                    /* Mixed type operations */
                    total += array[idx].value * (float)array[idx].weight;
                    
                    /* Conditional store based on volatile */
                    if ((int)(fcond * 1000) % 11 == (idx % 11)) {
                        array[idx].data[0] = (int)(total * 100);
                    }
                    
                    /* Complex reduction with multiple dependencies */
                    running_sum += (double)array[idx].value * 
                                   (double)array[idx].data[1] * 
                                   (c % 5 + 1);
                }
                
                /* Update volatile condition */
                fcond = fcond * 1.01f - 0.005f;
            }
            
            /* Inter-loop dependency */
            total = total * 0.99f + (float)running_sum * 0.01f;
        }
    }
    
    return total + (float)running_sum;
}

/* Function with unpredictable control flow */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
int unpredictable_control_flow(int* data, int size) {
    volatile int seed = g_volatile_counter;
    int result = 0;
    
    /* Triple nested loop with data-dependent bounds */
    for (int x = 0; x < 12; x++) {
        int x_bound = (seed + x) % 8 + 4;
        
        for (int y = 0; y < x_bound; y++) {
            int y_bound = (seed + x + y) % 6 + 3;
            
            for (int z = 0; z < y_bound; z++) {
                /* Complex addressing */
                int pos = (x * 19 + y * 13 + z * 11) % size;
                
                /* Multiple conditional paths */
                if ((data[pos] + seed) % 3 == 0) {
                    result += data[pos] * 2;
                    /* Create dependency chain */
                    data[pos] = result % 1000;
                } else if ((data[pos] + x) % 4 == 0) {
                    result -= data[pos];
                    /* Floating point in integer loop */
                    float ftemp = (float)result * 0.75f;
                    result = (int)ftemp;
                } else {
                    result ^= data[pos];
                }
                
                /* Volatile modification */
                seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            }
            
            /* Cross-iteration dependency */
            if (y % 2 == 0) {
                result = result * 3 - 1;
            }
        }
        
        /* Reduction across outer loops */
        result = result / (x + 1);
    }
    
    return result;
}

/* Initialize data with pseudo-random values */
void initialize_data(double* double_data, struct MixedData* mixed_data, 
                     int* int_data, int size) {
    unsigned int seed = 123456789;
    
    for (int i = 0; i < size; i++) {
        /* Simple LCG for reproducibility */
        seed = seed * 1103515245 + 12345;
        double_data[i] = (double)(seed % 10000) / 100.0;
        
        seed = seed * 1103515245 + 12345;
        int_data[i] = seed % 1000;
        
        if (i < size / 2) {
            mixed_data[i].id = i;
            mixed_data[i].value = (float)(seed % 1000) / 10.0f;
            seed = seed * 1103515245 + 12345;
            mixed_data[i].weight = (double)(seed % 10000) / 1000.0;
            mixed_data[i].tag = 'A' + (i % 26);
            for (int j = 0; j < 3; j++) {
                seed = seed * 1103515245 + 12345;
                mixed_data[i].data[j] = seed % 100;
            }
        }
    }
}

int main() {
    /* Allocate and initialize data */
    double* double_data = (double*)malloc(SIZE * sizeof(double));
    struct MixedData* mixed_data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    
    if (!double_data || !mixed_data || !int_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_data(double_data, mixed_data, int_data, SIZE);
    
    /* Perform multiple computations to increase scheduler activity */
    double result1 = 0.0;
    float result2 = 0.0f;
    int result3 = 0;
    
    /* Call functions multiple times with different parameters */
    for (int iter = 0; iter < 3; iter++) {
        g_volatile_counter = iter * 7;
        g_volatile_float = 0.5f + iter * 0.3f;
        
        result1 += complex_reduction(double_data, SIZE);
        result2 += process_mixed_types(mixed_data, SIZE / 2);
        result3 += unpredictable_control_flow(int_data, SIZE);
        
        /* Modify data slightly between iterations */
        for (int i = 0; i < SIZE; i += 17) {
            double_data[i] *= 1.01;
            int_data[i] += iter;
        }
    }
    
    /* Combine results to ensure they're used */
    double final_result = (double)result1 + (double)result2 + (double)result3;
    printf("Final result: %f\n", final_result);
    
    /* Clean up */
    free(double_data);
    free(mixed_data);
    free(int_data);
    
    return 0;
}
