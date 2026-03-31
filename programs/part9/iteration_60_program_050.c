/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* Prevent dead code elimination */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependencies */
        int temp = arr[i];
        
        /* Conditional store with dependency chain */
        if (temp > 100) {
            arr[i] = temp * 2 + i;
            /* Inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(temp) : "memory");
        } else {
            arr[i] = temp / 2 - i;
        }
        
        /* Nested loop for more scheduling complexity */
        for (j = 0; j < 5; j++) {
            sum += arr[i] + j;
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use result to prevent optimization */
    if (sum > 1000) {
        printf("Inner loop sum: %d\n", sum);
    }
}

/* Function 2: Nested loops with different iteration counts */
void test_nested_loops(int *matrix, int rows, int cols) {
    int i, j, k;
    volatile int acc = 0;
    
    /* Outer loop with dependency */
    for (i = 0; i < rows; i++) {
        int row_start = i * cols;
        
        /* Middle loop with conditional */
        for (j = 0; j < cols; j++) {
            int idx = row_start + j;
            int val = matrix[idx];
            
            /* Complex conditional with multiple branches */
            if (val & 1) {
                matrix[idx] = val * 3 + 1;
            } else {
                matrix[idx] = val / 2;
            }
            
            /* Innermost loop for pipelining opportunities */
            for (k = 0; k < 3; k++) {
                acc += matrix[idx] * k;
                /* Force dependency chain */
                asm volatile ("" : "+r"(acc) : : "memory");
            }
        }
        
        /* Function call within loop to create basic block boundaries */
        if (i % 2 == 0) {
            acc += rand() % 10;
        }
    }
    
    /* Use result */
    if (acc > 0) {
        printf("Nested loops acc: %d\n", acc);
    }
}

/* Function 3: Switch statement with computed goto for complex CFG */
void test_switch_complex(int mode, int *data, int size) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    int i;
    volatile int result = 0;
    
    if (mode < 0 || mode > 4) mode = 4;
    
    /* Jump to label based on mode */
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
        result -= data[i];
        /* Memory dependency */
        asm volatile ("" : : "r"(result) : "memory");
    }
    goto end;
    
case2:
    i = 0;
    while (i < size) {
        data[i] = data[i] ^ 0xFF;
        result |= data[i];
        i += 2;
    }
    goto end;
    
case3:
    do {
        data[0] = (data[0] << 1) | (data[0] >> 31);
        result = result ^ data[0];
        /* Prevent optimization */
        asm volatile ("" : "+r"(result) : : "memory");
    } while (result < 1000 && size-- > 0);
    goto end;
    
default_case:
    for (i = 0; i < size; i++) {
        if (i % 3 == 0) {
            data[i] = 0;
        } else {
            data[i] = i;
        }
        result += data[i];
    }
    
end:
    /* Use result to prevent dead code elimination */
    if (result != 0) {
        printf("Switch result: %d\n", result);
    }
}

/* Function 4: Mixed operations with pointer aliasing */
void test_mixed_ops(int *a, int *b, int *c, int n) {
    int i;
    volatile int check = 0;
    
    /* Loop with potential pointer aliasing */
    for (i = 0; i < n; i++) {
        /* Multiple memory accesses with dependencies */
        int x = a[i];
        int y = b[i];
        int z = c[i];
        
        /* Complex expression with multiple operations */
        int res = (x * y) + (z << 2) - (x / (y + 1));
        
        /* Conditional store with side effect */
        if (res > 0) {
            a[i] = res;
            b[i] = res ^ 0xABCD;
        } else {
            c[i] = -res;
            a[i] = y * z;
        }
        
        /* Update check with dependency */
        check = check + res + i;
        asm volatile ("" : "+r"(check) : : "memory");
    }
    
    /* Final computation */
    if (check > 10000) {
        printf("Mixed ops check: %d\n", check);
    }
}

/* Main driver to ensure all functions are compiled */
int main(int argc, char **argv) {
    int size = 100;
    int *arr1 = malloc(size * sizeof(int));
    int *arr2 = malloc(size * sizeof(int));
    int *arr3 = malloc(size * sizeof(int));
    int i;
    
    /* Initialize with random data */
    srand(time(NULL));
    for (i = 0; i < size; i++) {
        arr1[i] = rand() % 200;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 300;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(arr1, size);
    test_nested_loops(arr2, 10, 10);
    test_switch_complex(rand() % 5, arr3, size);
    test_mixed_ops(arr1, arr2, arr3, size);
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
