/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000
#define INNER_LOOP 50
#define MIDDLE_LOOP 30
#define OUTER_LOOP 20

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

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2")))
double complex_reduction(struct MixedData* arr, int n) {
    double acc = 0.0;
    volatile int v1 = g_volatile_counter;
    volatile float v2 = g_volatile_float;
    
    /* Outer loop with volatile condition */
    for (int i = 0; i < OUTER_LOOP && v1 < 100; i++) {
        v1++;
        
        /* Middle loop with data-dependent condition */
        for (int j = 2; j < MIDDLE_LOOP; j += (v1 % 3) + 1) {
            double temp = 0.0;
            
            /* Innermost loop with carried dependency and reduction */
            for (int k = 1; k < INNER_LOOP; k++) {
                /* Access with non-unit stride and mixed types */
                int idx = (i * MIDDLE_LOOP * INNER_LOOP + j * INNER_LOOP + k) % n;
                int prev_idx = (idx - 1 + n) % n;
                
                /* Complex reduction with carried dependency */
                acc = acc + arr[idx].value * arr[prev_idx].weight;
                
                /* Mixed operations to create diverse RTL */
                arr[idx].data[k % 3] = (int)(acc * 100.0);
                temp += arr[idx].weight * (v2 + 0.1f);
                
                /* Volatile-dependent conditional */
                if ((k & (v1 % 7)) == 0) {
                    arr[idx].value = (float)(temp * 0.5);
                }
            }
            
            /* Conditional store based on volatile */
            if ((j % ((int)v2 + 1)) == 0) {
                acc *= 0.99;
            }
        }
        
        /* Update volatile to affect loop conditions */
        v2 += 0.1f;
    }
    
    return acc;
}

/* Function with deeply nested loops and volatile conditionals */
__attribute__((optimize("O3")))
float nested_conditional_processing(struct MixedData* arr, int n) {
    float result = 0.0f;
    volatile int cond = g_volatile_counter;
    
    /* Triple nested loop with volatile conditions */
    for (int a = 0; a < 15 && cond < 50; a++) {
        cond += (a % 3);
        
        for (int b = 1; b < 25; b += (cond % 4) + 1) {
            float local_acc = 0.0f;
            
            for (int c = 0; c < 40; c++) {
                /* Non-contiguous access pattern */
                int idx = (a * 1000 + b * 40 + c * 3) % n;
                
                /* Mixed type operations */
                local_acc += arr[idx].value * (float)arr[idx].weight;
                
                /* Data-dependent conditional */
                if ((arr[idx].id & (cond % 5)) == 0) {
                    result += local_acc;
                    local_acc *= 0.8f;
                }
                
                /* Integer operations mixed with float */
                arr[idx].data[c % 3] = (int)(local_acc * 1000.0f) ^ (cond & 0xFF);
            }
            
            /* Volatile-dependent update */
            if ((b & (cond % 3)) == 0) {
                result -= local_acc * 0.5f;
            }
        }
        
        /* Function call with side effect to prevent optimization */
        g_volatile_counter = cond % 100;
    }
    
    return result;
}

/* Function with pointer arithmetic and complex addressing */
__attribute__((optimize("O2")))
int pointer_based_computation(struct MixedData* arr, int n) {
    int sum = 0;
    struct MixedData* ptr = arr;
    volatile int stride = (g_volatile_counter % 5) + 2;
    
    /* Loop with pointer chasing and volatile stride */
    for (int i = 0; i < n && stride < 8; i += stride) {
        /* Update stride based on computation */
        stride = (stride + (ptr->id % 3)) % 7 + 1;
        
        /* Complex addressing with mixed operations */
        for (int j = 0; j < 3; j++) {
            /* Conditional access pattern */
            if ((ptr->data[j] & 1) == 0) {
                sum += ptr->data[j] * (int)ptr->value;
                
                /* Floating point in integer loop */
                ptr->weight = (double)sum * 0.001;
            } else {
                sum -= (int)(ptr->weight * 100.0);
            }
            
            /* Volatile-dependent operation */
            if ((j + g_volatile_counter) % 4 == 0) {
                ptr->value = (float)(sum % 1000) * 0.01f;
            }
        }
        
        /* Non-linear pointer advance */
        ptr += stride;
        if (ptr >= arr + n) {
            ptr = arr + (ptr - arr) % n;
        }
    }
    
    return sum;
}

/* Simple PRNG to initialize data without external dependencies */
static unsigned int prng_state = 123456789;
unsigned int simple_rand() {
    prng_state = prng_state * 1103515245 + 12345;
    return (prng_state >> 16) & 0x7FFF;
}

/* Initialize array with pseudo-random data */
void initialize_data(struct MixedData* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i].id = simple_rand() % 1000;
        arr[i].value = (float)(simple_rand() % 1000) / 10.0f;
        arr[i].weight = (double)(simple_rand() % 1000) / 100.0;
        arr[i].tag = 'A' + (simple_rand() % 26);
        for (int j = 0; j < 3; j++) {
            arr[i].data[j] = simple_rand() % 10000;
        }
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
    
    /* Call computation functions with different patterns */
    double result1 = complex_reduction(data, SIZE);
    float result2 = nested_conditional_processing(data, SIZE);
    int result3 = pointer_based_computation(data, SIZE);
    
    /* Combine results to ensure code isn't optimized away */
    double final_result = result1 + result2 + result3;
    
    /* Print result to prevent dead code elimination */
    printf("Final computation result: %f\n", final_result);
    
    /* Additional volatile operations to affect scheduling */
    g_volatile_counter = (int)final_result % 1000;
    g_volatile_float = (float)(final_result * 0.01);
    
    free(data);
    return 0;
}
