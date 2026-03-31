/* test_sel_sched_debug.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumps
 * when compiled with appropriate flags. The goal is to reach the uncovered
 * lines in sel-sched-dump.cc that handle RTL instruction dumping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write
 * Creates a small loop with data dependencies that should trigger
 * selective scheduling with pipelining.
 */
int test_inner_loop(int n, int *arr) {
    volatile int sum = 0;  /* volatile prevents dead code elimination */
    int i, j;
    
    /* Complex loop with data dependencies */
    for (i = 0; i < n; i++) {
        /* Create artificial dependency chain */
        int temp = arr[i];
        
        /* Conditional store to create control flow */
        if (temp > 100) {
            arr[i] = temp / 2;
            sum += arr[i];
        } else {
            arr[i] = temp * 3;
            sum -= arr[i];
        }
        
        /* Inline assembly to create unschedulable dependencies */
        asm volatile ("" : : "r"(temp) : "memory");
    }
    
    /* Another loop with different pattern */
    for (j = 0; j < n/2; j++) {
        arr[j] = arr[j] + arr[n - j - 1];
        asm volatile ("" : : "r"(arr[j]) : "memory");
    }
    
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Designed for outer loop pipelining optimization.
 */
void test_nested_loops(int rows, int cols, int mat[rows][cols]) {
    int i, j, k;
    volatile int acc = 0;
    
    /* Triple nested loop - good candidate for outer loop scheduling */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            int val = mat[i][j];
            
            /* Inner computation loop */
            for (k = 0; k < 10; k++) {
                val = (val * 13 + 7) % 256;
                
                /* Conditional to create branches */
                if (val & 1) {
                    acc += val;
                } else {
                    acc -= val;
                }
            }
            
            mat[i][j] = val;
            
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : "r"(val), "r"(acc) : "memory");
        }
    }
    
    /* Use the result to prevent dead code elimination */
    if (acc > 1000) {
        printf("Accumulator: %d\n", acc);
    }
}

/* Function 3: Complex control flow with switch and computed goto
 * Creates irregular control flow patterns for the scheduler.
 */
int test_complex_control_flow(int mode, int iterations) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    volatile int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Switch with multiple cases */
        switch ((i + mode) % 5) {
            case 0:
                result += i * 2;
                asm volatile ("" : : "r"(result) : "memory");
                break;
            case 1:
                result -= i / 2;
                /* Fall through */
            case 2:
                result ^= 0x55;
                asm volatile ("" : : "r"(result) : "memory");
                break;
            case 3:
                result = (result << 3) | (result >> 29);
                break;
            case 4:
                result = ~result;
                asm volatile ("" : : "r"(result) : "memory");
                break;
        }
        
        /* Computed goto for additional complexity */
        if (i % 7 == 0) {
            goto *labels[i % 5];
        }
        
        L0: result += 1;
        L1: result *= 3;
        L2: result &= 0xFF;
        L3: result |= 0x80;
        L4: result -= 5;
    }
    
    return result;
}

/* Function 4: Mixed operations with function calls
 * Creates scheduling boundaries and register pressure.
 */
int test_mixed_operations(int size) {
    int *buffer = malloc(size * sizeof(int));
    volatile int checksum = 0;
    int i;
    
    if (!buffer) return -1;
    
    /* Initialize with pattern */
    for (i = 0; i < size; i++) {
        buffer[i] = (i * 7) % 256;
    }
    
    /* Process with stride */
    for (i = 0; i < size - 1; i += 2) {
        int a = buffer[i];
        int b = buffer[i + 1];
        
        /* Multiple operations with dependencies */
        int t1 = a + b;
        int t2 = a - b;
        int t3 = a * b;
        int t4 = (a << 2) | (b >> 2);
        
        /* Conditional update */
        if ((a ^ b) & 1) {
            buffer[i] = t1 + t2;
            checksum += t3;
        } else {
            buffer[i] = t3 - t4;
            checksum -= t4;
        }
        
        /* Memory operations with barriers */
        asm volatile ("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4) : "memory");
    }
    
    /* Final reduction */
    int final = 0;
    for (i = 0; i < size; i++) {
        final ^= buffer[i];
    }
    
    free(buffer);
    return final + checksum;
}

/* Main driver function */
int main(void) {
    int arr1[100];
    int arr2[50][20];
    int i, j;
    
    /* Initialize test data */
    srand(time(NULL));
    
    for (i = 0; i < 100; i++) {
        arr1[i] = rand() % 200;
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 20; j++) {
            arr2[i][j] = rand() % 256;
        }
    }
    
    /* Call all test functions to ensure they're compiled */
    int r1 = test_inner_loop(100, arr1);
    test_nested_loops(50, 20, arr2);
    int r2 = test_complex_control_flow(2, 50);
    int r3 = test_mixed_operations(64);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", r1, r2, r3);
    
    return 0;
}
