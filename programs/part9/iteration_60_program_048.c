/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumps
 * when compiled with appropriate flags. The goal is to reach the uncovered
 * lines in sel-sched-dump.cc that handle RTL instruction dumping.
 *
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test_sel_sched_dump.c
 * Or: gcc -O3 -fsel-sched-pipelining-outer-loops -dS -fdump-rtl-all -c test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization and create scheduling barriers */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[256];

/* Function 1: Inner loop with conditional branch and memory write
 * Creates data dependencies that challenge the scheduler */
int test_inner_loop(int n, int* arr) {
    int sum = 0;
    volatile int barrier = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        barrier = i;
        
        /* Complex conditional with side effect */
        if (i % 3 == 0) {
            arr[i] = arr[i] * 2 + 1;
            sum += arr[i];
            
            /* Inline assembly to create specific RTL patterns */
            asm volatile ("# Inner loop conditional" : : "r"(arr[i]));
        } else if (i % 3 == 1) {
            arr[i] = arr[i] / 2 - 1;
            sum -= arr[i];
            
            /* Memory barrier-like effect */
            asm volatile ("# Else branch" : : "r"(sum));
        } else {
            /* Complex computation with multiple operations */
            int temp = arr[i] * arr[i];
            arr[i] = temp % 100;
            sum ^= temp;
            
            /* Force register pressure */
            asm volatile ("# Complex else" : : "r"(temp), "r"(arr[i]), "r"(sum));
        }
        
        /* Volatile store to prevent dead code elimination */
        g_volatile_array[i & 255] = sum;
    }
    
    /* Final computation with dependency on loop result */
    for (int i = 0; i < 10; i++) {
        sum = (sum * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Designed for outer loop pipelining */
int test_nested_loops(int rows, int cols, int* matrix) {
    int total = 0;
    volatile int row_sum = 0;
    
    /* Outer loop - target for pipelining */
    for (int r = 0; r < rows; r++) {
        row_sum = 0;
        
        /* Inner loop with stride access */
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            
            /* Complex addressing mode */
            int val = matrix[idx] + (r * c);
            
            /* Conditional store with side effect */
            if (val > 0) {
                matrix[idx] = val;
                row_sum += val;
                
                /* Inline asm to create scheduling boundaries */
                asm volatile ("# Nested loop store" : : "r"(val), "r"(idx));
            } else {
                matrix[idx] = -val;
                row_sum -= val;
                
                /* Different asm template */
                asm volatile ("# Nested loop negate" : : "r"(-val));
            }
            
            /* Periodic volatile update */
            if ((c & 7) == 0) {
                g_volatile_counter = row_sum;
            }
        }
        
        /* Cross-iteration dependency */
        total += row_sum * (r + 1);
        
        /* Function call to create control flow complexity */
        if (total > 1000000) {
            total = total % 1000000;
        }
    }
    
    /* Post-processing loop */
    for (int i = 0; i < cols; i++) {
        total ^= matrix[i];
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto
 * Creates complex control flow for the scheduler */
int test_switch_complex(int mode, int iterations) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3, 
        &&case_4, &&case_5, &&case_default
    };
    
    int result = 0;
    volatile int state = mode;
    
    for (int i = 0; i < iterations; i++) {
        /* Computed goto - creates complex control flow in RTL */
        goto *jump_table[state % 7];
        
    case_0:
        result += i * 2;
        asm volatile ("# Case 0" : : "r"(result));
        state = (state + 1) ^ result;
        continue;
        
    case_1:
        result -= i * 3;
        asm volatile ("# Case 1" : : "r"(i));
        state = (state * 2) & 0xFF;
        continue;
        
    case_2:
        result ^= i;
        asm volatile ("# Case 2" : : "r"(result), "r"(i));
        state = (state + i) % 7;
        continue;
        
    case_3:
        result = result * 3 - i;
        /* Memory operation */
        g_volatile_array[i & 255] = result;
        state = (state >> 1) | 0x40;
        continue;
        
    case_4:
        result = (result << 3) | (i & 7);
        asm volatile ("# Case 4" : : "r"(result));
        state = state ^ 0x55;
        continue;
        
    case_5:
        result = ~result;
        /* Multiple volatile accesses */
        g_volatile_counter = i;
        state = (state + g_volatile_counter) % 7;
        continue;
        
    case_default:
        result = 0;
        asm volatile ("# Default case" : : "r"(i));
        state = mode;
        continue;
    }
    
    return result;
}

/* Function 4: Mixed operations with pointer chasing
 * Creates unpredictable data flow */
int test_pointer_chasing(int size, int* data) {
    int sum = 0;
    int* current = data;
    volatile int steps = 0;
    
    /* Create linked-list-like access pattern */
    for (int i = 0; i < size && current != NULL; i++) {
        /* Load with potential aliasing */
        int val = *current;
        
        /* Complex computation chain */
        val = ((val << 5) | (val >> 27)) ^ 0xDEADBEEF;
        
        /* Conditional store back */
        if (val > 0) {
            *current = val;
            sum += val;
            
            asm volatile ("# Pointer store positive" : : "r"(val));
        } else {
            *current = -val;
            sum -= val;
            
            asm volatile ("# Pointer store negative" : : "r"(-val));
        }
        
        /* Pointer arithmetic with dependency */
        int offset = (val & 0xF) + 1;
        current = data + ((current - data + offset) % size);
        
        steps++;
        g_volatile_counter = steps;
    }
    
    return sum;
}

/* Function 5: Recursive-like pattern using loop
 * Creates back-edge dependencies */
int test_recursive_pattern(int n, int* buffer) {
    int a = 1, b = 1;
    volatile int temp;
    
    if (n <= 0) return 0;
    
    buffer[0] = a;
    if (n > 1) buffer[1] = b;
    
    /* Loop with serial dependency (like Fibonacci) */
    for (int i = 2; i < n; i++) {
        temp = a + b;
        a = b;
        b = temp;
        
        /* Overflow check with branch */
        if (temp < 0) {
            temp = 0;
            asm volatile ("# Overflow reset" : : "r"(temp));
        }
        
        buffer[i] = temp;
        
        /* Periodic external side effect */
        if ((i & 15) == 0) {
            g_volatile_array[i & 255] = temp;
        }
    }
    
    /* Final reduction */
    int result = 0;
    for (int i = 0; i < n; i++) {
        result ^= buffer[i];
        result = (result << 1) | (result >> 31);
    }
    
    return result;
}

/* Main driver function - ensures all code paths are compiled */
int main(int argc, char** argv) {
    /* Initialize test data */
    int array1[100];
    int matrix[10][10];
    int buffer[50];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 100; i++) {
        array1[i] = (i * 37) & 0xFF;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = (i * 11 + j * 13) & 0xFF;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        buffer[i] = i;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += test_inner_loop(50, array1);
    result += test_nested_loops(8, 8, &matrix[0][0]);
    result += test_switch_complex(argc > 1 ? atoi(argv[1]) : 3, 20);
    result += test_pointer_chasing(40, buffer);
    result += test_recursive_pattern(30, buffer);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
