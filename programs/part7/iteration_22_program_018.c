/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 100
#define MIDDLE_LOOP 50
#define OUTER_LOOP 20

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double precision;
    char marker;
};

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2")))
double complex_reduction(struct MixedData* data, int count) {
    double acc = 0.0;
    volatile int v_cond = g_volatile_counter;
    
    /* Outer loop */
    for (int o = 0; o < OUTER_LOOP; o++) {
        /* Middle loop with volatile condition */
        for (int m = 0; m < MIDDLE_LOOP; m++) {
            if (v_cond > (rand() % 100)) {  /* Data-dependent branch */
                /* Innermost loop with carried dependency */
                for (int i = 2; i < INNER_LOOP; i++) {
                    /* Complex reduction with carried dependency across iterations */
                    int idx = (o * MIDDLE_LOOP * INNER_LOOP + m * INNER_LOOP + i) % count;
                    int prev_idx = (o * MIDDLE_LOOP * INNER_LOOP + m * INNER_LOOP + i - 1) % count;
                    
                    /* Mixed operations creating diverse RTL patterns */
                    double temp = data[idx].precision * data[prev_idx].value;
                    temp += (double)data[idx].id / (data[prev_idx].id + 1);
                    
                    /* Conditional accumulation */
                    if (data[idx].marker == 'A' || v_cond % 3 == 0) {
                        acc = acc + temp;
                    } else {
                        acc = acc - temp * 0.5;
                    }
                    
                    /* Additional floating point operations */
                    data[idx].value = (float)(acc * 0.01);
                }
            } else {
                /* Alternative path with different access pattern */
                for (int i = 0; i < INNER_LOOP; i += 3) {  /* Non-contiguous access */
                    int idx = (o * MIDDLE_LOOP * INNER_LOOP + m * INNER_LOOP + i) % count;
                    if (idx > 0) {
                        acc += data[idx].precision - data[idx-1].precision;
                    }
                }
            }
            
            /* Update volatile condition */
            v_cond = (v_cond * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed operations */
__attribute__((optimize("O3")))
float nested_mixed_operations(int* int_array, float* float_array, int size) {
    float result = 0.0f;
    volatile float v_float = g_volatile_float;
    
    /* Triple nested loop */
    for (int i = 0; i < size / 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                /* Data-dependent condition with volatile */
                if ((int_array[i * 4 + j] % (k + 2)) > (int)(v_float * 10)) {
                    /* Complex mixed-type computation */
                    float temp = float_array[i * 8 + k] * (j + 1);
                    temp += (float)(int_array[i * 4 + j] / (k + 1));
                    
                    /* Conditional store with stride */
                    if (temp > 0 && (i + j + k) % 3 == 0) {
                        float_array[(i * 8 + k + 2) % size] = temp;
                        result += temp * 0.3f;
                    }
                }
                
                /* Additional volatile-dependent operation */
                v_float = v_float * 1.01f - 0.5f;
            }
        }
        
        /* Reduction with dependency */
        if (i > 0) {
            result += result * 0.1f + float_array[i] - float_array[i-1];
        }
    }
    
    return result;
}

/* Function with pointer arithmetic and non-contiguous access */
__attribute__((optimize("O2")))
double pointer_stride_processing(struct MixedData* data, int count) {
    double sum = 0.0;
    struct MixedData* ptr = data;
    volatile int stride = (g_volatile_counter % 5) + 1;
    
    /* Process with variable stride */
    for (int i = 0; i < count; i += stride) {
        ptr = data + i;
        
        /* Complex pointer arithmetic */
        for (int offset = 0; offset < 4 && (i + offset) < count; offset++) {
            struct MixedData* current = ptr + offset;
            
            /* Mixed operations */
            double val = current->precision * current->value;
            val += (double)(current->id * offset) / 256.0;
            
            /* Conditional accumulation based on volatile */
            if ((g_volatile_counter + i + offset) % 7 == 0) {
                sum += val;
                current->value = (float)(sum * 0.01);
            } else {
                sum -= val * 0.3;
            }
            
            /* Update marker based on computation */
            current->marker = (sum > 0) ? 'A' : 'B';
        }
        
        /* Change stride dynamically */
        stride = (stride * 3) % 7 + 1;
    }
    
    return sum;
}

/* Main function with initialization and multiple computations */
int main() {
    /* Initialize data arrays */
    struct MixedData* mixed_data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with pseudo-random data */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        /* Simple LCG for reproducibility */
        seed = seed * 1103515245 + 12345;
        
        mixed_data[i].id = (int)(seed % 1000);
        mixed_data[i].value = (float)((seed % 10000) / 100.0);
        mixed_data[i].precision = (double)((seed % 100000) / 1000.0);
        mixed_data[i].marker = (seed % 2) ? 'A' : 'B';
        
        int_array[i] = (int)(seed % 5000);
        float_array[i] = (float)((seed % 3000) / 50.0);
    }
    
    /* Update volatile variables */
    g_volatile_counter = int_array[0] % 100;
    g_volatile_float = float_array[0] / 10.0f;
    
    /* Perform multiple complex computations */
    double result1 = complex_reduction(mixed_data, SIZE);
    printf("Result 1: %f\n", result1);
    
    float result2 = nested_mixed_operations(int_array, float_array, SIZE);
    printf("Result 2: %f\n", result2);
    
    double result3 = pointer_stride_processing(mixed_data, SIZE);
    printf("Result 3: %f\n", result3);
    
    /* Combine results to prevent dead code elimination */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    /* Cleanup */
    free(mixed_data);
    free(int_array);
    free(float_array);
    
    return (final_result > 0) ? 0 : 1;
}
