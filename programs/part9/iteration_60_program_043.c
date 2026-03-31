/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_result = 0;

/* Function 1: Inner loop with conditional branch and memory write
   This creates a scheduling region with data dependencies */
void test_inner_loop(int *arr, int n) {
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        local_volatile = i;
        
        /* Conditional store with memory access */
        if (i % 3 == 0) {
            arr[i] = arr[i] * 2 + 1;
        } else {
            arr[i] = arr[i] / 2;
        }
        
        /* Sum with dependency */
        sum += arr[i];
        
        /* Inline assembly to create specific RTL patterns */
        asm volatile ("" : : "r"(sum), "r"(arr[i]) : "memory");
    }
    
    g_volatile_result = sum;
    
    /* Another loop with different pattern */
    for (int i = n - 1; i >= 0; i--) {
        arr[i] += sum;
        /* Force memory barrier */
        asm volatile ("" : : : "memory");
    }
}

/* Function 2: Nested loops with different iteration counts
   Creates outer loop scheduling opportunities */
int test_nested_loops(int rows, int cols, int (*matrix)[10]) {
    int total = 0;
    volatile int row_sum = 0;
    
    /* Outer loop - target for outer loop pipelining */
    for (int r = 0; r < rows; r++) {
        row_sum = 0;
        
        /* Inner loop with complex addressing */
        for (int c = 0; c < cols; c++) {
            /* Create address calculation dependency */
            int idx = (r * cols + c) % (rows * cols);
            
            /* Conditional update with multiple operations */
            if (matrix[r][c] > 0) {
                matrix[r][c] = matrix[r][c] * 3 - 2;
                row_sum += matrix[r][c];
                
                /* Inline asm to prevent reordering */
                asm volatile ("# Dependency barrier" : : "r"(matrix[r][c]));
            } else {
                matrix[r][c] = matrix[r][c] / 2 + 1;
                row_sum -= matrix[r][c];
            }
            
            /* Additional computation to create more instructions */
            matrix[r][c] ^= 0xFF;
        }
        
        total += row_sum;
        
        /* Volatile store to prevent dead code elimination */
        g_volatile_counter = r;
    }
    
    /* Final computation with loop */
    for (int i = 0; i < rows; i++) {
        total ^= matrix[i][0];
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto for complex control flow */
int test_switch_complex(int mode, int iterations) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3, &&case_default
    };
    
    int result = 0;
    volatile int state = mode;
    
    for (int i = 0; i < iterations; i++) {
        /* Computed goto creates complex control flow graph */
        goto *jump_table[state % 5];
        
    case_0:
        result += i * 2;
        state = (state + 1) % 5;
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        continue;
        
    case_1:
        result -= i / 2;
        state = (state * 3 + 1) % 5;
        /* Force dependency */
        asm volatile ("" : : "r"(result));
        continue;
        
    case_2:
        result ^= 0xABCD;
        state = (state + 7) % 5;
        continue;
        
    case_3:
        result = result << 2;
        state = (state * 2) % 5;
        /* Another memory operation */
        asm volatile ("" : : : "memory");
        continue;
        
    case_default:
        result |= 0xFF00;
        state = 0;
        continue;
    }
    
    return result;
}

/* Function 4: Mixed operations with function calls in loop */
int test_mixed_ops(int n) {
    int* buffer = malloc(n * sizeof(int));
    if (!buffer) return -1;
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        buffer[i] = i * i - i;
    }
    
    int acc = 0;
    volatile int check = 0;
    
    /* Loop with mixed operations */
    for (int i = 1; i < n - 1; i++) {
        /* Load multiple values */
        int prev = buffer[i - 1];
        int curr = buffer[i];
        int next = buffer[i + 1];
        
        /* Complex calculation with dependencies */
        int temp = (prev * curr) / (next + 1);
        temp = temp ^ (prev + curr);
        
        /* Conditional update */
        if (temp % 2 == 0) {
            buffer[i] = temp << 1;
            acc += temp;
        } else {
            buffer[i] = temp >> 1;
            acc -= temp;
        }
        
        /* Volatile check */
        check = i;
        asm volatile ("# Mixed ops barrier" : : "r"(acc), "r"(buffer[i]));
    }
    
    /* Final reduction */
    int final = 0;
    for (int i = 0; i < n; i++) {
        final ^= buffer[i];
    }
    
    free(buffer);
    return final + acc;
}

/* Function 5: Pointer chasing loop - creates memory dependency chain */
int test_pointer_chase(int *data, int size, int steps) {
    int *ptr = data;
    int sum = 0;
    volatile int step_counter = 0;
    
    for (int s = 0; s < steps; s++) {
        /* Chase pointer through array */
        int idx = (*ptr) % size;
        ptr = &data[idx];
        
        /* Modify data */
        *ptr = (*ptr + s) ^ 0x1234;
        sum += *ptr;
        
        /* Inline asm for dependency */
        asm volatile ("" : : "r"(ptr), "r"(*ptr));
        
        step_counter = s;
    }
    
    return sum;
}

/* Main driver that ensures all code paths are compiled */
int main(int argc, char **argv) {
    int test_size = 100;
    int matrix[10][10];
    
    /* Initialize test data */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    int *array = malloc(test_size * sizeof(int));
    for (int i = 0; i < test_size; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(array, test_size);
    
    int result1 = test_nested_loops(10, 10, matrix);
    
    int result2 = test_switch_complex(argc > 1 ? atoi(argv[1]) : 2, 50);
    
    int result3 = test_mixed_ops(test_size);
    
    int *chase_data = malloc(test_size * sizeof(int));
    for (int i = 0; i < test_size; i++) {
        chase_data[i] = (i * 7) % test_size;
    }
    int result4 = test_pointer_chase(chase_data, test_size, 100);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", result1, result2, result3, result4);
    
    free(array);
    free(chase_data);
    
    return 0;
}
