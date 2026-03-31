/* test_sel_sched_dump.c
 * Program to trigger selective scheduler debug dumps in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[100];

/* Function 1: Inner loop with conditional branch and memory write
 * This creates a scheduling region with data dependencies */
int func_inner_loop(int *arr, int n) {
    int sum = 0;
    volatile int local_vol = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        local_vol = arr[i];
        
        /* Conditional store that creates control flow */
        if (arr[i] > 0) {
            sum += arr[i];
            /* Memory write with side effect */
            g_volatile_array[i % 100] = sum;
        } else {
            sum -= arr[i];
            /* Different memory write path */
            g_volatile_array[(i + 50) % 100] = sum;
        }
        
        /* Inline assembly to create unschedulable dependency */
        asm volatile ("" : : "r"(sum) : "memory");
    }
    
    /* Force another basic block with computation */
    if (sum > 1000) {
        for (int j = 0; j < 10; j++) {
            sum += j * local_vol;
            asm volatile ("" : : "r"(sum));
        }
    }
    
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Creates outer loop pipelining opportunities */
int func_nested_loops(int rows, int cols, int *matrix) {
    int total = 0;
    volatile int row_sum = 0;
    
    /* Outer loop - target for outer loop pipelining */
    for (int i = 0; i < rows; i++) {
        row_sum = 0;
        
        /* Inner loop with dependency chain */
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            /* Complex addressing mode */
            int val = matrix[idx] + (i * j);
            
            /* Conditional with both paths used */
            if (val % 2 == 0) {
                row_sum += val * 2;
            } else {
                row_sum += val / 2;
            }
            
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : "r"(row_sum), "m"(matrix[idx]));
        }
        
        total += row_sum;
        
        /* Volatile store to global */
        g_volatile_counter = total;
    }
    
    /* Post-loop computation */
    for (int k = 0; k < rows; k++) {
        total -= k;
        asm volatile ("" : : "r"(total));
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto-like pattern
 * Creates complex control flow graph */
int func_switch_complex(int x) {
    int result = 0;
    static volatile int state = 0;
    
    /* Switch with multiple cases */
    switch (x % 5) {
        case 0:
            result = x * 2;
            /* Function call simulation */
            asm volatile ("# case 0" : : : "memory");
            break;
        case 1:
            result = x + 100;
            /* Memory operation */
            state = result;
            asm volatile ("# case 1" : : "r"(result));
            break;
        case 2:
            /* Loop inside case */
            for (int i = 0; i < x % 10; i++) {
                result += i * i;
                asm volatile ("" : : "r"(result));
            }
            break;
        case 3:
            /* Nested condition */
            if (x > 0) {
                result = x * x;
                g_volatile_array[x % 100] = result;
            } else {
                result = -x;
                g_volatile_counter = result;
            }
            break;
        case 4:
            /* Another loop variant */
            result = 1;
            for (int j = 2; j <= (x % 20); j++) {
                result *= j;
                asm volatile ("" : : "r"(result));
            }
            break;
        default:
            result = -1;
    }
    
    /* Final computation with volatile */
    result += state;
    asm volatile ("" : : "r"(result), "m"(state));
    
    return result;
}

/* Function 4: Mixed operations with pointer chasing
 * Creates memory dependency chains */
int func_mixed_ops(int *data, int size) {
    int *ptr = data;
    int accum = 0;
    volatile int temp = 0;
    
    /* Pointer chasing loop */
    for (int i = 0; i < size; i++) {
        /* Load with potential aliasing */
        int val = *ptr;
        
        /* Arithmetic with dependency */
        accum = accum * 3 + val;
        
        /* Conditional pointer update */
        if (accum % 2 == 0) {
            ptr = &data[(i + 1) % size];
        } else {
            ptr = &data[(i + 2) % size];
        }
        
        /* Store to volatile */
        temp = accum;
        
        /* Barrier */
        asm volatile ("" : : "r"(accum), "r"(ptr), "m"(*ptr));
    }
    
    /* Another loop with different pattern */
    for (int j = 0; j < size / 2; j++) {
        accum -= data[j];
        asm volatile ("" : : "r"(accum));
    }
    
    return accum;
}

/* Function 5: Recursive-like pattern using loops
 * Simulates complex control flow */
int func_recursive_pattern(int n, int *cache) {
    int a = 1, b = 1;
    volatile int step = 0;
    
    /* Loop simulating recursive computation */
    for (int i = 2; i <= n; i++) {
        int next = a + b;
        
        /* Store to array with indexing */
        if (cache) {
            cache[i] = next;
            /* Dependency on memory */
            asm volatile ("" : : "m"(cache[i]));
        }
        
        /* Update with dependency chain */
        b = a;
        a = next;
        
        /* Conditional with side effect */
        if (i % 3 == 0) {
            step = next;
            g_volatile_counter = i;
        }
        
        /* Compiler barrier */
        asm volatile ("" : : "r"(a), "r"(b));
    }
    
    /* Final computation */
    int result = a;
    for (int j = 0; j < 5; j++) {
        result += j * step;
        asm volatile ("" : : "r"(result));
    }
    
    return result;
}

/* Main driver function - ensures all code paths are compiled */
int main(int argc, char **argv) {
    int test_array[200];
    int matrix[100];
    int cache[50];
    
    /* Initialize test data */
    for (int i = 0; i < 200; i++) {
        test_array[i] = (i * 37) % 100 - 50;  /* Mix of positive and negative */
    }
    
    for (int i = 0; i < 100; i++) {
        matrix[i] = i * i;
    }
    
    for (int i = 0; i < 50; i++) {
        cache[i] = 0;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result1 = func_inner_loop(test_array, 100);
    int result2 = func_nested_loops(10, 10, matrix);
    int result3 = func_switch_complex(argc > 1 ? atoi(argv[1]) : 42);
    int result4 = func_mixed_ops(test_array, 50);
    int result5 = func_recursive_pattern(20, cache);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = 
        result1 + result2 + result3 + result4 + result5;
    
    /* Print to ensure code isn't optimized away entirely */
    printf("Results: %d %d %d %d %d\n", 
           result1, result2, result3, result4, result5);
    
    return final_result != 0 ? 0 : 1;
}
