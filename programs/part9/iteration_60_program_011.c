/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumps
 * when compiled with appropriate flags. The runtime behavior is secondary;
 * the primary goal is to cause GCC to execute the uncovered debug dump
 * lines in sel-sched-dump.cc during compilation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent dead code elimination */
volatile int global_seed = 42;

/* Function 1: Inner loop with conditional branch and memory write
 * This creates a scheduling region with data dependencies and control flow
 */
void func1_inner_loop(int *arr, int n) {
    int i, sum = 0;
    volatile int barrier = 0;
    
    /* Loop with data dependency chain */
    for (i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        barrier = arr[i];
        
        /* Conditional store with dependency */
        if (arr[i] > 0) {
            sum += arr[i];
            arr[i] = sum;
        } else {
            sum -= arr[i];
            arr[i] = -sum;
        }
        
        /* Inline assembly to create specific RTL patterns */
        asm volatile ("" : : "r"(sum), "r"(arr[i]) : "memory");
    }
    
    /* Final store with side effect */
    arr[0] = sum;
    asm volatile ("" : : "r"(arr[0]) : "memory");
}

/* Function 2: Nested loops with different iteration counts
 * Creates outer loop scheduling opportunities
 */
int func2_nested_loops(int rows, int cols) {
    int i, j, total = 0;
    volatile int matrix[32][32];  /* Small fixed size to avoid stack overflow */
    
    /* Initialize with volatile to prevent optimization */
    for (i = 0; i < rows && i < 32; i++) {
        for (j = 0; j < cols && j < 32; j++) {
            matrix[i][j] = (i * 17 + j * 13) % 97;
        }
    }
    
    /* Nested loops with cross-iteration dependency */
    for (i = 1; i < rows && i < 32; i++) {
        int row_sum = 0;
        for (j = 1; j < cols && j < 32; j++) {
            /* Complex dependency chain */
            int val = matrix[i][j] + matrix[i-1][j] + matrix[i][j-1];
            row_sum += val;
            matrix[i][j] = row_sum;
            
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : "r"(val), "r"(row_sum) : "memory");
        }
        total += row_sum;
    }
    
    /* Conditional return based on computation */
    if (total > 1000) {
        asm volatile ("" : : "r"(total) : "memory");
        return total % 256;
    } else {
        asm volatile ("" : : "r"(total) : "memory");
        return (total * 3) % 256;
    }
}

/* Function 3: Switch statement with computed goto-like pattern
 * Creates complex control flow for the scheduler
 */
int func3_complex_control_flow(int mode, int iterations) {
    int i, result = 0;
    volatile int state = 0;
    
    /* Loop with switch inside */
    for (i = 0; i < iterations; i++) {
        switch (mode) {
            case 0:
                result += i * 2;
                state = result % 7;
                break;
            case 1:
                result -= i * 3;
                state = result % 11;
                break;
            case 2:
                result ^= i * 5;
                state = result % 13;
                break;
            case 3:
                result |= i * 7;
                state = result % 17;
                break;
            default:
                result &= i * 11;
                state = result % 19;
                break;
        }
        
        /* Mode transition based on state */
        mode = (mode + state) % 5;
        
        /* Force dependency */
        asm volatile ("" : : "r"(result), "r"(state), "r"(mode) : "memory");
    }
    
    return result;
}

/* Function 4: Mixed operations with pointer aliasing
 * Creates scheduling challenges due to potential aliasing
 */
void func4_mixed_ops(int *a, int *b, int *c, int n) {
    int i;
    volatile int acc = 0;
    
    for (i = 0; i < n; i++) {
        /* Potential pointer aliasing creates memory dependencies */
        a[i] = b[i] * 3 + c[i] * 7;
        
        /* Cross-iteration dependency through accumulator */
        acc += a[i];
        b[i] = acc;
        
        /* Conditional update of c based on acc */
        if (acc & 1) {
            c[i] = acc ^ b[i];
        } else {
            c[i] = acc | b[i];
        }
        
        /* Memory barrier with all live values */
        asm volatile ("" : : "r"(a[i]), "r"(b[i]), "r"(c[i]), "r"(acc) : "memory");
    }
}

/* Function 5: Recursive-like pattern implemented iteratively
 * Creates complex data flow graph
 */
int func5_data_flow_pattern(int n) {
    int i, x = 1, y = 1, z = 1;
    volatile int temp;
    
    for (i = 0; i < n; i++) {
        /* Fibonacci-like recurrence with twists */
        temp = x + y + z;
        x = y;
        y = z;
        z = temp;
        
        /* Conditional break */
        if (z > 1000000) {
            z = z % 1000;
            asm volatile ("" : : "r"(z) : "memory");
        }
        
        /* Periodic operation */
        if (i % 4 == 0) {
            x ^= y;
            y |= z;
            z &= x;
        }
        
        /* Force all values to be live */
        asm volatile ("" : : "r"(x), "r"(y), "r"(z) : "memory");
    }
    
    return x + y + z;
}

/* Main driver function that ensures all code paths are compiled */
int main(int argc, char **argv) {
    int arr1[20];
    int arr2[20];
    int arr3[20];
    int i, result = 0;
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 20; i++) {
        arr1[i] = (i * 3) % 19;
        arr2[i] = (i * 5) % 23;
        arr3[i] = (i * 7) % 29;
    }
    
    /* Call all test functions to ensure they're compiled */
    func1_inner_loop(arr1, 20);
    result += func2_nested_loops(8, 8);
    result += func3_complex_control_flow(argc > 1 ? atoi(argv[1]) % 5 : 0, 15);
    func4_mixed_ops(arr1, arr2, arr3, 10);
    result += func5_data_flow_pattern(12);
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = result;
    asm volatile ("" : : "r"(final_result) : "memory");
    
    printf("Result: %d\n", result);
    return 0;
}
