/* Test program to trigger selective scheduler debug dumps in GCC */
/* Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test.c -o test.o */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Inner loop with conditional branch and memory writes */
/* This creates data dependencies that require careful scheduling */
void test_inner_loop(int *arr, int n) {
    volatile int counter = 0;  /* Prevent optimization */
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Create data dependency chain */
        sum += arr[i];
        
        /* Conditional store with side effect */
        if (sum > 1000) {
            arr[i] = sum % 256;
            counter++;
        } else {
            arr[i] = sum;
        }
        
        /* Inline assembly to create unschedulable dependency */
        asm volatile ("" : : "r"(sum) : "memory");
    }
    
    /* Force use of results */
    asm volatile ("" : : "r"(counter), "r"(sum) : "memory");
}

/* Function 2: Nested loops with different iteration patterns */
/* Outer loop pipelining opportunities */
void test_nested_loops(int *matrix, int rows, int cols) {
    volatile int checksum = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with stride access */
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            row_sum += matrix[idx] * (i + j);
            
            /* Conditional update with branch */
            if (row_sum & 1) {
                matrix[idx] = row_sum;
            }
        }
        
        checksum += row_sum;
        
        /* Another dependency chain */
        for (int k = 0; k < cols; k += 2) {
            matrix[i * cols + k] ^= checksum;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(checksum) : "memory");
}

/* Function 3: Complex control flow with switch and computed goto */
/* Creates interesting control flow for the scheduler */
void test_complex_control(int *data, int size, int mode) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3 };
    volatile int result = 0;
    
    for (int i = 0; i < size; i++) {
        int val = data[i];
        
        /* Switch-like dispatch using computed goto */
        if (mode >= 0 && mode < 4) {
            goto *labels[mode];
        }
        
    case0:
        result += val * 2;
        continue;
        
    case1:
        result += val >> 1;
        if (result > 100) {
            data[i] = result;
        }
        continue;
        
    case2:
        result ^= val;
        /* Create loop-carried dependency */
        for (int j = 0; j < 3; j++) {
            result += j;
        }
        continue;
        
    case3:
        result = result * 3 + val;
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        continue;
    }
    
    /* Use result to prevent optimization */
    data[0] = result;
}

/* Function 4: Mixed operations with function calls */
/* Creates scheduling boundaries */
static int helper_multiply(int a, int b) {
    return a * b;
}

static int helper_conditional(int x) {
    return (x > 0) ? x : -x;
}

void test_mixed_operations(int *buf, int len) {
    int acc = 0;
    
    for (int i = 0; i < len; i++) {
        /* Function calls create scheduling boundaries */
        int prod = helper_multiply(buf[i], i);
        int abs_val = helper_conditional(prod);
        
        /* Complex expression with multiple operations */
        acc = (acc + abs_val) * 3 - 7;
        
        /* Store with condition */
        if (acc & 1) {
            buf[i] = acc;
        } else {
            buf[i] = acc >> 1;
        }
        
        /* Memory clobber to prevent reordering */
        asm volatile ("" : : : "memory");
    }
    
    /* Final computation with dependency on loop result */
    int final = 0;
    for (int i = 0; i < 10; i++) {
        final += helper_multiply(acc, i);
    }
    buf[0] = final;
}

/* Function 5: Pointer chasing with dependencies */
/* Creates load-use dependencies that challenge the scheduler */
void test_pointer_chasing(int **ptr_array, int count) {
    volatile int total = 0;
    int *current = ptr_array[0];
    
    for (int i = 1; i < count; i++) {
        /* Pointer chase creates true dependency */
        int value = *current;
        total += value;
        
        /* Conditional pointer update */
        if (value > 0) {
            current = ptr_array[i];
        } else {
            current = &total;
        }
        
        /* Arithmetic with result */
        *current = total + i;
        
        /* Compiler barrier */
        asm volatile ("" : : "r"(value), "r"(total) : "memory");
    }
}

/* Main driver function */
int main(int argc, char **argv) {
    /* Initialize test data */
    int data1[100];
    int data2[10][10];
    int *data3 = malloc(50 * sizeof(int));
    int *data4 = malloc(30 * sizeof(int));
    int *ptr_array[20];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        data1[i] = i * 3 + 7;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            data2[i][j] = i * j;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        data3[i] = i * 2 - 5;
    }
    
    for (int i = 0; i < 30; i++) {
        data4[i] = i * 4 + 1;
    }
    
    for (int i = 0; i < 20; i++) {
        ptr_array[i] = &data1[i * 5];
    }
    
    /* Call test functions with different parameters */
    test_inner_loop(data1, 100);
    test_nested_loops((int *)data2, 10, 10);
    test_complex_control(data3, 50, argc % 4);
    test_mixed_operations(data4, 30);
    test_pointer_chasing(ptr_array, 20);
    
    /* Cleanup */
    free(data3);
    free(data4);
    
    return 0;
}
