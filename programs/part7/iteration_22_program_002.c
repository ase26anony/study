/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20
#define OUTER_LOOP 10

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int index;
    float value;
    double weight;
    char tag;
    long counter;
};

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_seed = 42;
volatile int g_volatile_mod = 7;

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double complex_reduction(struct MixedData* data, int count) {
    double acc = 0.0;
    volatile int v_cond = g_volatile_seed;
    
    /* Outer loop - creates scheduling region */
    for (int i = 1; i < OUTER_LOOP; i++) {
        float temp_float = 0.0f;
        int temp_int = 0;
        
        /* Middle loop with volatile condition */
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            if ((v_cond + j) % g_volatile_mod) {
                /* Inner loop with carried dependency */
                for (int k = 1; k < INNER_LOOP; k++) {
                    /* Critical carried dependency across iterations */
                    int idx = (i * j * k) % count;
                    int prev_idx = (i * j * (k-1)) % count;
                    
                    /* Mixed operations creating diverse RTL */
                    double product = data[idx].weight * data[prev_idx].value;
                    acc = acc + product;
                    
                    /* Conditional store based on volatile */
                    if ((v_cond + idx) & 1) {
                        data[idx].value = (float)(acc * 0.01);
                    }
                    
                    /* Integer reduction with branching */
                    temp_int += data[idx].index;
                    if (temp_int > 1000000) {
                        temp_int = temp_int / 2;
                    }
                }
            } else {
                /* Alternative path with different access pattern */
                for (int k = 0; k < INNER_LOOP/2; k += 3) {
                    /* Non-contiguous access (every 3rd element) */
                    int idx = (i * j * k * 3) % count;
                    if (idx > 0) {
                        acc += data[idx].weight - data[idx-1].weight;
                        temp_float += data[idx].value;
                    }
                }
            }
            
            /* Update volatile to change control flow */
            v_cond = (v_cond * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed operations */
__attribute__((optimize("O3", "funroll-loops")))
float nested_mixed_operations(struct MixedData* data, int count) {
    float result = 0.0f;
    volatile int v_seed = g_volatile_seed;
    
    /* Triple nested loop with data-dependent conditions */
    for (int a = 0; a < 5; a++) {
        double local_acc = 0.0;
        
        for (int b = 0; b < 8; b++) {
            int mod_cond = (v_seed + a + b) % 5;
            
            for (int c = 0; c < 12; c++) {
                /* Complex addressing calculation */
                int idx = (a * 100 + b * 10 + c) % count;
                
                /* Mixed type operations */
                if (mod_cond > 2) {
                    result += data[idx].value * 1.5f;
                    local_acc += data[idx].weight;
                    
                    /* Conditional with side effect */
                    if (data[idx].index & 1) {
                        data[idx].tag = 'A';
                        result -= 0.5f;
                    } else {
                        data[idx].tag = 'B';
                        result += 0.5f;
                    }
                } else {
                    /* Different operation mix */
                    result -= data[idx].value * 0.75f;
                    local_acc -= data[idx].weight * 0.5;
                    
                    /* Memory access pattern with stride */
                    if (idx + 3 < count) {
                        result += data[idx + 3].value;
                    }
                }
                
                /* Update counter with overflow check */
                data[idx].counter++;
                if (data[idx].counter > 1000) {
                    data[idx].counter = data[idx].counter % 100;
                }
            }
            
            /* Volatile update in middle loop */
            v_seed = (v_seed * 1664525 + 1013904223) & 0x7fffffff;
        }
        
        /* Reduction across outer loop iterations */
        result += (float)(local_acc * 0.01);
    }
    
    return result;
}

/* Function with pointer chasing and indirect memory access */
__attribute__((optimize("O2")))
long pointer_chasing_reduction(struct MixedData* data, int count) {
    long total = 0;
    struct MixedData* current = &data[0];
    volatile int v_dir = 1;
    
    for (int i = 0; i < SIZE * 2; i++) {
        /* Pointer chasing with volatile direction */
        int offset = (current->index + v_dir) % count;
        if (offset < 0) offset = -offset;
        
        struct MixedData* next = &data[offset];
        
        /* Complex calculation with mixed types */
        double interm = current->weight * next->value;
        total += (long)(interm * 1000.0);
        
        /* Conditional update based on multiple factors */
        if ((total & 255) > (current->index & 255)) {
            next->value = (float)(interm * 0.1);
            v_dir = -v_dir;
        }
        
        /* Update for next iteration */
        current = next;
        
        /* Periodic reset to avoid infinite patterns */
        if (i % 1000 == 999) {
            current = &data[i % count];
            v_dir = (v_dir * 3) % 7;
        }
    }
    
    return total;
}

/* Initialize data with pseudo-random values */
void initialize_data(struct MixedData* data, int count) {
    unsigned int seed = 123456789;
    
    for (int i = 0; i < count; i++) {
        seed = seed * 1103515245 + 12345;
        data[i].index = (seed >> 16) & 0x7FFF;
        
        seed = seed * 1103515245 + 12345;
        data[i].value = (float)((seed & 0xFFFF) / 65536.0) * 100.0f;
        
        seed = seed * 1103515245 + 12345;
        data[i].weight = (double)((seed & 0xFFFFF) / 1048576.0) * 500.0;
        
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
    
    printf("Starting complex computations...\n");
    
    /* Call multiple functions with different patterns */
    double result1 = complex_reduction(data, SIZE);
    printf("Result 1: %f\n", result1);
    
    float result2 = nested_mixed_operations(data, SIZE);
    printf("Result 2: %f\n", result2);
    
    long result3 = pointer_chasing_reduction(data, SIZE);
    printf("Result 3: %ld\n", result3);
    
    /* Final combination to ensure all computations are used */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    /* Verify some data was modified */
    int modified_count = 0;
    for (int i = 0; i < 100; i++) {
        if (data[i].counter > 0 || data[i].tag != 'A') {
            modified_count++;
        }
    }
    printf("Modified elements in first 100: %d\n", modified_count);
    
    free(data);
    return 0;
}
