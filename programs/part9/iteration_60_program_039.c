/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[256];

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int n, int* arr) {
    int i, j;
    volatile int local_vol = 0;
    
    for (i = 0; i < n; i++) {
        /* Create data dependencies */
        int sum = 0;
        for (j = 0; j < 100; j++) {
            /* Complex expression with multiple operations */
            sum += arr[(i + j) % 256] * (j + 1);
            
            /* Conditional store to create control flow */
            if (sum > 1000) {
                arr[i] = sum % 256;
                /* Inline assembly to create specific RTL patterns */
                asm volatile ("" : : "r"(sum), "r"(arr[i]));
            }
        }
        
        /* Memory access with volatile */
        g_volatile_array[i % 256] = sum;
        local_vol = sum;
        
        /* Another conditional to create basic blocks */
        if (i % 3 == 0) {
            arr[i] = local_vol * 2;
        } else if (i % 3 == 1) {
            arr[i] = local_vol / 2;
        } else {
            arr[i] = local_vol + g_volatile_counter;
        }
    }
    
    /* Force side effect */
    g_volatile_counter += local_vol;
}

/* Function 2: Nested loops with different iteration counts */
int test_nested_loops(int rows, int cols, int (*matrix)[64]) {
    int i, j, k;
    int result = 0;
    volatile int temp = 0;
    
    /* Outer loop with pipelining opportunities */
    for (i = 0; i < rows; i++) {
        /* Middle loop */
        for (j = 0; j < cols; j++) {
            /* Inner loop with data dependency */
            int prod = 1;
            for (k = 0; k < 8; k++) {
                prod *= matrix[i][(j + k) % cols] + k;
                
                /* Conditional with function call to create complex CFG */
                if (prod > 1000000) {
                    prod = prod % 1000;
                    /* Inline asm to prevent optimization */
                    asm volatile ("# Dependency barrier" : : : "memory");
                }
            }
            
            /* Store with volatile */
            temp = prod;
            matrix[i][j] = temp;
            result += temp;
            
            /* Another conditional branch */
            if (temp < 0) {
                result -= temp;
                asm volatile ("" : : "r"(result));
            }
        }
        
        /* Update volatile global */
        g_volatile_counter += i;
    }
    
    return result;
}

/* Function 3: Switch statement with computed goto-like behavior */
int test_switch_complex(int x, int* outcomes) {
    int result = 0;
    volatile int selector = x;
    
    /* Multiple basic blocks from switch */
    switch (selector % 5) {
        case 0:
            result = x * 2;
            /* Memory access pattern */
            outcomes[0] = result;
            asm volatile ("" : : "r"(result), "m"(outcomes[0]));
            break;
            
        case 1:
            result = x + x;
            for (int i = 0; i < 4; i++) {
                outcomes[i] = result + i;
            }
            asm volatile ("# Case 1 operations" : : : "memory");
            break;
            
        case 2:
            result = x * x;
            /* Nested condition */
            if (result > 100) {
                outcomes[2] = result / 2;
                asm volatile ("" : : "r"(outcomes[2]));
            } else {
                outcomes[2] = result * 2;
            }
            break;
            
        case 3:
            /* Loop inside switch case */
            result = 1;
            for (int i = 1; i <= x % 10; i++) {
                result *= i;
                outcomes[3] = result;
            }
            asm volatile ("" : : "r"(result));
            break;
            
        case 4:
            result = x | 0xFF;
            outcomes[4] = result;
            /* Multiple memory operations */
            g_volatile_array[x % 256] = result;
            asm volatile ("# Final case" : : "r"(result), "m"(g_volatile_array[0]));
            break;
            
        default:
            result = -x;
            outcomes[5] = result;
    }
    
    /* Post-switch computation */
    result += g_volatile_counter;
    return result;
}

/* Function 4: Mixed control flow with function calls */
static int helper_func(int a, int b) {
    /* Non-inlineable function with side effects */
    volatile static int counter = 0;
    counter++;
    return (a * b) + counter;
}

void test_mixed_control_flow(int iterations, int* data) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        int val = data[i];
        
        /* Multiple conditionals */
        if (val < 0) {
            data[i] = helper_func(val, -1);
            asm volatile ("" : : "r"(data[i]));
        } else if (val == 0) {
            data[i] = g_volatile_counter;
            /* Loop inside conditional */
            for (int j = 0; j < 3; j++) {
                data[i + j] = j * 10;
            }
        } else if (val > 100) {
            data[i] = val / helper_func(2, val % 10);
            asm volatile ("# Large value path" : : : "memory");
        } else {
            data[i] = val * 2 + 1;
        }
        
        /* Update volatile */
        g_volatile_array[i % 256] = data[i];
        
        /* Another conditional at loop end */
        if (i % 7 == 0) {
            asm volatile ("" : : "r"(i), "m"(g_volatile_array[0]));
        }
    }
}

/* Main driver function */
int main(void) {
    int i;
    int array[256];
    int matrix[32][64];
    int outcomes[10];
    
    /* Initialize data */
    srand(time(NULL));
    for (i = 0; i < 256; i++) {
        array[i] = rand() % 100;
    }
    
    for (i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = rand() % 50;
        }
    }
    
    /* Call test functions to ensure they're compiled */
    test_inner_loop(128, array);
    
    int result1 = test_nested_loops(16, 64, matrix);
    
    for (i = 0; i < 10; i++) {
        outcomes[i] = 0;
    }
    int result2 = test_switch_complex(42, outcomes);
    
    test_mixed_control_flow(100, array);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, volatile counter: %d\n", 
           result1, result2, g_volatile_counter);
    
    return 0;
}
