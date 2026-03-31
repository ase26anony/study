/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumps
 * when compiled with appropriate flags. The goal is to reach the uncovered
 * lines in sel-sched-dump.cc that handle RTL instruction dumping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Function 1: Inner loop with conditional branch and memory write
 * Creates data dependencies that prevent simple scheduling */
int test_inner_loop(int n, int *arr) {
    volatile int sum = 0;  /* volatile prevents dead code elimination */
    int i, j;
    
    /* Complex loop with data dependencies */
    for (i = 0; i < n; i++) {
        int temp = arr[i];
        
        /* Inner conditional with memory access */
        if (temp > 0) {
            for (j = 0; j < 8; j++) {
                /* Create unschedulable dependency using inline asm */
                asm volatile ("" : "+r" (temp) : : "memory");
                temp = temp * 2 + j;
            }
            arr[i] = temp;
            sum += temp;
        } else {
            /* Different path with arithmetic */
            temp = -temp * 3 + i;
            asm volatile ("" : : "r" (temp) : "memory");
            sum -= temp;
        }
    }
    
    /* Force dependency chain */
    asm volatile ("" : : "r" (sum) : "memory");
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Creates opportunities for outer loop pipelining */
double test_nested_loops(double *matrix, int rows, int cols) {
    volatile double total = 0.0;
    int i, j, k;
    
    /* Triple nested loop for complex scheduling */
    for (i = 0; i < rows; i++) {
        double row_sum = 0.0;
        
        for (j = 0; j < cols; j++) {
            double cell = matrix[i * cols + j];
            
            /* Inner computation loop */
            for (k = 0; k < 4; k++) {
                /* Complex floating point operations */
                cell = cell * 1.5 - (k * 0.25);
                asm volatile ("" : "+r" (cell) : : "memory");
            }
            
            row_sum += cell;
            matrix[i * cols + j] = cell;
        }
        
        total += row_sum;
        
        /* Conditional store with dependency */
        if (row_sum > 100.0) {
            asm volatile ("" : : "r" (row_sum) : "memory");
            total *= 0.95;
        }
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto
 * Creates complex control flow for selective scheduling */
int test_complex_control_flow(int mode, int iterations) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3 };
    volatile int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Computed goto creates complex control flow */
        goto *labels[mode % 4];
        
    case0:
        /* Arithmetic operations */
        result += i * 2;
        asm volatile ("" : "+r" (result) : : "memory");
        mode = (mode + 1) % 4;
        continue;
        
    case1:
        /* Memory operations */
        result -= i / 2;
        {
            volatile int temp = result;
            asm volatile ("" : : "r" (temp) : "memory");
        }
        mode = (mode * 3 + 1) % 4;
        continue;
        
    case2:
        /* Conditional with both paths */
        if (i % 3 == 0) {
            result *= 2;
        } else {
            result /= 2;
        }
        asm volatile ("" : "+r" (result) : : "memory");
        mode = (mode + 2) % 4;
        continue;
        
    case3:
        /* Complex dependency chain */
        result = (result << 3) | (i & 0x7);
        {
            int temp = result ^ 0x55AA55AA;
            asm volatile ("" : : "r" (temp) : "memory");
        }
        mode = (mode * 5 + 1) % 4;
        continue;
    }
    
    return result;
}

/* Function 4: Mixed operations with function calls
 * Creates scheduling barriers */
static int helper1(int x) {
    return x * x - x + 1;
}

static float helper2(float x, float y) {
    volatile float r = x * y - x / y;
    asm volatile ("" : : "r" (r) : "memory");
    return r;
}

int test_mixed_operations(int limit) {
    volatile int acc = 0;
    float f_acc = 0.0f;
    int i;
    
    for (i = 0; i < limit; i++) {
        /* Mix integer and float operations */
        int int_val = helper1(i);
        float float_val = helper2(i * 0.5f, i + 1.0f);
        
        /* Conditional with both types */
        if (i % 2 == 0) {
            acc += int_val;
            f_acc += float_val;
        } else {
            acc -= int_val / 2;
            f_acc -= float_val * 0.5f;
        }
        
        /* Memory barrier */
        asm volatile ("" : : "r" (acc), "r" (f_acc) : "memory");
        
        /* Additional loop with dependency */
        for (int j = 0; j < 2; j++) {
            acc = (acc << 1) | (j & 1);
            asm volatile ("" : "+r" (acc) : : "memory");
        }
    }
    
    return acc + (int)f_acc;
}

/* Function 5: Array processing with pointer aliasing
 * Creates memory dependencies that challenge the scheduler */
void test_pointer_aliasing(int *a, int *b, int *c, int n) {
    volatile int checksum = 0;
    int i;
    
    /* Process arrays with potential aliasing */
    for (i = 0; i < n - 1; i++) {
        /* Read-modify-write with pointer arithmetic */
        int x = a[i];
        int y = b[i + 1];
        
        /* Complex dependency chain */
        x = x ^ y;
        x = x * 2 + i;
        
        /* Conditional store */
        if (x > y) {
            c[i] = x;
            checksum += x;
        } else {
            c[i] = y - x;
            checksum -= y;
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile ("" : : "r" (x), "r" (y), "r" (checksum) : "memory");
        
        /* Additional operation that depends on previous store */
        a[i + 1] = c[i] / 2;
    }
    
    /* Final operation */
    asm volatile ("" : : "r" (checksum) : "memory");
}

/* Main driver function that ensures all code paths are compiled */
int main(int argc, char **argv) {
    /* Initialize test data */
    int arr1[100];
    double matrix[10][10];
    int arr2[50], arr3[50], arr4[50];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr1[i] = i - 50;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 1.5 + j * 0.7;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        arr2[i] = i * 3;
        arr3[i] = i * 5;
        arr4[i] = 0;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result1 = test_inner_loop(100, arr1);
    double result2 = test_nested_loops(&matrix[0][0], 10, 10);
    int result3 = test_complex_control_flow(argc, 20);
    int result4 = test_mixed_operations(30);
    test_pointer_aliasing(arr2, arr3, arr4, 50);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + (int)result2 + result3 + result4;
    
    /* Print something to ensure execution */
    printf("Test completed. Final indicator: %d\n", final_result % 1000);
    
    return 0;
}
