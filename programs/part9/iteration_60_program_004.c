/* test_sel_sched_dump.c
 * Designed to trigger GCC's selective scheduler debug dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[100];

/* Function 1: Inner loop with conditional branch and memory write
 * This creates a scheduling region with data dependencies */
int func1_with_inner_loop(int *arr, int n) {
    int sum = 0;
    volatile int local_vol = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        local_vol = arr[i];
        
        /* Conditional branch inside loop */
        if (arr[i] > 0) {
            sum += arr[i] * 2;
            /* Memory write with dependency */
            arr[i] = sum % 100;
        } else {
            sum -= arr[i];
            arr[i] = -sum % 100;
        }
        
        /* Inline assembly to create specific RTL patterns */
        asm volatile ("" : : "r"(sum), "r"(arr[i]) : "memory");
    }
    
    /* Another loop with different pattern */
    for (int j = 0; j < n/2; j++) {
        int idx = j * 2;
        if (idx < n) {
            arr[idx] = (arr[idx] + sum) ^ j;
            /* Force memory barrier */
            asm volatile ("" : : : "memory");
        }
    }
    
    return sum;
}

/* Function 2: Nested loops with outer loop pipelining opportunity */
void func2_nested_loops(int *matrix, int rows, int cols) {
    volatile int temp = 0;
    
    /* Outer loop - target for outer loop pipelining */
    for (int i = 0; i < rows; i++) {
        int row_start = i * cols;
        int row_sum = 0;
        
        /* Inner loop with computation */
        for (int j = 0; j < cols; j++) {
            int idx = row_start + j;
            int val = matrix[idx];
            
            /* Complex conditional with multiple branches */
            if (val > 100) {
                matrix[idx] = val - 50;
                row_sum += matrix[idx];
            } else if (val < -100) {
                matrix[idx] = val + 50;
                row_sum -= matrix[idx];
            } else {
                matrix[idx] = val * 2;
                row_sum += matrix[idx] / 2;
            }
            
            /* Dependency chain */
            temp = row_sum;
            asm volatile ("" : : "r"(temp) : "memory");
        }
        
        /* Store result with volatile to prevent elimination */
        g_volatile_array[i % 100] = row_sum;
    }
    
    /* Post-processing loop */
    for (int k = 0; k < rows && k < 50; k++) {
        g_volatile_counter += matrix[k * cols];
    }
}

/* Function 3: Switch statement with computed goto-like behavior */
int func3_switch_complex(int x, int *results) {
    int result = 0;
    static volatile int switch_counter = 0;
    
    /* Multiple basic blocks from switch */
    switch (x % 7) {
        case 0:
            result = x * 2;
            results[0] = result;
            /* Force spill/reload */
            asm volatile ("" : : "r"(result) : "memory");
            break;
        case 1:
            result = x + x;
            for (int i = 0; i < 3; i++) {
                results[i] = result + i;
            }
            break;
        case 2:
            result = x >> 1;
            results[2] = result;
            /* Complex expression with multiple ops */
            result = (result * 3 + 7) / 2;
            break;
        case 3:
            result = x & 0xFF;
            /* Loop inside case */
            for (int j = 0; j < 5; j++) {
                results[j] = result ^ j;
            }
            break;
        case 4:
            result = -x;
            results[4] = result;
            /* Conditional inside case */
            if (result < 0) {
                result = -result;
                asm volatile ("" : : "r"(result) : "memory");
            }
            break;
        case 5:
            result = x * x;
            results[5] = result % 1000;
            break;
        default: /* case 6 */
            result = 100 - x;
            results[6] = result;
            /* Small loop */
            for (int k = 0; k < 2; k++) {
                results[k] += k;
            }
    }
    
    switch_counter++;
    return result;
}

/* Function 4: Mixed control flow with function calls */
int func4_mixed_control_flow(int seed, int iterations) {
    int array[50];
    int total = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 50; i++) {
        array[i] = (seed + i * 3) % 100;
    }
    
    /* Loop with early exit possibility */
    for (int iter = 0; iter < iterations; iter++) {
        int chunk_sum = 0;
        
        /* Process chunk with dependency */
        for (int j = 0; j < 25; j++) {
            int idx = (iter + j) % 50;
            int val = array[idx];
            
            /* Multiple conditionals */
            if (val > 75) {
                array[idx] = val - 25;
                chunk_sum += 3;
            } else if (val > 50) {
                array[idx] = val - 10;
                chunk_sum += 2;
                /* Inline asm for scheduling boundary */
                asm volatile ("" : : : "memory");
            } else if (val > 25) {
                array[idx] = val + 5;
                chunk_sum += 1;
            } else {
                array[idx] = val + 15;
            }
            
            /* Volatile access to prevent optimization */
            g_volatile_counter = chunk_sum;
        }
        
        total += chunk_sum;
        
        /* Conditional break */
        if (total > 1000) {
            /* Additional computation before break */
            for (int k = 0; k < 10; k++) {
                array[k] = total % (k + 1);
            }
            break;
        }
    }
    
    return total;
}

/* Function 5: Pointer chasing with unpredictable branches */
int func5_pointer_chasing(int *base, int steps) {
    int *current = base;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        /* Unpredictable branch */
        if ((*current & 1) == 0) {
            sum += *current;
            current = base + ((*current + i) % 50);
        } else {
            sum -= *current;
            current = base + ((*current - i) % 50);
        }
        
        /* Memory barrier */
        asm volatile ("" : : "r"(sum), "r"(current) : "memory");
        
        /* Additional computation with dependency */
        *current = (*current + sum) & 0xFF;
        
        /* Prevent tail recursion optimization */
        if (i % 10 == 0) {
            g_volatile_array[i % 100] = sum;
        }
    }
    
    return sum;
}

/* Main driver that ensures all functions are compiled */
int main(int argc, char **argv) {
    int test_array[100];
    int results[10];
    int matrix[10 * 20];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        test_array[i] = (i * 3 + 7) % 100;
    }
    
    for (int i = 0; i < 10 * 20; i++) {
        matrix[i] = (i * 5 - 10) % 200;
    }
    
    /* Call all test functions to ensure they're compiled */
    int r1 = func1_with_inner_loop(test_array, 100);
    func2_nested_loops(matrix, 10, 20);
    int r3 = func3_switch_complex(argc > 1 ? atoi(argv[1]) : 42, results);
    int r4 = func4_mixed_control_flow(r3, 20);
    int r5 = func5_pointer_chasing(test_array, 50);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", r1, r3, r4, r5);
    
    /* Access volatile globals */
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return 0;
}
