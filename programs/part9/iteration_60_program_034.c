/* test_sel_sched.c - Code to trigger selective scheduler debug dumps */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent dead code elimination */
volatile int global_seed = 42;

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int *arr, int n) {
    int i, j;
    volatile int sum = 0;
    
    /* Create data dependencies to prevent simple scheduling */
    for (i = 0; i < n; i++) {
        /* Complex condition with side effects */
        if (arr[i] > global_seed) {
            /* Memory write with dependency on previous read */
            arr[i] = arr[i] * 2 + global_seed;
            sum += arr[i];
            
            /* Inline assembly to create specific RTL patterns */
            asm volatile ("" : : "r"(arr[i]), "r"(sum) : "memory");
        } else {
            /* Different path with its own computations */
            arr[i] = arr[i] / 2 - global_seed;
            sum -= arr[i];
            
            /* Another inline assembly barrier */
            asm volatile ("" : : "r"(arr[i]), "r"(sum) : "memory");
        }
        
        /* Nested loop to create more scheduling opportunities */
        for (j = 0; j < 3; j++) {
            /* Complex expression with multiple operations */
            int temp = (arr[i] * j + global_seed) % 256;
            sum += temp;
            
            /* Prevent optimization */
            asm volatile ("" : "+r"(sum) : : "memory");
        }
    }
    
    /* Use the result to prevent dead code elimination */
    global_seed = sum % 1000;
}

/* Function 2: Nested loops with different iteration counts */
int test_nested_loops(int rows, int cols) {
    volatile int matrix[32][32];  /* Small enough to not overflow stack */
    int i, j, k;
    int result = 0;
    
    /* Initialize with pattern */
    for (i = 0; i < rows && i < 32; i++) {
        for (j = 0; j < cols && j < 32; j++) {
            matrix[i][j] = (i * 17 + j * 13 + global_seed) & 0xFF;
        }
    }
    
    /* Complex nested loop computation */
    for (i = 0; i < rows - 1 && i < 31; i++) {
        for (j = 0; j < cols - 1 && j < 31; j++) {
            /* Multiple accumulators with dependencies */
            int acc1 = matrix[i][j];
            int acc2 = matrix[i+1][j+1];
            
            /* Inner loop with variable bound */
            for (k = 0; k < 4; k++) {
                acc1 = (acc1 * 3 + matrix[i][j+1]) >> 1;
                acc2 = (acc2 * 5 + matrix[i+1][j]) >> 2;
                
                /* Cross-dependency between accumulators */
                if (k % 2 == 0) {
                    int temp = acc1;
                    acc1 = acc2;
                    acc2 = temp;
                }
                
                /* Memory barrier */
                asm volatile ("" : : "r"(acc1), "r"(acc2) : "memory");
            }
            
            result += acc1 + acc2;
            matrix[i][j] = result & 0xFF;
        }
    }
    
    return result;
}

/* Function 3: Switch statement with computed goto for complex control flow */
int test_switch_computed_goto(int mode) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    volatile int state = 0;
    int i;
    
    /* Use computed goto if supported, otherwise fallback to switch */
    if (mode & 1) {
        /* Computed goto creates complex control flow */
        goto *labels[mode % 4];
    }
    
label0:
    for (i = 0; i < 10; i++) {
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        if (state % 3 == 0) goto label1;
        if (state % 5 == 0) goto label2;
    }
    goto end;
    
label1:
    for (i = 0; i < 8; i++) {
        state = (state * 1664525 + 1013904223) & 0x7FFFFFFF;
        if (state % 7 == 0) goto label3;
    }
    goto label0;
    
label2:
    state = state ^ global_seed;
    /* Fall through */
    
label3:
    for (i = 0; i < 5; i++) {
        state = (state << 3) | (state >> 29);  /* Rotate left 3 */
        asm volatile ("" : "+r"(state) : : "memory");
    }
    
end:
    return state;
}

/* Function 4: Mixed control flow with function calls */
int test_mixed_control_flow(int iterations) {
    volatile int counter = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Multiple if-else branches */
        if (i % 4 == 0) {
            counter += i * 2;
            /* Call to external function (puts) creates call RTL */
            if (i % 16 == 0) {
                putchar('.');
            }
        } else if (i % 4 == 1) {
            counter -= i * 3;
            /* Memory operation */
            global_seed = (global_seed + counter) & 0xFF;
        } else if (i % 4 == 2) {
            counter = counter ^ (i << 2);
            /* Loop with early exit */
            int j;
            for (j = 0; j < 3; j++) {
                if (counter > 1000) break;
                counter += j * 17;
            }
        } else {
            counter = (counter * 13) % 997;
            /* Conditional store */
            if (counter > 500) {
                global_seed = counter;
            }
        }
        
        /* Dependency chain */
        asm volatile ("" : "+r"(counter) : : "memory");
    }
    
    return counter;
}

/* Function 5: Array processing with pointer arithmetic */
void test_pointer_arithmetic(int *data, int size) {
    int *ptr = data;
    int *end = data + size;
    volatile int checksum = 0;
    
    while (ptr < end) {
        /* Multiple pointer operations */
        int val1 = *ptr;
        int val2 = *(ptr + 1);
        int val3 = *(ptr + 2);
        
        /* Complex computation with all values */
        int result = (val1 * val2 + val3 * global_seed) / (val1 + 1);
        checksum += result;
        
        /* Store back with offset */
        *ptr = result;
        
        /* Update pointer with stride */
        ptr += 3;
        
        if (ptr >= end - 2) {
            ptr = data;  /* Wrap around for small arrays */
        }
        
        /* Memory and computation barrier */
        asm volatile ("" : : "r"(val1), "r"(val2), "r"(checksum) : "memory");
    }
    
    global_seed = checksum;
}

/* Main driver function */
int main(int argc, char **argv) {
    int test_array[64];
    int i, result = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < 64; i++) {
        test_array[i] = (i * 37 + global_seed) & 0xFF;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(test_array, 64);
    result += test_nested_loops(8, 8);
    result += test_switch_computed_goto(argc);
    result += test_mixed_control_flow(32);
    test_pointer_arithmetic(test_array, 64);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
