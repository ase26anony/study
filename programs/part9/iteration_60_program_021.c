/* test_sel_sched.c - Program to trigger selective scheduler debug dumps */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* volatile prevents dead code elimination */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Complex loop with data dependencies */
        int temp = arr[i];
        
        /* Conditional store with dependency chain */
        if (temp > 100) {
            arr[i] = temp * 2 + i;
            sum += arr[i];
        } else {
            arr[i] = temp / 2 - i;
            sum -= arr[i];
        }
        
        /* Inline assembly to create unschedulable dependencies */
        asm volatile ("" : : "r"(sum) : "memory");
    }
    
    /* Use the result to prevent optimization */
    asm volatile ("" : : "r"(sum));
}

/* Function 2: Nested loops with different iteration counts */
void test_nested_loops(int *matrix, int rows, int cols) {
    volatile int total = 0;
    int i, j, k;
    
    /* Outer loop with pipelining opportunities */
    for (i = 0; i < rows; i++) {
        /* Middle loop */
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Inner loop with reduction */
            for (k = 0; k < 3; k++) {
                val = (val * 13 + 7) % 256;
            }
            
            matrix[idx] = val;
            total += val;
            
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : "r"(total) : "memory");
        }
        
        /* Conditional break to create control flow complexity */
        if (total > 10000) {
            break;
        }
    }
    
    asm volatile ("" : : "r"(total));
}

/* Function 3: Switch statement with computed goto-like pattern */
void test_switch_complex(int mode, int *data, int size) {
    volatile int result = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Complex switch with multiple cases */
        switch (mode) {
            case 0:
                data[i] = data[i] + i * 2;
                result += data[i];
                break;
            case 1:
                data[i] = data[i] - i / 2;
                result -= data[i];
                /* Fall through */
            case 2:
                data[i] = data[i] * 3;
                result ^= data[i];
                break;
            case 3:
                data[i] = data[i] % 17;
                result |= data[i];
                break;
            default:
                data[i] = ~data[i];
                result &= data[i];
                break;
        }
        
        /* Conditional based on result */
        if (result & 1) {
            mode = (mode + 1) % 4;
        }
        
        asm volatile ("" : : "r"(result) : "memory");
    }
    
    asm volatile ("" : : "r"(result));
}

/* Function 4: Mixed control flow with function calls */
static int helper_func(int x, int y) {
    return (x * y) + (x / (y + 1));
}

void test_mixed_control(int *arr, int n) {
    volatile int acc = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Function call creates scheduling barriers */
        int val = helper_func(arr[i], i);
        
        /* Complex if-else chain */
        if (val < 0) {
            arr[i] = -val;
            acc -= arr[i];
        } else if (val < 100) {
            arr[i] = val * 2;
            acc += arr[i];
        } else if (val < 1000) {
            arr[i] = val / 2;
            acc ^= arr[i];
        } else {
            arr[i] = val % 100;
            acc |= arr[i];
        }
        
        /* Loop with early exit */
        int j;
        for (j = 0; j < 2; j++) {
            if (acc > 5000) {
                break;
            }
            acc += j;
        }
        
        asm volatile ("" : : "r"(acc) : "memory");
    }
    
    asm volatile ("" : : "r"(acc));
}

/* Function 5: Pointer chasing with indirect jumps */
void test_pointer_chasing(int **ptr_arr, int count) {
    volatile int trace = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        int **current = &ptr_arr[i];
        
        /* Dereference and modify */
        if (*current != NULL) {
            **current = (**current * 3 + 7) % 256;
            trace += **current;
        }
        
        /* Conditional goto simulation */
        if (trace & 1) {
            current = &ptr_arr[(i + 1) % count];
        }
        
        asm volatile ("" : : "r"(trace) : "memory");
    }
    
    asm volatile ("" : : "r"(trace));
}

/* Main driver function */
int main(int argc, char **argv) {
    const int size = 100;
    const int rows = 10;
    const int cols = 10;
    const int ptr_count = 20;
    
    /* Allocate and initialize test data */
    int *array1 = (int *)malloc(size * sizeof(int));
    int *array2 = (int *)malloc(size * sizeof(int));
    int *matrix = (int *)malloc(rows * cols * sizeof(int));
    int **ptr_array = (int **)malloc(ptr_count * sizeof(int *));
    int *data = (int *)malloc(ptr_count * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array1[i] = rand() % 256;
        array2[i] = rand() % 256;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = rand() % 256;
    }
    
    for (int i = 0; i < ptr_count; i++) {
        data[i] = rand() % 256;
        ptr_array[i] = &data[i];
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(array1, size);
    test_nested_loops(matrix, rows, cols);
    test_switch_complex(rand() % 5, array2, size);
    test_mixed_control(array1, size);
    test_pointer_chasing(ptr_array, ptr_count);
    
    /* Use results to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    
    for (int i = 0; i < rows * cols; i++) {
        checksum ^= matrix[i];
    }
    
    for (int i = 0; i < ptr_count; i++) {
        checksum ^= data[i];
    }
    
    /* Clean up */
    free(array1);
    free(array2);
    free(matrix);
    free(ptr_array);
    free(data);
    
    return checksum & 0xFF;  /* Return non-deterministic but valid value */
}
