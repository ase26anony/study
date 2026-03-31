/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumps
 * when compiled with appropriate flags. The runtime behavior is secondary;
 * the primary goal is to generate RTL instructions that cause the scheduler
 * to call dump_insn_rtx() with debug flags enabled.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write
 * Creates data dependencies that prevent simple scheduling */
int func1(int *arr, int n) {
    volatile int sum = 0;  /* volatile prevents dead code elimination */
    int i, j;
    
    /* Outer loop with multiple iterations */
    for (i = 0; i < n; i++) {
        /* Inner loop with data dependency */
        int temp = arr[i];
        for (j = 0; j < 10; j++) {
            /* Complex expression with dependency chain */
            temp = (temp * 1103515245 + 12345) & 0x7fffffff;
            
            /* Conditional store to create control flow */
            if (temp % 3 == 0) {
                arr[i] = temp;
            }
        }
        sum += temp;
        
        /* Inline assembly to create unschedulable dependencies */
        asm volatile ("" : : "r"(temp) : "memory");
    }
    
    /* Force use of sum to prevent optimization */
    asm volatile ("" : : "r"(sum));
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Creates scheduling opportunities for outer loop pipelining */
double func2(double *matrix, int rows, int cols) {
    volatile double result = 0.0;
    int i, j, k;
    
    /* Triple nested loop - creates many scheduling opportunities */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            double acc = 0.0;
            for (k = 0; k < 8; k++) {
                /* Complex floating point operations */
                acc += matrix[i * cols + j] * 
                       (1.0 + (double)((i + j + k) % 7) / 7.0);
                
                /* Conditional to create branches */
                if (acc > 100.0) {
                    acc = acc / 2.0;
                }
            }
            result += acc;
            
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : "r"(acc) : "memory");
        }
    }
    
    return result;
}

/* Function 3: Switch statement with computed goto
 * Creates complex control flow for the scheduler */
int func3(int mode, int iterations) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    volatile int counter = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Computed goto based on mode and iteration */
        int label_idx = (mode + i) % 4;
        goto *labels[label_idx];
        
    label0:
        counter += i * 2;
        asm volatile ("" : : "r"(counter));
        continue;
        
    label1:
        counter -= i / 2;
        /* Create memory dependency */
        asm volatile ("" : : "r"(counter) : "memory");
        continue;
        
    label2:
        counter = counter * 3 + 1;
        /* Conditional jump back */
        if (counter % 5 == 0) goto label0;
        continue;
        
    label3:
        counter = ~counter;
        /* Another memory barrier */
        asm volatile ("" : : "r"(counter) : "memory");
        continue;
    }
    
    return counter;
}

/* Function 4: Mixed operations with function calls
 * Creates scheduling boundaries */
static int helper1(int x) {
    return (x * 13 + 7) % 256;
}

static int helper2(int x, int y) {
    volatile int z = x ^ y;
    asm volatile ("" : : "r"(z));
    return z;
}

void func4(int *data, int size) {
    int i;
    
    for (i = 0; i < size; i++) {
        /* Mix of operations and function calls */
        int val = data[i];
        val = helper1(val);
        
        /* Conditional with side effect */
        if (val > 128) {
            data[i] = helper2(val, i);
        } else {
            data[i] = val + i;
        }
        
        /* Loop-carried dependency */
        if (i > 0) {
            data[i] += data[i-1] % 16;
        }
    }
}

/* Function 5: Array reduction with early exit
 * Creates unpredictable control flow */
int func5(int *arr, int n, int threshold) {
    volatile int total = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        total += arr[i];
        
        /* Early exit condition - creates branch prediction challenges */
        if (total > threshold) {
            /* Complex exit path */
            for (int j = 0; j < 3; j++) {
                total = (total << 3) | (total >> 29);  /* rotate */
            }
            break;
        }
        
        /* Another dependency chain */
        arr[i] = (arr[i] * 16807) % 2147483647;
    }
    
    asm volatile ("" : : "r"(total) : "memory");
    return total;
}

/* Main driver - ensures all functions are compiled and called */
int main(int argc, char **argv) {
    int i;
    
    /* Initialize test data */
    int n = 100;
    int *arr1 = (int*)malloc(n * sizeof(int));
    double *arr2 = (double*)malloc(50 * 50 * sizeof(double));
    int *arr3 = (int*)malloc(n * sizeof(int));
    
    /* Seed for reproducibility */
    srand(42);
    
    /* Initialize arrays */
    for (i = 0; i < n; i++) {
        arr1[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
    }
    
    for (i = 0; i < 50 * 50; i++) {
        arr2[i] = (double)rand() / RAND_MAX;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result1 = func1(arr1, n);
    double result2 = func2(arr2, 50, 50);
    int result3 = func3(argc > 1 ? atoi(argv[1]) : 1, 50);
    func4(arr3, n);
    int result5 = func5(arr1, n, 5000);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %f, %d, %d\n", 
           result1, result2, result3, result5);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
