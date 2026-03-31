/* test_sel_sched_dump.c
 * Designed to trigger GCC's selective scheduler debug dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test_sel_sched_dump.c
 * Or: gcc -O3 -fsel-sched-pipelining-outer-loops -dS -fdump-rtl-all -c test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
volatile int g_volatile_counter = 0;

/* Function 1: Inner loop with conditional branch and memory write
 * Creates data dependencies that require careful scheduling */
int func_inner_loop(int *arr, int n) {
    int sum = 0;
    volatile int barrier = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        barrier = i;
        
        /* Complex conditional with side effect */
        if (arr[i] > 0) {
            sum += arr[i] * 2;
            /* Memory write with dependency */
            arr[i] = sum % 100;
        } else {
            sum -= arr[i];
            arr[i] = -sum % 50;
        }
        
        /* Inline asm to create unschedulable dependency */
        asm volatile ("" : : "r"(sum) : "memory");
    }
    
    /* Another asm barrier */
    asm volatile ("" : : : "memory");
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Outer loop pipelining candidate */
int func_nested_loops(int *matrix, int rows, int cols) {
    int total = 0;
    volatile int row_barrier = 0;
    
    /* Outer loop - candidate for outer loop pipelining */
    for (int r = 0; r < rows; r++) {
        row_barrier = r;
        int row_sum = 0;
        
        /* Inner loop with stride access */
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            
            /* Complex expression with multiple operations */
            int val = matrix[idx];
            val = (val * 3 + 7) % 256;
            
            /* Conditional with early exit possibility */
            if (val > 128) {
                row_sum += val >> 1;
                matrix[idx] = row_sum & 0xFF;
            } else if (val < 64) {
                row_sum -= val;
                matrix[idx] = (255 - row_sum) & 0xFF;
            } else {
                row_sum += val;
                matrix[idx] = val;
            }
            
            /* Dependency chain */
            asm volatile ("" : "+r"(row_sum) : : "memory");
        }
        
        total += row_sum;
        
        /* Cross-iteration dependency */
        if (r > 0) {
            asm volatile ("" : : "r"(total), "r"(row_barrier) : "memory");
        }
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto-like behavior
 * Creates complex control flow graph */
int func_switch_complex(int x, int *results) {
    int result = 0;
    static volatile int state = 0;
    
    /* Multiple basic blocks with switches */
    switch (x % 4) {
        case 0:
            result = x * 2;
            /* Loop inside case */
            for (int i = 0; i < 3; i++) {
                result += i;
                asm volatile ("" : "+r"(result) : : );
            }
            break;
            
        case 1:
            result = x + 100;
            /* Conditional inside case */
            if (x > 50) {
                result *= 3;
                asm volatile ("" : : "r"(result) : "memory");
            } else {
                result /= 2;
            }
            break;
            
        case 2:
            /* Nested switch */
            switch (x % 3) {
                case 0: result = x << 2; break;
                case 1: result = x | 0xF0; break;
                case 2: result = x & 0x3F; break;
            }
            /* Memory access pattern */
            for (int i = 0; i < 5; i++) {
                results[i] = result + i;
                asm volatile ("" : : "r"(results[i]) : "memory");
            }
            break;
            
        case 3:
            result = -x;
            /* Small loop with dependency */
            for (int i = 0; i < 4; i++) {
                result = (result * 13 + 7) % 100;
                state = i;  /* volatile access */
            }
            break;
    }
    
    return result;
}

/* Function 4: Mixed operations with function calls
 * Creates scheduling barriers */
int helper1(int a, int b) {
    return (a * b) % 100;
}

int helper2(int a) {
    volatile int v = a;
    return v * 2 + 1;
}

int func_mixed_ops(int a, int b, int c) {
    int r1, r2, r3;
    
    /* Sequence with true dependencies */
    r1 = helper1(a, b);
    asm volatile ("" : : "r"(r1) : );
    
    r2 = helper2(r1);
    asm volatile ("" : "+r"(r2) : : "memory");
    
    /* Loop with function call */
    r3 = 0;
    for (int i = 0; i < c; i++) {
        r3 += helper1(r2, i);
        if (i % 2 == 0) {
            r3 = helper2(r3);
        }
        asm volatile ("" : "+r"(r3) : : );
    }
    
    /* Final computation */
    return r1 + r2 + r3;
}

/* Function 5: Pointer chasing with conditional
 * Creates memory dependency chain */
int func_pointer_chase(int *base, int steps) {
    int *ptr = base;
    int sum = 0;
    volatile int step_counter = 0;
    
    for (int i = 0; i < steps; i++) {
        step_counter = i;
        
        /* Pointer chase with offset */
        int offset = *ptr % 16;
        ptr = base + offset;
        
        /* Conditional based on value */
        if (*ptr > 100) {
            sum += *ptr;
            *ptr = sum % 256;
        } else {
            sum -= *ptr;
            *ptr = (255 - sum) % 256;
        }
        
        /* Strong memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    return sum;
}

/* Main driver function - ensures all code paths are compiled */
int main(int argc, char **argv) {
    /* Initialize test data */
    int arr1[100];
    int arr2[100];
    int matrix[10][10];
    int results[10];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 100; i++) {
        arr1[i] = (i * 13 + 7) % 100;
        arr2[i] = (i * 17 + 11) % 100;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = (i * 10 + j) % 100;
        }
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += func_inner_loop(arr1, 100);
    result += func_nested_loops(&matrix[0][0], 10, 10);
    result += func_switch_complex(argc, results);
    result += func_mixed_ops(argc, 7, 5);
    result += func_pointer_chase(arr2, 50);
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = result;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
