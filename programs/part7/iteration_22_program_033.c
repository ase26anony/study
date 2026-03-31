/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 5000
#define INNER_ITERS 100
#define MIDDLE_ITERS 50
#define OUTER_ITERS 20

/* Mixed data type structure with non-contiguous access pattern */
struct mixed_data {
    int index;
    float value;
    double weight;
    char tag;
    volatile int volatile_flag; /* Prevent optimization */
};

/* Global arrays to ensure memory operations */
struct mixed_data data_array[SIZE];
int int_array[SIZE];
double double_array[SIZE];

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double complex_reduction(int seed) {
    volatile int v_seed = seed; /* Volatile to prevent constant propagation */
    double acc = 0.0;
    double temp_acc = 0.0;
    int i, j, k;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        double_array[i] = (double)int_array[i] / 1000.0;
    }
    
    /* Triple nested loop with data-dependent control flow */
    for (i = 0; i < OUTER_ITERS; i++) {
        /* Outer loop with volatile condition */
        if (v_seed % (i + 2) == 0) {
            for (j = 0; j < MIDDLE_ITERS; j++) {
                /* Middle loop with mixed operations */
                double middle_acc = 0.0;
                for (k = 0; k < INNER_ITERS; k++) {
                    /* Innermost loop with carried dependency */
                    int idx = (i * 100 + j * 2 + k * 3) % SIZE;
                    int prev_idx = (idx + SIZE - 1) % SIZE;
                    
                    /* Critical carried dependency across iterations */
                    temp_acc = temp_acc + double_array[idx] * double_array[prev_idx];
                    
                    /* Conditional operation based on volatile */
                    if (v_seed % (k + 3) == 0) {
                        middle_acc += temp_acc * 0.5;
                    } else {
                        middle_acc -= temp_acc * 0.3;
                    }
                    
                    /* Mixed integer operations */
                    int_array[idx] = int_array[idx] ^ (int)(temp_acc * 1000);
                }
                
                /* Reduction with floating point */
                acc += middle_acc;
                
                /* Non-contiguous memory access */
                for (k = j; k < INNER_ITERS; k += 3) {
                    int idx = (i * 50 + j * 7 + k * 11) % SIZE;
                    double_array[idx] = double_array[idx] * 1.01 + acc * 0.001;
                }
            }
        } else {
            /* Alternative path with different access pattern */
            for (j = 1; j < MIDDLE_ITERS; j += 2) {
                for (k = 0; k < INNER_ITERS; k++) {
                    int idx = (i * 77 + j * 13 + k * 17) % SIZE;
                    acc += double_array[idx] * (k % 5);
                }
            }
        }
        
        /* Volatile update to prevent loop elimination */
        v_seed = (v_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return acc + temp_acc;
}

/* Function processing mixed data types with non-contiguous access */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
float process_mixed_types(int start) {
    volatile int v_start = start;
    float total = 0.0f;
    int i, j;
    
    /* Initialize mixed data array */
    for (i = 0; i < SIZE; i++) {
        data_array[i].index = i;
        data_array[i].value = (float)(i % 100) * 0.1f;
        data_array[i].weight = (double)(i % 50) * 0.02;
        data_array[i].tag = (char)('A' + (i % 26));
        data_array[i].volatile_flag = v_start + i;
    }
    
    /* Complex loop with stride access */
    for (i = 0; i < OUTER_ITERS * 2; i++) {
        float loop_acc = 0.0f;
        
        for (j = i; j < SIZE; j += 7) { /* Non-unit stride */
            /* Mixed type operations */
            double temp = data_array[j].weight * 2.0;
            
            /* Conditional based on volatile */
            if (data_array[j].volatile_flag % (j + 5) == 0) {
                loop_acc += (float)temp * data_array[j].value;
                
                /* Type conversion and store */
                data_array[j].value = (float)(temp * 0.5);
            } else {
                loop_acc -= (float)temp * data_array[j].value * 0.7f;
                
                /* Integer operation */
                data_array[j].index = data_array[j].index ^ (int)(temp * 100);
            }
            
            /* Additional floating point operation */
            data_array[j].weight = temp * 0.99;
        }
        
        total += loop_acc;
        
        /* Nested loop with different stride */
        for (j = 0; j < 30; j++) {
            int idx = (i * 19 + j * 23) % SIZE;
            if (idx % 3 == 0) {
                total += (float)data_array[idx].weight * 0.1f;
            }
        }
        
        /* Update volatile to prevent optimization */
        v_start = (v_start * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    return total;
}

/* Deeply nested loops with volatile conditionals */
__attribute__((hot, optimize("O2")))
int nested_conditional_computation(int base) {
    volatile int v_base = base;
    int result = 0;
    int a, b, c, d;
    
    /* Quadruple nested loop */
    for (a = 0; a < 15; a++) {
        if (v_base % (a + 4) == 0) {
            for (b = 1; b < 12; b++) {
                int inner_acc = 0;
                
                for (c = 0; c < 25; c++) {
                    /* Volatile in inner loop condition */
                    volatile int v_temp = v_base + c;
                    
                    for (d = 0; d < 40; d++) {
                        /* Data-dependent computation */
                        int idx = (a * 1000 + b * 100 + c * 10 + d) % SIZE;
                        
                        if (v_temp % (d + 6) == 0) {
                            inner_acc += int_array[idx] * (c + 1);
                            
                            /* Memory store with dependency */
                            int_array[idx] = inner_acc & 0xff;
                        } else {
                            inner_acc -= int_array[idx] / (b + 1);
                            
                            /* Floating point in integer loop */
                            double_array[idx] = double_array[idx] * 0.999;
                        }
                        
                        /* Additional conditional */
                        if ((inner_acc & 1) == 0) {
                            result ^= int_array[idx];
                        } else {
                            result |= int_array[idx];
                        }
                    }
                    
                    /* Update volatile */
                    v_temp = v_temp * 1103515245 + 12345;
                }
                
                result += inner_acc;
            }
        } else {
            /* Alternative path */
            for (b = 0; b < 8; b++) {
                for (c = 5; c < 20; c++) {
                    int idx = (a * 100 + b * 10 + c) % SIZE;
                    result = result * 31 + int_array[idx];
                }
            }
        }
        
        /* Prevent loop invariant code motion */
        v_base = (v_base * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return result;
}

/* Main driver with multiple computation patterns */
int main() {
    double total_result = 0.0;
    int i;
    
    printf("Starting complex selective scheduling test...\n");
    
    /* Multiple calls to increase scheduling opportunities */
    for (i = 0; i < 5; i++) {
        double reduction_result = complex_reduction(i * 1000);
        float mixed_result = process_mixed_types(i * 500);
        int nested_result = nested_conditional_computation(i * 200);
        
        /* Combine results to ensure all computations are used */
        total_result += reduction_result + mixed_result + nested_result;
        
        printf("Iteration %d: reduction=%.4f, mixed=%.4f, nested=%d\n",
               i, reduction_result, mixed_result, nested_result);
    }
    
    printf("Total combined result: %.6f\n", total_result);
    
    /* Final validation computation */
    double final_check = 0.0;
    for (i = 0; i < SIZE; i += 3) { /* Non-contiguous stride */
        final_check += double_array[i] * 0.001;
        final_check -= int_array[i % 1000] * 0.00001;
    }
    
    printf("Final check value: %.8f\n", final_check);
    
    return (total_result > 0 && final_check != 0.0) ? 0 : 1;
}
