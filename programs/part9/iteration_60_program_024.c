/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler
 * debug dumps, specifically targeting the dump_insn_rtx function
 * in sel-sched-dump.cc when compiled with appropriate flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_trigger = 1;

/* Function 1: Inner loop with conditional branch and memory write
 * This creates a scheduling region with data dependencies
 */
int test_inner_loop(int *arr, int n) {
    int sum = 0;
    int i, j;
    
    /* Outer loop to create region boundaries */
    for (i = 0; i < n; i++) {
        /* Inner loop with data dependency chain */
        for (j = 0; j < 100; j++) {
            /* Create anti-dependency with volatile */
            int temp = g_volatile_counter;
            
            /* Conditional store to create control flow */
            if (temp % 3 == 0) {
                arr[j] = temp + i;
            } else if (temp % 3 == 1) {
                arr[j] = temp * i;
            } else {
                arr[j] = temp - i;
            }
            
            /* Sum with dependency on arr[j] */
            sum += arr[j];
            
            /* Inline asm to create specific RTL patterns */
            asm volatile ("" : : "r"(arr[j]), "r"(sum));
        }
        
        /* Modify volatile to affect loop conditions */
        g_volatile_counter += i;
    }
    
    /* Another asm to prevent tail optimization */
    asm volatile ("" : : "r"(sum));
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Creates opportunities for outer-loop pipelining
 */
double test_nested_loops(double *matrix, int rows, int cols) {
    double total = 0.0;
    int i, j, k;
    
    /* Triple nested loop for complex scheduling */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            double accum = 0.0;
            
            /* Innermost loop with floating point ops */
            for (k = 0; k < 50; k++) {
                /* Create RAW dependency */
                double val = matrix[i * cols + j];
                
                /* Conditional floating point operation */
                if (g_volatile_trigger > 0) {
                    val = val * 1.5 + k;
                } else {
                    val = val / 1.5 - k;
                }
                
                /* Chain dependencies */
                accum += val;
                
                /* Memory barrier asm */
                asm volatile ("" : : "r"(val), "r"(accum));
            }
            
            matrix[i * cols + j] = accum;
            total += accum;
        }
        
        /* Volatile update in outer loop */
        g_volatile_trigger ^= i;
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto
 * Creates complex control flow for the scheduler
 */
int test_switch_complex(int mode, int iterations) {
    static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Switch creates multiple basic blocks */
        switch (mode) {
            case 0:
                result += i * 2;
                /* Fall through */
            case 1:
                result += i * 3;
                if (result > 1000) goto L0;
                break;
            case 2:
                result -= i;
                /* Create loop-carried dependency */
                g_volatile_counter = result;
                break;
            case 3:
                result *= 2;
                /* Conditional goto */
                if (result % 7 == 0) goto L1;
                break;
            default:
                result = result / 2;
                break;
        }
        
        /* Label-based jumps for additional complexity */
        if (i % 5 == 0) {
            goto *labels[i % 5];
        }
        
    L0:
        result += 1;
        continue;
    L1:
        result += 2;
        continue;
    L2:
        result += 3;
        continue;
    L3:
        result += 4;
        continue;
    L4:
        result += 5;
        continue;
    }
    
    /* Final asm to ensure RTL generation */
    asm volatile ("" : : "r"(result), "r"(mode));
    return result;
}

/* Function 4: Mixed operations with pointer aliasing
 * Creates ambiguity for the scheduler
 */
int test_mixed_ops(int *a, int *b, int *c, int n) {
    int i;
    int sum_a = 0, sum_b = 0, sum_c = 0;
    
    /* Loop with potential pointer aliasing */
    for (i = 0; i < n; i++) {
        /* Multiple memory accesses */
        int val_a = a[i];
        int val_b = b[i % 10];
        int val_c = c[i % 5];
        
        /* Complex dependency chain */
        val_a = val_a * val_b + g_volatile_counter;
        val_b = val_b - val_c * i;
        val_c = val_c + val_a / (i + 1);
        
        /* Conditional stores with side effects */
        if (val_a > val_b) {
            a[i] = val_a;
            sum_a += val_a;
        } else {
            b[i % 10] = val_b;
            sum_b += val_b;
        }
        
        /* Always update c */
        c[i % 5] = val_c;
        sum_c += val_c;
        
        /* Memory clobber to force scheduling constraints */
        asm volatile ("" : : "r"(val_a), "r"(val_b), "r"(val_c));
    }
    
    return sum_a + sum_b + sum_c;
}

/* Function 5: Recursive-like pattern with tail operations
 */
int test_tail_operations(int seed, int depth) {
    int result = seed;
    int i;
    
    for (i = 0; i < depth; i++) {
        /* Create serial dependency */
        result = result * 1103515245 + 12345;
        
        /* Tail operation that can't be easily moved */
        if (i == depth - 1) {
            /* Complex tail operation */
            result = (result & 0x7fffffff) / 65536;
            
            /* Asm with multiple outputs */
            int hi, lo;
            asm volatile ("mul %2, %3" 
                         : "=h"(hi), "=l"(lo) 
                         : "r"(result), "r"(i));
            result = hi + lo;
        }
        
        /* Volatile access in middle of dependency chain */
        g_volatile_counter = result % 100;
    }
    
    return result;
}

/* Main driver function */
int main(int argc, char **argv) {
    /* Initialize test data */
    int arr1[100];
    double matrix[10][10];
    int arr2[100], arr3[100], arr4[100];
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        arr4[i] = i * 4;
    }
    
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            matrix[i][j] = i * 10.0 + j;
        }
    }
    
    /* Call all test functions to ensure they're compiled */
    int result1 = test_inner_loop(arr1, 50);
    double result2 = test_nested_loops(&matrix[0][0], 10, 10);
    int result3 = test_switch_complex(argc > 1 ? atoi(argv[1]) : 2, 100);
    int result4 = test_mixed_ops(arr2, arr3, arr4, 100);
    int result5 = test_tail_operations(42, 50);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %.2f, %d, %d, %d\n", 
           result1, result2, result3, result4, result5);
    
    /* Final volatile operation */
    g_volatile_counter = result1 + result3 + result4 + result5;
    
    return g_volatile_counter == 0 ? 0 : 1;
}
