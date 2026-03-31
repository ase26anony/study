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

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2")))
double complex_reduction(struct MixedData* arr, int size) {
    double acc = 0.0;
    volatile int cond = g_volatile_counter;
    
    /* Outer loop */
    for (int o = 0; o < OUTER_LOOP; o++) {
        /* Middle loop with volatile condition */
        for (int m = 0; m < MIDDLE_LOOP; m++) {
            /* Data-dependent condition using volatile */
            if ((cond + m) % 3 == 0) {
                /* Innermost loop with carried dependency */
                for (int i = 2; i < size; i += 3) { /* Non-contiguous access */
                    /* Complex reduction with multiple dependencies */
                    double temp = arr[i].weight * arr[i-1].value;
                    
                    /* Conditional operation based on volatile */
                    if (g_volatile_float > 1.0f) {
                        temp += arr[i-2].data[0] * 0.5;
                    }
                    
                    /* Carried dependency across iterations */
                    acc = acc + temp;
                    
                    /* Mixed type operations */
                    arr[i].value = (float)(acc * 0.1);
                    arr[i].data[1] = (int)(temp * 100);
                    
                    /* Volatile read to prevent optimization */
                    cond = g_volatile_counter;
                }
            } else if ((cond + m) % 5 == 0) {
                /* Alternative path with different access pattern */
                for (int i = 1; i < size; i += 4) {
                    acc += arr[i].weight - arr[i].value;
                    arr[i].tag = (char)((int)acc % 128);
                }
            }
            
            /* More volatile-dependent operations */
            if (rand() % 100 < g_volatile_counter % 50) {
                g_volatile_float *= 1.1f;
            }
        }
        
        /* Update volatile condition */
        g_volatile_counter += o % 7;
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed operations */
__attribute__((optimize("O3")))
float nested_mixed_operations(struct MixedData* arr, int size) {
    float result = 0.0f;
    volatile float v_cond = g_volatile_float;
    
    /* Triple nested loop */
    for (int i = 0; i < INNER_LOOP; i++) {
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            for (int k = 0; k < OUTER_LOOP; k++) {
                /* Data-dependent index calculation */
                int idx = (i * 7 + j * 3 + k) % size;
                
                /* Complex conditional with volatile */
                if ((idx + (int)v_cond) % 2 == 0) {
                    /* Mixed precision operations */
                    double dval = arr[idx].weight;
                    float fval = arr[idx].value;
                    int ival = arr[idx].data[0];
                    
                    /* Chain of dependent operations */
                    result += (float)(dval * fval) / (ival + 1);
                    arr[idx].value = result * 0.9f;
                    
                    /* Non-linear memory access */
                    int next_idx = (idx * 13 + 7) % size;
                    arr[next_idx].data[2] = (int)result;
                    
                    /* Volatile update */
                    v_cond = g_volatile_float * 0.95f;
                } else {
                    /* Alternative computation path */
                    result -= arr[idx].value * 0.3f;
                    arr[idx].weight = result;
                }
                
                /* Additional volatile-dependent branch */
                if (rand() % 1000 < 500) {
                    result *= 1.01f;
                }
            }
        }
    }
    
    return result;
}

/* Function with pointer chasing and indirect memory access */
__attribute__((optimize("O2")))
int pointer_chasing_reduction(struct MixedData* arr, int size) {
    int sum = 0;
    volatile int seed = g_volatile_counter;
    
    /* Create linked-list like access pattern */
    for (int i = 0; i < size / 2; i++) {
        /* Non-linear pointer access */
        struct MixedData* current = &arr[i];
        struct MixedData* next = &arr[(i * 3 + 1) % size];
        
        /* Chain of dependent operations */
        for (int chain = 0; chain < 5; chain++) {
            /* Volatile-dependent condition */
            if ((seed + chain) % 4 == 0) {
                sum += current->data[0] * next->data[1];
                current->value = (float)sum * 0.01f;
            } else {
                sum -= next->data[2] - current->data[0];
                next->weight = sum * 0.001;
            }
            
            /* Pointer chasing */
            current = next;
            next = &arr[(current->id + chain) % size];
            
            /* Update volatile */
            seed = g_volatile_counter + chain;
        }
        
        /* Conditional store based on volatile */
        if (g_volatile_float > 2.0f) {
            arr[i].data[0] = sum % 1000;
        }
    }
    
    return sum;
}

/* Main driver function */
int main() {
    /* Allocate and initialize array with pseudo-random data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    if (!data) return 1;
    
    /* Simple LCG for deterministic but complex initialization */
    unsigned int lcg_seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        lcg_seed = lcg_seed * 1103515245 + 12345;
        data[i].id = i;
        data[i].value = (float)(lcg_seed % 1000) / 100.0f;
        data[i].weight = (double)(lcg_seed % 2000) / 50.0;
        data[i].tag = (char)(lcg_seed % 128);
        for (int j = 0; j < 3; j++) {
            lcg_seed = lcg_seed * 1103515245 + 12345;
            data[i].data[j] = lcg_seed % 10000;
        }
    }
    
    /* Initialize volatile globals */
    g_volatile_counter = 42;
    g_volatile_float = 2.5f;
    
    /* Call computation functions with different patterns */
    double result1 = complex_reduction(data, SIZE);
    float result2 = nested_mixed_operations(data, SIZE);
    int result3 = pointer_chasing_reduction(data, SIZE);
    
    /* Combine results to ensure live computation */
    double final_result = result1 + result2 + result3;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", final_result);
    
    /* Additional volatile operations to maintain complexity */
    for (int i = 0; i < 100; i++) {
        g_volatile_counter += i % 11;
        g_volatile_float *= 0.99f;
    }
    
    free(data);
    return 0;
}
