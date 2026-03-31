/* test_sel_sched_dump.c
 * Designed to trigger GCC's selective scheduler debug dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization and create scheduling dependencies */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[256];

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int n, int *arr) {
    int i, j;
    volatile int local_vol = 0;
    
    /* Create data dependencies that prevent simple scheduling */
    for (i = 0; i < n; i++) {
        /* Inline assembly to create specific RTL patterns */
        asm volatile ("# Dependency marker 1" : : "r"(i));
        
        for (j = 0; j < 100; j++) {
            /* Complex expression with multiple dependencies */
            int temp = arr[i] * j + g_volatile_counter;
            
            /* Conditional store with side effect */
            if (temp % 3 == 0) {
                arr[i] = temp;
                g_volatile_array[j & 255] = temp;
                
                /* Another inline asm to prevent reordering */
                asm volatile ("# Conditional store barrier" : : "r"(temp));
            } else {
                /* Different path with computation */
                arr[i] = temp * 2 - g_volatile_counter;
                local_vol += j;
            }
            
            /* Function call-like barrier */
            asm volatile ("# Loop iteration end" : : "r"(j), "r"(arr[i]));
        }
        
        /* Update volatile to create cross-iteration dependency */
        g_volatile_counter += i;
    }
}

/* Function 2: Nested loops with different iteration counts */
int test_nested_loops(int rows, int cols, int (*matrix)[64]) {
    int sum = 0;
    volatile int checksum = 0;
    int i, j, k;
    
    /* Outer loop with variable bounds */
    for (i = 0; i < rows; i++) {
        /* Middle loop with dependency on outer */
        for (j = 0; j < cols; j++) {
            /* Inner loop with computation */
            for (k = 0; k < 8; k++) {
                /* Complex addressing and computation */
                int idx = (i * cols + j + k) & 63;
                int val = matrix[i][idx] * (k + 1);
                
                /* Conditional with multiple branches */
                if (val > 1000) {
                    sum += val >> 2;
                    checksum ^= val;
                } else if (val > 100) {
                    sum += val >> 1;
                    checksum |= val;
                } else {
                    sum += val;
                    checksum &= val;
                }
                
                /* Memory barrier effect */
                asm volatile ("# Nested loop computation" : : "r"(val), "r"(sum));
            }
            
            /* Cross-iteration dependency */
            matrix[i][j] = sum + checksum;
        }
        
        /* Volatile update creates scheduling barrier */
        g_volatile_counter = (g_volatile_counter * 31 + i) & 0xFFFF;
    }
    
    return sum;
}

/* Function 3: Switch statement with computed goto-like control flow */
int test_switch_complex(int mode, int iterations) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3, 
        &&case_4, &&case_5, &&case_default
    };
    
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int op = (mode + i) % 7;
        
        /* Indirect jump simulation */
        goto *jump_table[op];
        
    case_0:
        result += i * 2;
        /* Create scheduling dependency */
        asm volatile ("# Case 0 operation" : : "r"(result));
        goto switch_end;
        
    case_1:
        result -= i * 3;
        g_volatile_array[i & 255] = result;
        goto switch_end;
        
    case_2:
        result ^= i * 5;
        /* Memory operation */
        result = g_volatile_array[(i + 1) & 255] ^ result;
        goto switch_end;
        
    case_3:
        result |= 0xABCD;
        result *= (i + 1);
        goto switch_end;
        
    case_4:
        result = (result << 3) | (result >> 29);
        result += g_volatile_counter;
        goto switch_end;
        
    case_5:
        result = ~result;
        /* Complex expression */
        result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
        goto switch_end;
        
    case_default:
        result = (result + mode) * i;
        break;
        
    switch_end:
        /* Continue loop */
        ;
    }
    
    return result;
}

/* Function 4: Mixed control flow with function calls */
int test_mixed_control_flow(int limit) {
    int a = 1, b = 1, c = 0;
    volatile int *ptr = &g_volatile_counter;
    
    /* Fibonacci-like computation with branches */
    for (int i = 0; i < limit; i++) {
        /* Multiple condition checks */
        if (i % 4 == 0) {
            c = a + b;
            *ptr += c;
            asm volatile ("# Branch 1" : : "r"(c));
        } else if (i % 4 == 1) {
            c = a - b;
            g_volatile_array[i & 255] = c;
            asm volatile ("# Branch 2" : : "r"(c));
        } else if (i % 4 == 2) {
            c = a * b;
            /* Dependency chain */
            b = a;
            a = c;
            asm volatile ("# Branch 3" : : "r"(a), "r"(b));
        } else {
            c = a ^ b;
            /* Memory and computation mix */
            a = b + g_volatile_array[(i >> 2) & 255];
            b = c;
            asm volatile ("# Branch 4" : : "r"(a), "r"(b));
        }
        
        /* Loop-carried dependency */
        if (c > 1000000) {
            c = c % 1000;
        }
    }
    
    return c;
}

/* Main driver to ensure all code paths are compiled */
int main(int argc, char **argv) {
    int arr1[100];
    int matrix[10][64];
    int i, result = 0;
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        arr1[i] = i * 3 + 1;
    }
    
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = (i * 64 + j) * 7;
        }
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(50, arr1);
    result += test_nested_loops(5, 32, matrix);
    result += test_switch_complex(argc > 1 ? atoi(argv[1]) : 2, 100);
    result += test_mixed_control_flow(200);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
