/* Complex test program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000
#define INNER_LOOP 50
#define MIDDLE_LOOP 30
#define OUTER_LOOP 20

/* Volatile variables to prevent optimization of control flow */
volatile int vol_cond1 = 0;
volatile int vol_cond2 = 1;
volatile float vol_float = 3.14f;

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
};

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double complex_reduction(struct MixedData* data, int n) {
    double acc = 0.0;
    volatile int v = vol_cond1;
    
    /* Outer loop */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Middle loop with volatile condition */
        for (int j = 1; j < MIDDLE_LOOP; j++) {
            /* Innermost loop with data-dependent control flow */
            for (int k = 2; k < INNER_LOOP; k++) {
                /* Volatile condition prevents optimization */
                if (v || (rand() % 100) > 50) {
                    /* Carried dependency across iterations */
                    int idx = (i * MIDDLE_LOOP * INNER_LOOP + j * INNER_LOOP + k) % n;
                    int prev_idx = (idx - 1 + n) % n;
                    
                    /* Mixed type operations */
                    double temp = data[idx].weight * data[prev_idx].value;
                    
                    /* Conditional operation based on volatile */
                    if (vol_cond2) {
                        temp += data[idx].id * 0.5;
                    }
                    
                    /* Reduction with carried dependency */
                    acc = acc + temp;
                    
                    /* Non-trivial floating point operation */
                    data[idx].value = (float)(acc * 0.01);
                }
                
                /* Additional volatile-dependent operation */
                if (vol_float > 2.0f) {
                    data[(i + j + k) % n].counter++;
                }
            }
            
            /* Middle loop operation with stride */
            if (j % 3 == 0) {
                double sum = 0.0;
                for (int s = j; s < INNER_LOOP; s += 4) {
                    int idx = (i * INNER_LOOP + s) % n;
                    sum += data[idx].weight * data[idx].value;
                }
                acc += sum * 0.1;
            }
        }
        
        /* Outer loop update with volatile */
        v = !v;
    }
    
    return acc;
}

/* Function with non-contiguous memory access pattern */
__attribute__((optimize("O3", "funroll-loops")))
float strided_processing(struct MixedData* data, int n) {
    float result = 0.0f;
    volatile float vf = vol_float;
    
    /* Process every 3rd element with stride */
    for (int i = 0; i < n; i += 3) {
        /* Complex condition with volatile */
        if ((i % 7 == 0) || (vf > 2.0f)) {
            /* Mixed type computation */
            double dval = data[i].weight;
            float fval = data[i].value;
            
            /* Non-linear computation */
            result += (float)(dval * fval * (data[i].id % 10));
            
            /* Conditional store */
            if (vol_cond1 || (rand() % 100) < 30) {
                data[i].value = result * 0.5f;
            }
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 5; j++) {
            if (vol_cond2 && (j % 2 == 0)) {
                int idx = (i + j) % n;
                result -= data[idx].value * 0.1f;
            }
        }
    }
    
    return result;
}

/* Function with deeply nested loops and volatile conditions */
__attribute__((hot, optimize("O2")))
int nested_conditional(struct MixedData* data, int n) {
    int total = 0;
    volatile int local_vol = vol_cond1;
    
    /* Level 1 loop */
    for (int a = 0; a < 15; a++) {
        /* Level 2 loop */
        for (int b = a; b < 25; b++) {
            /* Level 3 loop - innermost */
            for (int c = 0; c < 20; c++) {
                /* Data-dependent condition with volatile */
                int condition = (local_vol || (a + b + c) % 13 == 0);
                
                if (condition) {
                    /* Memory access with non-linear index */
                    int idx = (a * 100 + b * 10 + c) % n;
                    
                    /* Mixed operations */
                    total += data[idx].id;
                    total -= (int)(data[idx].value * 10);
                    
                    /* Floating point operation */
                    data[idx].weight = total * 0.01;
                }
                
                /* Additional volatile check */
                if (vol_cond2 && (c % 4 == 0)) {
                    total += (int)vol_float;
                }
            }
            
            /* Update volatile in middle loop */
            local_vol = !local_vol;
        }
        
        /* Reduction across outer loop */
        if (a % 3 == 0) {
            double sum = 0.0;
            for (int k = 0; k < 10; k++) {
                int idx = (a * 10 + k) % n;
                sum += data[idx].weight;
            }
            total += (int)(sum * 100);
        }
    }
    
    return total;
}

/* Initialize data with pseudo-random values */
void init_data(struct MixedData* data, int n) {
    unsigned int seed = 42;
    for (int i = 0; i < n; i++) {
        data[i].id = i;
        data[i].value = (float)((seed = seed * 1103515245 + 12345) % 1000) / 100.0f;
        data[i].weight = (double)((seed = seed * 1103515245 + 12345) % 2000) / 50.0;
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
    
    init_data(data, SIZE);
    
    printf("Starting complex computations...\n");
    
    /* Call all computation functions to increase scheduler activity */
    double result1 = complex_reduction(data, SIZE);
    printf("Result 1: %f\n", result1);
    
    float result2 = strided_processing(data, SIZE);
    printf("Result 2: %f\n", result2);
    
    int result3 = nested_conditional(data, SIZE);
    printf("Result 3: %d\n", result3);
    
    /* Final combination to ensure all results are used */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    /* Verify some data was modified */
    int modified_count = 0;
    for (int i = 0; i < SIZE; i++) {
        if (data[i].counter > 0 || data[i].value > 100.0f) {
            modified_count++;
        }
    }
    printf("Modified elements: %d\n", modified_count);
    
    free(data);
    return 0;
}
