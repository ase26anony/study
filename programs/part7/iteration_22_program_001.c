/* test_sel_sched.c - Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2048
#define INNER_LOOP 100
#define MID_LOOP 50
#define OUTER_LOOP 20

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int index;
    float value;
    double weight;
    char tag;
    volatile int flag; /* volatile to prevent optimization */
};

/* Global arrays to ensure memory operations */
static int int_array[SIZE];
static float float_array[SIZE];
static double double_array[SIZE];
static struct MixedData mixed_array[SIZE];

/* Initialize arrays with pseudo-random data */
__attribute__((noinline))
void initialize_arrays(void) {
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        /* Simple LCG for reproducibility */
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        int_array[i] = (int)(seed % 1000);
        float_array[i] = (float)(seed % 1000) * 0.001f;
        double_array[i] = (double)(seed % 1000) * 0.0001;
        mixed_array[i].index = i;
        mixed_array[i].value = float_array[i];
        mixed_array[i].weight = double_array[i];
        mixed_array[i].tag = (char)('A' + (i % 26));
        mixed_array[i].flag = (i % 7 == 0) ? 1 : 0;
    }
}

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2")))
double reduction_with_carry(void) {
    volatile int v_cond = 1; /* volatile to prevent dead code elimination */
    double acc = 0.0;
    double prev = double_array[0];
    
    /* Complex loop with data-dependent control flow */
    for (int i = 1; i < SIZE - 1; i++) {
        /* Volatile condition to prevent branch prediction optimization */
        if (v_cond && (int_array[i] % 3 == 0)) {
            /* Carried dependency: uses previous iteration's value */
            acc += prev * double_array[i] * (1.0 + float_array[i]);
            prev = double_array[i] * 0.5;
            
            /* Additional computation to create more RTL instructions */
            if (mixed_array[i].flag && (i % 5 == 0)) {
                acc -= double_array[i-1] * 0.25;
            }
        } else if (v_cond && (int_array[i] % 7 == 0)) {
            /* Alternative path with different operations */
            acc += double_array[i] / (float_array[i] + 0.001f);
            prev = double_array[i] * 0.75;
        } else {
            /* Default path */
            acc += double_array[i];
            prev = double_array[i];
        }
        
        /* Non-contiguous memory access */
        if (i % 4 == 0) {
            double_array[i] = acc * 0.01;
        }
    }
    
    return acc;
}

/* Function 2: Mixed data types with non-contiguous access */
__attribute__((optimize("O2")))
float process_mixed_types(void) {
    volatile int outer_flag = 1;
    float total = 0.0f;
    
    /* Triple nested loop */
    for (int i = 0; i < OUTER_LOOP && outer_flag; i++) {
        volatile int mid_flag = (i % 2 == 0) ? 1 : 0;
        
        for (int j = 0; j < MID_LOOP && mid_flag; j++) {
            volatile int inner_flag = (rand() % 10 > 3) ? 1 : 0;
            
            for (int k = 0; k < INNER_LOOP && inner_flag; k++) {
                int idx = (i * MID_LOOP * INNER_LOOP + j * INNER_LOOP + k) % SIZE;
                
                /* Complex conditional operations on mixed types */
                if (mixed_array[idx].flag) {
                    total += mixed_array[idx].value * (float)mixed_array[idx].weight;
                    
                    /* Conditional store with stride */
                    if (idx % 3 == 0) {
                        mixed_array[idx].value = total * 0.1f;
                        float_array[idx] = mixed_array[idx].value;
                    }
                } else if (int_array[idx] % 11 == 0) {
                    total -= mixed_array[idx].value / (float_array[idx] + 0.001f);
                    
                    /* Another conditional store pattern */
                    if (idx % 7 == 0) {
                        int_array[idx] = (int)(total * 100.0f);
                    }
                }
                
                /* Additional floating point operations */
                total = total * 0.999f + float_array[idx] * 0.001f;
            }
            
            /* Middle loop computation */
            if (j % 13 == 0) {
                total += (float)int_array[j % SIZE] * 0.01f;
            }
        }
        
        /* Outer loop computation with volatile dependency */
        if (outer_flag && (i % 17 == 0)) {
            total *= 1.01f;
        }
    }
    
    return total;
}

/* Function 3: Deeply nested loops with complex control flow */
__attribute__((optimize("O3")))
int complex_nested_loops(void) {
    volatile int control = 1;
    int result = 0;
    
    /* Four-level nested loop */
    for (int a = 0; a < 15 && control; a++) {
        volatile int level1_flag = (a % 3 == 0) ? 1 : 0;
        
        for (int b = 0; b < 25 && level1_flag; b++) {
            volatile int level2_flag = (rand() % 100 < 70) ? 1 : 0;
            
            for (int c = 0; c < 35 && level2_flag; c++) {
                volatile int level3_flag = (int_array[(a+b+c) % SIZE] % 5 == 0) ? 1 : 0;
                
                for (int d = 0; d < 45 && level3_flag; d++) {
                    int idx = (a * b * c * d) % SIZE;
                    
                    /* Very complex conditional structure */
                    if (level3_flag && mixed_array[idx].flag) {
                        result += int_array[idx] * (c + 1);
                        
                        if (level2_flag && (d % 11 == 0)) {
                            result -= int_array[(idx + 1) % SIZE] / 2;
                        }
                    } else if (level1_flag && (b % 7 == 0)) {
                        result += float_array[idx] > 0.5f ? 1 : -1;
                        
                        /* Nested conditional inside conditional */
                        if (control && (a % 4 == 0)) {
                            result *= (mixed_array[idx].tag - 'A' + 1);
                        }
                    }
                    
                    /* Mixed operations */
                    result = (result * 31 + 17) % 1000;
                    
                    /* Conditional store with data dependency */
                    if (result % 19 == 0) {
                        int_array[idx] = result;
                    }
                }
                
                /* Loop-carried dependency */
                if (c % 13 == 0) {
                    result += b * 7;
                }
            }
            
            /* Middle loop computation with volatile */
            if (level1_flag && control) {
                result -= a * 3;
            }
        }
        
        /* Outer loop adjustment */
        if (control) {
            result = (result + 1) & 0xFFF;
        }
    }
    
    return result;
}

/* Main function that combines all computations */
int main(void) {
    double total_result = 0.0;
    
    /* Initialize data */
    initialize_arrays();
    
    printf("Starting selective scheduling test...\n");
    
    /* Call all computation functions multiple times */
    for (int iteration = 0; iteration < 5; iteration++) {
        double reduction_result = reduction_with_carry();
        float mixed_result = process_mixed_types();
        int nested_result = complex_nested_loops();
        
        /* Combine results to ensure all computations are used */
        total_result += reduction_result + mixed_result + nested_result;
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < SIZE; i += 73) {
            int_array[i] = (int_array[i] * 3 + 1) % 1000;
            float_array[i] = float_array[i] * 1.1f;
        }
    }
    
    printf("Final combined result: %f\n", total_result);
    printf("Test completed.\n");
    
    return 0;
}
