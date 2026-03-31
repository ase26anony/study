/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumps
 * when compiled with appropriate flags. The runtime behavior is secondary;
 * the goal is to cause GCC to execute the uncovered debug dump code in
 * sel-sched-dump.cc during compilation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write
 * Creates data dependencies that prevent simple scheduling */
void func1_inner_loop(int *arr, int n) {
    volatile int sink = 0; /* Prevent dead code elimination */
    int sum = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create artificial dependency */
        sum += arr[i];
        
        /* Conditional store with side effect */
        if (sum > 1000) {
            arr[i] = sum % 256;
            /* Inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(sum) : "memory");
        } else {
            arr[i] = i;
        }
        
        /* Another asm barrier to prevent optimization */
        asm volatile ("" : "+r"(sum) : : "memory");
    }
    
    /* Use result to prevent elimination */
    sink = sum;
}

/* Function 2: Nested loops with different iteration counts
 * Creates outer loop pipelining opportunities */
int func2_nested_loops(int rows, int cols) {
    volatile int result = 0;
    int matrix[32][32]; /* Fixed size for simplicity */
    
    /* Initialize matrix */
    for (int i = 0; i < rows && i < 32; i++) {
        for (int j = 0; j < cols && j < 32; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Nested computation with dependencies */
    for (int i = 1; i < rows && i < 31; i++) {
        int row_sum = 0;
        for (int j = 1; j < cols && j < 31; j++) {
            /* Complex dependency chain */
            int val = matrix[i][j] 
                     + matrix[i-1][j] 
                     + matrix[i][j-1]
                     - matrix[i-1][j-1];
            
            /* Conditional update */
            if (val % 2 == 0) {
                matrix[i][j] = val >> 1;
            } else {
                matrix[i][j] = val * 3 + 1;
            }
            
            row_sum += matrix[i][j];
            
            /* Memory barrier */
            asm volatile ("" : : "r"(val), "r"(row_sum) : "memory");
        }
        
        result += row_sum;
        
        /* Prevent loop invariant code motion */
        asm volatile ("" : "+r"(result) : : "memory");
    }
    
    return result;
}

/* Function 3: Switch statement with computed goto
 * Creates complex control flow for the scheduler */
int func3_complex_control_flow(int x, int *output) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    volatile int ret = 0;
    
    /* Bounds check */
    int index = x % 5;
    if (index < 0) index = 0;
    
    /* Computed goto - creates complex control flow */
    goto *labels[index];
    
case0:
    for (int i = 0; i < 10; i++) {
        output[i] = i * x;
        asm volatile ("" : : "r"(output[i]) : "memory");
    }
    ret = 1;
    goto end;
    
case1:
    for (int i = 0; i < 10; i++) {
        output[i] = i + x;
        /* Create dependency chain */
        asm volatile ("" : "+r"(output[i]) : : "memory");
    }
    ret = 2;
    goto end;
    
case2:
    for (int i = 0; i < 10; i++) {
        output[i] = i - x;
        asm volatile ("" : : "r"(output[i]) : "memory");
    }
    ret = 3;
    goto end;
    
case3:
    for (int i = 0; i < 10; i++) {
        output[i] = i / (x + 1);
        asm volatile ("" : : "r"(output[i]) : "memory");
    }
    ret = 4;
    goto end;
    
default_case:
    for (int i = 0; i < 10; i++) {
        output[i] = i % (x + 2);
        asm volatile ("" : : "r"(output[i]) : "memory");
    }
    ret = 5;
    goto end;
    
end:
    return ret;
}

/* Function 4: Mixed operations with function calls
 * Creates scheduling barriers */
int helper1(int a, int b) {
    return (a * b) + (a ^ b);
}

int helper2(int a, int b) {
    volatile int res = 0;
    for (int i = 0; i < 4; i++) {
        res += (a >> i) & 1;
        res -= (b >> i) & 1;
    }
    return res;
}

void func4_mixed_ops(int *data, int n) {
    int temp[16];
    volatile int accumulator = 0;
    
    for (int i = 0; i < n && i < 16; i++) {
        /* Function call creates scheduling barrier */
        int x = helper1(data[i], i);
        
        /* Another function call */
        int y = helper2(data[i], x);
        
        /* Complex expression with multiple operations */
        temp[i] = (x * y) + (x << (y & 3)) - (y >> (x & 3));
        
        /* Memory operations with barrier */
        asm volatile ("" : : "r"(temp[i]) : "memory");
        
        accumulator += temp[i];
    }
    
    /* Final computation with dependency on accumulator */
    for (int i = 0; i < n && i < 16; i++) {
        data[i] = temp[i] + accumulator;
        asm volatile ("" : : "r"(data[i]) : "memory");
    }
}

/* Function 5: Loop with early exit and multiple exits
 * Creates interesting control flow for selective scheduling */
int func5_early_exit(int *arr, int n, int threshold) {
    volatile int found = 0;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        
        /* Early exit condition */
        if (sum > threshold) {
            found = 1;
            /* Asm barrier to prevent optimization */
            asm volatile ("" : : "r"(sum), "r"(found) : "memory");
            return sum;
        }
        
        /* Another exit condition */
        if (arr[i] < 0) {
            found = -1;
            asm volatile ("" : : "r"(arr[i]), "r"(found) : "memory");
            break;
        }
        
        /* Continue with computation */
        arr[i] = sum % 100;
        asm volatile ("" : "+r"(sum) : : "memory");
    }
    
    return found ? sum : -sum;
}

/* Main driver - ensures all functions are compiled */
int main(int argc, char **argv) {
    int data[32];
    int output[10];
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < 32; i++) {
        data[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    func1_inner_loop(data, 32);
    
    int res2 = func2_nested_loops(8, 8);
    
    int res3 = func3_complex_control_flow(argc > 1 ? atoi(argv[1]) : 2, output);
    
    func4_mixed_ops(data, 16);
    
    int res5 = func5_early_exit(data, 32, 1000);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = res2 + res3 + res5 + data[0];
    
    return final_result != 0 ? 0 : 1;
}
