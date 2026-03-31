/* test_sel_sched_dump.c
 * 
 * This code is designed to trigger GCC's selective scheduler debug dumps
 * when compiled with specific flags. The goal is to reach the uncovered
 * lines in sel-sched-dump.cc that handle RTL instruction dumping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Use volatile to prevent dead code elimination */
volatile int global_seed = 42;
volatile int global_result = 0;

/* Function 1: Inner loop with conditional branch and memory write
 * This creates data dependencies that challenge the scheduler */
int func_inner_loop(int *arr, int n) {
    int sum = 0;
    volatile int temp = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        temp = arr[i];
        
        /* Conditional store to create control flow */
        if (temp > 100) {
            arr[i] = temp / 2;
            sum += arr[i];
        } else {
            arr[i] = temp * 2;
            sum -= arr[i];
        }
        
        /* Inline assembly to create unschedulable dependency */
        asm volatile ("" : : "r"(temp) : "memory");
    }
    
    /* Another dependency chain */
    for (int i = n - 1; i >= 0; i--) {
        sum += arr[i] * i;
        asm volatile ("" : : "r"(sum));
    }
    
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Creates outer loop pipelining opportunities */
int func_nested_loops(int *matrix, int rows, int cols) {
    int total = 0;
    volatile int row_sum = 0;
    
    /* Outer loop - target for outer loop pipelining */
    for (int r = 0; r < rows; r++) {
        row_sum = 0;
        
        /* Inner loop with stride access */
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            
            /* Complex addressing mode */
            int val = matrix[idx] + (r * c);
            
            /* Conditional with multiple paths */
            if (val % 3 == 0) {
                matrix[idx] = val >> 1;
                row_sum += matrix[idx];
            } else if (val % 3 == 1) {
                matrix[idx] = val << 1;
                row_sum -= matrix[idx];
            } else {
                matrix[idx] = val + global_seed;
                row_sum ^= matrix[idx];
            }
            
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : "r"(val), "m"(matrix[idx]));
        }
        
        total += row_sum;
        
        /* Cross-iteration dependency */
        if (r > 0) {
            total -= matrix[(r-1) * cols];
        }
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto-like behavior
 * Creates complex control flow graph */
int func_switch_complex(int x, int *results) {
    int ret = 0;
    static volatile int jump_table[8] = {0};
    
    /* Initialize jump table */
    for (int i = 0; i < 8; i++) {
        jump_table[i] = i * i;
    }
    
    /* Switch with many cases - creates multiple basic blocks */
    switch (x & 7) {
        case 0:
            ret = jump_table[0] + x;
            asm volatile ("" : : "r"(ret));
            break;
        case 1:
            ret = jump_table[1] - x;
            /* Fall through to create edge in CFG */
        case 2:
            ret += jump_table[2] * x;
            asm volatile ("" : : "r"(ret), "r"(x));
            break;
        case 3:
            for (int i = 0; i < 4; i++) {
                ret += jump_table[i] >> x;
            }
            break;
        case 4:
            ret = jump_table[4] & x;
            if (ret > 100) goto special_case;
            break;
        case 5:
            ret = jump_table[5] | x;
            /* Loop inside switch case */
            for (int i = 0; i < 3; i++) {
                results[i] = ret + i;
                asm volatile ("" : : "r"(results[i]));
            }
            break;
        case 6:
            ret = jump_table[6] ^ x;
            /* Nested conditional */
            if (x > 0) {
                if (x < 100) {
                    ret *= 2;
                } else {
                    ret /= 2;
                }
            }
            break;
        case 7:
            ret = -jump_table[7];
            break;
        default:
            ret = 0;
    }
    
    return ret;
    
special_case:
    return ret * 2;
}

/* Function 4: Mixed control flow with function calls
 * Creates scheduling barriers */
int func_mixed_flow(int a, int b, int c) {
    volatile int x = a;
    volatile int y = b;
    volatile int z = c;
    int result = 0;
    
    /* Loop with early exit */
    for (int i = 0; i < 100; i++) {
        x = (x * 13 + 17) % 100;
        y = (y * 7 + 23) % 100;
        
        /* Multiple condition checks */
        if (x > y) {
            result += x - y;
            asm volatile ("" : : "r"(result));
        } else if (x < y) {
            result += y - x;
            /* Inline asm with multiple inputs */
            asm volatile ("" : : "r"(x), "r"(y), "r"(result));
        } else {
            result += z;
            z = (z + 1) % 50;
        }
        
        /* Break with condition - creates unpredictable branch */
        if (result > 1000) {
            break;
        }
    }
    
    /* Post-loop computation */
    result = (result * a) / (b + 1);
    
    return result;
}

/* Function 5: Pointer chasing with indirect memory access
 * Creates memory dependency chains */
int func_pointer_chase(int *base, int steps) {
    volatile int *current = base;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        /* Read through pointer */
        int val = *current;
        
        /* Modify based on value */
        if (val & 1) {
            *current = val + sum;
            sum += val * 2;
        } else {
            *current = val - sum;
            sum -= val / 2;
        }
        
        /* Move pointer - creates address generation dependency */
        current = base + (val % 16);
        
        /* Memory barrier */
        asm volatile ("" : : "r"(val), "m"(*current));
    }
    
    return sum;
}

/* Main driver that ensures all code paths are compiled */
int main(int argc, char **argv) {
    /* Initialize test data */
    int arr1[100];
    int arr2[10][10];
    int results[10];
    
    srand(time(NULL));
    
    /* Fill arrays with random data */
    for (int i = 0; i < 100; i++) {
        arr1[i] = rand() % 200;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr2[i][j] = rand() % 100;
        }
    }
    
    /* Call all test functions to ensure they're compiled */
    int r1 = func_inner_loop(arr1, 100);
    int r2 = func_nested_loops(&arr2[0][0], 10, 10);
    int r3 = func_switch_complex(rand() % 10, results);
    int r4 = func_mixed_flow(rand() % 50, rand() % 50, rand() % 50);
    int r5 = func_pointer_chase(arr1, 50);
    
    /* Use results to prevent dead code elimination */
    global_result = r1 + r2 + r3 + r4 + r5;
    
    printf("Result: %d\n", global_result);
    
    return 0;
}
