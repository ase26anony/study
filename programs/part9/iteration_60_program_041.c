/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* Prevent optimization */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependency */
        int temp = arr[i];
        
        /* Conditional store with side effect */
        if (temp > 100) {
            arr[i] = temp * 2;
            /* Inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(temp) : "memory");
        } else {
            arr[i] = temp / 2;
        }
        
        /* Nested loop for more complex control flow */
        for (j = 0; j < 5; j++) {
            sum += arr[i] + j;
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use result to prevent dead code elimination */
    if (sum > 1000) {
        printf("Inner loop sum: %d\n", sum);
    }
}

/* Function 2: Nested loops with different iteration counts */
int test_nested_loops(int size) {
    int i, j, k;
    volatile int result = 0;
    int *matrix = (int*)malloc(size * size * sizeof(int));
    
    if (!matrix) return -1;
    
    /* Initialize with pattern */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            matrix[i * size + j] = i * j + 1;
        }
    }
    
    /* Complex nested loop with data dependencies */
    for (i = 1; i < size - 1; i++) {
        for (j = 1; j < size - 1; j++) {
            int sum = 0;
            for (k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    sum += matrix[(i + k) * size + (j + l)];
                    /* Create artificial dependency chain */
                    asm volatile ("" : "+r"(sum) : : "memory");
                }
            }
            matrix[i * size + j] = sum / 9;
            result += matrix[i * size + j];
        }
    }
    
    /* Conditional early exit */
    if (result < 0) {
        free(matrix);
        return 0;
    }
    
    /* Another loop with different stride */
    for (i = 0; i < size; i += 2) {
        for (j = 0; j < size; j += 3) {
            result ^= matrix[i * size + j];
            /* Prevent optimization */
            asm volatile ("" : : "r"(result) : "cc");
        }
    }
    
    free(matrix);
    return result;
}

/* Function 3: Switch statement with computed goto for complex control flow */
void test_switch_complex(int mode, int *data, int len) {
    volatile int counter = 0;
    int i = 0;
    
    /* Label array for computed goto */
    static void *labels[] = {
        &&case0, &&case1, &&case2, &&case3, &&default_case
    };
    
    if (mode < 0 || mode > 3) mode = 4;
    
    /* Jump to appropriate case */
    goto *labels[mode];
    
case0:
    for (i = 0; i < len; i++) {
        data[i] = data[i] + i;
        counter++;
        if (counter % 7 == 0) {
            /* Conditional break */
            break;
        }
    }
    goto end;
    
case1:
    while (i < len) {
        data[i] = data[i] * 2;
        i += 2;
        counter++;
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    goto end;
    
case2:
    do {
        data[i] = data[i] - 1;
        i++;
        counter++;
        /* Create dependency */
        asm volatile ("" : "+r"(counter) : : "memory");
    } while (i < len && counter < 100);
    goto end;
    
case3:
    for (i = 0; i < len; i++) {
        if (data[i] > 50) {
            data[i] = 0;
            /* Function call-like barrier */
            asm volatile ("nop" : : : "memory");
        } else {
            data[i] = 100;
        }
        counter += data[i];
    }
    goto end;
    
default_case:
    for (i = 0; i < len; i++) {
        data[i] = i * i;
    }
    /* Fall through */
    
end:
    /* Use counter to prevent optimization */
    if (counter > 0) {
        printf("Switch processed %d iterations\n", counter);
    }
}

/* Function 4: Mixed control flow with function pointers */
typedef int (*op_func)(int, int);

int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a * b; }
int div_safe(int a, int b) { return b != 0 ? a / b : 0; }

void test_function_pointers(int *arr, int n) {
    op_func operations[] = {add, sub, mul, div_safe};
    volatile int result = 0;
    int i;
    
    for (i = 0; i < n - 1; i++) {
        int op_idx = arr[i] % 4;
        /* Function pointer call creates complex scheduling */
        result = operations[op_idx](arr[i], arr[i + 1]);
        
        /* Store result with side effect */
        arr[i] = result;
        
        /* Prevent optimization */
        asm volatile ("" : : "r"(result) : "memory");
        
        /* Conditional continue */
        if (result < 0) {
            continue;
        }
        
        /* Additional computation */
        arr[i + 1] = result % 256;
    }
    
    /* Final reduction */
    int final = 0;
    for (i = 0; i < n; i++) {
        final ^= arr[i];
    }
    
    if (final != 0) {
        printf("Final result: %d\n", final);
    }
}

/* Main driver function */
int main() {
    const int SIZE = 100;
    int *data = (int*)malloc(SIZE * sizeof(int));
    int i;
    
    /* Initialize with random-ish data */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        data[i] = rand() % 200;
    }
    
    /* Call all test functions to ensure compilation */
    test_inner_loop(data, SIZE);
    
    int nested_result = test_nested_loops(20);
    if (nested_result < 0) {
        printf("Nested loops failed\n");
    }
    
    int *data2 = (int*)malloc(SIZE * sizeof(int));
    for (i = 0; i < SIZE; i++) {
        data2[i] = i;
    }
    
    for (i = 0; i < 5; i++) {
        test_switch_complex(i % 5, data2, SIZE);
    }
    
    test_function_pointers(data, SIZE);
    
    /* Clean up */
    free(data);
    free(data2);
    
    return 0;
}
