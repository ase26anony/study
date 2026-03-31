/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MID_LOOP 20
#define OUTER_LOOP 10

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 0.0f;

/* Mixed data type structure with padding */
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
        for (int m = 0; m < MID_LOOP; m++) {
            if (cond++ % 3) {  /* Data-dependent branch */
                /* Innermost loop with carried dependency */
                for (int i = 2; i < size; i += 3) {  /* Non-contiguous access */
                    /* Complex reduction with mixed operations */
                    double temp = arr[i].weight * arr[i-1].value;
                    temp += (double)arr[i].data[0] * 0.5;
                    
                    /* Conditional operation based on volatile */
                    if (g_volatile_float > 0.5f) {
                        temp *= 1.1;
                    }
                    
                    /* Carried dependency across iterations */
                    acc = acc + temp;
                    
                    /* Mixed type operations */
                    arr[i].value = (float)(acc * 0.01);
                    arr[i].data[1] = (int)(temp * 100);
                }
            } else {
                /* Alternative path with different access pattern */
                for (int i = 1; i < size; i += 4) {
                    float fval = arr[i].value * 2.0f;
                    arr[i].weight = (double)fval / 3.14159;
                    acc += arr[i].weight * (i % 10);
                }
            }
            
            /* More volatile-dependent operations */
            if (rand() % 100 > 50) {  /* External function call */
                g_volatile_float += 0.1f;
            }
        }
        
        /* Additional computation between loop levels */
        for (int j = 0; j < INNER_LOOP; j++) {
            acc *= 0.999;
            g_volatile_counter++;
        }
    }
    
    return acc;
}

/* Function with deeply nested loops and complex conditions */
__attribute__((optimize("O3")))
float nested_conditional_processing(struct MixedData* arr, int size) {
    float result = 0.0f;
    volatile int v1 = g_volatile_counter;
    volatile float v2 = g_volatile_float;
    
    /* Triple nested loop */
    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 8; y++) {
            /* Volatile condition in middle loop */
            if (v1++ % (y + 1) == 0) {
                for (int z = 0; z < size / 100; z++) {
                    int idx = (x * 100 + y * 10 + z) % size;
                    
                    /* Complex conditional chain */
                    if (arr[idx].id % 2 == 0) {
                        if (v2 > 0.3f) {
                            result += arr[idx].value * 2.0f;
                            arr[idx].weight = result * 0.5;
                        } else {
                            result -= arr[idx].value * 0.5f;
                            arr[idx].data[2] = (int)result;
                        }
                    } else {
                        /* Mixed precision computation */
                        double dtemp = arr[idx].weight * 3.14159;
                        result += (float)(dtemp / (idx + 1));
                        
                        /* Memory access with stride */
                        if (idx > 0 && idx < size - 1) {
                            arr[idx-1].value = result;
                            arr[idx+1].value = result * 0.8f;
                        }
                    }
                    
                    /* Function call with side effect */
                    if (rand() % 7 == 0) {
                        g_volatile_float = result * 0.01f;
                    }
                }
            }
        }
        
        /* Reduction across outer loop iterations */
        for (int i = 0; i < size; i += 7) {  /* Stride 7 access */
            result += arr[i].value * (x + 1);
            arr[i].id += (int)result;
        }
    }
    
    return result;
}

/* Function with pointer chasing and indirect memory access */
__attribute__((optimize("O2")))
double pointer_chasing_reduction(struct MixedData* arr, int size) {
    double sum = 0.0;
    struct MixedData* current = &arr[0];
    volatile int selector = g_volatile_counter;
    
    for (int iter = 0; iter < 1000; iter++) {
        /* Data-dependent pointer chasing */
        int offset = (selector++ % 5) * 3;
        if (offset >= size) offset = 0;
        
        /* Process a chain of elements */
        for (int chain = 0; chain < 20; chain++) {
            /* Mixed operations on chased element */
            sum += current->weight * current->value;
            sum += current->data[chain % 3] * 0.25;
            
            /* Conditional store based on volatile */
            if (g_volatile_float > sum) {
                current->value = (float)sum;
                current->weight = sum * 1.1;
            }
            
            /* Move to next element with stride */
            int next_idx = (current - arr + offset) % size;
            current = &arr[next_idx];
            
            /* Additional computation */
            for (int inner = 0; inner < 3; inner++) {
                sum *= 0.99;
                if (rand() % 10 == 0) {
                    g_volatile_counter++;
                }
            }
        }
        
        /* Periodic reduction */
        if (iter % 10 == 0) {
            for (int i = 0; i < size; i += 11) {
                sum += arr[i].weight * i;
                arr[i].data[0] = (int)sum;
            }
        }
    }
    
    return sum;
}

/* Simple LCG for reproducible pseudo-random numbers */
static unsigned int lcg_seed = 12345;
static inline unsigned int lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main() {
    /* Allocate and initialize array with pseudo-random data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    if (!data) return 1;
    
    printf("Initializing data...\n");
    for (int i = 0; i < SIZE; i++) {
        data[i].id = lcg_rand() % 1000;
        data[i].value = (float)(lcg_rand() % 1000) / 100.0f;
        data[i].weight = (double)(lcg_rand() % 1000) / 50.0;
        data[i].tag = 'A' + (lcg_rand() % 26);
        for (int j = 0; j < 3; j++) {
            data[i].data[j] = lcg_rand() % 100;
        }
    }
    
    /* Seed system rand for volatile conditions */
    srand(42);
    
    printf("Starting complex computations...\n");
    
    /* Call all computation functions to increase scheduling opportunities */
    double result1 = complex_reduction(data, SIZE);
    printf("Result 1: %f\n", result1);
    
    float result2 = nested_conditional_processing(data, SIZE);
    printf("Result 2: %f\n", result2);
    
    double result3 = pointer_chasing_reduction(data, SIZE);
    printf("Result 3: %f\n", result3);
    
    /* Final reduction combining all results */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    /* Use results to prevent optimization */
    if (final_result > 1000000.0) {
        printf("Large result detected!\n");
    }
    
    free(data);
    return 0;
}
