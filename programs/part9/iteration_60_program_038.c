/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write */
/* This creates data dependencies that prevent simple scheduling */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* volatile prevents dead code elimination */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependency chain */
        int temp = arr[i];
        
        /* Conditional store with dependency */
        if (temp > 100) {
            arr[i] = temp * 2 + 1;
            /* Inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(temp) : "memory");
        } else {
            arr[i] = temp / 2 - 1;
        }
        
        /* Additional computation with dependency */
        for (j = 0; j < 3; j++) {
            sum += arr[i] + j;
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use the result to prevent optimization */
    if (sum > 1000) {
        printf("Inner loop sum: %d\n", sum);
    }
}

/* Function 2: Nested loops with different iteration counts */
/* Outer loop pipelining opportunity */
void test_nested_loops(int *matrix, int rows, int cols) {
    volatile int total = 0;
    int i, j, k;
    
    for (i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with stride access */
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Complex conditional with multiple branches */
            if (val & 1) {
                val = (val << 2) | 0x3;
            } else if (val & 2) {
                val = (val >> 1) & 0x7F;
            } else {
                val = val ^ 0xFF;
            }
            
            matrix[idx] = val;
            row_sum += val;
            
            /* Prevent optimization */
            asm volatile ("" : "+r"(row_sum) : : "memory");
        }
        
        /* Middle loop with dependency on inner loop */
        for (k = 0; k < 2; k++) {
            row_sum = (row_sum * 31) % 997;
        }
        
        total += row_sum;
    }
    
    /* Use result */
    if (total != 0) {
        printf("Nested loops total: %d\n", total);
    }
}

/* Function 3: Switch statement with computed goto for complex control flow */
void test_switch_complex(int mode, int *data, int size) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    volatile int result = 0;
    int i;
    
    if (mode < 0 || mode > 4) mode = 4;
    
    /* Computed goto creates complex control flow */
    goto *labels[mode];
    
case0:
    for (i = 0; i < size; i++) {
        data[i] = data[i] + i;
        result += data[i];
    }
    goto end;
    
case1:
    for (i = size - 1; i >= 0; i--) {
        data[i] = data[i] * 2 - i;
        result ^= data[i];
    }
    goto end;
    
case2:
    i = 0;
    while (i < size) {
        data[i] = (data[i] << 1) | (data[i] >> 31);
        result |= data[i];
        i += 2;
    }
    goto end;
    
case3:
    do {
        data[0] = (data[0] * 3) % 256;
        result = result * 7 + data[0];
        asm volatile ("" : "+r"(result) : : "memory");
    } while (result < 10000);
    goto end;
    
default_case:
    for (i = 0; i < size; i++) {
        if (i % 2 == 0) {
            data[i] = ~data[i];
        } else {
            data[i] = data[i] + data[i-1];
        }
        result = result - data[i];
    }
    /* Fall through */
    
end:
    /* Ensure result is used */
    if (result > 0) {
        printf("Switch result: %d\n", result);
    }
}

/* Function 4: Mixed operations with function calls */
/* Creates scheduling barriers */
static int helper1(int x) {
    return (x * 3 + 7) & 0xFF;
}

static int helper2(int x, int y) {
    volatile int z = x + y;
    asm volatile ("" : "+r"(z) : : "memory");
    return z * 2;
}

void test_mixed_ops(int *buf, int len) {
    int i;
    volatile int acc = 0;
    
    for (i = 0; i < len; i++) {
        /* Mix of operations */
        int val = buf[i];
        
        /* Function call creates scheduling barrier */
        val = helper1(val);
        
        /* Conditional with side effect */
        if (val > 128) {
            val = helper2(val, i);
            buf[i] = val / 3;
        } else {
            val = val ^ 0x55;
            buf[i] = val * 2;
        }
        
        /* Loop-carried dependency */
        acc = acc + val;
        
        /* Memory operation with barrier */
        asm volatile ("" : : "r"(acc), "m"(*buf) : "memory");
        
        /* Small inner loop */
        int j;
        for (j = 0; j < 2; j++) {
            acc = (acc << 1) | (acc >> 31);
        }
    }
    
    if (acc != 0) {
        printf("Mixed ops acc: %d\n", acc);
    }
}

/* Function 5: Pointer chasing with indirect memory access */
void test_pointer_chase(int **ptr_array, int count) {
    volatile int hash = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        int *ptr = ptr_array[i];
        if (ptr != NULL) {
            /* Dereference with potential aliasing */
            int val = *ptr;
            
            /* Complex transformation chain */
            val = ((val & 0x0F0F0F0F) << 4) | ((val & 0xF0F0F0F0) >> 4);
            val = val ^ (val >> 16);
            val = val * 0x9E3779B9;
            
            *ptr = val;
            hash = hash ^ val;
            
            /* Memory barrier for pointer access */
            asm volatile ("" : : "r"(ptr), "m"(*ptr) : "memory");
        }
        
        /* Additional computation */
        if (i % 3 == 0) {
            hash = (hash * 31 + 17) & 0x7FFFFFFF;
        }
    }
    
    if (hash > 0) {
        printf("Pointer chase hash: %d\n", hash);
    }
}

/* Main driver that ensures all code paths are compiled */
int main(void) {
    const int SIZE = 100;
    const int MATRIX_ROWS = 10;
    const int MATRIX_COLS = 10;
    const int PTR_COUNT = 50;
    
    int i;
    
    /* Allocate and initialize test data */
    int *array1 = (int *)malloc(SIZE * sizeof(int));
    int *array2 = (int *)malloc(SIZE * sizeof(int));
    int *matrix = (int *)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    int **ptr_array = (int **)malloc(PTR_COUNT * sizeof(int *));
    int *data_buffer = (int *)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        array1[i] = rand() % 256;
        array2[i] = rand() % 256;
        data_buffer[i] = rand() % 256;
    }
    
    for (i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = rand() % 512;
    }
    
    for (i = 0; i < PTR_COUNT; i++) {
        ptr_array[i] = (i % 3 == 0) ? NULL : &array1[i % SIZE];
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(array1, SIZE);
    test_nested_loops(matrix, MATRIX_ROWS, MATRIX_COLS);
    test_switch_complex(rand() % 5, array2, SIZE);
    test_mixed_ops(data_buffer, SIZE);
    test_pointer_chase(ptr_array, PTR_COUNT);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(matrix);
    free(ptr_array);
    free(data_buffer);
    
    return 0;
}
