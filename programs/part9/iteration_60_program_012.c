/* test_sel_sched_dump.c
 * Program to trigger selective scheduler debug dumps in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Function 1: Inner loop with conditional branch and memory write */
void func1(int *arr, int n) {
    volatile int sum = 0;  /* Prevent dead code elimination */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependencies */
        int temp = arr[i];
        
        /* Conditional store with dependency chain */
        if (temp > 0) {
            for (j = 0; j < 10; j++) {
                sum += temp * j;
                /* Memory access pattern that creates scheduling constraints */
                arr[i] = sum;
                
                /* Inline asm to create specific RTL patterns */
                asm volatile ("" : : "r"(sum) : "memory");
            }
        } else {
            /* Alternative path with different operations */
            sum -= temp;
            asm volatile ("" : : "r"(temp) : "memory");
        }
        
        /* Another dependency */
        arr[i] = sum % 256;
    }
    
    /* Force use of result */
    asm volatile ("" : : "r"(sum));
}

/* Function 2: Nested loops with different iteration counts */
int func2(int *matrix, int rows, int cols) {
    volatile int total = 0;
    int i, j, k;
    
    /* Outer loop with pipelining opportunities */
    for (i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Middle loop */
        for (j = 0; j < cols; j++) {
            int val = matrix[i * cols + j];
            
            /* Inner loop with computation */
            for (k = 0; k < 3; k++) {
                row_sum += val * k;
                
                /* Conditional with side effects */
                if (row_sum > 1000) {
                    row_sum /= 2;
                    asm volatile ("" : : "r"(row_sum));
                }
            }
            
            /* Memory barrier effect */
            asm volatile ("" : : "r"(val) : "memory");
        }
        
        total += row_sum;
        
        /* Complex condition with multiple branches */
        if (total < 0) {
            total = -total;
        } else if (total > 10000) {
            total %= 10000;
        }
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto for complex control flow */
void func3(int mode, int *data, int size) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3 };
    volatile int result = 0;
    int i;
    
    if (mode < 0 || mode > 3) mode = 0;
    
    /* Computed goto creates interesting control flow */
    goto *labels[mode];
    
case0:
    for (i = 0; i < size; i++) {
        data[i] = i * 2;
        result += data[i];
        asm volatile ("" : : "r"(result));
    }
    goto end;
    
case1:
    for (i = size - 1; i >= 0; i--) {
        data[i] = data[i] * 3 + 1;
        result -= data[i];
        if (result < 0) result = 0;
        asm volatile ("" : : "r"(result) : "memory");
    }
    goto end;
    
case2:
    i = 0;
    while (i < size) {
        data[i] = (data[i] << 1) | 1;
        result ^= data[i];
        i += 2;
        asm volatile ("" : : "r"(result));
    }
    goto end;
    
case3:
    do {
        data[0] = result;
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        asm volatile ("" : : "r"(result));
    } while (result % 100 != 0 && size-- > 0);
    goto end;
    
end:
    /* Final barrier */
    asm volatile ("" : : "r"(result) : "memory");
}

/* Function 4: Mixed operations with pointer aliasing */
int func4(int *a, int *b, int *c, int n) {
    volatile int acc = 0;
    int i;
    
    /* Create potential pointer aliasing */
    int *ptr1 = a;
    int *ptr2 = b;
    
    for (i = 0; i < n; i++) {
        /* Multiple memory accesses with dependencies */
        int x = *ptr1++;
        int y = *ptr2++;
        
        /* Complex arithmetic chain */
        int t1 = x * y;
        int t2 = t1 + (x << 3);
        int t3 = t2 - (y >> 2);
        
        /* Conditional store with memory barrier */
        if (t3 > 0) {
            c[i] = t3;
            asm volatile ("" : : "r"(t3) : "memory");
        } else {
            c[i] = -t3;
            asm volatile ("" : : "r"(t3));
        }
        
        acc += c[i];
        
        /* Loop-carried dependency */
        if (i % 4 == 0) {
            asm volatile ("" : : "r"(acc) : "memory");
        }
    }
    
    return acc;
}

/* Function 5: Recursive-like pattern with tail operations */
void func5(int *buf, int len, int depth) {
    volatile int counter = 0;
    int i;
    
    if (depth <= 0 || len <= 1) return;
    
    /* Process first half */
    for (i = 0; i < len/2; i++) {
        buf[i] = buf[i] * depth + i;
        counter += buf[i];
        asm volatile ("" : : "r"(counter));
    }
    
    /* Process second half with different stride */
    for (i = len/2; i < len; i += 2) {
        buf[i] = buf[i] / (depth + 1) - i;
        counter -= buf[i];
        asm volatile ("" : : "r"(counter) : "memory");
    }
    
    /* Recursive call simulation */
    func5(buf, len/2, depth - 1);
    func5(buf + len/2, len - len/2, depth - 1);
}

/* Main driver to ensure all functions are compiled */
int main(int argc, char **argv) {
    /* Initialize test data */
    int size = 100;
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    int *arr3 = (int*)malloc(size * sizeof(int));
    int *arr4 = (int*)malloc(size * sizeof(int));
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < size; i++) {
        arr1[i] = i - size/2;
        arr2[i] = (i * 3) % 17;
        arr3[i] = i;
        arr4[i] = 1;
    }
    
    /* Call all test functions to ensure they're compiled */
    func1(arr1, size);
    int r2 = func2(arr2, 10, 10);
    func3(1, arr3, size);
    int r4 = func4(arr1, arr2, arr3, size);
    func5(arr4, size, 3);
    
    /* Use results to prevent dead code elimination */
    volatile int dummy = r2 + r4;
    asm volatile ("" : : "r"(dummy));
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
