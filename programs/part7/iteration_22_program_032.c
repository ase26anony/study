/* Complex loop patterns to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20
#define OUTER_LOOP 10

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
    volatile int v = g_volatile_counter;
    
    /* Outer loop with volatile condition */
    for (int i = 0; i < OUTER_LOOP; i++) {
        if (v % 3 == 0) {
            /* Middle loop with data-dependent condition */
            for (int j = 1; j < MIDDLE_LOOP; j++) {
                volatile int inner_cond = g_volatile_counter + j;
                
                /* Innermost loop with carried dependency */
                for (int k = 2; k < INNER_LOOP; k++) {
                    /* Complex reduction with carried dependency across iterations */
                    double prev = (k > 2) ? arr[(k-1) % n].weight : 1.0;
                    double curr = arr[k % n].weight;
                    
                    /* Mixed operations creating diverse RTL patterns */
                    acc += curr * prev * (inner_cond % 5);
                    
                    /* Conditional store based on volatile */
                    if (inner_cond % 7 == 0) {
                        arr[k % n].value = (float)(acc * 0.01);
                    }
                    
                    /* Integer operations */
                    arr[k % n].data[k % 3] += (int)(acc * 100) % 256;
                    
                    /* Floating point operations */
                    double temp = arr[(k+1) % n].weight * 0.5;
                    acc += temp * (v % 11);
                }
                
                /* Non-contiguous memory access */
                if (j % 3 == 0) {
                    double sum = 0.0;
                    for (int stride = j; stride < INNER_LOOP; stride += 4) {
                        sum += arr[stride % n].weight * arr[(stride-2) % n].value;
                    }
                    acc += sum * (j % 13);
                }
            }
        }
        v = (v * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return acc;
}

/* Function with deeply nested loops and volatile conditionals */
__attribute__((optimize("O3", "funroll-loops")))
float nested_conditional_processing(struct MixedData* arr, int n) {
    float result = 0.0f;
    volatile int seed = g_volatile_counter;
    
    /* Triple nested loop with volatile conditions at each level */
    for (int a = 0; a < 8; a++) {
        volatile int cond_a = seed + a * 17;
        
        for (int b = 1; b < 12; b++) {
            volatile int cond_b = cond_a ^ (b * 23);
            
            for (int c = 2; c < 15; c++) {
                volatile int cond_c = cond_b | (c * 37);
                
                /* Data-dependent branching */
                if (cond_c % 3 == 0) {
                    /* Mixed type operations */
                    int idx = (a * b * c) % n;
                    result += arr[idx].value * (cond_c % 19);
                    
                    /* Conditional memory access with stride */
                    if (cond_c % 5 == 0) {
                        for (int offset = 0; offset < 5; offset++) {
                            int stride_idx = (idx + offset * 7) % n;
                            result -= arr[stride_idx].weight * 0.3f;
                        }
                    }
                } else if (cond_c % 7 == 0) {
                    /* Alternative computation path */
                    double temp = 0.0;
                    for (int d = 0; d < 5; d++) {
                        temp += arr[(c + d * 11) % n].weight;
                    }
                    result += (float)(temp * 0.1);
                }
                
                /* More arithmetic to create scheduling pressure */
                arr[c % n].data[a % 3] ^= (cond_c >> 3) & 0xFF;
                result *= 0.999f; /* Prevent overflow */
            }
            
            /* Middle loop reduction */
            double mid_acc = 0.0;
            for (int k = b; k < b + 10; k++) {
                mid_acc += arr[k % n].weight * (k % 17);
            }
            result += (float)(mid_acc * 0.01);
        }
        
        /* Outer loop update with volatile */
        seed = (seed * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    return result;
}

/* Function with pointer chasing and complex data flow */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
int pointer_chasing_reduction(struct MixedData* arr, int n) {
    int total = 0;
    volatile int selector = g_volatile_counter;
    
    /* Create artificial pointer chasing pattern */
    struct MixedData* current = &arr[0];
    
    for (int i = 0; i < SIZE * 2; i++) {
        /* Volatile condition affecting control flow */
        if ((selector + i) % 11 == 0) {
            /* Complex addressing calculation */
            int next_idx = (current->id + i * 3) % n;
            current = &arr[next_idx];
            
            /* Reduction with mixed operations */
            total += current->data[0] * (i % 31);
            total -= (int)(current->value * 100);
            
            /* Floating point in integer loop */
            double weight_acc = 0.0;
            for (int j = 0; j < 3; j++) {
                weight_acc += current->weight * (j + 1);
                current->data[j] = (int)(weight_acc) % 1000;
            }
            
            total += (int)weight_acc;
        } else if ((selector + i) % 13 == 0) {
            /* Alternative path with different access pattern */
            for (int stride = 1; stride < 8; stride += 2) {
                int idx = (i * stride) % n;
                total ^= arr[idx].data[stride % 3];
                
                /* More mixed operations */
                float temp = arr[idx].value * stride;
                total += (int)(temp * 10);
            }
        }
        
        /* Update volatile for next iteration */
        selector = (selector * 134775813 + 1) & 0x7fffffff;
    }
    
    return total;
}

/* Initialize array with pseudo-random data */
void init_data(struct MixedData* arr, int n) {
    unsigned int seed = 42;
    for (int i = 0; i < n; i++) {
        arr[i].id = i;
        seed = seed * 1103515245 + 12345;
        arr[i].value = (float)((seed >> 16) & 0x7FFF) / 1000.0f;
        seed = seed * 1103515245 + 12345;
        arr[i].weight = (double)(seed & 0xFFFFF) / 10000.0;
        arr[i].tag = 'A' + (i % 26);
        for (int j = 0; j < 3; j++) {
            seed = seed * 1103515245 + 12345;
            arr[i].data[j] = seed % 1000;
        }
    }
}

int main() {
    /* Allocate and initialize data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    if (!data) return 1;
    
    init_data(data, SIZE);
    
    /* Update volatile to affect control flow */
    g_volatile_counter = 123;
    g_volatile_float = 2.718f;
    
    /* Call all computation functions to maximize scheduler activity */
    double result1 = complex_reduction(data, SIZE);
    printf("Result 1: %f\n", result1);
    
    float result2 = nested_conditional_processing(data, SIZE);
    printf("Result 2: %f\n", result2);
    
    int result3 = pointer_chasing_reduction(data, SIZE);
    printf("Result 3: %d\n", result3);
    
    /* Combine results to ensure all computations are used */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    free(data);
    return 0;
}
