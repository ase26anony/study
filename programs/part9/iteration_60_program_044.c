/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumps
 * specifically targeting the dump_insn_rtx function in sel-sched-dump.cc.
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_trigger = 1;

/* Function 1: Inner loop with conditional branch and memory write
 * This creates a scheduling region with data dependencies */
int func1_with_inner_loop(int *arr, int n) {
    int sum = 0;
    int i, j;
    
    /* Outer loop - creates basic block structure */
    for (i = 0; i < n; i++) {
        /* Inner loop with data dependency */
        int temp = arr[i];
        for (j = 0; j < 10; j++) {
            /* Complex expression with multiple operations */
            temp = (temp * 3 + j) / 2;
            
            /* Conditional store to create control flow */
            if (temp > 100) {
                arr[i] = temp % 256;
                g_volatile_counter++;
            }
        }
        
        /* Memory access with pointer arithmetic */
        sum += arr[i] + *(arr + (i % 8));
        
        /* Inline assembly to create specific RTL patterns */
        asm volatile ("" : : "r"(temp), "r"(sum) : "memory");
    }
    
    /* Final computation with branch */
    if (sum > 1000) {
        return sum / 2;
    } else {
        return sum * 2;
    }
}

/* Function 2: Nested loops with different iteration counts
 * Triggers outer loop pipelining optimization */
void func2_nested_loops(int *matrix, int rows, int cols) {
    int i, j, k;
    
    /* Triple nested loop for complex scheduling */
    for (i = 0; i < rows; i++) {
        int row_start = i * cols;
        
        for (j = 0; j < cols; j++) {
            int idx = row_start + j;
            int acc = matrix[idx];
            
            /* Innermost loop with varying trip count */
            for (k = 0; k < (j % 5) + 2; k++) {
                /* Multiple arithmetic operations with dependencies */
                acc = (acc << 3) | (acc >> 5);
                acc ^= (i * j + k);
                
                /* Volatile access to prevent dead code elimination */
                if (g_volatile_trigger) {
                    acc += g_volatile_counter;
                }
            }
            
            /* Conditional store with pointer alias */
            if ((i + j) % 3 == 0) {
                matrix[idx] = acc & 0xFF;
            }
            
            /* Memory barrier via assembly */
            asm volatile ("" : : "r"(acc), "r"(idx) : "memory");
        }
        
        /* Function call within loop to create call instruction */
        if (i % 7 == 0) {
            g_volatile_counter = func1_with_inner_loop(matrix + row_start, cols / 2);
        }
    }
}

/* Function 3: Switch statement with computed goto for complex control flow */
int func3_complex_switch(int x, int *results) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3, &&case_default };
    int result = 0;
    
    /* Indirect jump via computed goto */
    goto *labels[x % 5];
    
case0:
    /* Multiple basic blocks with arithmetic */
    for (int i = 0; i < 8; i++) {
        result += (x << i) | (x >> (32 - i));
        asm volatile ("" : : "r"(result));
    }
    results[0] = result;
    goto end;
    
case1:
    /* Loop with early exit */
    for (int i = 0; i < 16; i++) {
        if (i > x) break;
        result ^= (results[i % 4] * i);
    }
    results[1] = result;
    goto end;
    
case2:
    /* Nested conditionals */
    if (x > 100) {
        result = x * 3;
        if (result < 1000) {
            result += 255;
        }
    } else {
        result = x / 3;
    }
    results[2] = result;
    goto end;
    
case3:
    /* Memory intensive operations */
    for (int i = 0; i < 12; i++) {
        results[i % 8] = (results[i % 8] + x) * (i + 1);
    }
    result = results[7];
    goto end;
    
case_default:
    result = x * x + x;
    results[3] = result;
    
end:
    /* Final operation with side effect */
    g_volatile_counter += result;
    return result;
}

/* Function 4: Mixed operations with array accesses and function calls */
float func4_mixed_ops(float *farr, int size) {
    float total = 0.0f;
    int int_buffer[16];
    
    /* Initialize buffer with volatile dependency */
    for (int i = 0; i < 16; i++) {
        int_buffer[i] = i * g_volatile_counter;
    }
    
    /* Mixed float/int operations */
    for (int i = 0; i < size; i++) {
        /* Type conversions create specific RTL patterns */
        int ival = (int)farr[i];
        float fval = (float)int_buffer[i % 16];
        
        /* Conditional floating point operation */
        if (ival % 2 == 0) {
            fval = fval * 1.5f + ival;
        } else {
            fval = fval / 1.3f - ival;
        }
        
        /* Array store with bounds check */
        if (i < size - 1) {
            farr[i + 1] = fval;
        }
        
        total += fval;
        
        /* Memory clobber to prevent reordering */
        asm volatile ("" : : "r"(ival), "r"(total) : "memory");
    }
    
    /* Call to another function */
    int temp = func3_complex_switch((int)total, int_buffer);
    return total + (float)temp;
}

/* Function 5: Recursive pattern with tail recursion */
int func5_recursive_pattern(int n, int *cache) {
    if (n <= 0) return 1;
    if (n < 16 && cache[n] != 0) return cache[n];
    
    int a = func5_recursive_pattern(n - 1, cache);
    int b = func5_recursive_pattern(n - 2, cache);
    int c = func5_recursive_pattern(n - 3, cache);
    
    int result = (a * b + c) % 10007;
    
    if (n < 16) {
        cache[n] = result;
    }
    
    /* Force spill/reload with large expression */
    return ((result << 4) | (result >> 28)) ^ g_volatile_counter;
}

/* Main driver function that ensures all code paths are compiled */
int main(int argc, char **argv) {
    /* Initialize test data */
    int array1[64];
    int matrix[8][8];
    float farray[32];
    int results[16];
    int cache[16] = {0};
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 64; i++) {
        array1[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        farray[i] = (float)i / 2.0f;
    }
    
    /* Call all test functions to ensure they're compiled */
    int sum1 = func1_with_inner_loop(array1, 64);
    
    func2_nested_loops(&matrix[0][0], 8, 8);
    
    int sum3 = func3_complex_switch(argc > 1 ? atoi(argv[1]) : 42, results);
    
    float sum4 = func4_mixed_ops(farray, 32);
    
    int sum5 = func5_recursive_pattern(12, cache);
    
    /* Use results to prevent dead code elimination */
    int final_result = sum1 + sum3 + (int)sum4 + sum5;
    
    /* Print to ensure runtime execution */
    printf("Result: %d (counter: %d)\n", final_result, g_volatile_counter);
    
    return final_result != 0 ? 0 : 1;
}
