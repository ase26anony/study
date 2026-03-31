/* test_sel_sched.c - Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int index;
    float weight;
    double value;
    char tag;
    volatile int flag; /* volatile to prevent optimization */
};

/* Function with complex reduction pattern and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
double complex_reduction(const double* data, int size) {
    volatile int seed = 12345; /* volatile to create data-dependent control flow */
    double acc = 0.0;
    double prev = data[0];
    
    /* Loop with carried dependency and volatile condition */
    for (int i = 1; i < size; i++) {
        /* Data-dependent branch with volatile variable */
        if ((seed & 0xFF) > 128) {
            acc += data[i] * prev;
        } else {
            acc += data[i] / (prev + 1.0);
        }
        prev = data[i];
        
        /* Modify volatile seed to create unpredictable control flow */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed data types */
__attribute__((optimize("O3", "funroll-loops")))
void process_mixed_data(struct MixedData* array, int size, int stride) {
    volatile int counter = 0;
    double total = 0.0;
    
    /* Triple nested loop structure */
    for (int outer = 0; outer < 3; outer++) {
        for (int middle = 0; middle < size / 100; middle++) {
            for (int inner = 0; inner < 100; inner++) {
                int idx = (middle * 100 + inner) * stride;
                if (idx >= size) continue;
                
                /* Data-dependent operations with volatile */
                if ((counter++ & 1) == 0) {
                    array[idx].value = array[idx].weight * array[idx].index;
                    total += array[idx].value;
                } else {
                    array[idx].value = array[idx].weight / (array[idx].index + 1);
                    total -= array[idx].value;
                }
                
                /* Complex conditional with floating point comparison */
                if (array[idx].value > 1000.0) {
                    array[idx].flag = 1;
                } else if (array[idx].value < -1000.0) {
                    array[idx].flag = -1;
                } else {
                    array[idx].flag = 0;
                }
            }
            
            /* Volatile memory barrier */
            asm volatile("" : : : "memory");
        }
    }
    
    /* Prevent dead code elimination */
    if (total > 1e10) {
        printf("Unexpected large total: %f\n", total);
    }
}

/* Function with non-contiguous access and reduction */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double stride_reduction(const double* data, int size, int stride) {
    double sum_even = 0.0, sum_odd = 0.0;
    volatile int toggle = 0;
    
    /* Process every stride-th element */
    for (int i = 0; i < size; i += stride) {
        /* Complex data-dependent branching */
        if (toggle ^= 1) {
            sum_even += data[i] * data[i];
            
            /* Nested conditional with volatile */
            if ((i & 0x3F) == 0 && toggle) {
                sum_even -= data[i] * 0.5;
            }
        } else {
            sum_odd += data[i] / (i + 1.0);
            
            /* Another level of nesting */
            if ((i % 13) == 0) {
                sum_odd *= 0.99;
            }
        }
        
        /* Cross-iteration dependency */
        if (i > 0) {
            sum_even += sum_odd * 0.01;
            sum_odd += sum_even * 0.01;
        }
    }
    
    return sum_even + sum_odd;
}

/* Main driver with initialization and multiple computation patterns */
int main() {
    const int DATA_SIZE = 10000;
    const int STRIDE = 3;
    
    /* Allocate and initialize data */
    double* data = (double*)malloc(DATA_SIZE * sizeof(double));
    struct MixedData* mixed = (struct MixedData*)malloc(DATA_SIZE * sizeof(struct MixedData));
    
    if (!data || !mixed) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    uint32_t lcg = 123456789;
    for (int i = 0; i < DATA_SIZE; i++) {
        /* Simple LCG for reproducibility */
        lcg = lcg * 1103515245 + 12345;
        data[i] = (double)(lcg % 10000) / 100.0 - 50.0;
        
        mixed[i].index = i;
        mixed[i].weight = (float)((lcg % 1000) / 10.0);
        mixed[i].value = data[i];
        mixed[i].tag = (char)('A' + (lcg % 26));
        mixed[i].flag = 0;
    }
    
    /* Perform multiple computations to increase scheduling opportunities */
    double result1 = complex_reduction(data, DATA_SIZE);
    process_mixed_data(mixed, DATA_SIZE, STRIDE);
    double result2 = stride_reduction(data, DATA_SIZE, STRIDE);
    
    /* Combine results to prevent optimization */
    double final_result = result1 + result2;
    for (int i = 0; i < DATA_SIZE; i += 100) {
        final_result += mixed[i].value;
    }
    
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(data);
    free(mixed);
    
    return 0;
}
