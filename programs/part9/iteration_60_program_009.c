/* test_sel_sched_dump.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write */
/* This creates a scheduling region with data dependencies */
int test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* volatile prevents dead code elimination */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependency chain */
        int temp = arr[i];
        
        /* Conditional store with side effect */
        if (temp > 0) {
            sum += temp * 2;
            /* Inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(sum) : "memory");
        } else {
            sum -= temp;
            asm volatile ("" : : "r"(sum) : "memory");
        }
        
        /* Nested loop with variable bounds */
        for (j = 0; j < (temp & 0x3); j++) {
            sum += j;
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : : "memory");
        }
    }
    
    return sum;
}

/* Function 2: Nested loops with different iteration counts */
/* Targets outer loop pipelining */
int test_nested_loops(int size) {
    int i, j, k;
    volatile int result = 0;
    int *matrix = (int*)malloc(size * size * sizeof(int));
    
    if (!matrix) return -1;
    
    /* Initialize with pattern */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            matrix[i * size + j] = (i * j) % 256;
        }
    }
    
    /* Complex nested loop structure */
    for (i = 1; i < size - 1; i++) {
        for (j = 1; j < size - 1; j++) {
            int sum = 0;
            /* Inner kernel with dependencies */
            for (k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    sum += matrix[(i + k) * size + (j + l)];
                    /* Prevent optimization */
                    asm volatile ("" : "+r"(sum) : : "memory");
                }
            }
            result += sum / 9;
            /* Conditional update with side effect */
            if (result > 1000) {
                result = result % 1000;
                asm volatile ("" : : "r"(result) : "memory");
            }
        }
    }
    
    free(matrix);
    return result;
}

/* Function 3: Switch statement with computed goto */
/* Creates complex control flow for selective scheduling */
int test_switch_complex(int mode, int iterations) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    volatile int counter = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int op = (i + mode) % 4;
        
        /* Computed goto for complex control flow */
        goto *labels[op];
        
    label0:
        counter += i * 2;
        asm volatile ("" : : "r"(counter) : "memory");
        continue;
        
    label1:
        counter -= i / 2;
        /* Memory operation with dependency */
        {
            int temp = counter;
            asm volatile ("" : "+r"(temp) : : "memory");
            counter = temp;
        }
        continue;
        
    label2:
        counter = counter ^ i;
        /* Function call simulation */
        {
            volatile int dummy = counter;
            asm volatile ("# dummy function call" : : "r"(dummy) : "memory");
        }
        continue;
        
    label3:
        counter = (counter << 3) | (counter >> 29);
        asm volatile ("" : : "r"(counter) : "memory");
        continue;
    }
    
    return counter;
}

/* Function 4: Mixed control flow with array processing */
int test_mixed_control_flow(int *data, int len) {
    volatile int acc = 0;
    int i = 0;
    
    while (i < len) {
        /* Multiple basic blocks */
        if (data[i] < 0) {
            acc -= data[i];
            /* Loop with early exit */
            do {
                acc += i;
                i++;
                if (i >= len) break;
            } while (data[i] < 0);
        } else if (data[i] > 100) {
            acc += data[i] / 2;
            i += 2;  /* Skip pattern */
            asm volatile ("" : : "r"(acc) : "memory");
        } else {
            /* Small processing loop */
            for (int j = 0; j < 3 && (i + j) < len; j++) {
                acc += data[i + j] * j;
                asm volatile ("" : : : "memory");
            }
            i += 3;
        }
        
        /* Periodic check */
        if ((i & 0x7) == 0) {
            acc = acc & 0xFFF;
        }
    }
    
    return acc;
}

/* Function 5: Recursive pattern with tail processing */
int test_recursive_pattern(int n, int depth) {
    volatile int result = 0;
    
    if (depth > 5 || n <= 1) {
        return n;
    }
    
    /* Process in chunks */
    for (int i = 0; i < n; i += 4) {
        int chunk = (i + 3 < n) ? 4 : (n - i);
        
        switch (chunk) {
            case 4:
                result += (i * 4);
                asm volatile ("" : : "r"(result) : "memory");
                /* fall through */
            case 3:
                result += (i * 3);
                asm volatile ("" : : "r"(result) : "memory");
                /* fall through */
            case 2:
                result += (i * 2);
                asm volatile ("" : : "r"(result) : "memory");
                /* fall through */
            case 1:
                result += i;
                asm volatile ("" : : "r"(result) : "memory");
                break;
        }
        
        /* Recursive call with reduced depth */
        if ((i % 8) == 0) {
            int sub = test_recursive_pattern(chunk, depth + 1);
            result += sub;
            asm volatile ("" : : "r"(result) : "memory");
        }
    }
    
    return result;
}

/* Main driver to ensure all code paths are compiled */
int main(int argc, char **argv) {
    int test_data[100];
    int i, result = 0;
    
    /* Initialize test data with pattern */
    srand(time(NULL));
    for (i = 0; i < 100; i++) {
        test_data[i] = rand() % 200 - 50;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += test_inner_loop(test_data, 100);
    result += test_nested_loops(32);
    result += test_switch_complex(argc > 1 ? atoi(argv[1]) : 1, 50);
    result += test_mixed_control_flow(test_data, 100);
    result += test_recursive_pattern(64, 0);
    
    /* Use result to prevent dead code elimination */
    volatile int final = result;
    asm volatile ("" : : "r"(final) : "memory");
    
    printf("Result: %d\n", result);
    return 0;
}
