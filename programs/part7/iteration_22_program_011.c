/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 100
#define MIDDLE_LOOP 50
#define OUTER_LOOP 20

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 0.0f;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double precision;
    char tag;
    int data[3];
};

/* Simple LCG for pseudo-random numbers */
static unsigned int lcg_seed = 12345;
static inline unsigned int lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double reduction_with_carry(struct MixedData* array, int size) {
    double acc = 0.0;
    volatile int v_cond = g_volatile_counter;
    
    /* Complex loop with carried dependency */
    for (int i = 1; i < size; i++) {
        /* Data-dependent condition using volatile */
        if ((v_cond & (1 << (i % 16))) != 0) {
            /* Carried dependency: uses previous iteration's value */
            acc = acc + array[i].value * array[i-1].precision;
            
            /* Additional operations to create more RTL instructions */
            array[i].data[0] = (int)(acc * 1000);
            array[i].data[1] = array[i-1].data[2] + lcg_rand() % 100;
        } else {
            /* Alternative path with different operations */
            acc = acc - array[i].value * 0.5;
            array[i].data[2] = (int)(array[i].precision * array[i].value);
        }
        
        /* Non-contiguous memory access pattern */
        if (i % 3 == 0) {
            array[i].tag = (char)((int)acc % 26 + 'A');
        }
        
        /* Volatile read to prevent dead code elimination */
        if (g_volatile_float > 0.5f) {
            acc *= 1.001;
        }
    }
    
    return acc;
}

/* Function 2: Deeply nested loops with volatile conditionals */
__attribute__((optimize("O3", "funroll-loops")))
float nested_loops_complex(struct MixedData* array, int size) {
    float total = 0.0f;
    volatile int outer_volatile = g_volatile_counter % 10;
    
    /* Triple nested loop structure */
    for (int o = 0; o < OUTER_LOOP; o++) {
        volatile int mid_volatile = outer_volatile + o;
        
        for (int m = 0; m < MIDDLE_LOOP; m++) {
            /* Data-dependent condition using pseudo-random */
            int rand_val = lcg_rand() % 100;
            volatile int inner_volatile = mid_volatile * rand_val;
            
            for (int i = 0; i < INNER_LOOP; i++) {
                int idx = (o * MIDDLE_LOOP * INNER_LOOP + 
                          m * INNER_LOOP + i) % size;
                
                /* Complex conditional with mixed operations */
                if ((inner_volatile & (1 << (i % 8))) != 0) {
                    total += array[idx].value * 2.0f;
                    array[idx].precision = total / (i + 1);
                    
                    /* Integer operations */
                    array[idx].id = (int)total ^ array[idx].data[i % 3];
                } else if (rand_val > 50) {
                    total -= array[idx].value * 0.75f;
                    array[idx].precision = array[idx].precision * 0.99;
                    
                    /* Bit manipulation */
                    array[idx].data[0] = array[idx].data[0] << 2;
                    array[idx].data[1] = array[idx].data[1] >> 1;
                } else {
                    /* Third path with different operations */
                    total = total * 0.9f + array[idx].value;
                    array[idx].precision = (double)total / array[idx].value;
                    
                    /* Memory access with stride */
                    if (idx > 0 && idx % 5 == 0) {
                        array[idx].data[2] = array[idx-1].data[1] + 
                                            array[idx].data[0];
                    }
                }
                
                /* Additional volatile check */
                if (g_volatile_float < (float)(i % 100) / 100.0f) {
                    total += 0.001f;
                }
            }
            
            /* Middle loop operation with volatile */
            if (mid_volatile % 3 == 0) {
                total *= 1.01f;
            }
        }
        
        /* Outer loop operation */
        if (outer_volatile % 2 == 0) {
            total = total / (o + 1);
        }
    }
    
    return total;
}

/* Function 3: Mixed data type processing with non-contiguous access */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
int mixed_type_processing(struct MixedData* array, int size) {
    int int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    volatile int stride_volatile = g_volatile_counter % 7 + 2;
    
    /* Process with non-unit stride */
    for (int i = 0; i < size; i += stride_volatile) {
        /* Multiple independent operations to create parallel opportunities */
        int_sum += array[i].id * 2;
        int_sum -= array[i].data[0];
        
        float_sum += array[i].value * 3.14f;
        if (array[i].value > 0.5f) {
            float_sum *= 1.05f;
        }
        
        double_sum += array[i].precision / (i + 1);
        double_sum = double_sum * 0.999;
        
        /* Conditional store based on complex condition */
        if ((int_sum ^ (int)float_sum) > (int)double_sum) {
            array[i].tag = 'X';
            array[i].data[1] = int_sum % 1000;
        } else if ((lcg_rand() % 100) > 75) {
            array[i].tag = 'Y';
            array[i].data[2] = (int)(double_sum * 100);
        }
        
        /* Nested loop inside main loop */
        for (int j = 0; j < 3; j++) {
            if (j == 0) {
                array[i].data[j] += int_sum;
            } else if (j == 1) {
                array[i].data[j] -= (int)float_sum;
            } else {
                array[i].data[j] ^= (int)double_sum;
            }
            
            /* Volatile access in inner loop */
            if (g_volatile_counter > 1000) {
                array[i].data[j] += 1;
            }
        }
    }
    
    /* Final reduction combining all types */
    return int_sum + (int)float_sum + (int)double_sum;
}

/* Main function that orchestrates all computations */
int main() {
    /* Allocate and initialize array with pseudo-random data */
    struct MixedData* data_array = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    if (!data_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varied data */
    for (int i = 0; i < SIZE; i++) {
        data_array[i].id = lcg_rand() % 10000;
        data_array[i].value = (float)(lcg_rand() % 1000) / 1000.0f;
        data_array[i].precision = (double)(lcg_rand() % 10000) / 10000.0;
        data_array[i].tag = (char)((i % 26) + 'A');
        for (int j = 0; j < 3; j++) {
            data_array[i].data[j] = lcg_rand() % 1000;
        }
    }
    
    /* Update volatile variables */
    g_volatile_counter = lcg_rand() % 1000;
    g_volatile_float = (float)(lcg_rand() % 1000) / 1000.0f;
    
    /* Call all computation functions to trigger scheduler activity */
    double result1 = reduction_with_carry(data_array, SIZE);
    printf("Result 1 (reduction with carry): %.6f\n", result1);
    
    float result2 = nested_loops_complex(data_array, SIZE);
    printf("Result 2 (nested loops): %.6f\n", result2);
    
    int result3 = mixed_type_processing(data_array, SIZE);
    printf("Result 3 (mixed type processing): %d\n", result3);
    
    /* Combine results to ensure all computations are live */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %.6f\n", final_result);
    
    /* Additional volatile operations */
    g_volatile_counter = (int)final_result % 1000;
    g_volatile_float = (float)((int)final_result % 1000) / 1000.0f;
    
    free(data_array);
    return 0;
}
