/* Complex test program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2048
#define INNER_LOOP 128
#define MIDDLE_LOOP 64
#define OUTER_LOOP 32

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
    volatile int v = g_volatile_counter;
    
    /* Outer loop with volatile condition */
    for (int i = 1; i < OUTER_LOOP; i++) {
        if (v % (i + 1)) {  /* Data-dependent condition */
            /* Middle loop with mixed operations */
            for (int j = 2; j < MIDDLE_LOOP; j += 3) {  /* Non-unit stride */
                float temp_float = g_volatile_float;
                
                /* Innermost loop with carried dependency */
                for (int k = 0; k < INNER_LOOP; k++) {
                    /* Complex reduction with carried dependency across iterations */
                    if (k > 0) {
                        acc += arr[(i * j + k) % n].value * 
                               arr[(i * j + k - 1) % n].weight;
                    }
                    
                    /* Mixed integer operations */
                    int idx = (i * 7 + j * 11 + k * 13) % n;
                    arr[idx].data[k % 3] = (arr[idx].id * k) + (int)(temp_float * j);
                    
                    /* Conditional store based on volatile */
                    if (v % 17 == (i * j + k) % 17) {
                        arr[idx].value = (float)(acc * 0.01);
                    }
                }
                
                /* Additional floating point operations */
                temp_float *= 1.01f;
                if (temp_float > 100.0f) {
                    temp_float = 1.0f;
                }
            }
        }
        v = (v * 1103515245 + 12345) & 0x7fffffff;  /* Simple PRNG update */
    }
    
    return acc;
}

/* Function with deeply nested loops and volatile conditionals */
__attribute__((optimize("O3")))
float nested_conditional_processing(struct MixedData* arr, int n) {
    float result = 0.0f;
    volatile int cond1 = g_volatile_counter % 7;
    volatile float cond2 = g_volatile_float;
    
    /* Triple nested loop with volatile conditions */
    for (int a = 0; a < 16; a++) {
        if (cond1 > a) {  /* Volatile condition at outer level */
            for (int b = 1; b < 24; b += 2) {  /* Odd stride */
                float local_acc = 0.0f;
                
                for (int c = 0; c < 48; c++) {
                    /* Access pattern with multiple strides */
                    int idx = (a * 19 + b * 23 + c * 29) % n;
                    
                    /* Complex conditional chain */
                    if (cond2 > 0.5f) {
                        local_acc += arr[idx].value * arr[idx].weight;
                    } else if (cond1 % 3 == 0) {
                        local_acc -= arr[idx].value / arr[idx].weight;
                    }
                    
                    /* Mixed type operations with conversion */
                    arr[idx].id = (int)(local_acc * 100) + c;
                    
                    /* Another volatile check */
                    if ((cond1 + c) % 5 == 0) {
                        arr[idx].tag = (char)('A' + (local_acc * 10));
                    }
                }
                
                result += local_acc;
                cond2 *= 0.99f;  /* Modify volatile between iterations */
            }
        }
        cond1 = (cond1 * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    return result;
}

/* Function with pointer chasing and complex data flow */
__attribute__((optimize("O2")))
int pointer_chasing_reduction(struct MixedData* arr, int n) {
    int sum = 0;
    struct MixedData* current = &arr[0];
    volatile int skip = g_volatile_counter % 11;
    
    /* Loop with pointer chasing pattern */
    for (int i = 0; i < n; i += (skip + 1)) {
        skip = (skip * 3 + 1) % 13;  /* Update volatile */
        
        /* Nested loop with reduction */
        for (int j = 0; j < 8; j++) {
            /* Complex addressing calculation */
            int offset = (i * j + skip) % n;
            
            /* Mixed operations creating various RTL patterns */
            sum += current->id * arr[offset].data[j % 3];
            sum -= (int)(current->value * arr[offset].weight);
            
            /* Conditional update with volatile */
            if (skip % (j + 2) == 0) {
                current = &arr[offset];
            }
        }
        
        /* Additional inner loop with floating point */
        float temp = 0.0f;
        for (int k = 0; k < 4; k++) {
            temp += arr[(i + k) % n].value * k;
        }
        sum += (int)(temp * 100);
    }
    
    return sum;
}

/* Main function that drives all computations */
int main() {
    /* Allocate and initialize array with pseudo-random data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    if (!data) return 1;
    
    /* Initialize with deterministic but complex pattern */
    unsigned int seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        data[i].id = (seed >> 16) & 0x7FFF;
        
        seed = seed * 1103515245 + 12345;
        data[i].value = (float)((seed & 0xFFFF) / 65536.0) * 100.0f;
        
        seed = seed * 1103515245 + 12345;
        data[i].weight = (double)((seed & 0xFFFF) / 65536.0) * 10.0;
        
        data[i].tag = 'A' + (i % 26);
        for (int j = 0; j < 3; j++) {
            seed = seed * 1103515245 + 12345;
            data[i].data[j] = (seed >> 16) & 0xFF;
        }
    }
    
    /* Update volatile variables to affect control flow */
    g_volatile_counter = data[0].id;
    g_volatile_float = data[0].value;
    
    /* Call all computation functions to create diverse scheduling scenarios */
    double result1 = complex_reduction(data, SIZE);
    float result2 = nested_conditional_processing(data, SIZE);
    int result3 = pointer_chasing_reduction(data, SIZE);
    
    /* Combine results to prevent dead code elimination */
    double final_result = result1 + result2 + result3;
    
    /* Print result to ensure code is live */
    printf("Final result: %f\n", final_result);
    
    /* Additional volatile operations to maintain complexity */
    for (int i = 0; i < 100; i++) {
        g_volatile_counter = (g_volatile_counter * 3 + i) % 1000;
        if (g_volatile_counter % 19 == 0) {
            g_volatile_float *= 1.1f;
        }
    }
    
    free(data);
    return 0;
}
