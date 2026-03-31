/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20
#define OUTER_LOOP 10

/* Volatile variables to prevent optimization */
volatile int v_cond1 = 0;
volatile int v_cond2 = 1;
volatile float v_float = 3.14f;
volatile double v_double = 2.71828;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
};

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
double complex_reduction(struct MixedData* data, int n) {
    double acc = 0.0;
    double prev = data[0].weight;
    
    /* Outer loop */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Middle loop with volatile condition */
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            /* Data-dependent condition using volatile */
            if (v_cond1 || (j % 3 == 0)) {
                /* Inner loop with carried dependency */
                for (int k = 0; k < INNER_LOOP; k++) {
                    int idx = (i * MIDDLE_LOOP * INNER_LOOP + j * INNER_LOOP + k) % n;
                    
                    /* Complex reduction with carried dependency */
                    double current = data[idx].weight;
                    acc += current * prev;
                    prev = current;
                    
                    /* Mixed operations */
                    data[idx].value = (float)(acc * 0.01);
                    data[idx].counter += (int)(data[idx].value * 100);
                    
                    /* Conditional store based on volatile */
                    if (v_cond2 && (k & 1)) {
                        data[idx].tag = 'A' + (k % 26);
                    }
                }
            } else {
                /* Alternative path with different access pattern */
                for (int k = 0; k < INNER_LOOP / 2; k++) {
                    int idx = (j * INNER_LOOP + k * 3) % n;  /* Non-contiguous access */
                    acc += data[idx].weight * v_double;
                    data[idx].value *= v_float;
                }
            }
        }
        
        /* Additional computation between outer loop iterations */
        acc *= 0.99;
        if (v_cond1) {
            acc += 1.0;
        }
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed data types */
__attribute__((optimize("O3", "funroll-loops", "fsel-sched-pipelining")))
float nested_mixed_processing(struct MixedData* data, int n) {
    float total = 0.0f;
    
    /* Triple nested loop */
    for (int a = 0; a < 5; a++) {
        volatile int local_volatile = a * 2;  /* Prevent optimization */
        
        for (int b = 0; b < 8; b++) {
            /* Data-dependent condition with volatile */
            if (local_volatile > b || v_cond2) {
                for (int c = 0; c < 12; c++) {
                    /* Complex index calculation */
                    int idx = (a * 100 + b * 10 + c) % n;
                    
                    /* Mixed type operations */
                    double temp = data[idx].weight;
                    float val = data[idx].value;
                    
                    /* Conditional operations */
                    if (idx % 7 == 0) {
                        total += val * (float)temp;
                        data[idx].id = (int)(total * 1000);
                    } else if (idx % 3 == 0) {
                        total -= val / (float)(temp + 1.0);
                        data[idx].counter++;
                    } else {
                        total *= 0.95f;
                    }
                    
                    /* Memory access with stride */
                    if (c % 4 == 0) {
                        int stride_idx = (idx + 5) % n;
                        data[stride_idx].weight = temp * 0.9;
                    }
                }
            }
        }
        
        /* Loop with reduction and volatile dependency */
        float local_sum = 0.0f;
        for (int i = 0; i < n; i += 3) {  /* Non-unit stride */
            local_sum += data[i].value;
            if (v_cond1 && (i % 11 == 0)) {
                data[i].value = local_sum * 0.1f;
            }
        }
        total += local_sum;
    }
    
    return total;
}

/* Function with pointer chasing and complex control flow */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
int pointer_chasing_reduction(struct MixedData* data, int n) {
    int sum = 0;
    struct MixedData* current = &data[0];
    
    /* Complex loop with pointer chasing */
    for (int i = 0; i < n * 2; i++) {
        /* Volatile condition affecting control flow */
        if (v_cond1 || (i % 13 == 0)) {
            /* Multiple operations creating dependency chain */
            int base = current->id;
            float fval = current->value;
            double dval = current->weight;
            
            /* Complex expression with mixed types */
            sum += base + (int)(fval * 100) + (int)(dval * 10);
            
            /* Update with carried dependency */
            current->counter = sum % 1000;
            
            /* Pointer chase with stride */
            int next_idx = (base + i) % n;
            current = &data[next_idx];
            
            /* Conditional store */
            if (v_cond2 && (sum & 1)) {
                current->tag = 'X';
            }
        } else {
            /* Alternative path */
            sum -= current->counter;
            current = &data[(i * 7) % n];
        }
        
        /* Inner small loop */
        int temp = 0;
        for (int j = 0; j < 4; j++) {
            temp += (current->id >> j) & 1;
        }
        sum += temp;
        
        /* Volatile read affecting next iteration */
        if (v_float > 2.0f) {
            current->value += 0.5f;
        }
    }
    
    return sum;
}

/* Initialize data with pseudo-random values */
void initialize_data(struct MixedData* data, int n) {
    unsigned int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = seed * 1103515245 + 12345;
        data[i].id = (seed >> 16) & 0x7FFF;
        
        seed = seed * 1103515245 + 12345;
        data[i].value = (float)((seed & 0xFFFF) / 65536.0 * 100.0);
        
        seed = seed * 1103515245 + 12345;
        data[i].weight = (double)((seed & 0xFFFF) / 65536.0 * 50.0);
        
        data[i].tag = 'A' + (i % 26);
        data[i].counter = 0;
    }
}

int main() {
    /* Allocate and initialize data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_data(data, SIZE);
    
    /* Update volatile conditions */
    v_cond1 = 1;
    v_cond2 = 0;
    v_float = 2.5f;
    v_double = 3.0;
    
    /* Call all computation functions to trigger various scheduling scenarios */
    double result1 = complex_reduction(data, SIZE);
    printf("Result 1: %f\n", result1);
    
    v_cond1 = 0;
    v_cond2 = 1;
    
    float result2 = nested_mixed_processing(data, SIZE);
    printf("Result 2: %f\n", result2);
    
    v_cond1 = 1;
    v_cond2 = 1;
    
    int result3 = pointer_chasing_reduction(data, SIZE);
    printf("Result 3: %d\n", result3);
    
    /* Final combination to ensure all results are used */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    /* Clean up */
    free(data);
    
    return 0;
}
