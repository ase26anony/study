/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function with inner loop and conditional branch - triggers basic selective scheduling */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* volatile prevents optimization */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependencies */
        int temp = arr[i];
        
        /* Conditional store with side effect */
        if (temp > 100) {
            arr[i] = temp - 50;
            /* Inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(temp) : "memory");
        } else {
            arr[i] = temp + 20;
        }
        
        /* Nested loop with varying iteration count */
        for (j = 0; j < (i % 8) + 1; j++) {
            sum += arr[i] * j;
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use the result to prevent dead code elimination */
    if (sum > 1000) {
        printf("Inner loop sum: %d\n", sum);
    }
}

/* Function with nested loops - triggers outer loop pipelining */
void test_outer_loop_pipelining(int *matrix, int rows, int cols) {
    volatile int total = 0;
    int i, j, k;
    
    /* Triple nested loop for complex scheduling */
    for (i = 0; i < rows; i++) {
        int row_sum = 0;
        
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Complex conditional with function call */
            if (val % 3 == 0) {
                val = val * 2 + 1;
                /* Function call creates scheduling barrier */
                row_sum += abs(val);
            } else if (val % 5 == 0) {
                val = val / 2 - 1;
                row_sum += val * val;
            } else {
                /* Multiple operations with dependencies */
                int t1 = val + 7;
                int t2 = t1 * 3;
                int t3 = t2 - val;
                row_sum += t3;
                
                /* Inline asm with register constraints */
                asm volatile ("# dependency" : "+r"(t3) : "r"(val));
            }
            
            matrix[idx] = val;
        }
        
        /* Outer loop computation with dependency on inner loop */
        for (k = 0; k < 4; k++) {
            total += row_sum >> k;
            /* Prevent optimization across iterations */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use result */
    if (total != 0) {
        printf("Matrix total: %d\n", total);
    }
}

/* Function with switch and computed goto - creates complex control flow */
void test_complex_control_flow(int mode, int *data, int size) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3, &&case4 };
    volatile int result = 0;
    int i;
    
    if (mode < 0 || mode > 4) return;
    
    /* Computed goto for complex control flow */
    goto *labels[mode];
    
case0:
    /* Simple accumulation */
    for (i = 0; i < size; i++) {
        result += data[i];
        data[i] = result;
    }
    goto end;
    
case1:
    /* Conditional update with early exit */
    for (i = 0; i < size; i++) {
        if (data[i] < 0) break;
        result += data[i] * 2;
        /* Memory operation with barrier */
        asm volatile ("" : : "r"(result) : "memory");
    }
    goto end;
    
case2:
    /* Loop with multiple exits */
    i = 0;
    while (1) {
        if (i >= size) break;
        if (data[i] == 0) {
            result = -1;
            break;
        }
        result += data[i] / 2;
        i++;
    }
    goto end;
    
case3:
    /* Nested switches inside loop */
    for (i = 0; i < size; i++) {
        switch (data[i] % 4) {
            case 0: result += 1; break;
            case 1: result += 3; break;
            case 2: result += 5; break;
            case 3: result += 7; break;
        }
        /* Prevent loop invariant code motion */
        asm volatile ("" : : "r"(i) : "memory");
    }
    goto end;
    
case4:
    /* Mixed operations */
    for (i = 0; i < size; i += 2) {
        int a = data[i];
        int b = (i + 1 < size) ? data[i + 1] : 0;
        result += a * b - a + b;
    }
    /* fall through */
    
end:
    /* Use result */
    if (result > 100) {
        printf("Control flow result: %d\n", result);
    }
}

/* Function with pointer chasing and indirect calls - creates alias analysis challenges */
void test_pointer_chasing(int **ptr_array, int count) {
    volatile int hash = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        int *ptr = ptr_array[i];
        if (ptr) {
            /* Dereference with offset */
            int val = *ptr + i;
            
            /* Conditional update based on value */
            if (val & 1) {
                *ptr = val ^ 0x55AA;
                hash += *ptr;
            } else {
                *ptr = val & 0xFF;
                hash -= *ptr;
            }
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use hash */
    if (hash != 0) {
        printf("Pointer hash: %d\n", hash);
    }
}

/* Main driver - ensures all code paths are compiled */
int main(int argc, char **argv) {
    int i;
    
    /* Test data */
    int array1[100];
    int array2[10][10];
    int *ptr_array[20];
    int data[50];
    
    /* Initialize with random-ish data */
    srand(time(NULL));
    
    for (i = 0; i < 100; i++) {
        array1[i] = rand() % 200;
    }
    
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            array2[i][j] = rand() % 100;
        }
    }
    
    for (i = 0; i < 20; i++) {
        ptr_array[i] = &array1[i * 5];
    }
    
    for (i = 0; i < 50; i++) {
        data[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(array1, 100);
    test_outer_loop_pipelining(&array2[0][0], 10, 10);
    test_complex_control_flow(rand() % 5, data, 50);
    test_pointer_chasing(ptr_array, 20);
    
    /* Additional calls with different parameters */
    test_inner_loop(data, 50);
    test_complex_control_flow(2, array1, 30);
    
    return 0;
}
