/* test_sel_sched_debug.c
 * 
 * This code is designed to trigger GCC's selective scheduler debug output
 * when compiled with specific flags that enable selective scheduling and
 * RTL debugging dumps.
 * 
 * Compile with:
 *   gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test_sel_sched_debug.c -o test.o
 * or
 *   gcc -O3 -fsel-sched-pipelining-outer-loops -dS -fdump-rtl-all -c test_sel_sched_debug.c -o test.o
 * 
 * The coverage happens at compile-time within GCC's RTL backend when
 * the selective scheduler processes instructions and calls dump_insn_rtx
 * with debug flags enabled.
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* Function 1: Inner loop with conditional branch and memory write
 * This creates a scheduling region with data dependencies */
int func_inner_loop(int *arr, int n) {
    int sum = 0;
    volatile int temp = 0;  /* volatile prevents optimization */
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        temp = arr[i];
        
        /* Conditional branch inside loop */
        if (temp > 100) {
            sum += temp * 2;
            /* Memory write with dependency */
            arr[i] = sum % 256;
        } else {
            sum += temp / 2;
            /* Different memory write path */
            arr[i] = (sum + i) % 256;
        }
        
        /* Inline assembly to create specific RTL patterns */
        asm volatile ("" : : "r"(sum), "r"(arr[i]) : "memory");
    }
    
    /* Another dependency chain */
    for (int i = n - 1; i >= 0; i--) {
        sum -= arr[i];
        asm volatile ("" : : "r"(sum));
    }
    
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Outer loop pipelining opportunities */
int func_nested_loops(int rows, int cols, int (*matrix)[64]) {
    int total = 0;
    volatile int row_sum = 0;
    
    /* Nested loops for outer-loop pipelining */
    for (int i = 0; i < rows; i++) {
        row_sum = 0;
        for (int j = 0; j < cols; j++) {
            /* Complex addressing with multiple operations */
            int val = matrix[i][j];
            
            /* Conditional with multiple paths */
            if (val & 1) {
                row_sum += val * 3 + j;
            } else {
                row_sum += (val >> 1) - i;
            }
            
            /* Modify matrix with dependency */
            matrix[i][j] = (row_sum + global_counter++) & 0xFF;
            
            /* Memory barrier-like asm */
            asm volatile ("" : : "r"(row_sum), "r"(matrix[i][j]) : "memory");
        }
        
        /* Cross-iteration dependency */
        total += row_sum * (i + 1);
        
        /* Volatile access to prevent optimization */
        asm volatile ("" : : "r"(total));
    }
    
    /* Additional loop with different pattern */
    for (int i = 0; i < rows; i += 2) {
        for (int j = 0; j < cols; j += 3) {
            total ^= matrix[i][j];
            asm volatile ("" : : "r"(total));
        }
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto-like pattern
 * Creates complex control flow for selective scheduling */
int func_switch_complex(int x, int *results) {
    int ret = 0;
    volatile int selector = x % 5;
    
    /* Switch with multiple cases - creates jump table */
    switch (selector) {
        case 0:
            ret = x * 2;
            /* Memory operation */
            results[0] = ret;
            asm volatile ("" : : "r"(ret), "r"(results[0]));
            break;
            
        case 1:
            ret = x + x;
            for (int i = 0; i < 4; i++) {
                ret += results[i];
                asm volatile ("" : : "r"(ret));
            }
            results[1] = ret;
            break;
            
        case 2:
            ret = x * x;
            /* Nested conditional */
            if (ret > 1000) {
                results[2] = ret / 2;
                asm volatile ("" : : "r"(results[2]));
            } else {
                results[2] = ret * 2;
                asm volatile ("" : : "r"(results[2]));
            }
            break;
            
        case 3:
            /* Small loop inside case */
            ret = 1;
            for (int i = 0; i < 8; i++) {
                ret = (ret << 1) | (x & 1);
                x >>= 1;
                asm volatile ("" : : "r"(ret), "r"(x));
            }
            results[3] = ret;
            break;
            
        default: /* case 4 */
            ret = x;
            /* Multiple memory accesses */
            for (int i = 0; i < 3; i++) {
                results[i] = ret + i;
                ret += results[i];
                asm volatile ("" : : "r"(ret), "r"(results[i]) : "memory");
            }
            break;
    }
    
    /* Post-switch processing */
    if (ret < 0) {
        ret = -ret;
        asm volatile ("" : : "r"(ret));
    }
    
    return ret;
}

/* Function 4: Mixed control flow with function calls
 * Creates scheduling regions with call instructions */
int func_mixed_control(int a, int b, int c) {
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    int result = 0;
    
    /* Loop with early exit */
    for (int i = 0; i < 100; i++) {
        /* Complex condition */
        if ((v1 * i) > (v2 + v3)) {
            result += i * 2;
            /* Conditional break */
            if (result > 1000) break;
        } else {
            result -= i;
            /* Nested condition */
            if (i % 3 == 0) {
                result += v3;
                asm volatile ("" : : "r"(result));
            }
        }
        
        /* Modify volatiles */
        v1 = (v1 + 1) & 0xFF;
        v2 = (v2 * 2) & 0xFF;
        v3 = (v3 - 1) & 0xFF;
        
        asm volatile ("" : : "r"(v1), "r"(v2), "r"(v3), "r"(result) : "memory");
    }
    
    /* Do-while loop for different control flow */
    int j = 0;
    do {
        result ^= (v1 << j);
        j++;
        asm volatile ("" : : "r"(result), "r"(j));
    } while (j < 8);
    
    return result;
}

/* Main driver function - ensures all code paths are compiled */
int main(int argc, char **argv) {
    /* Initialize test data */
    int array1[128];
    int matrix1[32][64];
    int results[8];
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < 128; i++) {
        array1[i] = (i * 37) & 0xFF;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            matrix1[i][j] = (i * 64 + j * 13) & 0xFF;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        results[i] = i * 50;
    }
    
    /* Call all test functions to ensure they're compiled */
    int r1 = func_inner_loop(array1, 128);
    int r2 = func_nested_loops(16, 64, matrix1);
    int r3 = func_switch_complex(argc > 1 ? atoi(argv[1]) : 42, results);
    int r4 = func_mixed_control(r1, r2, r3);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = r1 + r2 + r3 + r4;
    
    /* Print to avoid optimization (but coverage happens at compile time) */
    printf("Results: %d %d %d %d (Final: %d)\n", 
           r1, r2, r3, r4, final_result);
    
    return 0;
}
