/* test_sel_sched.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write */
/* This creates data dependencies that challenge the scheduler */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* volatile prevents dead code elimination */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependency chain */
        int temp = arr[i];
        
        /* Conditional store with dependency */
        if (temp > 0) {
            sum += temp * 2;
            arr[i] = sum;
        } else {
            sum -= temp;
            arr[i] = -temp;
        }
        
        /* Inline assembly to create unschedulable dependency */
        asm volatile ("" : : "r"(sum) : "memory");
    }
    
    /* Force use of result to prevent optimization */
    asm volatile ("" : : "r"(sum));
}

/* Function 2: Nested loops with different iteration counts */
/* Outer loop pipelining candidate */
void test_nested_loops(int *matrix, int rows, int cols) {
    volatile int total = 0;
    int i, j, k;
    
    for (i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with stride access */
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Complex expression with multiple operations */
            int val = matrix[idx];
            val = (val * 3 + 7) & 0xFF;
            
            /* Conditional update */
            if (val > 128) {
                row_sum += val - 64;
            } else {
                row_sum += val * 2;
            }
            
            /* Store back with transformation */
            matrix[idx] = val ^ 0x55;
        }
        
        /* Cross-iteration dependency */
        total += row_sum;
        
        /* Memory barrier to prevent reordering */
        asm volatile ("" : : "r"(row_sum), "m"(matrix[i * cols]) : "memory");
    }
    
    /* Use result */
    asm volatile ("" : : "r"(total));
}

/* Function 3: Switch statement with computed goto-like pattern */
/* Creates complex control flow */
int test_switch_complex(int x, int *results) {
    volatile int state = 0;
    int i;
    
    for (i = 0; i < 100; i++) {
        /* Switch with multiple cases */
        switch (x % 7) {
            case 0:
                state += i * 2;
                results[0] = state;
                x = (x * 3 + 1) & 0xFF;
                break;
            case 1:
                state -= i;
                results[1] = state;
                x = (x >> 1) | 0x80;
                break;
            case 2:
                state ^= i;
                results[2] = state;
                x = (x + 17) & 0xFF;
                break;
            case 3:
                state *= 2;
                results[3] = state;
                x = (x - 23) & 0xFF;
                break;
            case 4:
                state /= 2;
                results[4] = state;
                x = (x ^ 0xAA) & 0xFF;
                break;
            case 5:
                state = state << 1;
                results[5] = state;
                x = (x + 41) & 0xFF;
                break;
            case 6:
                state = state >> 1;
                results[6] = state;
                x = (x * 5) & 0xFF;
                break;
        }
        
        /* Dependency between iterations */
        asm volatile ("" : : "r"(state), "r"(x));
    }
    
    return state;
}

/* Function 4: Mixed operations with pointer chasing */
/* Creates memory dependencies */
void test_pointer_chasing(int *data, int size) {
    volatile int accumulator = 0;
    int *ptr = data;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Load with potential aliasing */
        int val = *ptr;
        
        /* Arithmetic chain */
        val = ((val + 1) * 3 - 2) & 0xFFFF;
        
        /* Store with offset */
        *(ptr + (val & 0x3)) = val;
        
        /* Update pointer with dependency */
        ptr = data + ((val + i) % size);
        
        /* Update accumulator */
        accumulator ^= val;
        
        /* Memory barrier */
        asm volatile ("" : : "r"(val), "r"(ptr), "m"(*data));
    }
    
    asm volatile ("" : : "r"(accumulator));
}

/* Function 5: Loop with early exit and multiple exits */
/* Creates control flow challenges */
int test_early_exit(int *arr, int n, int threshold) {
    volatile int found = 0;
    int i, result = 0;
    
    for (i = 0; i < n; i++) {
        /* Multiple conditions */
        if (arr[i] > threshold) {
            result = arr[i];
            found = 1;
            /* Early exit */
            break;
        } else if (arr[i] < -threshold) {
            result = -arr[i];
            found = -1;
            /* Another early exit point */
            if (i > n/2) break;
        } else {
            /* Continue processing */
            result += arr[i] * i;
        }
        
        /* Side effect that can't be eliminated */
        arr[i] = result & 0xFF;
        
        asm volatile ("" : : "r"(result));
    }
    
    return result * found;
}

/* Main driver that ensures all code paths are compiled */
int main(int argc, char **argv) {
    /* Initialize data */
    const int size = 256;
    int *data = malloc(size * sizeof(int));
    int *matrix = malloc(16 * 16 * sizeof(int));
    int results[7] = {0};
    
    /* Seed for reproducibility */
    srand(time(NULL));
    
    /* Fill with random data */
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 1000;
    }
    
    for (int i = 0; i < 16 * 16; i++) {
        matrix[i] = rand() % 256;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(data, size);
    test_nested_loops(matrix, 16, 16);
    int switch_result = test_switch_complex(data[0], results);
    test_pointer_chasing(data, size);
    int exit_result = test_early_exit(data, size, 500);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = 
        data[0] + matrix[0] + switch_result + exit_result;
    
    /* Print something to ensure execution */
    printf("Test completed. Final marker: %d\n", final_result);
    
    free(data);
    free(matrix);
    
    return 0;
}
