/* Complex loop structures to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ARRAY_SIZE 2048
#define ITERATIONS 100000

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 0.0f;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
};

/* Function with complex reduction pattern and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double complex_reduction(const double* data, int size) {
    double acc = 0.0;
    volatile int cond = g_volatile_counter % 7;
    
    /* Outer loop with data-dependent condition */
    for (int i = 1; i < size; i++) {
        /* Inner loop with volatile condition */
        for (int j = 0; j < 3; j++) {
            if (cond > (j * 2)) {
                /* Carried dependency across iterations */
                acc = acc + data[i] * data[i-1] * (j + 1);
                
                /* Additional floating point operations */
                double temp = sin(data[i] * 0.01) * cos(data[i-1] * 0.01);
                acc += temp * 0.1;
            }
        }
        
        /* Middle loop with mixed operations */
        for (int k = 0; k < 2; k++) {
            /* Complex conditional with volatile */
            if ((g_volatile_counter + i + k) % 5 == 0) {
                acc += sqrt(fabs(data[i])) * k;
            } else {
                acc -= log(fabs(data[i]) + 1.0) * 0.5;
            }
        }
        
        /* Update volatile condition */
        cond = (cond + i) % 11;
    }
    
    return acc;
}

/* Function processing mixed data types with non-contiguous access */
__attribute__((optimize("O3", "funroll-loops")))
float process_mixed_data(struct MixedData* data, int count) {
    float total = 0.0f;
    volatile int skip = g_volatile_counter % 4;
    
    /* Process every 3rd element with stride */
    for (int i = 0; i < count; i += 3) {
        /* Deeply nested loops */
        for (int layer1 = 0; layer1 < 2; layer1++) {
            for (int layer2 = 0; layer2 < 2; layer2++) {
                for (int layer3 = 0; layer3 < 2; layer3++) {
                    /* Data-dependent condition with volatile */
                    if ((g_volatile_counter + i + layer1 + layer2 + layer3) % 3 == skip) {
                        /* Mixed type operations */
                        total += data[i].value * data[i].weight;
                        total -= data[i].id * 0.01f;
                        
                        /* Conditional store */
                        if (total > 100.0f) {
                            data[i].counter++;
                            total *= 0.99f;
                        }
                    } else {
                        total += sin(data[i].value) * cos(data[i].weight);
                    }
                }
            }
        }
        
        /* Additional non-contiguous access pattern */
        int idx = (i * 7) % count;
        if (idx < count) {
            total += data[idx].value * 0.3f;
            total -= data[idx].weight * 0.2f;
        }
    }
    
    return total;
}

/* Function with deeply nested loops and complex control flow */
__attribute__((hot, optimize("O2")))
int nested_loop_computation(int* array, int size) {
    int result = 0;
    volatile int v1 = g_volatile_counter;
    volatile int v2 = g_volatile_float * 10;
    
    /* Triple nested loop with data-dependent conditions */
    for (int i = 0; i < size; i++) {
        /* First level condition */
        if (v1 > (i % 10)) {
            for (int j = 0; j < 8; j++) {
                /* Second level condition with function call */
                if ((rand() % 100) > 50) {
                    for (int k = 0; k < 4; k++) {
                        /* Complex arithmetic with mixed operations */
                        int temp = array[i] * j * k;
                        
                        /* Floating point in integer loop */
                        float ftemp = temp * 0.5f;
                        
                        /* Conditional with volatile */
                        if (v2 > (temp % 7)) {
                            result += temp + (int)ftemp;
                            
                            /* Memory access pattern */
                            int idx = (i + j + k) % size;
                            result -= array[idx];
                        } else {
                            result += temp / (k + 1);
                        }
                        
                        /* Additional operation to create scheduling complexity */
                        result ^= (array[i] << (j % 3));
                    }
                }
            }
        }
        
        /* Update volatile variables */
        v1 = (v1 + i) % 23;
        v2 = (v2 + array[i]) % 17;
    }
    
    return result;
}

/* Main driver with initialization and multiple computation patterns */
int main() {
    /* Initialize data arrays */
    double* double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct MixedData* mixed_data = (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    /* Simple LCG for reproducible pseudo-random data */
    unsigned int seed = 42;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        double_data[i] = (seed % 1000) / 100.0;
        
        seed = seed * 1103515245 + 12345;
        mixed_data[i].id = seed % 100;
        
        seed = seed * 1103515245 + 12345;
        mixed_data[i].value = (seed % 1000) / 100.0f;
        
        seed = seed * 1103515245 + 12345;
        mixed_data[i].weight = (seed % 1000) / 50.0;
        
        seed = seed * 1103515245 + 12345;
        mixed_data[i].tag = 'A' + (seed % 26);
        mixed_data[i].counter = 0;
        
        seed = seed * 1103515245 + 12345;
        int_array[i] = seed % 1000;
    }
    
    /* Update volatile globals */
    g_volatile_counter = seed % 100;
    g_volatile_float = (seed % 100) / 10.0f;
    
    /* Perform multiple computations to increase scheduling opportunities */
    double total_result = 0.0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call different computation patterns */
        double red_result = complex_reduction(double_data, ARRAY_SIZE);
        float mixed_result = process_mixed_data(mixed_data, ARRAY_SIZE);
        int nested_result = nested_loop_computation(int_array, ARRAY_SIZE);
        
        /* Combine results to prevent optimization */
        total_result += red_result + mixed_result + nested_result;
        
        /* Modify volatile to change control flow */
        g_volatile_counter = (g_volatile_counter * 13 + 17) % 101;
        g_volatile_float = sin(g_volatile_counter * 0.1f);
        
        /* Early exit condition to prevent infinite runtime */
        if (iter % 1000 == 0 && total_result > 1e9) {
            break;
        }
    }
    
    /* Print result to ensure code is live */
    printf("Final result: %f\n", total_result);
    
    /* Cleanup */
    free(double_data);
    free(mixed_data);
    free(int_array);
    
    return 0;
}
